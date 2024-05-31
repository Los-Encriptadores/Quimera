#include "LorenzAttractor.h"
#include <cmath>
#include <vector>

namespace utils::math {
    void LorenzAttractor::mixSeedWithLorenzEntropy(std::array<uint8_t, 32> &seed) {
        double x = USE_ENHANCED_LORENZ_INITIAL_VALUES ? 1.01 : 1.0;
        double y = USE_ENHANCED_LORENZ_INITIAL_VALUES ? 1.02 : 1.0;
        double z = USE_ENHANCED_LORENZ_INITIAL_VALUES ? 1.03 : 1.0;
        constexpr double beta = 8.0 / 3.0;

        std::vector<uint8_t> lorenzEntropy;
        lorenzEntropy.reserve(LORENZ_ENTROPY_STEPS * 3);

        for (size_t i = 0; i < 100; ++i) {
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
}
