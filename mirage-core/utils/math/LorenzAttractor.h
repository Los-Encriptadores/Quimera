#ifndef LORENZ_ATTRACTOR_H
#define LORENZ_ATTRACTOR_H

#include <array>
#include <cstdint>
#include <vector>

#define LORENZ_ENTROPY_STEPS 100
#define USE_ENHANCED_LORENZ_INITIAL_VALUES true

namespace utils::math {
    /**
     * @brief A class for mixing seeds with Lorenz entropy.
     */
    class LorenzAttractor {
    public:
        /**
         * @brief Mixes the provided seed with Lorenz entropy.
         *
         * This method enhances the provided seed by mixing it with entropy derived from the
         * Lorenz attractor. The enhanced seed provides better randomness for cryptographic purposes.
         *
         * @param seed A reference to the seed array to be mixed with Lorenz entropy.
         */
        static void mixSeedWithLorenzEntropy(std::array<uint8_t, 32> &seed);
    };
} // namespace utils::math

#endif // LORENZ_ATTRACTOR_H
