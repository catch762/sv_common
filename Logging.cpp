#include "Logging.h"

Logger& Logger::instance()
{
    static Logger logger;
    return logger;
}

void Logger::log(const std::string& msg, Level level, const char* category, const std::string_view FILE, int LINE)
{
    bool isAllowed = levelIsAllowed(level) && categoryIsAllowed(category);
    bool isForceAllowed = level == Level::Assert;
    if (!isAllowed && !isForceAllowed) return;

    auto levelData = getLevelData(level);
    std::string cat = category ? std::format(" [{}]", category) : "";

    auto finalText = std::format(R"({}, {} {}{}: {})", FILE, std::to_string(LINE), levelData.name, cat, msg);

    doLogMessage(finalText, std::string(levelData.ansiColor));
}

void Logger::doLogMessage(const std::string& finalMsg, const std::string& ansiColor)
{
    printMessageToTerminal(finalMsg, ansiColor);

    if constexpr (SV_LOGGER_LOG_TO_FILE_ENABLED)
    {
        appendMessageToFile(finalMsg);
    }
}

void Logger::logAppLaunchMessage()
{
    auto workFolder = std::filesystem::current_path().string();

    auto text = std::format("\n"
        "**************************************\n"
        "--- app launch:  {} \n"
        "--- work folder: {} \n"
        "**************************************\n",
        getCurrentTimeHMS(),
        workFolder);
    doLogMessage(text, std::format("{}{}", ANSICodes::bold, ANSICodes::cyan));
}

void Logger::logAppExitMessage(int returnCode)
{
    auto text = std::format("-------- app exited with {} ---------", returnCode);
    doLogMessage(text, std::format("{}{}", ANSICodes::bold, ANSICodes::cyan));
}

void Logger::setPrintOnlyTheseCategoriesMode(const CategoryList& whitelistCategories)
{
    categoriesForFilter = whitelistCategories;
    filterIsWhitelist = true;
}
void Logger::setPrintEverythingExceptTheseCategoriesMode(const CategoryList& blacklistCategories)
{
    categoriesForFilter = blacklistCategories;
    filterIsWhitelist = false;
}

void Logger::truncateLogFileIfNeeded(int willTruncIfLargerThanThisSize, int truncationToLastBytesSize)
{
    ifFileLargerThanLimitTruncateToLastNBytes(logFile, willTruncIfLargerThanThisSize, truncationToLastBytesSize);
}

bool Logger::levelIsAllowed(Level level)
{
    if (level == Level::Assert) return true;
    return int(level) >= int(allowedUpTo);
}

bool Logger::categoryIsAllowed(const char* category)
{
    if (!category)
    {
        return !filterIsWhitelist;
    }

    bool existsInCategoryList = categoriesForFilter.find(category) != categoriesForFilter.end();
    return filterIsWhitelist ? existsInCategoryList : !existsInCategoryList;
}

const Logger::LevelData& Logger::getLevelData(Level level)
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
        static LevelData errVal{ "WrongLoggingLevel", ANSICodes::cyan };
        return errVal;
    }

    return dataArray[idx];
};

void Logger::printMessageToTerminal(const std::string& msg, const std::string& ansiColorCode)
{
    std::cout << ANSICodes::reset << ansiColorCode << msg << ANSICodes::reset << std::endl;
}

void Logger::appendMessageToFile(const std::string& msg)
{
    std::ofstream ofs(logFile, std::ios::app);
    ofs << msg << std::endl;
}