#include "Core/Reproductor.hpp"
#include <iostream>
#include <string>

void limpiarConsola() {
    // Para Windows
    #ifdef _WIN32
        system("cls");
    #else
        system("clear");
    #endif
}

void mostrarMenuPrincipal(Reproductor& rep) {
    std::cout << "\n=== REPRODUCTOR DE MUSICA ===\n\n";

    // Mostrar estado actual
    std::string estado = rep.getEstadoReproduccion();
    std::string modoAleatorio = rep.getModoAleatorioStr();
    std::string modoRep = rep.getModoRepeticionStr();
    std::string cancion = rep.getCancionActualStr();

    std::cout << estado;
    if (!modoAleatorio.empty() || !modoRep.empty()) {
        std::cout << " (";
        if (!modoAleatorio.empty()) std::cout << modoAleatorio;
        if (!modoAleatorio.empty() && !modoRep.empty()) std::cout << "-";
        if (!modoRep.empty()) std::cout << modoRep;
        std::cout << ")";
    }
    std::cout << ": " << cancion << "\n\n";

    std::cout << "Opciones:\n";
    std::cout << "W - Reproducir/Pausar\n";
    std::cout << "Q - Pista Anterior\n";
    std::cout << "E - Pista Siguiente\n";
    std::cout << "S - Activar/Desactivar modo aleatorio\n";
    std::cout << "R - Repeticion (Desactivado/Repetir una/Repetir todas)\n";
    std::cout << "A - Ver lista de reproduccion actual\n";
    std::cout << "L - Listado de canciones\n";
    std::cout << "X - Salir\n";
    std::cout << "Ingrese opcion: ";
}

int main() {
    Reproductor rep("music_source.txt", "status.cfg");
    rep.cargarEstado();

    bool ejecutando = true;
    std::string opcion;

    while (ejecutando) {
        limpiarConsola();
        mostrarMenuPrincipal(rep);

        std::cin >> opcion;

        // Convertir a mayúscula
        if (!opcion.empty()) {
            opcion[0] = toupper(opcion[0]);
        }

        switch (opcion[0]) {
            case 'W':
                rep.play();
                break;
            case 'Q':
                rep.prev();
                break;
            case 'E':
                rep.next();
                break;
            case 'S':
                rep.toggleShuffle();
                break;
            case 'R':
                rep.toggleRepeat();
                break;
            case 'A':
                rep.seePlaylist();
                break;
            case 'L':
                rep.seeSongList();
                break;
            case 'X':
                ejecutando = false;
                std::cout << "Guardando estado...\n";
                rep.guardarEstado();
                std::cout << "¡Hasta luego!\n";
                break;
            default:
                std::cout << "Opcion inválida. Presione Enter para continuar...";
                std::cin.ignore();
                std::cin.get();
        }
    }

    return 0;
}