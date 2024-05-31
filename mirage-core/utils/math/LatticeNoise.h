#ifndef LATTICE_NOISE_H
#define LATTICE_NOISE_H

#include <vector>

#define MODULUS 256

namespace utils::math {
    class LatticeNoise {
    public:
        /**
         * @brief Adds lattice noise to the provided data.
         *
         * This method adds lattice noise to the provided data by XORing it with random bytes.
         *
         * @param data The data to which lattice noise will be added.
         */
        static void addLatticeNoise(std::vector<unsigned char> &data);
    };
} // namespace utils::math

#endif // LATTICE_NOISE_H
