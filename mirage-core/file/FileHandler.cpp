#include "FileHandler.h"

namespace file {
    FileHandler::FileHandler(const std::string &inputFilename, const std::string &outputFilename)
        : inputFile(inputFilename, std::ios::binary), outputFile(outputFilename, std::ios::binary) {
        if (!inputFile.is_open() || !outputFile.is_open()) {
            throw std::runtime_error("Failed to open file");
        }
    }

    FileHandler::~FileHandler() {
        if (inputFile.is_open()) inputFile.close();
        if (outputFile.is_open()) outputFile.close();
    }
}
