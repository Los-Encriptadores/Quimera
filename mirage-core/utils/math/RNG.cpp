#include "RNG.h"
#include "LorenzAttractor.h"
#include <algorithm>
#include <random>
#include <sodium.h>
#include <stdexcept>
#include <vector>

namespace utils::math {
    RNG::RNG() {
        if (sodium_init() < 0) {
            throw std::runtime_error("libsodium initialization failed");
        }

        std::array<uint32_t, std::mt19937::state_size> seed_data{};
        randombytes_buf(seed_data.data(), seed_data.size() * sizeof(uint32_t));

        std::seed_seq seq(seed_data.begin(), seed_data.end());
        rng_.seed(seq);
    }

    std::array<uint8_t, 32> RNG::generateSeed() {
        std::array<uint8_t, 32> seed{};

        // Get OS entropy
        uint8_t osSeed[32];
        randombytes_buf(osSeed, 32);

        // Get Lorenz attractor entropy
        std::array<uint8_t, 32> lorenzEntropy{};
        LorenzAttractor::generateEntropy(lorenzEntropy, 32);

        // Mix OS entropy with Lorenz attractor entropy
        std::transform(osSeed, osSeed + 32, lorenzEntropy.begin(), seed.begin(), std::bit_xor());

        return seed;
    }

    template<typename T>
    void RNG::fillBufferWithRandomBytes(T &buffer, const size_t size) {
        std::uniform_int_distribution<int> dist(0, 255);
        for (size_t i = 0; i < size; ++i) {
            buffer[i] = static_cast<typename T::value_type>(dist(rng_));
        }
    }

    template<typename T>
    T RNG::random(const size_t count) {
        static_assert(std::is_integral_v<T>, "T must be an integral type");

        std::vector<uint8_t> buffer(sizeof(T) * count);

        std::array<uint8_t, 32> seed = generateSeed();

        std::seed_seq seq(seed.begin(), seed.end());
        rng_.seed(seq);

        fillBufferWithRandomBytes(buffer, sizeof(T) * count);

        T result = 0;
        for (size_t i = 0; i < sizeof(T); ++i) {
            result = result << 8 | buffer[i];
        }

        return result;
    }

    template uint8_t RNG::random<uint8_t>(size_t count);

    template uint16_t RNG::random<uint16_t>(size_t count);

    template uint32_t RNG::random<uint32_t>(size_t count);

    template uint64_t RNG::random<uint64_t>(size_t count);
} // namespace utils::math
