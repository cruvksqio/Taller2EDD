#include "Reproductor.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <ctime>

Reproductor::Reproductor()
    : reproduciendo(false), modoAleatorio(false), modoRepeticion(0),
      ultimoId(0), archivoMusica("music_source.txt"), archivoConfig("status.cfg") {
    srand(time(nullptr));
}

Reproductor::Reproductor(const std::string& archivoMusica, const std::string& archivoConfig)
    : reproduciendo(false), modoAleatorio(false), modoRepeticion(0),
      ultimoId(0), archivoMusica(archivoMusica), archivoConfig(archivoConfig) {
    srand(time(nullptr));
}

void Reproductor::cargarCanciones() {
    std::ifstream archivo(archivoMusica);

    if (!archivo.is_open()) {
        std::cerr << "Advertencia: No se pudo abrir el archivo " << archivoMusica << ".\n";
        return;
    }

    std::string linea;
    while (std::getline(archivo, linea)) {
        std::stringstream ss(linea);
        std::string dato;

        int id, anio, duracion;
        std::string nombre, artista, album, ruta;

        try {
            std::getline(ss, dato, ','); id = std::stoi(dato);

            std::getline(ss, nombre, ',');
            if (!nombre.empty() && nombre[0] == ' ') nombre.erase(0, 1);

            std::getline(ss, artista, ',');
            if (!artista.empty() && artista[0] == ' ') artista.erase(0, 1);

            std::getline(ss, album, ',');
            if (!album.empty() && album[0] == ' ') album.erase(0, 1);

            std::getline(ss, dato, ','); anio = std::stoi(dato);
            std::getline(ss, dato, ','); duracion = std::stoi(dato);

            std::getline(ss, ruta, ',');
            if (!ruta.empty() && ruta[0] == ' ') ruta.erase(0, 1);

            Cancion nuevaCancion(id, nombre, artista, album, anio, duracion, ruta);
            registroTotal.agregarAlFinal(nuevaCancion);

            if (id > ultimoId) ultimoId = id;

        } catch (const std::exception& e) {
            std::cerr << "Error al procesar línea: " << e.what() << '\n';
            continue;
        }
    }

    archivo.close();
}

void Reproductor::guardarCanciones() {
    std::ofstream archivo(archivoMusica);

    if (!archivo.is_open()) {
        std::cerr << "Error: No se pudo guardar el archivo " << archivoMusica << "\n";
        return;
    }
    // PARTE INCOMPLETA, HACER
    SongList<Cancion> temp;

    archivo.close();
}

void Reproductor::mezclarListaReproduccion() {
    if (listaReproduccion.obtenerLongitud() <= 1) return;

    // Guardar la canción actual
    Cancion* cancionActual = listaReproduccion.obtenerActual();
    if (cancionActual == nullptr) return;

    int idActual = cancionActual->getId();

    // Crear array temporal con las canciones (excepto la actual)
    int longitud = listaReproduccion.obtenerLongitud();
    Cancion* canciones = new Cancion[longitud];
    int index = 0;

    // Extraer todas las canciones a un array

    // Mezclar usando Fisher-Yates
    for (int i = longitud - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        Cancion temp = canciones[i];
        canciones[i] = canciones[j];
        canciones[j] = temp;
    }

    // Reconstruir la lista
    listaReproduccion.vaciar();
    for (int i = 0; i < longitud; i++) {
        listaReproduccion.agregarAlFinal(canciones[i]);
    }

    delete[] canciones;
}

// ============ Main methods!!! ============

void Reproductor::cargarEstado() {
    // Primero cargar las canciones
    cargarCanciones();

    // Luego cargar la configuración
    std::ifstream archivo(archivoConfig);

    if (!archivo.is_open()) {
        std::cout << "Archivo status.cfg no encontrado. Generando estado por defecto...\n";
        reproduciendo = false;
        modoAleatorio = false;
        modoRepeticion = 0;
        guardarEstado();
        return;
    }

    std::string linea;
    while (std::getline(archivo, linea)) {
        if (linea.empty()) continue;

        std::stringstream ss(linea);
        std::string parametro;
        std::string valor;

        ss >> parametro;
        std::getline(ss >> std::ws, valor);

        if (parametro == "MODO_ALEATORIO") {
            modoAleatorio = (valor == "1");
        }
        else if (parametro == "MODO_REPETICION") {
            modoRepeticion = std::stoi(valor);
        }
        else if (parametro == "ESTADO_REPRODUCCION") {
            int estado = std::stoi(valor);
            reproduciendo = (estado == 1);
        }
        else if (parametro == "COLA_PENDIENTE") {
            if (valor != "VACIA") {
                std::stringstream ssIds(valor);
                std::string idStr;

                while (std::getline(ssIds, idStr, ',')) {
                    int id = std::stoi(idStr);
                    Cancion* cancion = registroTotal.buscarPorId(id);
                    if (cancion != nullptr) {
                        listaReproduccion.agregarAlFinal(*cancion);
                    }
                }
            }
        }
    }

    archivo.close();
}

