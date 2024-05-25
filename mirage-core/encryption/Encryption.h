#ifndef ENCRYPTION_H
#define ENCRYPTION_H

#include <string>

/**
 * @class Encryption
 * @brief This class provides methods for encryption and decryption of files.
 *
 * The Encryption class uses libsodium for encryption and decryption. It provides methods to generate a key,
 * encrypt a file, and decrypt a file. The key is stored in a protected memory space and is inaccessible
 * outside the class.
 */
class Encryption {
public:
 Encryption();

 ~Encryption();

 void generateKey() const;

 [[nodiscard]] bool encryptFile(const std::string &inputFile) const;

 [[nodiscard]] bool decryptFile(const std::string &inputFile) const;

 /**
  * @brief Hashes a message using libsodium's generic hashing function.
  *
  * @param message The message to hash.
  * @return A string representing the hexadecimal hash of the message.
  */
 static std::string hashMessage(const std::string &message);

private:
 unsigned char *key;
};

#endif // ENCRYPTION_H
