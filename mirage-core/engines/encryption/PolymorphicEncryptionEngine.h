#ifndef POLYMORPHICENCRYPTIONENGINE_H
#define POLYMORPHICENCRYPTIONENGINE_H

#include <string>
#include <sodium/crypto_secretstream_xchacha20poly1305.h>

#define PARANOID_MODE true
#define POLYMORPHIC_KEY_SIZE 16
#define DEFAULT_CHUNK_SIZE 4096
#define PADDING_BLOCK_SIZE 16
#define MIN_REKEY_INTERVAL 100
#define MAX_REKEY_INTERVAL 1000

namespace engines::encryption {
 /**
  * @class PolymorphicEncryptionEngine
  * @brief This class provides methods for encryption and decryption of files using a polymorphic encryption.
  *
  * The PolymorphicEncryptionEngine class uses an encryption module combined with XOR operations
  * to add a layer of polymorphism on top of the encryption. This ensures enhanced security by applying
  * an additional XOR-based transformation to the encrypted data.
  */
 class PolymorphicEncryptionEngine final {
 public:
  /**
   * @brief Constructs a new PolymorphicEncryptionEngine object.
   *
   * Initializes the PolymorphicEncryptionEngine, generates the encryption key, and the XOR key.
   */
  explicit PolymorphicEncryptionEngine(size_t chunkSize = DEFAULT_CHUNK_SIZE);

  /**
   * @brief Destroys the PolymorphicEncryptionEngine object.
   *
   * Cleans up and securely erases the encryption key and XOR key.
   */
  ~PolymorphicEncryptionEngine();

  /**
   * @brief Encrypts a file.
   *
   * This method reads the input file, encrypts its contents, applies an XOR operation, and writes the
   * encrypted data to the output file.
   *
   * @param inputFilename The path to the input file.
   * @param outputFilename The path to the output file.
   */
  void encryptFile(const std::string &inputFilename, const std::string &outputFilename) const;

  /**
   * @brief Decrypts a file.
   *
   * This method reads the input file, applies an XOR operation to its contents, decrypts the data, and writes the
   * decrypted data to the output file.
   *
   * @param inputFilename The path to the input file.
   * @param outputFilename The path to the output file.
   */
  void decryptFile(const std::string &inputFilename, const std::string &outputFilename) const;

 private:
  unsigned char xor_key[POLYMORPHIC_KEY_SIZE]{}; /**< XOR key used for additional polymorphic encryption. */
  unsigned char *key{}; /**< Encryption key used for the primary encryption method. */
  size_t chunkSize; /**< Size of the chunks used for encryption and decryption. */

  /**
   * @brief Generates the XOR key.
   *
   * Generates a random XOR key using the custom RNG.
   */
  void generateXorKey();

  /**
   * @brief Generates the encryption key.
   *
   * Allocates and initializes the encryption key using the custom RNG.
   */
  void generateEncryptionKey();

  /**
   * @brief Applies an XOR operation to a buffer.
   *
   * This method applies an XOR operation to the given buffer using the XOR key.
   *
   * @param buffer The buffer to be XORed.
   * @param length The length of the buffer.
   */
  void xorBuffer(unsigned char *buffer, size_t length) const;

  /**
   * @brief Rekeys the encryption state.
   *
   * Updates the encryption state with a new key.
   *
   * @param state The current encryption state.
   */
  void rekey(crypto_secretstream_xchacha20poly1305_state &state) const;
 };
} // namespace engines::encryption

#endif // POLYMORPHICENCRYPTIONENGINE_H
