#ifndef ENCRYPTION_H
#define ENCRYPTION_H

#include <sodium.h>
#include <cstddef>

/**
 * @class Encryption
 * @brief This class provides methods for encryption and decryption of data using libsodium.
 *
 * The Encryption class uses xChaCha20-Poly1305 for encryption and decryption. It provides methods to generate a key,
 * encrypt a buffer, and decrypt a buffer.
 */
class Encryption {
public:
 explicit Encryption(const unsigned char *key);

 ~Encryption();

 void encrypt(unsigned char *data, size_t size) const;

 void decrypt(unsigned char *data, size_t size) const;

private:
 unsigned char key[crypto_secretstream_xchacha20poly1305_KEYBYTES]{};
};

#endif // ENCRYPTION_H