void Reproductor::guardarEstado() {
    // Al abrirlo así, C++ lo crea en el "Working Directory" actual
    std::ofstream archivo(archivoConfig);

    if (!archivo.is_open()) {
        std::cerr << "Error: No se pudo crear el archivo " << archivoConfig << "\n";
        return;
    }

    archivo << "MODO_ALEATORIO " << (modoAleatorio ? "1" : "0") << "\n";
    archivo << "MODO_REPETICION " << modoRepeticion << "\n";
    archivo << "ESTADO_REPRODUCCION " << (reproduciendo ? "1" : "0") << "\n";
    archivo << "COLA_PENDIENTE " << listaReproduccion.obtenerIdsComoString() << "\n";

    if (!archivo.is_open()) {
        std::cout << "Archivo no encontrado en la raiz. Creando uno nuevo...\n";
        guardarEstado(); // Esto creará el archivo en la carpeta del main
        return;
    }

    archivo.close();
}

bool Reproductor::play() {
    if (!hayCancionesDisponibles()) return false;

    reproduciendo = !reproduciendo;

    // Si estamos iniciando reproducción y no hay canción actual, tomar la primera
    if (reproduciendo && listaReproduccion.obtenerActual() == nullptr) {
        // Si la lista de reproducción está vacía, llenarla con todas las canciones
        if (listaReproduccion.estaVacia()) {
            // Clonar registroTotal a listaReproduccion
            SongList<Cancion> temp;
            // Implementar clonacion
        }
    }

    guardarEstado();
    return true;
}

bool Reproductor::prev() {
    if (!hayCancionesDisponibles()) return false;

    if (modoRepeticion == 1) {
        // Repetir una: mantener la canción actual
        return true;
    }

    if (!listaReproduccion.retroceder()) {
        // Si no hay anterior y está en modo repetir todas
        if (modoRepeticion == 2) {
            listaReproduccion.irAlFinal();
            if (modoAleatorio) {
                mezclarListaReproduccion();
            }
        } else {
            return false;
        }
    }

    reproduciendo = true;
    guardarEstado();
    return true;
}

bool Reproductor::next() {
    if (!hayCancionesDisponibles()) return false;

    if (modoRepeticion == 1) {
        // Repetir una: mantener el tema actual
        return true;
    }

    if (!listaReproduccion.avanzar()) {
        // Si no hay siguiente
        if (modoRepeticion == 2) {
            // Repetir todas: volver al inicio
            listaReproduccion.irAlInicio();
            if (modoAleatorio) {
                mezclarListaReproduccion();
            }
        } else if (listaReproduccion.estaVacia()) {
            // Lista vacía: seleccionar aleatoriamente del registro
            // Implementar selección aleatoria
        } else {
            return false;
        }
    }

    reproduciendo = true;
    guardarEstado();
    return true;
}

void Reproductor::toggleShuffle() {
    if (!hayCancionesDisponibles()) return;

    modoAleatorio = !modoAleatorio;

    if (modoAleatorio && !listaReproduccion.estaVacia()) {
        mezclarListaReproduccion();
    }

    guardarEstado();
}

void Reproductor::toggleRepeat() {
    if (!hayCancionesDisponibles()) return;

    // Ciclar entre: 0 (Desactivado) -> 1 (Repetir una) -> 2 (Repetir todas) -> 0
    modoRepeticion = (modoRepeticion + 1) % 3;

    guardarEstado();
}

void Reproductor::seePlaylist() {
    if (!hayCancionesDisponibles()) return;

    std::cout << "\n=== LISTA DE REPRODUCCION ACTUAL ===\n";

    Cancion* cancionActual = listaReproduccion.obtenerActual();
    if (cancionActual != nullptr) {
        std::cout << "Actual (" << getModoAleatorioStr() << "-"
                  << getModoRepeticionStr() << "): ";
        cancionActual->imprimirDatos();
    }

    std::cout << "\nLista de reproduccion actual:\n";
    if (listaReproduccion.estaVacia()) {
        std::cout << "Vacía\n";
    } else {
        listaReproduccion.imprimirLista();
    }

    std::cout << "\nOpciones:\n";
    std::cout << "S<num> - Saltar a la cancion seleccionada\n";
    std::cout << "V - Volver al menú principal\n";
    std::cout << "Ingrese opción: ";

    std::string opcion;
    std::cin >> opcion;

    if (opcion[0] == 'S' || opcion[0] == 's') {
        int num = std::stoi(opcion.substr(1));
        if (listaReproduccion.saltarA(num)) {
            reproduciendo = true;
            std::cout << "Saltando a canción " << num << "...\n";
        } else {
            std::cout << "Posición inválida.\n";
        }
    }

    guardarEstado();
}

