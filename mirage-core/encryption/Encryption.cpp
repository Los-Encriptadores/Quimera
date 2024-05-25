#include "Encryption.h"
#include <iostream>
#include <fstream>
#include <sodium.h>
#include <vector>

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

Encryption::~Encryption() {
    sodium_mprotect_readwrite(key);
    sodium_memzero(key, crypto_secretstream_xchacha20poly1305_KEYBYTES);
    sodium_free(key);
}

void Encryption::generateKey() const {
    sodium_mprotect_readwrite(key);
    crypto_secretstream_xchacha20poly1305_keygen(key);
    sodium_mprotect_noaccess(key);
}

bool Encryption::encryptFile(const std::string &inputFile) const {
    std::ifstream inFile(inputFile, std::ios::binary);
    if (!inFile.is_open()) {
        std::cerr << "Failed to open file for encryption: " << inputFile << std::endl;
        return false;
    }

    std::ofstream outFile(inputFile + ".lol", std::ios::binary);
    if (!outFile.is_open()) {
        std::cerr << "Failed to open file for writing: " << inputFile + ".lol" << std::endl;
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

    std::vector<unsigned char> buffer(1024 * 1024);
    std::vector<unsigned char> ciphertext(buffer.size() + crypto_secretstream_xchacha20poly1305_ABYTES);
    while (inFile) {
        inFile.read(reinterpret_cast<char *>(buffer.data()), buffer.size());
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

    inFile.close();
    outFile.close();
    return true;
}

bool Encryption::decryptFile(const std::string &inputFile) const {
    std::ifstream inFile(inputFile, std::ios::binary);
    if (!inFile.is_open()) {
        std::cerr << "Failed to open file for decryption: " << inputFile << std::endl;
        return false;
    }

    std::ofstream outFile(inputFile.substr(0, inputFile.size() - 4), std::ios::binary); // Remove ".lol" extension
    if (!outFile.is_open()) {
        std::cerr << "Failed to open file for writing: " << inputFile.substr(0, inputFile.size() - 4) << std::endl;
        return false;
    }

    unsigned char header[crypto_secretstream_xchacha20poly1305_HEADERBYTES];
    crypto_secretstream_xchacha20poly1305_state state;

    inFile.read(reinterpret_cast<char *>(header), sizeof(header));
    if (inFile.gcount() != sizeof(header)) {
        std::cerr << "Failed to read header" << std::endl;
        return false;
    }

    sodium_mprotect_readonly(key);
    if (crypto_secretstream_xchacha20poly1305_init_pull(&state, header, key) != 0) {
        sodium_mprotect_noaccess(key);
        std::cerr << "Failed to initialize decryption stream" << std::endl;
        return false;
    }
    sodium_mprotect_noaccess(key);

    std::vector<unsigned char> buffer(1024 * 1024 + crypto_secretstream_xchacha20poly1305_ABYTES);
    std::vector<unsigned char> decrypted(buffer.size());
    while (inFile) {
        inFile.read(reinterpret_cast<char *>(buffer.data()), buffer.size());
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

    inFile.close();
    outFile.close();
    return true;
}
