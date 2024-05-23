#include "Encryption.h"
#include "methods/AES/AESGCM128.h"

namespace encryption {
    class AESGCM128EncryptionMethod final : public EncryptionMethod {
    public:
        explicit AESGCM128EncryptionMethod(const std::array<uint8_t, 16> &key)
            : aesgcm128_(key) {
        }

        std::vector<uint8_t> encrypt(const std::vector<uint8_t> &plaintext,
                                     const std::vector<uint8_t> &aad,
                                     std::vector<uint8_t> &tag) override {
            return aesgcm128_.encrypt(plaintext, aad, tag);
        }

        std::vector<uint8_t> decrypt(const std::vector<uint8_t> &ciphertext,
                                     const std::vector<uint8_t> &aad,
                                     const std::vector<uint8_t> &tag) override {
            return aesgcm128_.decrypt(ciphertext, aad, tag);
        }

    private:
        methods::AESGCM128 aesgcm128_;
    };

    Encryption::Encryption(Method method, const std::array<uint8_t, 16> &key) {
        switch (method) {
            case Method::AESGCM128:
                method_ = std::make_unique<AESGCM128EncryptionMethod>(key);
                break;
            // Add other methods here in the future
        }
    }

    std::vector<uint8_t> Encryption::encrypt(const std::vector<uint8_t> &plaintext,
                                             const std::vector<uint8_t> &aad,
                                             std::vector<uint8_t> &tag) const {
        return method_->encrypt(plaintext, aad, tag);
    }

    std::vector<uint8_t> Encryption::decrypt(const std::vector<uint8_t> &ciphertext,
                                             const std::vector<uint8_t> &aad,
                                             const std::vector<uint8_t> &tag) const {
        return method_->decrypt(ciphertext, aad, tag);
    }
}
