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
         * @param length The length of the data.
         */
        static void addLatticeNoise(std::vector<unsigned char> &data, size_t length);

        /**
         * @brief Removes lattice noise from the provided data.
         *
         * This method removes lattice noise from the provided data by reversing the XOR operation.
         *
         * @param data The data from which lattice noise will be removed.
         * @param length The length of the data.
         */
        static void removeLatticeNoise(std::vector<unsigned char> &data, size_t length);
    };
} // namespace utils::math

#endif // LATTICE_NOISE_H
