#include <stdexcept>
#include "AES.h"
#include "GF256.h"
#include "AESGCM128.h"
#include <algorithm>
#include <vector>
#include <stdexcept>
#include "../../../utils/math/RNG.h"

namespace encryption::methods {
    AESGCM128::AESGCM128(const std::array<uint8_t, 16> &key) : key_(key) {
    }

    void AESGCM128::handleErrors(const std::string &errorMsg) {
        throw std::runtime_error(errorMsg);
    }

    void AESGCM128::GCM_GHASH(const uint8_t *aad, const size_t aad_len,
                              const uint8_t *ciphertext, const size_t ciphertext_len,
                              uint8_t *tag) const {
        std::array<uint8_t, BLOCK_SIZE> H = {0}; // Hash subkey
        std::array<uint8_t, BLOCK_SIZE> Y = {0}; // GHASH result
        std::array<uint8_t, BLOCK_SIZE> block{}; // Temporary block

        // Compute H (hash subkey, AES encryption of all-zero block with key)
        AES128EncryptBlock(H.data(), H.data(), key_.data());

        auto gf_mul = [this](const std::array<uint8_t, BLOCK_SIZE> &x,
                             const std::array<uint8_t, BLOCK_SIZE> &y) -> std::array<uint8_t, BLOCK_SIZE> {
            std::array<uint8_t, BLOCK_SIZE> z = {0};
            std::array<uint8_t, BLOCK_SIZE> v = y;
            for (int i = 0; i < BLOCK_SIZE * 8; ++i) {
                if (x[i / 8] & (1 << (7 - (i % 8)))) {
                    for (int j = 0; j < BLOCK_SIZE; ++j) {
                        z[j] ^= v[j];
                    }
                }
                bool lsb = v[BLOCK_SIZE - 1] & 1;
                for (int j = BLOCK_SIZE - 1; j > 0; --j) {
                    v[j] = (v[j] >> 1) | (v[j - 1] << 7);
                }
                v[0] >>= 1;
                if (lsb) {
                    v[0] ^= 0xe1;
                }
            }
            return z;
        };

        auto xor_blocks = [this](const uint8_t *a, const uint8_t *b, uint8_t *out) {
            for (size_t i = 0; i < BLOCK_SIZE; ++i) {
                out[i] = a[i] ^ b[i];
            }
        };

        auto ghash = [&](const uint8_t *data, size_t len) {
            for (size_t i = 0; i < len; i += BLOCK_SIZE) {
                std::copy_n(data + i, BLOCK_SIZE, block.begin());
                xor_blocks(Y.data(), block.data(), Y.data());
                Y = gf_mul(Y, H);
            }
        };

        ghash(aad, aad_len);
        ghash(ciphertext, ciphertext_len);

        std::array<uint8_t, 16> lengths = {0};
        uint64_t aad_bits = aad_len * 8;
        uint64_t ciphertext_bits = ciphertext_len * 8;
        for (int i = 0; i < 8; ++i) {
            lengths[7 - i] = aad_bits & 0xff;
            lengths[15 - i] = ciphertext_bits & 0xff;
            aad_bits >>= 8;
            ciphertext_bits >>= 8;
        }

        xor_blocks(Y.data(), lengths.data(), Y.data());
        Y = gf_mul(Y, H);

        std::ranges::copy(Y, tag);
    }

    void AESGCM128::incrementCounter(uint8_t *counter) {
        for (int i = BLOCK_SIZE - 1; i >= 0; --i) {
            if (++counter[i]) break;
        }
    }

    void AESGCM128::xorBlocks(const uint8_t *a, const uint8_t *b, uint8_t *out) {
        for (size_t i = 0; i < BLOCK_SIZE; ++i) {
            out[i] = a[i] ^ b[i];
        }
    }

    std::vector<uint8_t> AESGCM128::encrypt(const std::vector<uint8_t> &plaintext,
                                            const std::vector<uint8_t> &aad,
                                            std::vector<uint8_t> &tag) const {
        std::vector<uint8_t> iv(IV_SIZE);
        std::vector<uint8_t> ciphertext(plaintext.size());
        std::vector<uint8_t> block(BLOCK_SIZE);
        std::vector<uint8_t> counter(BLOCK_SIZE, 0);
        std::array<uint8_t, 176> roundKeys{}; // 176 bytes for 11 round keys (AES-128)

        // Generate random IV
        utils::math::RNG rng;
        rng.generateRandomBytes(iv);

        // Key expansion
        keyExpansion(key_.data(), roundKeys.data());

        // Initialize counter with IV
        std::ranges::copy(iv, counter.begin());

        // Encrypt plaintext
        for (size_t i = 0; i < plaintext.size(); i += BLOCK_SIZE) {
            AES128EncryptBlock(counter.data(), block.data(), roundKeys.data());
            incrementCounter(counter.data());
            xorBlocks(plaintext.data() + i, block.data(), ciphertext.data() + i);
        }

        // Compute tag
        GCM_GHASH(aad.data(), aad.size(), ciphertext.data(), plaintext.size(), tag.data());

        // Prepend IV to ciphertext
        std::vector<uint8_t> result(iv.begin(), iv.end());
        result.insert(result.end(), ciphertext.begin(), ciphertext.end());

        return result;
    }

    std::vector<uint8_t> AESGCM128::decrypt(const std::vector<uint8_t> &ciphertext,
                                            const std::vector<uint8_t> &aad,
                                            const std::vector<uint8_t> &tag) const {
        if (ciphertext.size() < IV_SIZE) {
            handleErrors("Ciphertext too short");
        }

        std::vector<uint8_t> iv(ciphertext.begin(), ciphertext.begin() + IV_SIZE);
        std::vector<uint8_t> ciphertext_data(ciphertext.begin() + IV_SIZE, ciphertext.end());
        std::vector<uint8_t> plaintext(ciphertext_data.size());
        std::vector<uint8_t> block(BLOCK_SIZE);
        std::vector<uint8_t> counter(BLOCK_SIZE, 0);
        std::array<uint8_t, 176> roundKeys{}; // 176 bytes for 11 round keys (AES-128)

        // Key expansion
        keyExpansion(key_.data(), roundKeys.data());

        // Initialize counter with IV
        std::ranges::copy(iv, counter.begin());

        // Decrypt ciphertext
        for (size_t i = 0; i < ciphertext_data.size(); i += BLOCK_SIZE) {
            AES128EncryptBlock(counter.data(), block.data(), roundKeys.data());
            incrementCounter(counter.data());
            xorBlocks(ciphertext_data.data() + i, block.data(), plaintext.data() + i);
        }

        // Compute and verify tag
        std::vector<uint8_t> computed_tag(TAG_SIZE);
        GCM_GHASH(aad.data(), aad.size(), ciphertext_data.data(), plaintext.size(), computed_tag.data());

        if (!std::equal(computed_tag.begin(), computed_tag.end(), tag.begin())) {
            handleErrors("Tag verification failed");
        }

        return plaintext;
    }
} // namespace encryption::methods
