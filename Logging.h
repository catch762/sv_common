#pragma once
#include "Common.h"
#include "FileUtils.h"
#include <filesystem>
#include <cassert>


//******************************************
// <  COMPILE FLAGS YOU CAN SET EXTERNALLY > 
//******************************************

#ifndef SV_LOGGER_ENABLED
#define SV_LOGGER_ENABLED 1
#endif

// three logging categories you can enable/disable separately:
// (you cant disable assert category btw)
#ifndef SV_LOGGER_INFO_ENABLED
#define SV_LOGGER_INFO_ENABLED 1
#endif
#ifndef SV_LOGGER_WARN_ENABLED
#define SV_LOGGER_WARN_ENABLED 1
#endif
#ifndef SV_LOGGER_ERROR_ENABLED
#define SV_LOGGER_ERROR_ENABLED 1
#endif

#ifndef SV_LOGGER_LOG_TO_FILE_ENABLED
#define SV_LOGGER_LOG_TO_FILE_ENABLED 1
#endif

//******************************************
// <  COMPILE FLAGS YOU CAN SET EXTERNALLY > 
//******************************************


//************************************************
// <  UGLY ASS INTERNALS - DO NOT BOTHER READING > 
//************************************************

constexpr std::string_view getJustFileName(std::string_view fullPath)
{
    size_t last_slash = fullPath.find_last_of("\\/");
    if (last_slash == std::string_view::npos) {
        return fullPath;
    }
    return fullPath.substr(last_slash + 1);
}

#if defined(__FILE_NAME__)
    //Clang, GCC
    #define SV_FILE_NAME __FILE_NAME__
#else
    //MSVC
    #define SV_FILE_NAME getJustFileName(__FILE__)
#endif

// <internal: these get used by other logging macros>
// the single call, it all boils down to this line:
#define SV_DO_LOG_FULL(LEVEL, MSG, CATEGORY)  if constexpr(SV_LOGGER_ENABLED){Logger::instance().log(MSG, LEVEL, CATEGORY, SV_FILE_NAME, __LINE__);}
#define SV_DO_LOG_BASE(LEVEL, MSG)            SV_DO_LOG_FULL(LEVEL, MSG, nullptr)
#define SV_GET_LOG_MACRO_WITH_1_OR_2_ARGS(_1, _2, NAME, ...) NAME

#define SV_INFO1(MSG)               if constexpr(SV_LOGGER_INFO_ENABLED)  {SV_DO_LOG_BASE( Logger::Level::Info,    MSG            )}
#define SV_INFO2(CATEGORY, MSG)     if constexpr(SV_LOGGER_INFO_ENABLED)  {SV_DO_LOG_FULL( Logger::Level::Info,    MSG, CATEGORY  )}
#define SV_WARN1(MSG)               if constexpr(SV_LOGGER_WARN_ENABLED)  {SV_DO_LOG_BASE( Logger::Level::Warn,    MSG            )}
#define SV_WARN2(CATEGORY, MSG)     if constexpr(SV_LOGGER_WARN_ENABLED)  {SV_DO_LOG_FULL( Logger::Level::Warn,    MSG, CATEGORY  )}
#define SV_ERROR1(MSG)              if constexpr(SV_LOGGER_ERROR_ENABLED) {SV_DO_LOG_BASE( Logger::Level::Error,   MSG            )}
#define SV_ERROR2(CATEGORY, MSG)    if constexpr(SV_LOGGER_ERROR_ENABLED) {SV_DO_LOG_FULL( Logger::Level::Error,   MSG, CATEGORY  )}
// </internal>

// For SV_UNREACHABLE:                           
#ifdef __cpp_lib_unreachable  // C++23, from <utility>
#define SV_UNREACHABLE_CALL() std::unreachable()
#elif defined(__GNUC__) || defined(__clang__)
#define SV_UNREACHABLE_CALL() __builtin_unreachable()
#elif defined(_MSC_VER)
#define SV_UNREACHABLE_CALL() __assume(false)
#else
#define SV_UNREACHABLE_CALL()
#endif

//************************************************
// < /UGLY ASS INTERNALS - DO NOT BOTHER READING > 
//************************************************


//***************************
// <  ACTUAL MACROS YOU USE > 
//***************************

//These 3 macros take following arguments: ("MSG") or ("CATEGORY", "MSG").  
#define SV_INFO(...)    SV_EXP(SV_GET_LOG_MACRO_WITH_1_OR_2_ARGS(__VA_ARGS__, SV_INFO2,    SV_INFO1)   (__VA_ARGS__))
#define SV_WARN(...)    SV_EXP(SV_GET_LOG_MACRO_WITH_1_OR_2_ARGS(__VA_ARGS__, SV_WARN2,    SV_WARN1)   (__VA_ARGS__))
#define SV_ERROR(...)   SV_EXP(SV_GET_LOG_MACRO_WITH_1_OR_2_ARGS(__VA_ARGS__, SV_ERROR2,   SV_ERROR1)  (__VA_ARGS__))

