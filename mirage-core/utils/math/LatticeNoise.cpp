#include "LatticeNoise.h"
#include <random>

namespace utils::math {
    void LatticeNoise::addLatticeNoise(std::vector<unsigned char> &data, size_t length) {
        std::default_random_engine generator(std::random_device{}());
        std::uniform_int_distribution distribution(0, MODULUS - 1);

        for (size_t i = 0; i < length; ++i) {
            const int noise = distribution(generator);
            data[i] = static_cast<unsigned char>((static_cast<int>(data[i]) + noise) % MODULUS);
        }
    }

    void LatticeNoise::removeLatticeNoise(std::vector<unsigned char> &data, size_t length) {
        std::default_random_engine generator(std::random_device{}());
        std::uniform_int_distribution distribution(0, MODULUS - 1);

        for (size_t i = 0; i < length; ++i) {
            const int noise = distribution(generator);
            data[i] = static_cast<unsigned char>((static_cast<int>(data[i]) - noise + MODULUS) % MODULUS);
        }
    }
}
