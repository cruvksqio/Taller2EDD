#include "Reproductor.hpp"
#include "../Structure/Heap.hpp"
#include "../Structure/Avl.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <ctime>
#include <cctype>
#include <limits>

namespace {
    // Comparador para el Heap del TOP 10 de canciones: prioriza más
    // reproducciones y, en caso de empate, orden alfabético por nombre
    // y luego por artista (tal como exige la pauta).
    struct ComparadorCancionPorReproducciones {
        bool operator()(Cancion* const& a, Cancion* const& b) const {
            if (a->getReproducciones() != b->getReproducciones()) {
                return a->getReproducciones() > b->getReproducciones();
            }
            if (a->getNombre() != b->getNombre()) {
                return a->getNombre() < b->getNombre();
            }
            return a->getArtista() < b->getArtista();
        }
    };

    // Comparador para el Heap del TOP 10 de artistas: prioriza más
    // reproducciones acumuladas y, en empate, orden alfabético del artista.
    struct ComparadorArtistaPorReproducciones {
        bool operator()(const EntradaArtista& a, const EntradaArtista& b) const {
            if (a.getTotalReproducciones() != b.getTotalReproducciones()) {
                return a.getTotalReproducciones() > b.getTotalReproducciones();
            }
            return a.getNombreArtista() < b.getNombreArtista();
        }
    };

    struct ComparadorCancionPorArtistaYNombre {
        bool operator()(Cancion* const& a, Cancion* const& b) const {
            if (a->getArtista() != b->getArtista()) {
                return a->getArtista() < b->getArtista();
            }
            if (a->getNombre() != b->getNombre()) {
                return a->getNombre() < b->getNombre();
            }
            return a->getId() < b->getId();
        }
    };
}

Reproductor::Reproductor()
    : reproduciendo(false), modoAleatorio(false), modoRepeticion(0),
      ultimoId(0), archivoMusica("music_source.txt"), archivoConfig("status.cfg"),
      archivoRanking("song_ranking.txt") {
    srand(time(nullptr));
}

Reproductor::Reproductor(const std::string& archivoMusica, const std::string& archivoConfig)
    : reproduciendo(false), modoAleatorio(false), modoRepeticion(0),
      ultimoId(0), archivoMusica(archivoMusica), archivoConfig(archivoConfig),
      archivoRanking("song_ranking.txt"), A_trieNombres(), A_trieArtistas() {
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
    reconstruirTries();
    cargarRanking(); // Restaura los contadores de reproducciones guardados
}

void Reproductor::guardarCanciones() {
    std::ofstream archivo(archivoMusica);

    if (!archivo.is_open()) {
        std::cerr << "Error: No se pudo guardar el archivo " << archivoMusica << "\n";
        return;
    }

    registroTotal.forEach([&archivo](const Cancion& cancion) {
        archivo << cancion.toString() << "\n";
    });

    archivo.close();
}

void Reproductor::mezclarListaReproduccion() {
    if (listaReproduccion.obtenerLongitud() <= 1) return;
    listaReproduccion.mezclar();
}

void Reproductor::reconstruirTries() {
    A_trieNombres.vaciarTodo();
    A_trieArtistas.vaciarTodo();

    // Se indexa por ID (no por posición): la posición cambia cuando se
    // agregan o eliminan canciones, y el ID es estable.
    registroTotal.forEach([this](const Cancion& cancion) {
        A_trieNombres.insertar(cancion.getNombre(), cancion.getId());
        A_trieArtistas.insertar(cancion.getArtista(), cancion.getId());
    });
}

void Reproductor::reproducirCancionEspecifica(Cancion& cancion) {
    // "cancion" viene de registroTotal (búsqueda, listado o Top10), así
    // que registrarReproduccion incrementa la entrada maestra correcta.
    registrarReproduccion(cancion);

    listaReproduccion.vaciar();
    listaReproduccion.clonarDesde(registroTotal);
    listaReproduccion.mezclar();
    listaReproduccion.saltarPorId(cancion.getId());
    reproduciendo = true;
}

