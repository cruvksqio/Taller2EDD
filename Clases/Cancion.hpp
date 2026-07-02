#ifndef CANCION_HPP
#define CANCION_HPP

#include <string>

class Cancion {
private:
    int id;
    std::string nombre;
    std::string artista;
    std::string album;
    int anio;
    int duracion; // en segundos
    std::string ruta;

public:
    Cancion();
    Cancion(int id, std::string nombre, std::string artista, std::string album,
            int anio, int duracion, std::string ruta);

    // Getters
    int getId() const;
    std::string getNombre() const;
    std::string getArtista() const;
    std::string getAlbum() const;
    int getAnio() const;
    int getDuracion() const;
    std::string getRuta() const;

    // Setters (añadir setId)
    void setId(int id);
    void setNombre(std::string nombre);
    void setArtista(std::string artista);
    void setAlbum(std::string album);
    void setAnio(int anio);
    void setDuracion(int duracion);
    void setRuta(std::string ruta);

    void imprimirDatos() const;
    std::string toString() const; // Para guardar en archivos
};

#endif