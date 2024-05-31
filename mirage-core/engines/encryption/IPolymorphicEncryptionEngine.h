#ifndef IPOLYMORPHICENCRYPTIONENGINE_H
#define IPOLYMORPHICENCRYPTIONENGINE_H

#include <cstddef>

namespace engines::encryption {
 /**
 * @class IPolymorphicEncryptionEngine
 * @brief This class provides an interface for polymorphic encryption engines.
 *
 * The IPolymorphicEncryptionEngine class defines an interface for polymorphic encryption engines.
 * It declares methods for encrypting and decrypting data, ensuring that any derived class
 * implements these essential functionalities.
 */
 class IPolymorphicEncryptionEngine {
 public:
  /**
   * @brief Virtual destructor for the interface.
   *
   * Ensures derived classes can clean up resources properly when an object is destroyed.
   */
  virtual ~IPolymorphicEncryptionEngine() = default;

  /**
   * @brief Encrypts the provided data.
   *
   * This method encrypts the data pointed to by the provided pointer. The size of the data
   * to be encrypted is specified by the data_size parameter.
   *
   * @param data Pointer to the data to be encrypted.
   * @param data_size Size of the data to be encrypted.
   */
  virtual void encrypt(unsigned char *data, size_t data_size) = 0;

  /**
   * @brief Decrypts the provided data.
   *
   * This method decrypts the data pointed to by the provided pointer. The size of the data
   * to be decrypted is specified by the data_size parameter.
   *
   * @param data Pointer to the data to be decrypted.
   * @param data_size Size of the data to be decrypted.
   */
  virtual void decrypt(unsigned char *data, size_t data_size) = 0;
 };
}

#endif // IPOLYMORPHICENCRYPTIONENGINE_H