Cancion** Reproductor::buscarCancionesPorTermino(const std::string& termino, int& cantidadResultados) {
    cantidadResultados = 0;
    if (termino.empty()) {
        return nullptr;
    }

    std::string busqueda = termino;
    for (char& c : busqueda) {
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }

    int totalCanciones = registroTotal.obtenerLongitud();
    if (totalCanciones == 0) {
        return nullptr;
    }

    // Arreglo dinámico crudo (no STL) para acumular IDs únicos encontrados.
    int* idsEncontrados = new int[totalCanciones];
    int cantidadIds = 0;

    auto agregarIdUnico = [&idsEncontrados, &cantidadIds](int id) {
        for (int i = 0; i < cantidadIds; ++i) {
            if (idsEncontrados[i] == id) return;
        }
        idsEncontrados[cantidadIds++] = id;
    };

    A_trieNombres.buscarCoincidencias(busqueda, agregarIdUnico);
    A_trieArtistas.buscarCoincidencias(busqueda, agregarIdUnico);

    if (cantidadIds == 0) {
        delete[] idsEncontrados;
        return nullptr;
    }

    Cancion** resultados = new Cancion*[cantidadIds];
    int encontrados = 0;
    for (int i = 0; i < cantidadIds; ++i) {
        Cancion* cancion = registroTotal.buscarPorId(idsEncontrados[i]);
        if (cancion != nullptr) {
            resultados[encontrados++] = cancion;
        }
    }

    delete[] idsEncontrados;
    cantidadResultados = encontrados;
    return resultados;
}

// ============ RANKING TOP 10 (Heap) ============

void Reproductor::cargarRanking() {
    std::ifstream archivo(archivoRanking);

    if (!archivo.is_open()) {
        // No existe todavía: todas las canciones parten en 0 reproducciones
        // (condición de borde: archivo inexistente).
        return;
    }

    std::string linea;
    while (std::getline(archivo, linea)) {
        if (linea.empty()) continue;

        std::stringstream ss(linea);
        std::string idStr, contadorStr;

        if (!std::getline(ss, idStr, ',')) continue;
        if (!std::getline(ss, contadorStr, ',')) continue;

        try {
            int id = std::stoi(idStr);
            int contador = std::stoi(contadorStr);
            Cancion* cancion = registroTotal.buscarPorId(id);
            if (cancion != nullptr) {
                cancion->setReproducciones(contador);
            }
        } catch (const std::exception&) {
            continue; // Línea corrupta: se ignora (condición de borde)
        }
    }

    archivo.close();
}

void Reproductor::guardarRanking() {
    std::ofstream archivo(archivoRanking);

    if (!archivo.is_open()) {
        std::cerr << "Error: No se pudo guardar " << archivoRanking << "\n";
        return;
    }

    registroTotal.forEach([&archivo](const Cancion& cancion) {
        archivo << cancion.getId() << "," << cancion.getReproducciones() << "\n";
    });

    archivo.close();
}

void Reproductor::registrarReproduccion(Cancion& cancion) {
    cancion.incrementarReproducciones();
    guardarRanking();
}

Cancion** Reproductor::construirTopCanciones(int& cantidad) {
    cantidad = 0;
    int total = registroTotal.obtenerLongitud();
    if (total == 0) return nullptr;

    Heap<Cancion*, ComparadorCancionPorReproducciones> A_heapCanciones;
    for (int i = 1; i <= total; ++i) {
        Cancion* cancion = registroTotal.obtenerPorPosicion(i);
        if (cancion != nullptr) {
            A_heapCanciones.insertar(cancion);
        }
    }

    int maximo = (total < 10) ? total : 10;
    Cancion** top = new Cancion*[maximo];
    int extraidos = 0;
    Cancion* siguienteMejor = nullptr;
    while (extraidos < maximo && A_heapCanciones.extraerMayor(siguienteMejor)) {
        top[extraidos++] = siguienteMejor;
    }

    cantidad = extraidos;
    return top;
}

