cmake_minimum_required(VERSION 3.16)
project(Taller2EDD)

set(CMAKE_CXX_STANDARD 17)

# Esto copia los archivos .txt al directorio del ejecutable
configure_file(
        ${CMAKE_SOURCE_DIR}/music_source.txt
        ${CMAKE_BINARY_DIR}/music_source.txt
        COPYONLY
)

add_executable(Taller2EDD
        main.cpp
        Clases/Cancion.cpp
        Core/Reproductor.cpp
        Structure/Trie.hpp
)