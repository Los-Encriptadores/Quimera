#ifndef CRYPTOSTATEHANDLER_H
#define CRYPTOSTATEHANDLER_H

#include <sodium.h>
#include <fstream>

namespace utils::crypto {
 /**
  * @class CryptoStateHandler
  * @brief This class manages the cryptographic state for encryption and decryption.
  *
  * The CryptoStateHandler class encapsulates the initialization and handling of the cryptographic state
  * used for encryption and decryption operations. It ensures the state is correctly initialized and
  * provides a clean interface for managing the state.
  */
 class CryptoStateHandler {
 public:
  /**
   * @brief Constructs a new CryptoStateHandler for encryption.
   *
   * Initializes the cryptographic state for encryption and writes the header to the output file.
   *
   * @param key The encryption key.
   * @param outputFile The output file stream to write the header to.
   */
  CryptoStateHandler(const unsigned char *key, std::ofstream &outputFile);

  /**
   * @brief Constructs a new CryptoStateHandler for decryption.
   *
   * Initializes the cryptographic state for decryption by reading the header from the input file.
   *
   * @param key The encryption key.
   * @param inputFile The input file stream to read the header from.
   */
  CryptoStateHandler(const unsigned char *key, std::ifstream &inputFile);

  /**
   * @brief Destroys the CryptoStateHandler object.
   *
   * Securely erases and deallocates the state and header.
   */
  ~CryptoStateHandler();

  /**
   * @brief Gets the cryptographic state.
   *
   * @return The current cryptographic state.
   */
  [[nodiscard]] crypto_secretstream_xchacha20poly1305_state &getState();

  /**
   * @brief Gets the header.
   *
   * @return The current header.
   */
  [[nodiscard]] unsigned char *getHeader();

  /**
   * @brief Sets the cryptographic state.
   *
   * @param newState The new cryptographic state.
   */
  void setState(const crypto_secretstream_xchacha20poly1305_state &newState);

  /**
   * @brief Sets the header.
   *
   * @param newHeader The new header.
   */
  void setHeader(const unsigned char *newHeader);

 private:
  crypto_secretstream_xchacha20poly1305_state state{}; /**< The cryptographic state used for encryption/decryption. */
  unsigned char header[crypto_secretstream_xchacha20poly1305_HEADERBYTES]{};
  /**< The header for the cryptographic stream. */
 };
} // namespace utils::crypto

#endif // CRYPTOSTATEHANDLER_H
