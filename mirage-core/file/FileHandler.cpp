/**
 * @file FileHandler.cpp
 * @brief This file contains the implementation of the FileHandler class.
 * @author 0x5844
 * @copyright GPLv3
 * @date 2024
*/

#include "FileHandler.h"
#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

namespace file {
    FileHandler::~FileHandler() {
        closeFile();
    }

    void FileHandler::closeFile() {
        if (fileDescriptor != -1) {
            close(fileDescriptor);
            fileDescriptor = -1;
        }
    }

    bool FileHandler::mapFilePart(off_t offset, size_t size, int prot, void *&mappedData) const {
        mappedData = mmap(nullptr, size, prot, MAP_SHARED, fileDescriptor, offset);
        if (mappedData == MAP_FAILED) {
            std::cerr << "Error: Cannot map file part at offset " << offset << "\n";
            return false;
        }
        return true;
    }

    void FileHandler::unmapFile(void *mappedData, const size_t size) {
        if (mappedData) {
            munmap(mappedData, size);
        }
    }

    bool FileHandler::loadFile(const std::string &filePath, size_t chunkSize) {
        fileDescriptor = open(filePath.c_str(), O_RDONLY);
        if (fileDescriptor == -1) {
            std::cerr << "Error: Cannot open file " << filePath << "\n";
            return false;
        }

        struct stat fileStat{};
        if (fstat(fileDescriptor, &fileStat) == -1) {
            std::cerr << "Error: Cannot get file size " << filePath << "\n";
            closeFile();
            return false;
        }

        fileSize = fileStat.st_size;
        off_t offset = 0;

        while (offset < static_cast<off_t>(fileSize)) {
            const off_t size = std::min(static_cast<off_t>(chunkSize), static_cast<off_t>(fileSize - offset));
            if (!loadFileChunk(offset, size)) {
                closeFile();
                return false;
            }
            offset += size;
        }

        closeFile();
        return true;
    }

    bool FileHandler::loadFileChunk(off_t offset, size_t size) {
        void *data = nullptr;
        if (!mapFilePart(offset, size, PROT_READ, data)) {
            return false;
        }

        std::lock_guard<std::mutex> lock(fileMutex);
        fileContent.append(static_cast<char *>(data), size);

        unmapFile(data, size);
        return true;
    }

    bool FileHandler::saveFile(const std::string &filePath, const std::string &content, const size_t chunkSize) {
        fileDescriptor = open(filePath.c_str(), O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
        if (fileDescriptor == -1) {
            std::cerr << "Error: Cannot open file " << filePath << "\n";
            return false;
        }

        if (ftruncate(fileDescriptor, static_cast<off_t>(content.size())) == -1) {
            std::cerr << "Error: Cannot set file size " << filePath << "\n";
            closeFile();
            return false;
        }

        off_t offset = 0;
        while (offset < static_cast<off_t>(content.size())) {
            const off_t size = std::min(static_cast<off_t>(chunkSize), static_cast<off_t>(content.size()) - offset);
            if (!saveFileChunk(content, offset, size)) {
                closeFile();
                return false;
            }
            offset += size;
        }

        closeFile();
        return true;
    }

    bool FileHandler::saveFileChunk(const std::string &content, const off_t offset, size_t size) const {
        void *data = nullptr;
        if (!mapFilePart(offset, size, PROT_READ | PROT_WRITE, data)) {
            return false;
        }

        std::memcpy(data, content.data() + offset, size);

        if (msync(data, size, MS_SYNC) == -1) {
            std::cerr << "Error: Cannot sync file part at offset " << offset << "\n";
            unmapFile(data, size);
            return false;
        }

        unmapFile(data, size);
        return true;
    }

    std::string FileHandler::getFileContent() const {
        return fileContent;
    }
}
