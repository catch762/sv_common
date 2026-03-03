#pragma once
#include "Common.h"

// <internal: these get used by other logging macros>
#define SV_DO_LOG_FULL(LEVEL, MSG, CATEGORY)  Logger::instance().log(MSG, LEVEL, CATEGORY, __FILE_NAME__, __LINE__);
#define SV_DO_LOG_BASE(LEVEL, MSG)            SV_DO_LOG_FULL(LEVEL, MSG, std::string())
#define SV_GET_LOG_MACRO_WITH_1_OR_2_ARGS(_1, _2, NAME, ...) NAME

#define SV_LOG2(CATEGORY, MSG)      SV_DO_LOG_FULL( Logger::Level::Info,    MSG, CATEGORY )
#define SV_LOG1(MSG)                SV_DO_LOG_BASE( Logger::Level::Info,    MSG )
#define SV_WARN2(CATEGORY, MSG)     SV_DO_LOG_FULL( Logger::Level::Warn,    MSG, CATEGORY )
#define SV_WARN1(MSG)               SV_DO_LOG_BASE( Logger::Level::Warn,    MSG )
#define SV_ERROR2(CATEGORY, MSG)    SV_DO_LOG_FULL( Logger::Level::Error,   MSG, CATEGORY )
#define SV_ERROR1(MSG)              SV_DO_LOG_BASE( Logger::Level::Error,   MSG )
// </internal>


//These 3 macros take following arguments: ("MSG") or ("CATEGORY", "MSG").  
#define SV_LOG(...)     SV_GET_LOG_MACRO_WITH_1_OR_2_ARGS(__VA_ARGS__, SV_LOG2,     SV_LOG1)    (__VA_ARGS__)
#define SV_WARN(...)    SV_GET_LOG_MACRO_WITH_1_OR_2_ARGS(__VA_ARGS__, SV_WARN2,    SV_WARN1)   (__VA_ARGS__)
#define SV_ERROR(...)   SV_GET_LOG_MACRO_WITH_1_OR_2_ARGS(__VA_ARGS__, SV_ERROR2,   SV_ERROR1)  (__VA_ARGS__)

//basic assert is fine, but i really need to also print assert to logs, so:
#define SV_ASSERT(COND) { if(!static_cast<bool>(COND)) {\
                            SV_DO_LOG_BASE(Logger::Level::Assert, #COND)\
                            assert(false);} }

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

    static Logger& instance()
    {
        static Logger logger;
        return logger;
    }

    void log(const std::string &msg, Level level, const std::string &category, const std::string &FILE, int LINE)
    {
        bool isAllowed      = levelIsAllowed(level) && categoryIsAllowed(category);
        bool isForceAllowed = level == Level::Assert;
        if (!isAllowed && !isForceAllowed) return;

        auto levelData = getLevelData(level);
        std::string cat = category.empty() ? "" : format(" [{}]", category);

        auto finalText = std::format(R"({}, {} {}{}: {})", FILE, std::to_string(LINE), levelData.name, cat, msg);

        printMessageToTerminal(finalText, levelData.ansiColor);

        if (printToFile)
        {
            appendMessageToFile(finalText);
        }
    }

    // e.g. setting it to Level::Warn will completely filter out Level::Info
    // messages from all logs, because its less significant level.
    // Setting it to Level::Info allows everything.
    void setMinimallySignificantAllowedLevel(Level level)
    {
        allowedUpTo = level;
    }

    // Logger operates in one of these two modes, whitelist or blacklist:
    void setPrintOnlyTheseCategoriesMode(const CategoryList& whitelistCategories)
    {
        categoriesForFilter = whitelistCategories;
        filterIsWhitelist = true;
    }
    void setPrintEverythingExceptTheseCategoriesMode(const CategoryList& blacklistCategories)
    {
        categoriesForFilter = blacklistCategories;
        filterIsWhitelist = false;
    }

private:
    bool levelIsAllowed(Level level)
    {
        if (level == Level::Assert) return true;
        return int(level) >= int(allowedUpTo);
    }

    bool categoryIsAllowed(const std::string &category)
    {
        bool existsInCategoryList = categoriesForFilter.find(category) != categoriesForFilter.end();
        return filterIsWhitelist ? existsInCategoryList : !existsInCategoryList;
    }

    struct LevelData
    {
        std::string name;
        std::string ansiColor;
    };
    const LevelData& getLevelData(Level level)
    {
        const int entriesCount = 4;
        static_assert(entriesCount == int(Level::Last) + 1);
        static LevelData dataArray[entriesCount] = {
            {"INFO", ANSICodes::none},
            {"WARN", ANSICodes::orange},
            {"ERROR", ANSICodes::red},
            {"ASSERT FAILED", ANSICodes::red} 
        };

        int idx = int(level);

        if (idx < 0 || idx >= entriesCount)
        {
            static LevelData errVal{"WrongLoggingLevel", ANSICodes::cyan};
            return errVal;
        }

        return dataArray[idx];
    };

    void printMessageToTerminal(const std::string &msg, const std::string &ansiColorCode)
    {
        std::cout << ansiColorCode + msg + ANSICodes::reset << std::endl;
    }

    void appendMessageToFile(const std::string &msg)
    {
        std::ofstream ofs(logFile, std::ios::app);
	    ofs << msg << std::endl;
    }

private:
    Level        allowedUpTo = Level::Info;
    CategoryList categoriesForFilter;
    bool         filterIsWhitelist = false;

    bool         printToFile = true;
    std::string  logFile = "log.txt";
};