EntradaArtista* Reproductor::construirTopArtistas(int& cantidad) {
    cantidad = 0;
    int total = registroTotal.obtenerLongitud();
    if (total == 0) return nullptr;

    // Arreglo temporal (a lo más "total" artistas distintos) para agrupar
    // las reproducciones por artista antes de meterlas al Heap.
    EntradaArtista* artistasUnicos = new EntradaArtista[total];
    int cantidadArtistas = 0;

    for (int i = 1; i <= total; ++i) {
        Cancion* cancion = registroTotal.obtenerPorPosicion(i);
        if (cancion == nullptr) continue;

        bool encontrado = false;
        for (int j = 0; j < cantidadArtistas; ++j) {
            if (artistasUnicos[j].getNombreArtista() == cancion->getArtista()) {
                artistasUnicos[j].sumarReproducciones(cancion->getReproducciones());
                encontrado = true;
                break;
            }
        }
        if (!encontrado) {
            artistasUnicos[cantidadArtistas++] =
                EntradaArtista(cancion->getArtista(), cancion->getReproducciones());
        }
    }

    Heap<EntradaArtista, ComparadorArtistaPorReproducciones> A_heapArtistas;
    for (int i = 0; i < cantidadArtistas; ++i) {
        A_heapArtistas.insertar(artistasUnicos[i]);
    }
    delete[] artistasUnicos;

    int maximo = (cantidadArtistas < 10) ? cantidadArtistas : 10;
    EntradaArtista* top = new EntradaArtista[maximo];
    int extraidos = 0;
    EntradaArtista siguienteMejor;
    while (extraidos < maximo && A_heapArtistas.extraerMayor(siguienteMejor)) {
        top[extraidos++] = siguienteMejor;
    }

    cantidad = extraidos;
    return top;
}

Cancion** Reproductor::obtenerCancionesDeArtista(const std::string& artista, int& cantidad) {
    cantidad = 0;
    if (registroTotal.obtenerLongitud() == 0) return nullptr;

    Avl<Cancion*, ComparadorCancionPorArtistaYNombre> A_artistas;

    registroTotal.forEach([&A_artistas, &artista](Cancion& cancion) {
        if (cancion.getArtista() == artista) {
            A_artistas.insertar(&cancion);
        }
    });

    int encontrados = A_artistas.obtenerCantidad();
    if (encontrados == 0) return nullptr;

    Cancion** resultado = new Cancion*[encontrados];
    int indice = 0;
    A_artistas.recorridoInOrden([&resultado, &indice](Cancion* const& cancion) {
        resultado[indice++] = cancion;
    });

    cantidad = indice;
    return resultado;
}

