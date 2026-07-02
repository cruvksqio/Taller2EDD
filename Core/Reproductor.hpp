#ifndef REPRODUCTOR_HPP
#define REPRODUCTOR_HPP

#include <string>
#include "../Structure/SongList.hpp"
#include "../Clases/Cancion.hpp"

class Reproductor {
private:
    SongList<Cancion> registroTotal;      // Todas las canciones cargadas
    SongList<Cancion> listaReproduccion;  // Cola de reproducción actual

    // Variables de estado
    bool reproduciendo;                   // true = reproduciendo, false = pausa/detenido
    bool modoAleatorio;
    int modoRepeticion;                   // 0: Desactivado, 1: Repetir una (R1), 2: Repetir todas (RA)
    int ultimoId;                         // Último ID usado para nuevas canciones

    // Archivos
    std::string archivoMusica;
    std::string archivoConfig;

    // Métodos privados de ayuda
    void cargarCanciones();
    void guardarCanciones();
    void mezclarListaReproduccion();

public:
    Reproductor();
    Reproductor(const std::string& archivoMusica, const std::string& archivoConfig);

    // Métodos principales del reproductor
    void cargarEstado();
    void guardarEstado();

    // Acciones del menú principal
    bool play();                          // W - Reproducir/Pausar
    bool prev();                          // Q - Pista Anterior
    bool next();                          // E - Pista Siguiente
    void toggleShuffle();                 // S - Activar/Desactivar modo aleatorio
    void toggleRepeat();                  // R - Cambiar modo repetición
    void seePlaylist();                   // A - Ver lista de reproducción actual
    void seeSongList();                   // L - Listado de canciones

    // Getters para mostrar el estado
    std::string getEstadoReproduccion() const;
    std::string getModoAleatorioStr() const;
    std::string getModoRepeticionStr() const;
    std::string getCancionActualStr() const;
    bool hayCancionesDisponibles() const;
};

#endif