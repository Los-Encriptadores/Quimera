# *Quimera* 🔒

![Quimera](./assets/Quimera.jpg)
*generated by GPT-4o with the prompt: "A Windows 98 style artwork for a project named 'Quimera'. The design should include elements that reflect the Mexican meaning and style behind 'Los-Encriptadores'. The image should be minimalistic, featuring retro computer graphics, vibrant colors, and a mix of traditional Mexican patterns or symbols like serpents or skulls. Incorporate a lock or encryption symbol to represent the cybersecurity theme. The text 'Quimera' should be prominently displayed in a bold, retro font."*
## Overview

This project implements a polymorphic encryption engine using the Libsodium library for secure encryption and decryption of files. The engine employs an additional XOR-based transformation to enhance security, providing a polymorphic layer on top of the standard encryption methods.

## Features

- **File Encryption and Decryption**: Encrypt and decrypt files using secure cryptographic algorithms.
- **Polymorphic Encryption**: Adds an extra layer of security by applying XOR-based transformations to the encrypted data.
- **High-Quality RNG**: Utilizes a custom Random Number Generator (RNG) with enhanced entropy for key generation.

## Prerequisites

- **Libsodium**: Ensure that Libsodium is installed on your system. You can install it using package managers or from the source. [doc.libsodium.org](https://doc.libsodium.org/)

## Installation

1. **Clone the Repository**:
    ```bash
    git clone https://github.com/0x5844/mirage-cpp.git
    cd mirage-cpp
    ```

2. **Build the Project**:
    ```bash
    mkdir build
    cd build
    cmake ..
    make
    ```

## Usage

1. **Generate Test File**: Creates a test file with random data.
2. **Encrypt File**: Encrypts the generated test file.
3. **Decrypt File**: Decrypts the encrypted file.
4. **Exit**: Deletes the generated files and exits the application.

Run the application:
```bash
./PolymorphicEncryptionEngine
```

Follow the on-screen menu to choose between encryption, decryption, and exiting the application.

## Code Structure

### `main.cpp`

- **Main Function**: Provides a menu-driven interface to encrypt, decrypt, or exit the application.
- **generateTestFile**: Generates a test file with random data of a specified size.
- **displayMenu**: Displays the user menu.
- **deleteFiles**: Deletes the test, encrypted, and decrypted files.
- **formatDuration**: Formats the duration in milliseconds for display.

### `engines/IPolymorphicEncryptionEngine.h`

Defines the interface for the polymorphic encryption engine, ensuring that any derived class implements essential encryption and decryption functionalities.

### `engines/encryption/PolymorphicEncryptionEngine.h` & `.cpp`

Implements the polymorphic encryption engine:
- **Constructor**: Initializes the encryption engine, generates encryption and XOR keys.
- **Destructor**: Cleans up and securely erases the keys.
- **encryptFile**: Encrypts a file, applies an XOR operation, and writes the encrypted data to the output file.
- **decryptFile**: Decrypts a file, applies an XOR operation, and writes the decrypted data to the output file.
- **generateXorKey**: Generates a random XOR key.
- **generateEncryptionKey**: Generates the encryption key using a custom RNG.
- **xorBuffer**: Applies an XOR operation to a buffer.
- **rekey**: Updates the encryption state with a new key.

### `utils/math/RNG.h` & `.cpp`

Custom Random Number Generator with enhanced entropy:
- **Constructor**: Seeds the RNG using a combination of `std::random_device` and `std::seed_seq`.
- **generateSeed**: Generates a 32-byte seed by combining OS entropy and additional entropy from `std::mt19937`.
- **mixSeedWithLorenzEntropy**: Enhances the seed with entropy derived from the Lorenz attractor.
- **fillBufferWithRandomBytes**: Fills a buffer with random bytes using a uniform distribution.
- **random**: Generates random numbers of the specified integral type.

### Cryptographic Mathematics

- **XOR-based Transformation**: Enhances security by applying an XOR operation using a polymorphic key. This key changes with each encryption session, adding a layer of variability.
- **Lorenz Attractor**: Adds entropy to the RNG seed, enhancing the randomness quality. The Lorenz attractor is a system of differential equations known for its chaotic solutions, which help in generating high-quality random numbers. [Lorenz System](https://en.wikipedia.org/wiki/Lorenz_system)

## Libraries Used

- **Libsodium**: Used for cryptographic functions and secure memory management.
- **C++ Standard Library**: Utilized for file handling, random number generation, and general utilities.

## License

This project is licensed under the GPL-3.0 License. See the [LICENSE](https://github.com/0x5844/mirage-cpp/blob/main/LICENSE) file for details.

## Disclaimer

This project is intended for research purposes only. Use it at your own risk. The authors are not responsible for any misuse or damage caused by this software.

*[Generated by GPT4o @ 31-May-2024]*

---

I wanted to take a moment to appreciate the work of [PATHBYTER-Hybrid-Encryption-Ransomware-with-Multiprocessing-in-Python](https://github.com/0x00wolf/PATHBYTER-Hybrid-Encryption-Ransomware-with-Multiprocessing-in-Python); it played a huge role in motivating me towards tipping my toes in this research. Hats off to the author, [0x00wolf](https://github.com/0x00wolf) 🫡

---

[WIP] So far what I have:
```
Generating test file: test.ini
Successfully created test file: test.iniwith the size of 1048576000 bytes
Generated test file: test.ini
Initializing PolymorphicEncryptionEngine
PolymorphicEncryptionEngine initialized

1. Encrypt
2. Decrypt
3. Exit
Choose an option: 1
Encrypting file: test.ini
Encrypted file: encrypted_test.ini
Encryption time: 3767.908 ms

1. Encrypt
2. Decrypt
3. Exit
Choose an option: 2
Decrypting file: encrypted_test.ini
Decrypted file: decrypted_test.ini
Decryption time: 3754.052 ms

1. Encrypt
2. Decrypt
3. Exit
Choose an option: 3
Exiting application.
Encryption and decryption operations completed successfully.
Destroying PolymorphicEncryptionEngine
PolymorphicEncryptionEngine destroyed

Process finished with exit code 0
```