void Reproductor::mostrarTop10Canciones() {
    int cantidad = 0;
    Cancion** top = construirTopCanciones(cantidad);

    std::cout << "\nRanking TOP " << cantidad << " Canciones más escuchadas:\n";
    for (int i = 0; i < cantidad; ++i) {
        std::cout << (i + 1) << ". [" << top[i]->getReproducciones() << "] "
                  << top[i]->getNombre() << " - " << top[i]->getArtista() << "\n";
    }

    std::cout << "\nOpciones:\n";
    std::cout << "R<num> - Reproducir canción seleccionada\n";
    std::cout << "A<num> - Agregar canción seleccionada al final de la lista de reproducción actual\n";
    std::cout << "A - Top 10 artistas más escuchados\n";
    std::cout << "V - Volver al menú principal\n";
    std::cout << "Ingrese Opción: ";

    std::string opcion;
    std::getline(std::cin, opcion);

    if (opcion.empty()) {
        delete[] top;
        return;
    }

    // "A" sola (sin número) significa ir al TOP 10 de artistas.
    if (opcion.size() == 1 && (opcion[0] == 'A' || opcion[0] == 'a')) {
        delete[] top;
        mostrarTop10Artistas();
        return;
    }

    char letra = static_cast<char>(std::toupper(static_cast<unsigned char>(opcion[0])));

    if (letra == 'R' || letra == 'A') {
        try {
            int num = std::stoi(opcion.substr(1));
            if (num >= 1 && num <= cantidad) {
                Cancion* seleccionada = top[num - 1];
                if (letra == 'R') {
                    reproducirCancionEspecifica(*seleccionada);
                    std::cout << "\nReproduciendo: " << seleccionada->getNombre()
                              << " - " << seleccionada->getArtista() << "\n";
                } else {
                    listaReproduccion.agregarAlFinal(*seleccionada);
                    std::cout << "\nCanción agregada al final de la lista de reproducción.\n";
                }
                guardarEstado();
            } else {
                std::cout << "\nNúmero fuera de rango.\n";
            }
        } catch (const std::exception&) {
            std::cout << "\nOpción inválida. Use el formato R<num> o A<num>.\n";
        }
    }
    // "V" o cualquier otra opción: vuelve al menú principal.

    delete[] top;
}

void Reproductor::mostrarTop10Artistas() {
    int cantidad = 0;
    EntradaArtista* top = construirTopArtistas(cantidad);

    std::cout << "\nRanking TOP " << cantidad << " Artistas más escuchados:\n";
    for (int i = 0; i < cantidad; ++i) {
        std::cout << (i + 1) << ". [" << top[i].getTotalReproducciones() << "] "
                  << top[i].getNombreArtista() << "\n";
    }

    std::cout << "\nOpciones:\n";
    std::cout << "S<num> - Mostrar canciones del artista\n";
    std::cout << "C - Top 10 canciones más escuchadas\n";
    std::cout << "V - Volver al menú principal\n";
    std::cout << "Ingrese Opción: ";

    std::string opcion;
    std::getline(std::cin, opcion);

    if (opcion.empty()) {
        delete[] top;
        return;
    }

    char letra = static_cast<char>(std::toupper(static_cast<unsigned char>(opcion[0])));

    if (letra == 'C' && opcion.size() == 1) {
        delete[] top;
        mostrarTop10Canciones();
        return;
    }

    if (letra == 'S') {
        try {
            int num = std::stoi(opcion.substr(1));
            if (num >= 1 && num <= cantidad) {
                std::string artistaElegido = top[num - 1].getNombreArtista();
                delete[] top;
                mostrarCancionesDeArtista(artistaElegido);
                return;
            } else {
                std::cout << "\nNúmero fuera de rango.\n";
            }
        } catch (const std::exception&) {
            std::cout << "\nOpción inválida. Use el formato S<num>.\n";
        }
    }
    // "V" o cualquier otra opción: vuelve al menú principal.

    delete[] top;
}

