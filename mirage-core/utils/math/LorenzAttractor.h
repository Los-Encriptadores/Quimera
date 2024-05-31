#ifndef LORENZATTRACTOR_H
#define LORENZATTRACTOR_H

#include <array>

namespace utils::math {
    /**
     * @class LorenzAttractor
     * @brief A class for generating entropy using the Lorenz attractor.
     *
     * This class simulates the Lorenz system to generate chaotic values that can be used as entropy.
     */
    class LorenzAttractor {
    public:
        /**
         * @brief Generates entropy using the Lorenz attractor.
         *
         * This method uses the Lorenz equations to generate a chaotic sequence of values
         * and stores them in the provided buffer.
         *
         * @param buffer The buffer to be filled with entropy values.
         * @param size The size of the buffer.
         */
        static void generateEntropy(std::array<uint8_t, 32> &buffer, size_t size);

    private:
        static void step(double &x, double &y, double &z, double dt);
    };
} // namespace utils::math

#endif // LORENZATTRACTOR_H
