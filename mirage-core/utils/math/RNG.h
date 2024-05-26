#ifndef RNG_H
#define RNG_H

#include <array>
#include <cstdint>
#include <random>
#include <vector>

namespace utils::math {
    class RNG {
    public:
        RNG();

        void generateRandomBytes(std::array<uint8_t, 12> &buffer);

        void generateRandomBytes(std::vector<uint8_t> &buffer);

    private:
        std::array<uint8_t, 32> generateCombinedSeed();

        static void xorSeedWithLorenzEntropy(std::array<uint8_t, 32> &seed,
                                             const std::vector<uint8_t> &lorenzEntropy);

        static std::vector<uint8_t> generateRandomizedLorenzEntropy(size_t steps,
                                                                    const std::tuple<double, double, double> &
                                                                    initialConditions);

        void shuffleSeed(std::array<uint8_t, 32> &seed);

        void generateUniformBytes(auto &buffer);

        std::mt19937 rng_;
    };
}

#endif // RNG_H
