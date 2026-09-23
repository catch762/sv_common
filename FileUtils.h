#pragma once
#include "Common.h"
#include <filesystem>
#include <fstream>
#include <system_error>

//it actually keeps last keepSize+4096 bytes, but i dont care tbh 
StringErrOpt ifFileLargerThanLimitTruncateToLastNBytes( const std::filesystem::path&    file_path,
                                                        uint64_t                        maxSizeLimit, 
                                                        uint64_t                        keepSize );