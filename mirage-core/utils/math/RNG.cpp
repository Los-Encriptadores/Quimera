#include "RNG.h"
#include <random>
#include <algorithm>
#include <functional>
#include <stdexcept>
#include <cmath>
#include <sodium.h>

constexpr uint8_t LORENZ_ENTROPY_STEPS = 100;
constexpr bool USE_ENHANCED_LORENZ_INITIAL_VALUES = true; // TODO: for now; we'll read it from config later.

namespace utils::math {
    RNG::RNG() {
        if (sodium_init() < 0) {
            throw std::runtime_error("libsodium initialization failed");
        }

        std::random_device rd;
        std::array<uint32_t, std::mt19937::state_size> seed_data{};
        std::generate_n(seed_data.data(), seed_data.size(), std::ref(rd));
        std::seed_seq seq(seed_data.begin(), seed_data.end());
        rng_.seed(seq);
    }

    std::array<uint8_t, 32> RNG::generateSeed() {
        std::array<uint8_t, 32> seed{};

        // Allocate memory for osSeed and entropySeed using sodium_malloc
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

        std::transform(osSeed, osSeed + 32, entropySeed, seed.begin(), std::bit_xor<>());

        sodium_memzero(osSeed, 32);
        sodium_free(osSeed);

        sodium_memzero(entropySeed, 32);
        sodium_free(entropySeed);

        return seed;
    }

    void RNG::mixSeedWithLorenzEntropy(std::array<uint8_t, 32> &seed) {
        double x = USE_ENHANCED_LORENZ_INITIAL_VALUES ? 1.01 : 1.0;
        double y = USE_ENHANCED_LORENZ_INITIAL_VALUES ? 1.02 : 1.0;
        double z = USE_ENHANCED_LORENZ_INITIAL_VALUES ? 1.03 : 1.0;
        constexpr double beta = 8.0 / 3.0;

        std::vector<uint8_t> lorenzEntropy;
        lorenzEntropy.reserve(LORENZ_ENTROPY_STEPS * 3);

        for (size_t i = 0; i < LORENZ_ENTROPY_STEPS; ++i) {
            constexpr double dt = 0.01;
            constexpr double rho = 28.0;
            constexpr double sigma = 10.0;
            const double dx = sigma * (y - x) * dt;
            const double dy = (x * (rho - z) - y) * dt;
            const double dz = (x * y - beta * z) * dt;

            x += dx;
            y += dy;
            z += dz;

            lorenzEntropy.push_back(static_cast<uint8_t>(std::fmod(std::abs(x), 256.0)));
            lorenzEntropy.push_back(static_cast<uint8_t>(std::fmod(std::abs(y), 256.0)));
            lorenzEntropy.push_back(static_cast<uint8_t>(std::fmod(std::abs(z), 256.0)));
        }

        for (size_t i = 0; i < lorenzEntropy.size(); ++i) {
            seed[i % 32] ^= lorenzEntropy[i];
        }
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

        // Allocate memory for buffer using sodium_malloc
        auto *buffer = static_cast<uint8_t *>(sodium_malloc(sizeof(T) * count));
        if (!buffer) {
            throw std::bad_alloc();
        }
        sodium_mprotect_readwrite(buffer);

        std::array<uint8_t, 32> seed = generateSeed();
        mixSeedWithLorenzEntropy(seed);
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

    // Explicit instantiation for uint8_t and other types as needed
    template uint8_t RNG::random<uint8_t>(size_t count);

    template uint16_t RNG::random<uint16_t>(size_t count);

    template uint32_t RNG::random<uint32_t>(size_t count);

    template uint64_t RNG::random<uint64_t>(size_t count);
}
