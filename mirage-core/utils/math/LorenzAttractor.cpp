#include "LorenzAttractor.h"
#include <cmath>

#include "../../engines/encryption/PolymorphicEncryptionEngine.h"

namespace utils::math {
    void LorenzAttractor::step(double &x, double &y, double &z, double dt) {
        constexpr double sigma = 10.0;
        constexpr double rho = 28.0;
        constexpr double beta = 8.0 / 3.0;

        const double dx = sigma * (y - x);
        const double dy = x * (rho - z) - y;
        const double dz = x * y - beta * z;

        x += dx * dt;
        y += dy * dt;
        z += dz * dt;
    }

    void LorenzAttractor::generateEntropy(std::array<uint8_t, 32> &buffer, size_t size) {
        double x = PARANOID_MODE ? 1.01 : 1.0;
        double y = PARANOID_MODE ? 1.02 : 1.0;
        double z = PARANOID_MODE ? 1.03 : 1.0;

        for (size_t i = 0; i < size; ++i) {
            double dt = 0.01;
            step(x, y, z, dt);
            buffer[i] = static_cast<uint8_t>(std::fmod(std::abs(x + y + z), 256));
        }
    }
} // namespace utils::math
