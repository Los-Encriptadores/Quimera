#ifndef ENCRYPTION_H
#define ENCRYPTION_H

#include <string>

/**
 * @class Encryption
 * @brief This class provides methods for encryption and decryption of files using libsodium.
 *
 * The Encryption class uses xChaCha20-Poly1305 for encryption and decryption. It provides methods to generate a key,
 * encrypt a file, and decrypt a file. The key is then hashed using BLAKE2b and stored in a protected memory space
 * and is inaccessible outside the class.
 */
class Encryption {
public:
 Encryption();

 ~Encryption();

 void generateAndHashKey() const;

 [[nodiscard]] bool encryptFile(const std::string &inputFile) const;

 [[nodiscard]] bool decryptFile(const std::string &inputFile) const;

private:
 unsigned char *key;
};

#endif // ENCRYPTION_H