void Reproductor::mostrarCancionesDeArtista(const std::string& artista) {
    int cantidad = 0;
    Cancion** canciones = obtenerCancionesDeArtista(artista, cantidad);

    std::cout << "\nRanking TOP 10 Artistas más escuchados:\n";
    std::cout << "Artista: " << artista << "\n";
    for (int i = 0; i < cantidad; ++i) {
        std::cout << (i + 1) << ". " << canciones[i]->getNombre() << "\n";
    }

    std::cout << "\nOpciones:\n";
    std::cout << "R<num> - Reproducir canción seleccionada\n";
    std::cout << "A<num> - Agregar canción seleccionada al final de la lista de reproducción actual\n";
    std::cout << "V - Volver al listado de TOP 10 artistas\n";
    std::cout << "X - Volver al menú principal\n";
    std::cout << "Ingrese Opción: ";

    std::string opcion;
    std::getline(std::cin, opcion);

    if (opcion.empty()) {
        delete[] canciones;
        return;
    }

    char letra = static_cast<char>(std::toupper(static_cast<unsigned char>(opcion[0])));

    if (letra == 'V' && opcion.size() == 1) {
        delete[] canciones;
        mostrarTop10Artistas();
        return;
    }

    if (letra == 'X') {
        delete[] canciones;
        return; // Vuelve al menú principal
    }

    if (letra == 'R' || letra == 'A') {
        try {
            int num = std::stoi(opcion.substr(1));
            if (num >= 1 && num <= cantidad) {
                Cancion* seleccionada = canciones[num - 1];
                if (letra == 'R') {
                    reproducirCancionEspecifica(*seleccionada);
                    std::cout << "\nReproduciendo: " << seleccionada->getNombre()
                              << " - " << seleccionada->getArtista() << "\n";
                } else {
                    listaReproduccion.agregarAlFinal(*seleccionada);
                    std::cout << "\nCanción agregada al final de la lista de reproducción.\n";
                }
                guardarEstado();
            } else {
                std::cout << "\nNúmero fuera de rango.\n";
            }
        } catch (const std::exception&) {
            std::cout << "\nOpción inválida. Use el formato R<num> o A<num>.\n";
        }
    }

    delete[] canciones;
}

void Reproductor::menuTop10() {
    if (registroTotal.estaVacia()) {
        std::cout << "No hay canciones registradas.\n";
        return;
    }

    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::cout << "\nRanking TOP\n";
    std::cout << "C - Top 10 canciones más escuchadas\n";
    std::cout << "A - Top 10 artistas más escuchados\n";
    std::cout << "X - Salir\n";
    std::cout << "Ingrese Opción: ";

    std::string opcion;
    std::getline(std::cin, opcion);
    if (opcion.empty()) return;

    char letra = static_cast<char>(std::toupper(static_cast<unsigned char>(opcion[0])));

    if (letra == 'C') {
        mostrarTop10Canciones();
    } else if (letra == 'A') {
        mostrarTop10Artistas();
    }
    // "X" o cualquier otra opción: vuelve al menú principal.
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

    archivo.close();
}

