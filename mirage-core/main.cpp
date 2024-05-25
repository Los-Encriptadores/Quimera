#include <iostream>
#include <fstream>
#include <chrono>
#include <vector>

#include "encryption/Encryption.h"

void log(const std::string &message) {
    std::cout << message << std::endl;
}

void createLargeFile(const std::string &fileName, std::size_t size) {
    std::ofstream outFile(fileName, std::ios::binary);
    if (!outFile.is_open()) {
        std::cerr << "Failed to create file: " << fileName << std::endl;
        return;
    }
    std::vector<unsigned char> buffer(100 * 1024 * 1024, 'A'); // 100MB buffer of 'A's
    for (std::size_t written = 0; written < size; written += buffer.size()) {
        outFile.write(reinterpret_cast<char *>(buffer.data()), buffer.size());
    }
    std::cout << "File created successfully" << std::endl;
    outFile.close();
}

void deleteFile(const std::string &file) {
    log("Deleting the file...");
    std::remove(file.c_str());
    std::remove((file + ".lol").c_str());
    std::cout << "File deleted" << std::endl;
}

int main() {
    try {
        Encryption encryption;
        encryption.generateKey();

        const std::string inputFile = "largefile.dat";

        log("Creating a large file...");
        createLargeFile(inputFile, 10000 * 1024 * 1024); // 10GB file

        log("Encrypting the file...");
        auto startEncrypt = std::chrono::high_resolution_clock::now();
        if (encryption.encryptFile(inputFile)) {
            auto endEncrypt = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> elapsedEncrypt = endEncrypt - startEncrypt;
            log("Encryption successful");
            log("Time taken for encryption: " + std::to_string(elapsedEncrypt.count()) + " seconds");
        } else {
            log("Encryption failed");
            deleteFile(inputFile);
            return 1;
        }

        log("Decrypting the file...");
        auto startDecrypt = std::chrono::high_resolution_clock::now();
        if (encryption.decryptFile(inputFile + ".lol")) {
            auto endDecrypt = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> elapsedDecrypt = endDecrypt - startDecrypt;
            log("Decryption successful");
            log("Time taken for decryption: " + std::to_string(elapsedDecrypt.count()) + " seconds");
        } else {
            log("Decryption failed");
            deleteFile(inputFile);
            return 1;
        }

        // Demonstrating the new hashing functionality
        const std::string message = "This is a test message.";
        log("Hashing the message: " + message);
        std::string hash = Encryption::hashMessage(message);
        log("Hashed message: " + hash);

        deleteFile(inputFile);
    } catch (const std::exception &e) {
        log(std::string("An error occurred: ") + e.what());
        return 1;
    }

    return 0;
}
