#ifndef ENCRYPTION_H
#define ENCRYPTION_H

#include <vector>
#include <array>
#include <cstdint>
#include <memory>

namespace encryption {
    class EncryptionMethod {
    public:
        virtual ~EncryptionMethod() = default;

        virtual std::vector<uint8_t> encrypt(const std::vector<uint8_t> &plaintext,
                                             const std::vector<uint8_t> &aad,
                                             std::vector<uint8_t> &tag) = 0;

        virtual std::vector<uint8_t> decrypt(const std::vector<uint8_t> &ciphertext,
                                             const std::vector<uint8_t> &aad,
                                             const std::vector<uint8_t> &tag) = 0;
    };

    class Encryption {
    public:
        enum class Method {
            AESGCM128
        };

        explicit Encryption(Method method, const std::array<uint8_t, 16> &key);

        std::vector<uint8_t> encrypt(const std::vector<uint8_t> &plaintext,
                                     const std::vector<uint8_t> &aad,
                                     std::vector<uint8_t> &tag) const;

        std::vector<uint8_t> decrypt(const std::vector<uint8_t> &ciphertext,
                                     const std::vector<uint8_t> &aad,
                                     const std::vector<uint8_t> &tag) const;

    private:
        std::unique_ptr<EncryptionMethod> method_;
    };
}

#endif // ENCRYPTION_H