void Reproductor::seeSongList() {
    if (registroTotal.estaVacia()) {
        std::cout << "No hay canciones registradas.\n";
        return;
    }

    std::cout << "\n=== LISTADO DE CANCIONES ===\n";

    Cancion* cancionActual = listaReproduccion.obtenerActual();
    if (cancionActual != nullptr) {
        std::cout << "Actual (" << getModoAleatorioStr() << "-"
                  << getModoRepeticionStr() << "): ";
        cancionActual->imprimirDatos();
    }

    std::cout << "\nCanciones registradas:\n";
    registroTotal.imprimirLista();

    std::cout << "\nOpciones:\n";
    std::cout << "R<num> - Reproducir cancion seleccionada\n";
    std::cout << "A<num> - Agregar cancion al final de la lista\n";
    std::cout << "N - Agregar nueva cancion al registro\n";
    std::cout << "D<num> - Eliminar cancion seleccionada\n";
    std::cout << "V - Volver al menu principal\n";
    std::cout << "Ingrese opcion: ";

    std::string opcion;
    std::cin >> opcion;

    if (opcion[0] == 'R' || opcion[0] == 'r') {
        try {
            int num = std::stoi(opcion.substr(1));

            if (registroTotal.saltarA(num)) {
                Cancion* cancionSeleccionada = registroTotal.obtenerActual();

                if (cancionSeleccionada != nullptr) {
                    // Vaciar lista actual
                    listaReproduccion.vaciar();

                    // Agregar la canción seleccionada primero
                    listaReproduccion.agregarAlFinal(*cancionSeleccionada);

                    // Agregar todas las demás canciones en orden
                    registroTotal.irAlInicio();
                    do {
                        Cancion* actual = registroTotal.obtenerActual();
                        if (actual != nullptr && actual->getId() != cancionSeleccionada->getId()) {
                            listaReproduccion.agregarAlFinal(*actual);
                        }
                    } while (registroTotal.avanzar());

                    // Mezclar la lista (excepto la primera canción que es la seleccionada)
                    // Para esto necesitarías implementar un método en SongList
                    // o hacer una mezcla manual

                    reproduciendo = true;
                    std::cout << "Reproduciendo: " << cancionSeleccionada->getNombre()
                              << " - " << cancionSeleccionada->getArtista() << "\n";
                }
            }
        } catch (const std::exception& e) {
            std::cout << "Error: Entrada inválida. Use formato R<num> (ejemplo: R3)\n";
        }
    }

    else if (opcion[0] == 'A' || opcion[0] == 'a') {
        int num = std::stoi(opcion.substr(1));
        if (registroTotal.saltarA(num)) {
            Cancion* cancion = registroTotal.obtenerActual();
            if (cancion != nullptr) {
                listaReproduccion.agregarAlFinal(*cancion);
                std::cout << "Cancion agregada al final de la lista.\n";
            }
        }
    }
    else if (opcion[0] == 'N' || opcion[0] == 'n') {
        // Agregar nueva canción
        std::cin.ignore();
        std::string nombre, artista, album, ruta;
        int anio, duracion;

        std::cout << "Nombre de la canción: ";
        std::getline(std::cin, nombre);
        std::cout << "Artista: ";
        std::getline(std::cin, artista);
        std::cout << "Álbum: ";
        std::getline(std::cin, album);
        std::cout << "Año: ";
        std::cin >> anio;
        std::cout << "Duración (segundos): ";
        std::cin >> duracion;
        std::cin.ignore();
        std::cout << "Ruta del archivo: ";
        std::getline(std::cin, ruta);

        ultimoId++;
        Cancion nueva(ultimoId, nombre, artista, album, anio, duracion, ruta);
        registroTotal.agregarAlFinal(nueva);

        std::cout << "Cancion agregada exitosamente.\n";
    }
    else if (opcion[0] == 'D' || opcion[0] == 'd') {
        int num = std::stoi(opcion.substr(1));
        if (registroTotal.eliminarPorPosicion(num)) {
            std::cout << "Cancion eliminada.\n";
        } else {
            std::cout << "Error al eliminar.\n";
        }
    }

    guardarEstado();
}

// ============ GETTERS DE ESTADO ============

std::string Reproductor::getEstadoReproduccion() const {
    if (!reproduciendo) return "Detenida";

    // Aquí deberías verificar si está en pausa
    return "Reproduciendo";
}

std::string Reproductor::getModoAleatorioStr() const {
    return modoAleatorio ? "S" : "";
}

std::string Reproductor::getModoRepeticionStr() const {
    if (modoRepeticion == 1) return "R1";
    if (modoRepeticion == 2) return "RA";
    return "";
}

std::string Reproductor::getCancionActualStr() const {
    const Cancion* cancion = listaReproduccion.obtenerActual();
    if (cancion != nullptr) {
        return cancion->getNombre() + " - " + cancion->getArtista();
    }
    return "Sin cancion";
}

bool Reproductor::hayCancionesDisponibles() const {
    return !registroTotal.estaVacia();
};