#ifndef IPOLYMORPHICENCRYPTIONENGINE_H
#define IPOLYMORPHICENCRYPTIONENGINE_H

#include <cstddef>

/**
 * @class IPolymorphicEncryptionEngine
 * @brief This class provides an interface for polymorphic encryption engines.
 *
 * The IPolymorphicEncryptionEngine class provides an interface for polymorphic encryption engines. It provides methods to encrypt
 * and decrypt data.
 */
class IPolymorphicEncryptionEngine {
public:
    virtual ~IPolymorphicEncryptionEngine() = default;

    virtual void encrypt(unsigned char *data, size_t data_size) = 0;

    virtual void decrypt(unsigned char *data, size_t data_size) = 0;
};

#endif // IPOLYMORPHICENCRYPTIONENGINE_H
