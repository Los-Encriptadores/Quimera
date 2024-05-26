#include "Encryption.h"
#include <iostream>
#include <fstream>
#include <sodium.h>
#include <vector>

#define BASE_BUFFER_SIZE (64 * 1024) // 64 KB
#define FILE_EXTENSION ".crypt"      // Encrypted file extension

// Constructor for the Encryption class
Encryption::Encryption() {
    if (sodium_init() == -1) {
        throw std::runtime_error("Failed to initialize libsodium");
    }

    key = static_cast<unsigned char *>(sodium_malloc(crypto_secretstream_xchacha20poly1305_KEYBYTES));
    if (!key) {
        throw std::bad_alloc();
    }

    sodium_mprotect_noaccess(key);
}

// Destructor for the Encryption class
Encryption::~Encryption() {
    sodium_mprotect_readwrite(key);
    sodium_memzero(key, crypto_secretstream_xchacha20poly1305_KEYBYTES);
    sodium_free(key);
}

// Generates a new encryption key and hashes it using BLAKE2b
void Encryption::generateAndHashKey() const {
    unsigned char raw_key[crypto_secretstream_xchacha20poly1305_KEYBYTES];
    crypto_secretstream_xchacha20poly1305_keygen(raw_key);

    unsigned char hash[crypto_generichash_BYTES];
    if (crypto_generichash(hash, sizeof(hash), raw_key, sizeof(raw_key), nullptr, 0) != 0) {
        sodium_memzero(raw_key, sizeof(raw_key));
        throw std::runtime_error("Failed to hash the key using BLAKE2b");
    }

    sodium_mprotect_readwrite(key);
    std::memcpy(key, hash, crypto_secretstream_xchacha20poly1305_KEYBYTES);
    sodium_mprotect_noaccess(key);

    sodium_memzero(raw_key, sizeof(raw_key));
}

// Encrypts a file in place
bool Encryption::encryptFile(const std::string &inputFile) const {
    std::ifstream inFile(inputFile, std::ios::binary);
    if (!inFile.is_open()) {
        std::cerr << "Failed to open file for reading: " << inputFile << std::endl;
        return false;
    }

    std::ofstream outFile(inputFile + FILE_EXTENSION, std::ios::binary);
    if (!outFile.is_open()) {
        std::cerr << "Failed to open file for writing: " << inputFile + FILE_EXTENSION << std::endl;
        return false;
    }

    unsigned char header[crypto_secretstream_xchacha20poly1305_HEADERBYTES];
    crypto_secretstream_xchacha20poly1305_state state;

    sodium_mprotect_readonly(key);
    if (crypto_secretstream_xchacha20poly1305_init_push(&state, header, key) != 0) {
        sodium_mprotect_noaccess(key);
        std::cerr << "Failed to initialize encryption stream" << std::endl;
        return false;
    }
    sodium_mprotect_noaccess(key);

    outFile.write(reinterpret_cast<char *>(header), sizeof(header));

    std::vector<unsigned char> buffer(BASE_BUFFER_SIZE);
    std::vector<unsigned char> ciphertext(BASE_BUFFER_SIZE + crypto_secretstream_xchacha20poly1305_ABYTES);

    while (true) {
        inFile.read(reinterpret_cast<char *>(buffer.data()), buffer.size());
        std::streamsize bytesRead = inFile.gcount();
        if (bytesRead == 0) break;

        unsigned char tag = inFile.eof()
                                ? crypto_secretstream_xchacha20poly1305_TAG_FINAL
                                : crypto_secretstream_xchacha20poly1305_TAG_MESSAGE;

        unsigned long long ciphertext_len;
        if (crypto_secretstream_xchacha20poly1305_push(&state, ciphertext.data(), &ciphertext_len, buffer.data(),
                                                       bytesRead,
                                                       nullptr, 0, tag) != 0) {
            std::cerr << "Encryption failed" << std::endl;
            return false;
        }

        outFile.write(reinterpret_cast<char *>(ciphertext.data()), ciphertext_len);
    }

    inFile.close();
    outFile.close();
    return true;
}

// Decrypts a file in place
bool Encryption::decryptFile(const std::string &inputFile) const {
    std::ifstream inFile(inputFile, std::ios::binary);
    if (!inFile.is_open()) {
        std::cerr << "Failed to open file for reading: " << inputFile << std::endl;
        return false;
    }

    std::string outputFile = inputFile.substr(0, inputFile.size() - strlen(FILE_EXTENSION));
    std::ofstream outFile(outputFile, std::ios::binary);
    if (!outFile.is_open()) {
        std::cerr << "Failed to open file for writing: " << outputFile << std::endl;
        return false;
    }

    unsigned char header[crypto_secretstream_xchacha20poly1305_HEADERBYTES];
    inFile.read(reinterpret_cast<char *>(header), sizeof(header));
    if (inFile.gcount() != sizeof(header)) {
        std::cerr << "Failed to read header, expected " << sizeof(header) << " bytes, but got " << inFile.gcount() <<
                " bytes." << std::endl;
        return false;
    }

    crypto_secretstream_xchacha20poly1305_state state;
    sodium_mprotect_readonly(key);
    if (crypto_secretstream_xchacha20poly1305_init_pull(&state, header, key) != 0) {
        sodium_mprotect_noaccess(key);
        std::cerr << "Failed to initialize decryption stream" << std::endl;
        return false;
    }
    sodium_mprotect_noaccess(key);

    std::vector<unsigned char> buffer(BASE_BUFFER_SIZE + crypto_secretstream_xchacha20poly1305_ABYTES);
    std::vector<unsigned char> plaintext(BASE_BUFFER_SIZE);

    while (true) {
        inFile.read(reinterpret_cast<char *>(buffer.data()), buffer.size());
        std::streamsize bytesRead = inFile.gcount();
        if (bytesRead == 0) break;

        unsigned char tag;
        unsigned long long plaintext_len;
        if (crypto_secretstream_xchacha20poly1305_pull(&state, plaintext.data(), &plaintext_len, &tag, buffer.data(),
                                                       bytesRead, nullptr, 0) != 0) {
            std::cerr << "Decryption failed during pull operation" << std::endl;
            std::cerr << "Bytes read: " << bytesRead << std::endl;
            std::cerr << "Buffer size: " << buffer.size() << std::endl;
            std::cerr << "Expected decrypted size: " << plaintext_len << std::endl;
            std::cerr << "Tag: " << static_cast<int>(tag) << std::endl;
            std::cerr << "Buffer data: ";
            return false;
        }

        outFile.write(reinterpret_cast<char *>(plaintext.data()), plaintext_len);

        if (tag == crypto_secretstream_xchacha20poly1305_TAG_FINAL) {
            break;
        }
    }

    inFile.close();
    outFile.close();
    return true;
}
