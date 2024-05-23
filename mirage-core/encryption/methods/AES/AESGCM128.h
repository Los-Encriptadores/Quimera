#ifndef AESGCM128_H
#define AESGCM128_H

#include <vector>
#include <array>
#include <cstdint>

namespace encryption::methods {
    class AESGCM128 {
    public:
        explicit AESGCM128(const std::array<uint8_t, 16> &key);

        std::vector<uint8_t> encrypt(const std::vector<uint8_t> &plaintext,
                                     const std::vector<uint8_t> &aad,
                                     std::vector<uint8_t> &tag) const;

        std::vector<uint8_t> decrypt(const std::vector<uint8_t> &ciphertext,
                                     const std::vector<uint8_t> &aad,
                                     const std::vector<uint8_t> &tag) const;

    private:
        std::array<uint8_t, 16> key_;
        static constexpr std::size_t BLOCK_SIZE = 16;
        static constexpr std::size_t TAG_SIZE = 16;
        static constexpr std::size_t IV_SIZE = 12;

        void GCM_GHASH(const uint8_t *aad, std::size_t aad_len,
                       const uint8_t *ciphertext, std::size_t ciphertext_len,
                       uint8_t *tag) const;

        static void incrementCounter(uint8_t *counter);

        static void xorBlocks(const uint8_t *a, const uint8_t *b, uint8_t *out);

        static void handleErrors(const std::string &errorMsg);
    };
} // namespace encryption::methods

#endif //AESGCM128_H
