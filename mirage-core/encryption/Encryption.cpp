#include "Encryption.h"
#include <stdexcept>
#include <cstring>
#include <vector>
#include <iostream>

Encryption::Encryption(const unsigned char *key) {
    std::cout << "Initializing Encryption with key" << std::endl;
    if (sodium_init() == -1) {
        throw std::runtime_error("Failed to initialize libsodium");
    }
    std::memcpy(this->key, key, crypto_secretstream_xchacha20poly1305_KEYBYTES);
    std::cout << "Encryption initialized" << std::endl;
}

Encryption::~Encryption() {
    std::cout << "Destroying Encryption" << std::endl;
}

void Encryption::encrypt(unsigned char *data, size_t size) const {
    unsigned char header[crypto_secretstream_xchacha20poly1305_HEADERBYTES];
    crypto_secretstream_xchacha20poly1305_state state;

    if (crypto_secretstream_xchacha20poly1305_init_push(&state, header, key) != 0) {
        throw std::runtime_error("Failed to initialize encryption stream");
    }

    std::vector<unsigned char> ciphertext(size + crypto_secretstream_xchacha20poly1305_ABYTES);
    unsigned long long ciphertext_len;

    if (crypto_secretstream_xchacha20poly1305_push(&state, ciphertext.data(), &ciphertext_len, data, size, nullptr, 0,
                                                   crypto_secretstream_xchacha20poly1305_TAG_FINAL) != 0) {
        throw std::runtime_error("Encryption failed");
    }

    std::memmove(data + crypto_secretstream_xchacha20poly1305_HEADERBYTES, ciphertext.data(), ciphertext_len);
    std::memcpy(data, header, crypto_secretstream_xchacha20poly1305_HEADERBYTES);

    std::cout << "Data encrypted, ciphertext length: " << ciphertext_len << std::endl;
}

void Encryption::decrypt(unsigned char *data, size_t size) const {
    unsigned char header[crypto_secretstream_xchacha20poly1305_HEADERBYTES];
    crypto_secretstream_xchacha20poly1305_state state;
    std::memcpy(header, data, crypto_secretstream_xchacha20poly1305_HEADERBYTES);

    if (crypto_secretstream_xchacha20poly1305_init_pull(&state, header, key) != 0) {
        throw std::runtime_error("Failed to initialize decryption stream");
    }

    const size_t ciphertext_len = size - crypto_secretstream_xchacha20poly1305_HEADERBYTES;
    std::vector<unsigned char> decrypted(ciphertext_len);
    unsigned long long decrypted_len;
    unsigned char tag;

    if (crypto_secretstream_xchacha20poly1305_pull(&state, decrypted.data(), &decrypted_len, &tag,
                                                   data + crypto_secretstream_xchacha20poly1305_HEADERBYTES,
                                                   ciphertext_len, nullptr, 0) != 0) {
        throw std::runtime_error("Decryption failed");
    }

    if (tag != crypto_secretstream_xchacha20poly1305_TAG_FINAL) {
        throw std::runtime_error("Incomplete decryption");
    }

    std::memcpy(data, decrypted.data(), decrypted_len);
    std::cout << "Data decrypted, decrypted length: " << decrypted_len << std::endl;
}
