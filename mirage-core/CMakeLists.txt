cmake_minimum_required(VERSION 3.28)
project(mirage_core)

set(CMAKE_CXX_STANDARD 26)

# Add libsodium paths
set(LIBSODIUM_INCLUDE_DIR /opt/homebrew/opt/libsodium/include)
set(LIBSODIUM_LIBRARY /opt/homebrew/opt/libsodium/lib/libsodium.dylib)

# Include the directory for libsodium
include_directories(${LIBSODIUM_INCLUDE_DIR})

# Add the executable and the source files
add_executable(mirage_core main.cpp
        utils/math/RNG.cpp
        utils/math/RNG.h
        file/FileHandler.cpp
        file/FileHandler.h
        engines/encryption/PolymorphicEncryptionEngine.cpp
        engines/encryption/IPolymorphicEncryptionEngine.h
        engines/encryption/PolymorphicEncryptionEngine.h
        utils/math/LorenzAttractor.cpp
        utils/math/LorenzAttractor.h
        utils/math/LatticeNoise.cpp
        utils/math/LatticeNoise.h
)

# Link libsodium library
target_link_libraries(mirage_core ${LIBSODIUM_LIBRARY})