bool Reproductor::play() {
    if (!hayCancionesDisponibles()) return false;

    bool estabaSinCancion = (listaReproduccion.obtenerActual() == nullptr);
    reproduciendo = !reproduciendo;

    // Si estamos iniciando reproducción y no hay canción actual, tomar la primera
    if (reproduciendo && estabaSinCancion) {
        if (listaReproduccion.estaVacia()) {
            listaReproduccion.clonarDesde(registroTotal);
            if (modoAleatorio) {
                listaReproduccion.mezclar();
            }
            listaReproduccion.irAlInicio();
        }

        // Se cuenta como una reproducción nueva (no cada vez que se
        // pausa/reanuda el mismo tema con W). Se busca la entrada
        // maestra en registroTotal porque listaReproduccion solo
        // contiene copias (clonarDesde), y el ranking se lee siempre
        // desde registroTotal.
        Cancion* cancionActual = listaReproduccion.obtenerActual();
        if (cancionActual != nullptr) {
            Cancion* cancionMaestra = registroTotal.buscarPorId(cancionActual->getId());
            if (cancionMaestra != nullptr) {
                registrarReproduccion(*cancionMaestra);
            }
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
    Cancion* cancionActual = listaReproduccion.obtenerActual();
    if (cancionActual != nullptr) {
        Cancion* cancionMaestra = registroTotal.buscarPorId(cancionActual->getId());
        if (cancionMaestra != nullptr) {
            registrarReproduccion(*cancionMaestra);
        }
    }
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
    Cancion* cancionActual = listaReproduccion.obtenerActual();
    if (cancionActual != nullptr) {
        Cancion* cancionMaestra = registroTotal.buscarPorId(cancionActual->getId());
        if (cancionMaestra != nullptr) {
            registrarReproduccion(*cancionMaestra);
        }
    }
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
        reconstruirTries(); // Actualiza la búsqueda con la nueva canción

        std::cout << "Cancion agregada exitosamente.\n";
    }
    else if (opcion[0] == 'D' || opcion[0] == 'd') {
        int num = std::stoi(opcion.substr(1));
        if (registroTotal.eliminarPorPosicion(num)) {
            reconstruirTries(); // Actualiza la búsqueda tras la eliminación
            std::cout << "Cancion eliminada.\n";
        } else {
            std::cout << "Error al eliminar.\n";
        }
    }

    guardarEstado();
}

void Reproductor::searchSongs() {
    if (registroTotal.estaVacia()) {
        std::cout << "No hay canciones registradas.\n";
        return;
    }

    bool seguirBuscando = true;

    while (seguirBuscando) {
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "\nBusqueda de canciones\n";
        std::cout << "Buscar canciones que contengan: ";

        std::string termino;
        std::getline(std::cin, termino);

        // Si no se ingresó texto, se vuelve al menú principal.
        if (termino.empty()) {
            return;
        }

        int cantidadResultados = 0;
        Cancion** resultados = buscarCancionesPorTermino(termino, cantidadResultados);

        if (cantidadResultados == 0) {
            std::cout << "\nNo se encontraron canciones ni artistas que contengan \""
                      << termino << "\".\n";
            std::cout << "Presione Enter para volver a intentar, o escriba V para volver al menú: ";
            std::string respuesta;
            std::getline(std::cin, respuesta);
            if (!respuesta.empty() && (respuesta[0] == 'V' || respuesta[0] == 'v')) {
                delete[] resultados;
                return;
            }
            delete[] resultados;
            continue;
        }

        std::cout << "\nBusqueda de canciones\n";
        std::cout << "Canciones que contienen \"" << termino << "\":\n";
        for (int i = 0; i < cantidadResultados; ++i) {
            std::cout << " " << (i + 1) << ". " << resultados[i]->getNombre()
                      << " - " << resultados[i]->getArtista() << "\n";
        }

        std::cout << "\nOpciones:\n";
        std::cout << "R<num> - Reproducir canción seleccionada\n";
        std::cout << "A<num> - Agregar canción seleccionada al final de la lista de reproducción actual\n";
        std::cout << "F - Repetir búsqueda con un texto diferente\n";
        std::cout << "V - Volver al menú principal\n";
        std::cout << "Ingrese opción: ";

        std::string opcion;
        std::getline(std::cin, opcion);

        if (opcion.empty()) {
            delete[] resultados;
            continue;
        }

        char letra = static_cast<char>(std::toupper(static_cast<unsigned char>(opcion[0])));

        if (letra == 'V') {
            seguirBuscando = false;
        } else if (letra == 'F') {
            // No hace nada más: el while vuelve a pedir un texto nuevo.
        } else if (letra == 'R' || letra == 'A') {
            try {
                int num = std::stoi(opcion.substr(1));
                if (num >= 1 && num <= cantidadResultados) {
                    Cancion* seleccionada = resultados[num - 1];
                    if (letra == 'R') {
                        reproducirCancionEspecifica(*seleccionada);
                        std::cout << "\nReproduciendo: " << seleccionada->getNombre()
                                  << " - " << seleccionada->getArtista() << "\n";
                    } else {
                        listaReproduccion.agregarAlFinal(*seleccionada);
                        std::cout << "\nCanción agregada al final de la lista de reproducción.\n";
                    }
                    guardarEstado();
                } else {
                    std::cout << "\nNúmero fuera de rango.\n";
                }
            } catch (const std::exception&) {
                std::cout << "\nOpción inválida. Use el formato R<num> o A<num> (ej. R2).\n";
            }
            seguirBuscando = false;
        } else {
            std::cout << "\nOpción inválida.\n";
            seguirBuscando = false;
        }

        delete[] resultados;
    }
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