#include "RNG.h"
#include "LorenzAttractor.h"

#include <algorithm>
#include <functional>
#include <random>
#include <sodium.h>
#include <stdexcept>

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

        auto *osSeed = static_cast<uint8_t *>(sodium_malloc(32));
        if (!osSeed) {
            throw std::bad_alloc();
        }
        sodium_mprotect_readwrite(osSeed);

        randombytes_buf(osSeed, 32);

        auto *entropySeed = static_cast<uint8_t *>(sodium_malloc(32));
        if (!entropySeed) {
            sodium_free(osSeed);
            throw std::bad_alloc();
        }
        sodium_mprotect_readwrite(entropySeed);

        std::generate_n(entropySeed, 32, std::ref(rng_));

        std::transform(osSeed, osSeed + 32, entropySeed, seed.begin(), std::bit_xor());

        sodium_memzero(osSeed, 32);
        sodium_free(osSeed);

        sodium_memzero(entropySeed, 32);
        sodium_free(entropySeed);

        return seed;
    }

    template<typename T>
    void RNG::fillBufferWithRandomBytes(T &buffer, const size_t size) {
        std::uniform_int_distribution dist(0, 255);
        for (size_t i = 0; i < size; ++i) {
            buffer[i] = dist(rng_);
        }
    }

    template<typename T>
    T RNG::random(const size_t count) {
        static_assert(std::is_integral_v<T>, "T must be an integral type");

        auto *buffer = static_cast<uint8_t *>(sodium_malloc(sizeof(T) * count));
        if (!buffer) {
            throw std::bad_alloc();
        }
        sodium_mprotect_readwrite(buffer);

        std::array<uint8_t, 32> seed = generateSeed();
        LorenzAttractor::mixSeedWithLorenzEntropy(seed);
        std::ranges::shuffle(seed, rng_);
        fillBufferWithRandomBytes(buffer, sizeof(T) * count);

        T result = 0;
        for (size_t i = 0; i < sizeof(T); ++i) {
            result = (result << 8) | buffer[i];
        }

        sodium_memzero(buffer, sizeof(T) * count);
        sodium_free(buffer);

        return result;
    }

    template uint8_t RNG::random<uint8_t>(size_t count);

    template uint16_t RNG::random<uint16_t>(size_t count);

    template uint32_t RNG::random<uint32_t>(size_t count);

    template uint64_t RNG::random<uint64_t>(size_t count);
}
