#ifndef FILEHANDLER_H
#define FILEHANDLER_H

#include <vector>
#include <string>
#include <filesystem>

class FileHandler {
public:
    static std::vector<uint8_t> readFile(const std::filesystem::path &filePath);

    static void writeFile(const std::filesystem::path &filePath, const std::vector<uint8_t> &data);
};

#endif // FILEHANDLER_H
