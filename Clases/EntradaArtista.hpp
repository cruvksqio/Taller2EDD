#ifndef ENTRADAARTISTA_HPP
#define ENTRADAARTISTA_HPP

#include <string>

// Representa el total de reproducciones acumuladas por un artista
// (sumando las reproducciones de todas sus canciones en la biblioteca).
// Se usa como el tipo de dato que se inserta en el Heap del TOP 10 de
// artistas.
class EntradaArtista {
private:
    std::string nombreArtista;
    int totalReproducciones;

public:
    EntradaArtista() : nombreArtista(""), totalReproducciones(0) {}

    EntradaArtista(const std::string& nombreArtista, int totalReproducciones)
        : nombreArtista(nombreArtista), totalReproducciones(totalReproducciones) {}

    std::string getNombreArtista() const { return nombreArtista; }
    int getTotalReproducciones() const { return totalReproducciones; }

    void sumarReproducciones(int cantidad) { totalReproducciones += cantidad; }
};

#endif // ENTRADAARTISTA_HPP
