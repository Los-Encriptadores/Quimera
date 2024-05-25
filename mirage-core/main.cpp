#include <iostream>
#include <fstream>
#include <vector>
#include <sodium.h>
#include <chrono>

#include "encryption/Encryption.h"

void log(const std::string &message) {
    std::cout << message << std::endl;
}

void createLargeFile(const std::string &filename, std::streamsize size) {
    std::ofstream outFile(filename, std::ios::binary);
    std::vector<unsigned char> buffer(1024 * 1024, 0); // 1MB buffer filled with zeros

    for (std::streamsize i = 0; i < size; i += buffer.size()) {
        outFile.write(reinterpret_cast<char *>(buffer.data()), buffer.size());
    }
    outFile.close();
}

void deleteFile(const std::string &file) {
    log("Deleting the file...");
    std::remove(file.c_str());
    std::remove((file + ".lol").c_str());
}

int main() {
    try {
        Encryption encryption;
        encryption.generateKey();

        const std::string inputFile = "largefile.dat";

        log("Creating a large file...");
        createLargeFile(inputFile, 2000 * 1024 * 1024); // 2GB file

        log("Encrypting the file...");
        const auto startEncrypt = std::chrono::high_resolution_clock::now();
        if (encryption.encryptFile(inputFile)) {
            const auto endEncrypt = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> elapsedEncrypt = endEncrypt - startEncrypt;
            log("Encryption successful");
            log("Time taken for encryption: " + std::to_string(elapsedEncrypt.count()) + " seconds");
        } else {
            log("Encryption failed");
            deleteFile(inputFile);
            return 1;
        }

        log("Decrypting the file...");
        const auto startDecrypt = std::chrono::high_resolution_clock::now();
        if (encryption.decryptFile(inputFile + ".lol")) {
            const auto endDecrypt = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> elapsedDecrypt = endDecrypt - startDecrypt;
            log("Decryption successful");
            log("Time taken for decryption: " + std::to_string(elapsedDecrypt.count()) + " seconds");
        } else {
            log("Decryption failed");
            deleteFile(inputFile);
            return 1;
        }
        deleteFile(inputFile);
    } catch (const std::exception &e) {
        log(std::string("An error occurred: ") + e.what());
        return 1;
    }

    return 0;
}
