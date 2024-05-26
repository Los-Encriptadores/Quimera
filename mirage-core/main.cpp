#include <iostream>
#include <fstream>
#include <chrono>
#include <vector>
#include <random>

#define MIN_FILE_SIZE 100 // in MB
#define MAX_FILE_SIZE 1000 // in MB
#define NUM_FILES 2
#define FILES_PREFIX "mirage-test-file-"
#define FILE_EXTENSION ".dat"
#define ENCRYPTED_EXTENSION ".crypt"
#define BASE_BUFFER_SIZE (64 * 1024) // 64KB

#include "encryption/Encryption.h"

// Function to generate random data
void generateRandomData(std::vector<char> &buffer) {
    const size_t size = buffer.size();
    for (size_t i = 0; i < size; ++i) {
        buffer[i] = static_cast<char>(random() % 256);
    }
}

void createMultipleLargeFiles(const size_t fileSizeMin = MIN_FILE_SIZE, const size_t fileSizeMax = MAX_FILE_SIZE) {
    std::cout << "* Creating " << NUM_FILES << " large files (" << fileSizeMin << "MB - " << fileSizeMax << "MB)" <<
            std::endl;
    std::size_t totalSize = 0;
    for (size_t i = 0; i < NUM_FILES; ++i) {
        const std::string fileName = std::string(FILES_PREFIX).append(std::to_string(i)).append(FILE_EXTENSION);
        const size_t fileSizeMB = fileSizeMin + rand() % (fileSizeMax - fileSizeMin + 1); // in MB
        const size_t fileSize = fileSizeMB * 1024 * 1024; // in bytes
        std::cout << "  - Creating file " << fileName << " (" << fileSizeMB << "MB)" << std::endl;
        totalSize += fileSize;

        std::ofstream outFile(fileName, std::ios::binary);
        if (!outFile) {
            std::cerr << "Failed to create file: " << fileName << std::endl;
            return;
        }

        std::vector<char> buffer(BASE_BUFFER_SIZE);
        size_t bytesWritten = 0;
        while (bytesWritten < fileSize) {
            generateRandomData(buffer);
            size_t bytesToWrite = std::min(buffer.size(), fileSize - bytesWritten);
            outFile.write(buffer.data(), bytesToWrite);
            bytesWritten += bytesToWrite;
        }

        outFile.close();
        std::cout << "  + File created" << std::endl;
    }

    std::cout << "+ Files created, total size:" << (totalSize / (1024 * 1024)) << "MB" << std::endl;
}

void encryptFiles(const Encryption &encryption) {
    for (size_t i = 0; i < NUM_FILES; ++i) {
        const std::string fileName = std::string(FILES_PREFIX).append(std::to_string(i)).append(FILE_EXTENSION);
        auto start = std::chrono::high_resolution_clock::now();
        if (encryption.encryptFile(fileName)) {
            auto end = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double, std::milli> elapsed = end - start;
            std::cout << "  - File encrypted: " << fileName << " (" << elapsed.count() << "ms)" << std::endl;
        } else {
            std::cerr << "Failed to encrypt file: " << fileName << std::endl;
        }
    }
}

void decryptFiles(const Encryption &encryption) {
    for (size_t i = 0; i < NUM_FILES; ++i) {
        const std::string encryptedFileName = std::string(FILES_PREFIX).append(std::to_string(i)).append(FILE_EXTENSION)
                .append(ENCRYPTED_EXTENSION);
        auto start = std::chrono::high_resolution_clock::now();
        if (encryption.decryptFile(encryptedFileName)) {
            auto end = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double, std::milli> elapsed = end - start;
            std::cout << "  - File decrypted: " << encryptedFileName << " (" << elapsed.count() << "ms)" << std::endl;
        } else {
            std::cerr << "Failed to decrypt file: " << encryptedFileName << std::endl;
        }
    }
}

int main() {
    createMultipleLargeFiles();

    Encryption encryption;
    encryption.generateAndHashKey();

    std::cout << "Encrypting test files..." << std::endl;
    auto start = std::chrono::high_resolution_clock::now();
    encryptFiles(encryption);
    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "Total encryption time: " << std::chrono::duration<double, std::milli>(end - start).count() << "ms"
            << std::endl;

    std::cout << "Decrypting test files..." << std::endl;
    start = std::chrono::high_resolution_clock::now();
    decryptFiles(encryption);
    end = std::chrono::high_resolution_clock::now();
    std::cout << "Total decryption time: " << std::chrono::duration<double, std::milli>(end - start).count() << "ms"
            << std::endl;

    std::cout << "* Deleting " << NUM_FILES << " test files" << std::endl;
    for (size_t i = 0; i < NUM_FILES; ++i) {
        const std::string fileName = std::string(FILES_PREFIX).append(std::to_string(i)).append(FILE_EXTENSION);
        const std::string encryptedFileName = fileName + ENCRYPTED_EXTENSION;
        if (remove(fileName.c_str()) != 0) {
            std::cerr << "Failed to delete file: " << fileName << std::endl;
        } else {
            std::cout << fileName << "  + File deleted" << std::endl;
        }
        if (remove(encryptedFileName.c_str()) != 0) {
            std::cerr << "Failed to delete file: " << encryptedFileName << std::endl;
        } else {
            std::cout << encryptedFileName << "  + File deleted" << std::endl;
        }
    }

    return 0;
}
