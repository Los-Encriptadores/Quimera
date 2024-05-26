#include "RNG.h"
#include <random>
#include <algorithm>
#include <functional>

constexpr u_int8_t STEPS = 100;

namespace utils::math {
    RNG::RNG() {
        std::random_device rd;
        std::array<uint32_t, std::mt19937::state_size> seed_data{};
        std::generate_n(seed_data.data(), seed_data.size(), std::ref(rd));
        std::seed_seq seq(seed_data.begin(), seed_data.end());
        rng_.seed(seq);
    }

    void RNG::generateRandomBytes(std::array<uint8_t, 12> &buffer) {
        std::array<uint8_t, 32> seed = generateCombinedSeed();
        constexpr u_int8_t STEPS = 100;
        const std::vector<uint8_t> lorenzEntropy = generateRandomizedLorenzEntropy(STEPS, {rng_(), rng_(), rng_()});
        xorSeedWithLorenzEntropy(seed, lorenzEntropy);
        shuffleSeed(seed);

        std::uniform_int_distribution dist(0, 255);
        for (auto &byte: buffer) {
            byte = dist(rng_);
        }
    }

    void RNG::generateRandomBytes(std::vector<uint8_t> &buffer) {
        std::array<uint8_t, 32> seed = generateCombinedSeed();

        const std::vector<uint8_t> lorenzEntropy = generateRandomizedLorenzEntropy(STEPS, {rng_(), rng_(), rng_()});
        xorSeedWithLorenzEntropy(seed, lorenzEntropy);
        shuffleSeed(seed);

        std::uniform_int_distribution<int> dist(0, 255);
        buffer.resize(buffer.size());
        for (auto &byte: buffer) {
            byte = dist(rng_);
        }
    }

    std::array<uint8_t, 32> RNG::generateCombinedSeed() {
        std::random_device rd;
        std::array<uint8_t, 32> osSeedBytes{};
        for (auto &byte: osSeedBytes) {
            byte = rd();
        }

        std::array<uint8_t, 32> entropySeedBytes{};
        for (auto &byte: entropySeedBytes) {
            byte = rng_();
        }

        std::array<uint8_t, 32> combinedSeed{};
        for (size_t i = 0; i < 32; ++i) {
            combinedSeed[i] = osSeedBytes[i] ^ entropySeedBytes[i];
        }

        return combinedSeed;
    }

    void RNG::xorSeedWithLorenzEntropy(std::array<uint8_t, 32> &seed, const std::vector<uint8_t> &lorenzEntropy) {
        for (size_t i = 0; i < lorenzEntropy.size(); ++i) {
            seed[i % 32] ^= lorenzEntropy[i];
        }
    }

    std::vector<uint8_t> RNG::generateRandomizedLorenzEntropy(const size_t steps,
                                                              const std::tuple<double, double, double> &
                                                              initialConditions) {
        double x = std::get<0>(initialConditions);
        double y = std::get<1>(initialConditions);
        double z = std::get<2>(initialConditions);

        constexpr double beta = 8.0 / 3.0;

        std::vector<uint8_t> entropy;
        entropy.reserve(steps * 3);

        for (size_t i = 0; i < steps; ++i) {
            constexpr double dt = 0.01;
            constexpr double rho = 28.0;
            constexpr double sigma = 10.0;
            const double dx = sigma * (y - x) * dt;
            const double dy = (x * (rho - z) - y) * dt;
            const double dz = (x * y - beta * z) * dt;

            x += dx;
            y += dy;
            z += dz;

            entropy.push_back(static_cast<uint8_t>(std::fmod(std::abs(x), 256.0)));
            entropy.push_back(static_cast<uint8_t>(std::fmod(std::abs(y), 256.0)));
            entropy.push_back(static_cast<uint8_t>(std::fmod(std::abs(z), 256.0)));
        }

        return entropy;
    }

    void RNG::shuffleSeed(std::array<uint8_t, 32> &seed) {
        std::ranges::shuffle(seed, rng_);
    }
}
