#include <iostream>
#include <fstream>
#include <vector>
#include <array>
#include <chrono>
#include <filesystem>
#include "encryption/Encryption.h"

using namespace encryption;

const std::size_t CHUNK_SIZE = 4096; // Size of each chunk to read and process

void processFileInPlace(const std::filesystem::path &filePath, Encryption &encryption, bool encrypt) {
    std::fstream file(filePath, std::ios::in | std::ios::out | std::ios::binary);
    if (!file) {
        throw std::runtime_error("Unable to open file for reading and writing: " + filePath.string());
    }

    std::vector<uint8_t> aad; // Additional Authenticated Data (optional)
    std::vector<uint8_t> tag(16); // Tag for encryption
    std::vector<uint8_t> buffer(CHUNK_SIZE);

    auto start = std::chrono::high_resolution_clock::now();

    file.seekg(0, std::ios::end);
    std::streampos fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    std::streampos position = 0;

    while (position < fileSize) {
        file.seekg(position);
        file.read(reinterpret_cast<char *>(buffer.data()), CHUNK_SIZE);
        std::streamsize bytesRead = file.gcount();
        buffer.resize(bytesRead);

        std::vector<uint8_t> processedChunk;
        if (encrypt) {
            processedChunk = encryption.encrypt(buffer, aad, tag);
            // Append tag to the encrypted chunk
            processedChunk.insert(processedChunk.end(), tag.begin(), tag.end());
        } else {
            if (buffer.size() < 16) {
                throw std::runtime_error("File chunk too small to contain tag.");
            }
            std::vector<uint8_t> chunkTag(buffer.end() - 16, buffer.end());
            buffer.resize(buffer.size() - 16);

            processedChunk = encryption.decrypt(buffer, aad, chunkTag);
        }

        file.seekp(position);
        file.write(reinterpret_cast<const char *>(processedChunk.data()), processedChunk.size());
        position += bytesRead;
    }

    file.close();

    // Rename the file if encrypting or decrypting
    if (encrypt) {
        std::filesystem::rename(filePath, filePath.string() + ".lolgetfck3d");
    } else {
        if (filePath.extension() == ".lolgetfck3d") {
            std::filesystem::rename(filePath, filePath.stem());
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end - start;
    std::cout << "File processed successfully in " << duration.count() << " seconds.\n";
}

int main() {
    const std::array<uint8_t, 16> key = {
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F
    };
    Encryption encryption(Encryption::Method::AESGCM128, key);

    std::string filename;
    std::cout << "Enter filename: ";
    std::cin >> filename;

    // Print the current working directory to ensure we are in the right place
    std::cout << "Current working directory: " << std::filesystem::current_path() << std::endl;

    // Convert filename to absolute path relative to the project root
    std::filesystem::path projectRoot = std::filesystem::current_path().parent_path();
    std::filesystem::path filePath = projectRoot / filename;

    // Check if the file exists
    std::ifstream fileCheck(filePath);
    if (!fileCheck) {
        std::cerr << "Error: File does not exist or cannot be opened: " << filePath.string() << "\n";
        return 1;
    }
    fileCheck.close();

    while (true) {
        std::cout << "Options: (1) Encrypt (2) Decrypt (3) Exit\n";
        std::cout << "Enter option: ";
        int option;
        std::cin >> option;

        if (option == 1) {
            try {
                processFileInPlace(filePath, encryption, true);
            } catch (const std::exception &e) {
                std::cerr << "Error: " << e.what() << "\n";
            }
        } else if (option == 2) {
            try {
                processFileInPlace(filePath, encryption, false);
            } catch (const std::exception &e) {
                std::cerr << "Error: " << e.what() << "\n";
            }
        } else if (option == 3) {
            std::cout << "Exiting...\n";
            break;
        } else {
            std::cout << "Invalid option. Please try again.\n";
        }
    }

    return 0;
}
