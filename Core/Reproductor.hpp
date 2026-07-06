#ifndef REPRODUCTOR_HPP
#define REPRODUCTOR_HPP

#include <string>
#include "../Structure/SongList.hpp"
#include "../Structure/Trie.hpp"
#include "../Clases/Cancion.hpp"
#include "../Clases/EntradaArtista.hpp"

class Reproductor {
private:
    SongList<Cancion> registroTotal;      // Todas las canciones cargadas
    SongList<Cancion> listaReproduccion;  // Cola de reproducción actual
    Trie<int> A_trieNombres;              // Arbol Trie para búsqueda por nombre (guarda IDs)
    Trie<int> A_trieArtistas;             // Arbol Trie para búsqueda por artista (guarda IDs)

    // Variables de estado
    bool reproduciendo;                   // true = reproduciendo, false = pausa/detenido
    bool modoAleatorio;
    int modoRepeticion;                   // 0: Desactivado, 1: Repetir una (R1), 2: Repetir todas (RA)
    int ultimoId;                         // Último ID usado para nuevas canciones

    // Archivos
    std::string archivoMusica;
    std::string archivoConfig;
    std::string archivoRanking;           // song_ranking.txt: id,contador (independiente de music_source.txt)

    // Métodos privados de ayuda
    void cargarCanciones();
    void guardarCanciones();
    void mezclarListaReproduccion();
    void reconstruirTries();
    // Reproduce "cancion" de inmediato y baraja el resto de la biblioteca
    // en la lista de reproducción (comportamiento compartido por R<num>).
    void reproducirCancionEspecifica(Cancion& cancion);
    // Busca por nombre y/o artista. Devuelve un arreglo dinámico (no STL)
    // con punteros a las canciones encontradas y su cantidad en
    // "cantidadResultados". El llamador es responsable de hacer
    // delete[] al arreglo devuelto (puede ser nullptr si no hubo resultados).
    Cancion** buscarCancionesPorTermino(const std::string& termino, int& cantidadResultados);

    // --- Ranking TOP 10 (Heap) ---
    void cargarRanking();                 // Lee song_ranking.txt y asigna los contadores
    void guardarRanking();                // Persiste el contador de cada canción
    // Marca que una canción empezó a sonar: suma 1 a su contador y persiste.
    void registrarReproduccion(Cancion& cancion);
    // Arma (con un Heap) el TOP N canciones ordenadas por reproducciones
    // desc. y, en empate, alfabéticamente. "cantidad" recibe cuántas se
    // devolvieron (hasta 10, o menos si hay menos canciones registradas).
    // El llamador debe hacer delete[] al resultado.
    Cancion** construirTopCanciones(int& cantidad);
    // Igual que el anterior, pero agrupando por artista.
    EntradaArtista* construirTopArtistas(int& cantidad);
    // Devuelve (ordenadas alfabéticamente) las canciones de un artista.
    // NOTA: esto es un filtro simple mientras se integra el árbol AVL
    // (parte de Fer) que debe encargarse de este listado por artista.
    Cancion** obtenerCancionesDeArtista(const std::string& artista, int& cantidad);

    void mostrarTop10Canciones();
    void mostrarTop10Artistas();
    void mostrarCancionesDeArtista(const std::string& artista);

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
    void searchSongs();                   // F - Búsqueda de canciones/artistas
    void menuTop10();                     // T - TOP 10 Artistas y Canciones

    // Getters para mostrar el estado
    std::string getEstadoReproduccion() const;
    std::string getModoAleatorioStr() const;
    std::string getModoRepeticionStr() const;
    std::string getCancionActualStr() const;
    bool hayCancionesDisponibles() const;
};

#endif