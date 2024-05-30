#include <fstream>
#include <iostream>
#include <sodium.h>
#include <string>
#include <vector>
#include <ctime>
#include <chrono>
#include <iomanip>
#include <sstream>

#include "engines/encryption/PolymorphicEncryptionEngine.h"

// Function to generate a test file with a random message
void generateTestFile(const std::string &filename) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file for writing: " + filename);
    }

    // Write random data to the file
    constexpr size_t fileSize = 200 * 1024 * 1024; // 200 MB
    std::string message;
    message.reserve(fileSize);
    srandom(std::time(nullptr));
    for (size_t i = 0; i < fileSize; ++i) {
        message.push_back(random() % 256);
    }

    if (!file.write(message.c_str(), message.size())) {
        throw std::runtime_error("Failed to write data to file: " + filename);
    }
}

void displayMenu() {
    std::cout << "\n1. Encrypt\n";
    std::cout << "2. Decrypt\n";
    std::cout << "3. Exit\n";
    std::cout << "Choose an option: ";
}

void deleteFiles() {
    std::remove("test.ini");
    std::remove("encrypted_test.ini");
    std::remove("decrypted_test.ini");
}

std::string formatDuration(const double milliseconds) {
    std::stringstream ss;
    ss << std::fixed << std::setprecision(3) << milliseconds << " ms";
    return ss.str();
}

int main() {
    try {
        const std::string filename = "test.ini";
        const std::string encryptedFilename = "encrypted_test.ini";
        const std::string decryptedFilename = "decrypted_test.ini";

        // Delete the encrypted and decrypted files if they exist
        deleteFiles();

        // Generate a test file with a random message
        std::cout << "Generating test file: " << filename << std::endl;
        generateTestFile(filename);
        std::cout << "Generated test file: " << filename << std::endl;

        // Create the encryption engine instance
        PolymorphicEncryptionEngine engine;

        int choice;
        do {
            displayMenu();
            std::cin >> choice;

            switch (choice) {
                case 1: {
                    std::cout << "Encrypting file: " << filename << std::endl;
                    auto start = std::chrono::high_resolution_clock::now();
                    engine.encryptFile(filename, encryptedFilename);
                    auto end = std::chrono::high_resolution_clock::now();
                    std::chrono::duration<double, std::milli> encryptionTime = end - start;
                    std::cout << "Encrypted file: " << encryptedFilename << std::endl;
                    std::cout << "Encryption time: " << formatDuration(encryptionTime.count()) << std::endl;
                    break;
                }

                case 2: {
                    std::cout << "Decrypting file: " << encryptedFilename << std::endl;
                    auto start = std::chrono::high_resolution_clock::now();
                    engine.decryptFile(encryptedFilename, decryptedFilename);
                    auto end = std::chrono::high_resolution_clock::now();
                    std::chrono::duration<double, std::milli> decryptionTime = end - start;
                    std::cout << "Decrypted file: " << decryptedFilename << std::endl;
                    std::cout << "Decryption time: " << formatDuration(decryptionTime.count()) << std::endl;
                    break;
                }

                case 3:
                    std::cout << "Exiting application." << std::endl;
                    deleteFiles();
                    break;

                default:
                    std::cout << "Invalid choice. Please try again." << std::endl;
                    break;
            }
        } while (choice != 3);

        std::cout << "Encryption and decryption operations completed successfully." << std::endl;
    } catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
