#include "Encryption.h"
#include <iostream>
#include <fstream>
#include <sodium.h>
#include <vector>

// File extension for encrypted files
const std::string FILE_EXTENSION = ".lol";

// Constructor for the Encryption class
// Initializes libsodium and allocates memory for the encryption key
Encryption::Encryption() {
    // Initialize libsodium
    if (sodium_init() == -1) {
        throw std::runtime_error("Failed to initialize libsodium");
    }
    // Allocate memory for the encryption key
    key = static_cast<unsigned char *>(sodium_malloc(crypto_secretstream_xchacha20poly1305_KEYBYTES));
    if (!key) {
        throw std::bad_alloc();
    }
    // Protect the encryption key from being accessed
    sodium_mprotect_noaccess(key);
}

// Destructor for the Encryption class
// Frees the memory allocated for the encryption key
Encryption::~Encryption() {
    // Allow access to the encryption key
    sodium_mprotect_readwrite(key);
    // Zero out the memory of the encryption key
    sodium_memzero(key, crypto_secretstream_xchacha20poly1305_KEYBYTES);
    // Free the memory of the encryption key
    sodium_free(key);
}

// Generates a new encryption key
void Encryption::generateKey() const {
    // Allow access to the encryption key
    sodium_mprotect_readwrite(key);
    // Generate a new encryption key
    crypto_secretstream_xchacha20poly1305_keygen(key);
    // Protect the encryption key from being accessed
    sodium_mprotect_noaccess(key);
}

// Returns the size of a file
size_t getFileSize(const std::string &fileName) {
    std::ifstream in(fileName, std::ifstream::ate | std::ifstream::binary);
    return in.tellg();
}

// Encrypts a file
bool Encryption::encryptFile(const std::string &inputFile) const {
    // Open the input file
    std::ifstream inFile(inputFile, std::ios::binary);
    if (!inFile.is_open()) {
        std::cerr << "Failed to open file for encryption: " << inputFile << std::endl;
        return false;
    }

    // Open the output file
    std::ofstream outFile(inputFile + FILE_EXTENSION, std::ios::binary);
    if (!outFile.is_open()) {
        std::cerr << "Failed to open file for writing: " << inputFile + FILE_EXTENSION << std::endl;
        return false;
    }

    // Initialize the encryption state
    unsigned char header[crypto_secretstream_xchacha20poly1305_HEADERBYTES];
    crypto_secretstream_xchacha20poly1305_state state;

    // Allow read access to the encryption key
    sodium_mprotect_readonly(key);
    if (crypto_secretstream_xchacha20poly1305_init_push(&state, header, key) != 0) {
        // Protect the encryption key from being accessed
        sodium_mprotect_noaccess(key);
        std::cerr << "Failed to initialize encryption stream" << std::endl;
        return false;
    }
    // Protect the encryption key from being accessed
    sodium_mprotect_noaccess(key);

    // Write the header to the output file
    outFile.write(reinterpret_cast<char *>(header), sizeof(header));

    // Encrypt the input file and write the encrypted data to the output file
    size_t fileSize = getFileSize(inputFile);
    size_t bufferSize = std::min(static_cast<size_t>(100 * 1024 * 1024), fileSize / 10);
    std::vector<unsigned char> buffer(bufferSize);
    std::vector<unsigned char> ciphertext(buffer.size() + crypto_secretstream_xchacha20poly1305_ABYTES);
    while (inFile) {
        inFile.read(reinterpret_cast<char *>(buffer.data()), static_cast<std::streamsize>(buffer.size()));
        std::streamsize rlen = inFile.gcount();
        if (crypto_secretstream_xchacha20poly1305_push(&state, ciphertext.data(), nullptr,
                                                       buffer.data(), rlen, nullptr, 0,
                                                       inFile.eof()
                                                           ? crypto_secretstream_xchacha20poly1305_TAG_FINAL
                                                           : 0) != 0) {
            std::cerr << "Encryption failed" << std::endl;
            return false;
        }
        outFile.write(reinterpret_cast<char *>(ciphertext.data()), rlen + crypto_secretstream_xchacha20poly1305_ABYTES);
    }

    // Close the input and output files
    inFile.close();
    outFile.close();
    return true;
}

// Decrypts a file
bool Encryption::decryptFile(const std::string &inputFile) const {
    // Open the input file
    std::ifstream inFile(inputFile, std::ios::binary);
    if (!inFile.is_open()) {
        std::cerr << "Failed to open file for decryption: " << inputFile << std::endl;
        return false;
    }

    // Determine the output file name
    std::string outputFile = inputFile;
    if (outputFile.size() >= FILE_EXTENSION.size() &&
        outputFile.substr(outputFile.size() - FILE_EXTENSION.size()) == FILE_EXTENSION) {
        outputFile = outputFile.substr(0, outputFile.size() - FILE_EXTENSION.size());
    } else {
        std::cerr << "File extension mismatch or too short to remove extension" << std::endl;
        return false;
    }

    // Open the output file
    std::ofstream outFile(outputFile, std::ios::binary);
    if (!outFile.is_open()) {
        std::cerr << "Failed to open file for writing: " << outputFile << std::endl;
        return false;
    }

    // Initialize the decryption state
    unsigned char header[crypto_secretstream_xchacha20poly1305_HEADERBYTES];
    crypto_secretstream_xchacha20poly1305_state state;

    // Read the header from the input file
    inFile.read(reinterpret_cast<char *>(header), sizeof(header));
    if (inFile.gcount() != sizeof(header)) {
        std::cerr << "Failed to read header" << std::endl;
        return false;
    }

    // Allow read access to the encryption key
    sodium_mprotect_readonly(key);
    if (crypto_secretstream_xchacha20poly1305_init_pull(&state, header, key) != 0) {
        // Protect the encryption key from being accessed
        sodium_mprotect_noaccess(key);
        std::cerr << "Failed to initialize decryption stream" << std::endl;
        return false;
    }
    // Protect the encryption key from being accessed

    // Decrypt the input file and write the decrypted data to the output file
    size_t fileSize = getFileSize(inputFile);
    size_t bufferSize = std::min(static_cast<size_t>(100 * 1024 * 1024), fileSize / 10);
    std::vector<unsigned char> buffer(bufferSize + crypto_secretstream_xchacha20poly1305_ABYTES);
    std::vector<unsigned char> decrypted(buffer.size());
    while (inFile) {
        inFile.read(reinterpret_cast<char *>(buffer.data()), static_cast<std::streamsize>(buffer.size()));
        std::streamsize rlen = inFile.gcount();
        unsigned char tag;
        if (crypto_secretstream_xchacha20poly1305_pull(&state, decrypted.data(), nullptr, &tag,
                                                       buffer.data(), rlen, nullptr, 0) != 0) {
            std::cerr << "Decryption failed" << std::endl;
            return false;
        }
        outFile.write(reinterpret_cast<char *>(decrypted.data()), rlen - crypto_secretstream_xchacha20poly1305_ABYTES);
        if (tag == crypto_secretstream_xchacha20poly1305_TAG_FINAL) {
            break;
        }
    }

    // Close the input and output files
    inFile.close();
    outFile.close();
    return true;
}
