#ifndef POLYMORPHICENCRYPTIONENGINE_H
#define POLYMORPHICENCRYPTIONENGINE_H

#include <string>

#define POLYMORPHIC_KEY_SIZE 16
#define CHUNK_SIZE 4096

/**
 * @class PolymorphicEncryptionEngine
 * @brief This class provides methods for encryption and decryption of data using a polymorphic engine.
 *
 * The PolymorphicEncryptionEngine class uses a polymorphic engine to encrypt and decrypt data. The engine generates a random key
 * and uses XOR encryption to encrypt and decrypt data.
 */
class PolymorphicEncryptionEngine final {
public:
    PolymorphicEncryptionEngine();

    ~PolymorphicEncryptionEngine();

    void encryptFile(const std::string &inputFilename, const std::string &outputFilename) const;

    void decryptFile(const std::string &inputFilename, const std::string &outputFilename) const;

private:
    unsigned char xor_key[POLYMORPHIC_KEY_SIZE]{};
    unsigned char *key{};

    void generateXorKey();

    void generateKey();
};

#endif // POLYMORPHICENCRYPTIONENGINE_H
