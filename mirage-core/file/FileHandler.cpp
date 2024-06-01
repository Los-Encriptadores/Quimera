#include "FileHandler.h"
#include <stdexcept>
#include <iostream>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

namespace file {
    FileHandler::FileHandler(const std::string &inputFilename, const std::string &outputFilename)
        : inputFile(inputFilename, std::ios::binary), outputFile(outputFilename, std::ios::binary), inputFd(-1),
          fileSize(0), fileData(nullptr) {
        if (!inputFile.is_open() || !outputFile.is_open()) {
            throw std::runtime_error("Failed to open file streams");
        }

        inputFd = open(inputFilename.c_str(), O_RDONLY);
        if (inputFd == -1) {
            throw std::runtime_error("Failed to open input file descriptor");
        }

        struct stat sb{};
        if (fstat(inputFd, &sb) == -1) {
            close(inputFd);
            throw std::runtime_error("Failed to get file size");
        }
        fileSize = sb.st_size;

        fileData = static_cast<const unsigned char *>(mmap(nullptr, fileSize, PROT_READ, MAP_PRIVATE, inputFd, 0));
        if (fileData == MAP_FAILED) {
            close(inputFd);
            throw std::runtime_error("Failed to map file to memory");
        }
    }

    FileHandler::~FileHandler() {
        if (inputFile.is_open()) inputFile.close();
        if (outputFile.is_open()) outputFile.close();
        if (fileData != nullptr) {
            munmap(const_cast<unsigned char *>(fileData), fileSize);
        }
        if (inputFd != -1) {
            close(inputFd);
        }
    }
}
