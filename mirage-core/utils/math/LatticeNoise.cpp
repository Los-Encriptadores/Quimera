#include "LatticeNoise.h"
#include <random>

namespace utils::math {
    void LatticeNoise::addLatticeNoise(std::vector<unsigned char> &data) {
        std::default_random_engine generator(std::random_device{}());
        std::uniform_int_distribution distribution(0, MODULUS - 1);

        for (auto &byte: data) {
            const int noise = distribution(generator);
            byte = static_cast<unsigned char>((static_cast<int>(byte) + noise) % MODULUS);
        }
    }
}