//basic assert is fine, but i really need to also print assert to logs, so:
#define SV_ASSERT(COND) { if(!static_cast<bool>(COND)) {\
                            SV_DO_LOG_BASE(Logger::Level::Assert, #COND)\
                            assert(false);} }

#define SV_UNREACHABLE() {SV_ASSERT(false && "Unreachable reached!"); SV_UNREACHABLE_CALL();}


//these are for creating macros for some module/category. See example just below.
#define SV_INFO_FOR_LC2(LCMASTERFLAG, MSG)			                if constexpr (LCMASTERFLAG)				{SV_INFO1(MSG);}
#define SV_INFO_FOR_LC4(LCMASTERFLAG, MSG, CATEGORY, CATEGORYTEXT)	if constexpr (LCMASTERFLAG && CATEGORY)	{SV_INFO2(CATEGORYTEXT, MSG);}

//***************************
// < /ACTUAL MACROS YOU USE > 
//***************************

//*****************************************************************
// <  EXAMPLE OF CREATING LOGGING MACROS FOR SOME MODULE/CATEGORY > 
//*****************************************************************

//All the flags are written like that, so that they have
//a default value, only if they werent set externally (like, in Cmake)

//Master flag for this logging category, if its 0, all logs disabled.
#ifndef SV_LC_KEKMODULE
#define SV_LC_KEKMODULE 1
#endif

//Subcategory which is OFF by default, unless set externally
#ifndef SV_LC_KEKMODULE_APPLES
#define SV_LC_KEKMODULE_APPLES 0
#endif

//Subcategory which is ON by default, unless set externally
#ifndef SV_LC_KEKMODULE_ORANGES
#define SV_LC_KEKMODULE_ORANGES 1
#endif

//Two logging macros you will use
#define SV_KEKMODULE_INFO(MSG)				SV_INFO_FOR_LC2(SV_LC_KEKMODULE, MSG)
#define SV_KEKMODULE_INFO_C(CATEGORY, MSG)	SV_INFO_FOR_LC4(SV_LC_KEKMODULE, MSG, CATEGORY, #CATEGORY)

//Usage:
 
//  (Ofocurse both macros can only work when this logging level in general is enabled, i.e.
//  SV_LOGGER_ENABLED and SV_LOGGER_INFO_ENABLED are 1)

//  Prints when SV_LC_KEKMODULE is 1
//  SV_KEKMODULE_INFO("Hello")
// 
//  Prints when SV_LC_KEKMODULE and SV_LC_KEKMODULE_ORANGES are 1
//  SV_KEKMODULE_INFO_C(SV_LC_KEKMODULE_ORANGES, "World")

//*****************************************************************
// < /EXAMPLE OF CREATING LOGGING MACROS FOR SOME MODULE/CATEGORY > 
//*****************************************************************


class Logger
{
public:
    using CategoryList = std::set<std::string>;

    //From least to most significant. Values used as array indices.
    enum class Level : int
    {
        Info = 0,
        Warn,
        Error,
        Assert, //Messages with this level are ALWAYS logged, no matter what.
        Last = Assert
    };

    static Logger& instance();

    //todo this should ve been string view.
    //category can be nullptr
    void log(const std::string& msg, Level level, const char* category, const std::string_view FILE, int LINE);

    void doLogMessage(const std::string& finalMsg, const std::string& ansiColor);

    void logAppLaunchMessage();

    void logAppExitMessage(int returnCode);

    // e.g. setting it to Level::Warn will completely filter out Level::Info
    // messages from all logs, because its less significant level.
    // Setting it to Level::Info allows everything.
    void setMinimallySignificantAllowedLevel(Level level);

    // Logger operates in one of these two modes, whitelist or blacklist:
    void setPrintOnlyTheseCategoriesMode(const CategoryList& whitelistCategories);
    void setPrintEverythingExceptTheseCategoriesMode(const CategoryList& blacklistCategories);

    void truncateLogFileIfNeeded(int willTruncIfLargerThanThisSize  = 1024 * 256,
                                 int truncationToLastBytesSize      = 1024 * 128);

private:
    bool levelIsAllowed(Level level);

    bool categoryIsAllowed(const char* category);

    struct LevelData
    {
        std::string name;
        std::string_view ansiColor;
    };
    const LevelData& getLevelData(Level level);

    void printMessageToTerminal(const std::string& msg, const std::string& ansiColorCode);

    void appendMessageToFile(const std::string& msg);

private:
    Level        allowedUpTo = Level::Info;
    CategoryList categoriesForFilter;
    bool         filterIsWhitelist = false;

    std::string  logFile = "log.txt";
};