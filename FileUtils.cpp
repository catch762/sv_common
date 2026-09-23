#include "FileUtils.h"

StringErrOpt ifFileLargerThanLimitTruncateToLastNBytes( const std::filesystem::path&    file_path,
                                                        uint64_t                        maxSizeLimit, 
                                                        uint64_t                        keepSize ) 
{
    namespace fs = std::filesystem;
    std::error_code ec;

    // 1. Check if the file exists and get its size
    uint64_t file_size = fs::file_size(file_path, ec);
    if (ec)
    {
        return "Error reading file size: " + ec.message();
    }

    // 2. Return early if the file does not exceed N bytes
    if (file_size <= maxSizeLimit) {
        return {};
    }

    // Edge case: If M is greater than or equal to the file size, nothing needs to be removed
    if (keepSize >= file_size) {
        return {};
    }

    // 3. Open the file in binary mode for reading and writing
    std::fstream file(file_path, std::ios::in | std::ios::out | std::ios::binary);
    if (!file.is_open())
    {
        return "Failed to open file for modifying";
    }

    // 4. Shift the last M bytes to the beginning of the file
    if (keepSize > 0)
    {
        // Position read pointer to the start of the last M bytes
        file.seekg(file_size - keepSize, std::ios::beg);

        // Use a buffer to safely copy data (chunks prevent high memory usage on large M)
        constexpr size_t buffer_size = 4096;
        char buffer[buffer_size];

        uint64_t bytes_moved = 0;
        while (bytes_moved < keepSize)
        {
            uint64_t to_read = std::min(static_cast<uint64_t>(buffer_size), keepSize - bytes_moved);

            file.read(buffer, to_read);
            std::streamsize read_bytes = file.gcount();
            if (read_bytes <= 0) break;

            // Position write pointer to the current destination index
            file.seekp(bytes_moved, std::ios::beg);
            file.write(buffer, read_bytes);

            bytes_moved += read_bytes;

            // Restore read pointer location for the next iteration
            file.seekg(file_size - keepSize + bytes_moved, std::ios::beg);
        }
    }

    // Flush and close the stream before using filesystem resize
    file.close();

    // 5. Truncate the file to exactly M bytes
    fs::resize_file(file_path, keepSize, ec);
    if (ec)
    {
        return "Error truncate-resizing file: " + ec.message();
    }

    return {};
}