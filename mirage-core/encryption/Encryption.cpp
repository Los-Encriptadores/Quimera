#include "Encryption.h"
#include <iostream>
#include <fstream>
#include <sodium.h>
#include <cstdio>

Encryption::Encryption() {
    if (sodium_init() == -1) {
        throw std::runtime_error("Failed to initialize libsodium");
    }
    key = static_cast<unsigned char *>(sodium_malloc(crypto_secretstream_xchacha20poly1305_KEYBYTES));
    if (!key) {
        throw std::bad_alloc();
    }
    sodium_mprotect_noaccess(key); // Initially make the key inaccessible
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
    std::fstream file(inputFile, std::ios::in | std::ios::out | std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Failed to open file for encryption: " << inputFile << std::endl;
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

    std::vector<unsigned char> buffer(1024 * 1024); // 1MB buffer
    std::vector<unsigned char> ciphertext(buffer.size() + crypto_secretstream_xchacha20poly1305_ABYTES);
    unsigned long long ciphertext_len;

    file.seekp(0, std::ios::end);
    std::streamoff fileSize = file.tellp();
    file.seekp(0, std::ios::beg);

    // Write header at the beginning of the file
    file.write(reinterpret_cast<char *>(header), sizeof(header));

    while (file.good() && file.tellg() < fileSize) {
        file.read(reinterpret_cast<char *>(buffer.data()), buffer.size());
        const std::streamsize readSize = file.gcount();

        if (const unsigned char tag = file.eof() ? crypto_secretstream_xchacha20poly1305_TAG_FINAL : 0;
            crypto_secretstream_xchacha20poly1305_push(&state, ciphertext.data(), &ciphertext_len,
                                                       buffer.data(), readSize, nullptr, 0, tag) != 0) {
            std::cerr << "Failed to encrypt chunk" << std::endl;
            return false;
        }

        // Move back the read position by the size of the ciphertext and write the ciphertext
        file.seekp(static_cast<std::streamoff>(file.tellg()) - readSize);
        file.write(reinterpret_cast<char *>(ciphertext.data()), ciphertext_len);
    }

    file.close();

    // Rename the file to indicate it has been encrypted
    std::string encryptedFile = inputFile + ".lol";
    std::rename(inputFile.c_str(), encryptedFile.c_str());

    std::cerr << "File encrypted and renamed to: " << encryptedFile << std::endl;
    return true;
}

bool Encryption::decryptFile(const std::string &inputFile) const {
    std::fstream file(inputFile, std::ios::in | std::ios::out | std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Failed to open file for decryption: " << inputFile << std::endl;
        return false;
    }

    unsigned char header[crypto_secretstream_xchacha20poly1305_HEADERBYTES];
    crypto_secretstream_xchacha20poly1305_state state;

    // Read the header from the beginning of the file
    file.read(reinterpret_cast<char *>(header), sizeof(header));

    sodium_mprotect_readonly(key);
    if (crypto_secretstream_xchacha20poly1305_init_pull(&state, header, key) != 0) {
        sodium_mprotect_noaccess(key);
        std::cerr << "Failed to initialize decryption stream" << std::endl;
        return false;
    }
    sodium_mprotect_noaccess(key);

    std::vector<unsigned char> buffer(1024 * 1024 + crypto_secretstream_xchacha20poly1305_ABYTES); // 1MB buffer
    std::vector<unsigned char> decrypted(buffer.size());
    unsigned long long decrypted_len;
    unsigned char tag;

    file.seekp(sizeof(header), std::ios::beg);

    while (file.good()) {
        file.read(reinterpret_cast<char *>(buffer.data()), buffer.size());
        const std::streamsize readSize = file.gcount();

        if (crypto_secretstream_xchacha20poly1305_pull(&state, decrypted.data(), &decrypted_len, &tag,
                                                       buffer.data(), readSize, nullptr, 0) != 0) {
            std::cerr << "Failed to decrypt chunk" << std::endl;
            return false;
        }

        // Move back the read position by the size of the decrypted text and write the decrypted text
        file.seekp(static_cast<std::streamoff>(file.tellg()) - readSize);
        file.write(reinterpret_cast<char *>(decrypted.data()), decrypted_len);

        if (tag == crypto_secretstream_xchacha20poly1305_TAG_FINAL) {
            break;
        }
    }

    file.close();

    // Rename the file to indicate it has been decrypted
    std::string decryptedFile = inputFile.substr(0, inputFile.size() - 4); // Remove ".lol" extension
    std::rename(inputFile.c_str(), decryptedFile.c_str());

    std::cerr << "File decrypted and renamed to: " << decryptedFile << std::endl;
    return true;
}
