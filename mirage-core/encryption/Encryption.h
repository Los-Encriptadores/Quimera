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
 /**
  * @brief Constructor for the Encryption class.
  *
  * Initializes libsodium and allocates memory for the encryption key. The key is protected from being accessed.
  */
 Encryption();

 /**
  * @brief Destructor for the Encryption class.
  *
  * Frees the memory allocated for the encryption key after zeroing it out.
  */
 ~Encryption();

 /**
  * @brief Generates a new encryption key.
  *
  * The key is generated using libsodium's crypto_secretstream_xchacha20poly1305_keygen function.
  */
 void generateKey() const;

 /**
  * @brief Encrypts a file.
  *
  * @param inputFile The name of the file to be encrypted.
  * @return true if the file was successfully encrypted, false otherwise.
  */
 [[nodiscard]] bool encryptFile(const std::string &inputFile) const;

 /**
  * @brief Decrypts a file.
  *
  * @param inputFile The name of the file to be decrypted.
  * @return true if the file was successfully decrypted, false otherwise.
  */
 [[nodiscard]] bool decryptFile(const std::string &inputFile) const;

private:
 /**
  * @brief The encryption key.
  *
  * This key is used for encryption and decryption. It is stored in a protected memory space and is
  * inaccessible outside the class.
  */
 unsigned char *key;
};

#endif // ENCRYPTION_H
