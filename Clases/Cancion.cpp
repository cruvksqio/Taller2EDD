#include "Cancion.hpp"
#include <iostream>

// Constructor por defecto
Cancion::Cancion() {
    this->id = 0;
    this->nombre = "Desconocido";
    this->artista = "Desconocido";
    this->album = "Desconocido";
    this->anio = 0;
    this->duracion = 0;
    this->ruta = "";
}

// Constructor parametrizado
Cancion::Cancion(int id, std::string nombre, std::string artista, std::string album,
                 int anio, int duracion, std::string ruta) {
    this->id = id;
    this->nombre = nombre;
    this->artista = artista;
    this->album = album;
    this->anio = anio;
    this->duracion = duracion;
    this->ruta = ruta;
}

// Implementación de Getters
int Cancion::getId() const { return id; }
std::string Cancion::getNombre() const { return nombre; }
std::string Cancion::getArtista() const { return artista; }
std::string Cancion::getAlbum() const { return album; }
int Cancion::getAnio() const { return anio; }
int Cancion::getDuracion() const { return duracion; }
std::string Cancion::getRuta() const { return ruta; }

// Implementación de Setters
void Cancion::setId(int id) { this->id = id; }
void Cancion::setNombre(std::string nombre) { this->nombre = nombre; }
void Cancion::setArtista(std::string artista) { this->artista = artista; }
void Cancion::setAlbum(std::string album) { this->album = album; }
void Cancion::setAnio(int anio) { this->anio = anio; }
void Cancion::setDuracion(int duracion) { this->duracion = duracion; }
void Cancion::setRuta(std::string ruta) { this->ruta = ruta; }

// Metodo similar a toString que imprime datos
void Cancion::imprimirDatos() const {
    std::cout << nombre << " - " << artista << " (" << album << ") [" << anio << "]" << std::endl;
}

// Método para convertir a string
std::string Cancion::toString() const {
    return std::to_string(id) + ", " + nombre + ", " + artista + ", " +
           album + ", " + std::to_string(anio) + ", " +
           std::to_string(duracion) + ", " + ruta;
}