/**
 * @file FileHandler.h
 * @brief This file contains the FileHandler class which is used for file operations.
 * @author 0x5844
 * @copyright GPLv3
 * @date 2024
*/

#pragma once
#include <string>
#include <vector>
#include <mutex>
#include <sys/mman.h>

/**
 * @class FileHandler
 * @brief The FileHandler class provides methods for loading and saving files.
*/
namespace file {
    class FileHandler {
    public:
        FileHandler() = default;

        ~FileHandler();

        /**
          * @brief Load a file into memory.
          * @param filePath The path of the file to load.
          * @param chunkSize The size of the chunks to load the file in.
          * @return true if the file was successfully loaded, false otherwise.
        */
        bool loadFile(const std::string &filePath, size_t chunkSize = 4096);

        /**
          * @brief Save a string to a file.
          * @param filePath The path of the file to save to.
          * @param content The content to save to the file.
          * @param chunkSize The size of the chunks to save the file in.
          * @return true if the file was successfully saved, false otherwise.
        */
        bool saveFile(const std::string &filePath, const std::string &content, size_t chunkSize = 4096);

        /**
          * @brief Get the content of the loaded file.
          * @return The content of the loaded file.
        */
        [[nodiscard]] std::string getFileContent() const;

    private:
        std::string fileContent;
        std::mutex fileMutex;
        int fileDescriptor = -1;
        size_t fileSize = 0;

        bool mapFilePart(off_t offset, size_t size, int prot, void *&mappedData) const;

        bool loadFileChunk(off_t offset, size_t size);

        [[nodiscard]] bool saveFileChunk(const std::string &content, off_t offset, size_t size) const;

        static void unmapFile(void *mappedData, size_t size);

        void closeFile();
    };
}
