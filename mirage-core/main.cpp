#include <iostream>
#include <fstream>
#include <vector>
#include <array>
#include <chrono>
#include <filesystem>
#include <future>
#include <thread>
#include "encryption/Encryption.h"

using namespace encryption;

const std::size_t CHUNK_SIZE = 4096; // Size of each chunk to process in parallel

void encryptChunk(const std::vector<uint8_t> &input, std::vector<uint8_t> &output, const Encryption &encryption,
                  std::vector<uint8_t> &tag, size_t start) {
    std::vector<uint8_t> aad; // Additional Authenticated Data (optional)
    std::vector<uint8_t> buffer(input.begin() + start, input.begin() + std::min(input.size(), start + CHUNK_SIZE));
    output = encryption.encrypt(buffer, aad, tag);
    output.insert(output.end(), tag.begin(), tag.end());
}

void decryptChunk(const std::vector<uint8_t> &input, std::vector<uint8_t> &output, const Encryption &encryption,
                  std::vector<uint8_t> &tag, size_t start) {
    std::vector<uint8_t> aad; // Additional Authenticated Data (optional)
    size_t chunkSize = std::min(input.size(), start + CHUNK_SIZE);
    std::vector<uint8_t> buffer(input.begin() + start, input.begin() + chunkSize - tag.size());
    std::vector<uint8_t> chunkTag(input.begin() + chunkSize - tag.size(), input.begin() + chunkSize);
    output = encryption.decrypt(buffer, aad, chunkTag);
}

void processFileInPlace(const std::filesystem::path &filePath, Encryption &encryption, bool encrypt) {
    // Read the entire file into memory with retry mechanism
    std::vector<uint8_t> fileContent;
    for (int attempt = 0; attempt < 5; ++attempt) {
        std::ifstream file(filePath, std::ios::binary | std::ios::ate);
        if (file) {
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

    // Process the file content in parallel
    size_t numChunks = (fileContent.size() + CHUNK_SIZE - 1) / CHUNK_SIZE;
    std::vector<std::future<void> > futures;
    std::vector<std::vector<uint8_t> > processedChunks(numChunks);
    std::vector<std::vector<uint8_t> > tags(numChunks, std::vector<uint8_t>(16));

    auto start = std::chrono::high_resolution_clock::now();

    for (size_t i = 0; i < numChunks; ++i) {
        size_t startIdx = i * CHUNK_SIZE;
        if (encrypt) {
            futures.push_back(std::async(std::launch::async, encryptChunk, std::cref(fileContent),
                                         std::ref(processedChunks[i]), std::cref(encryption), std::ref(tags[i]),
                                         startIdx));
        } else {
            futures.push_back(std::async(std::launch::async, decryptChunk, std::cref(fileContent),
                                         std::ref(processedChunks[i]), std::cref(encryption), std::ref(tags[i]),
                                         startIdx));
        }
    }

    for (auto &f: futures) {
        f.get();
    }

    // Write the processed content back to the file
    std::ofstream outFile(filePath, std::ios::binary);
    if (!outFile) {
        throw std::runtime_error("Unable to open file for writing: " + filePath.string());
    }

    for (const auto &chunk: processedChunks) {
        outFile.write(reinterpret_cast<const char *>(chunk.data()), chunk.size());
    }
    outFile.close();

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end - start;
    std::cout << "File " << (encrypt ? "encrypted" : "decrypted") << " successfully in " << duration.count() <<
            " seconds.\n";
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
