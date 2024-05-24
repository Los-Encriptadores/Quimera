cmake_minimum_required(VERSION 3.28)
project(mirage_core)

set(CMAKE_CXX_STANDARD 26)

add_executable(mirage_core main.cpp
        encryption/Encryption.cpp
        encryption/Encryption.h
        encryption/methods/AES/AESGCM128.cpp
        encryption/methods/AES/AESGCM128.h
        encryption/methods/AES/AES.cpp
        encryption/methods/AES/AES.h
        encryption/methods/AES/GF256.cpp
        encryption/methods/AES/GF256.h
        encryption/methods/AES/SBox.cpp
        encryption/methods/AES/SBox.h
        utils/math/RNG.cpp
        utils/math/RNG.h
        file/FileHandler.cpp
        file/FileHandler.h
)
