#include "FileHandler.h"
#include <fstream>
#include <stdexcept>
#include <thread>
#include <chrono>
#include <iostream>

std::vector<uint8_t> FileHandler::readFile(const std::filesystem::path &filePath) {
    std::vector<uint8_t> fileContent;
    for (int attempt = 0; attempt < 5; ++attempt) {
        if (std::ifstream file(filePath, std::ios::binary | std::ios::ate); file) {
            std::streamsize fileSize = file.tellg();
            file.seekg(0, std::ios::beg);
            fileContent.resize(fileSize);
            if (file.read(reinterpret_cast<char *>(fileContent.data()), fileSize)) {
                file.close();
                break;
            }
        }
        if (attempt < 4) {
            std::cerr << "Unable to open file, retrying...\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        } else {
            throw std::runtime_error("Unable to open file for reading: " + filePath.string());
        }
    }
    return fileContent;
}

void FileHandler::writeFile(const std::filesystem::path &filePath, const std::vector<uint8_t> &data) {
    std::ofstream outFile(filePath, std::ios::binary);
    if (!outFile) {
        throw std::runtime_error("Unable to open file for writing: " + filePath.string());
    }
    if (data.size() > static_cast<std::vector<uint8_t>::size_type>(std::numeric_limits<std::streamsize>::max())) {
        throw std::runtime_error("Data size too large for writing: " + filePath.string());
    }
    outFile.write(reinterpret_cast<const char *>(data.data()), static_cast<std::streamsize>(data.size()));
    outFile.close();
}
