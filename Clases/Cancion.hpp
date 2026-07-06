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
    int reproducciones; // contador para el ranking TOP 10 (Heap)

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
    int getReproducciones() const;

    // Setters (añadir setId)
    void setId(int id);
    void setNombre(std::string nombre);
    void setArtista(std::string artista);
    void setAlbum(std::string album);
    void setAnio(int anio);
    void setDuracion(int duracion);
    void setRuta(std::string ruta);
    void setReproducciones(int reproducciones);

    // Suma 1 al contador de reproducciones (se llama cada vez que la
    // canción empieza a sonar).
    void incrementarReproducciones();

    void imprimirDatos() const;
    std::string toString() const; // Para guardar en archivos
};

#endif