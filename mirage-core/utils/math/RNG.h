#ifndef RNG_H
#define RNG_H

#include <array>
#include <cstdint>
#include <random>
#include <sodium.h>

namespace utils::math {
    class RNG {
    public:
        RNG();

        template<typename T>
        T random(size_t count = 1);

    private:
        std::array<uint8_t, 32> generateSeed();

        static void mixSeedWithLorenzEntropy(std::array<uint8_t, 32> &seed);

        template<typename T>
        void fillBufferWithRandomBytes(T &buffer, size_t size);

        std::mt19937 rng_;
    };
}

#endif // RNG_H
