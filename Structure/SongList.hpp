#ifndef SONGLIST_HPP
#define SONGLIST_HPP

#include "Nodo.hpp"
#include "../Clases/Cancion.hpp"
#include <iostream>
#include <sstream>


template <typename T>
class SongList {
private:
    Nodo<T>* cabeza;
    Nodo<T>* cola;
    Nodo<T>* actual; // puntero para saber que esta sonando
    int longitud;

public:
    // Constructor
    SongList() : cabeza(nullptr), cola(nullptr), actual(nullptr), longitud(0) {}

    // Destructor
    ~SongList() {
        vaciar();
    }

    // Métodos de inserción
    void agregarAlFinal(T valor) {
        Nodo<T>* nuevoNodo = new Nodo<T>(valor);

        if (cabeza == nullptr) {
            cabeza = nuevoNodo;
            cola = nuevoNodo;
            actual = nuevoNodo;
        } else {
            cola->siguiente = nuevoNodo;
            nuevoNodo->anterior = cola;
            cola = nuevoNodo;
        }
        longitud++;
    }

    void agregarAlInicio(T valor) {
        Nodo<T>* nuevoNodo = new Nodo<T>(valor);

        if (cabeza == nullptr) {
            cabeza = nuevoNodo;
            cola = nuevoNodo;
            actual = nuevoNodo;
        } else {
            nuevoNodo->siguiente = cabeza;
            cabeza->anterior = nuevoNodo;
            cabeza = nuevoNodo;
        }
        longitud++;
    }

    // Main methods pala reproduccion
    bool avanzar() {
        if (actual != nullptr && actual->siguiente != nullptr) {
            actual = actual->siguiente;
            return true;
        }
        return false;
    }

    bool retroceder() {
        if (actual != nullptr && actual->anterior != nullptr) {
            actual = actual->anterior;
            return true;
        }
        return false;
    }

    T* obtenerActual() {
        if (actual != nullptr) {
            return &(actual->dato);
        }
        return nullptr;
    }

    const T* obtenerActual() const {
        if (actual != nullptr) {
            return &(actual->dato);
        }
        return nullptr;
    }

    // Métodos de utilidad
    bool estaVacia() const {
        return longitud == 0;
    }

    int obtenerLongitud() const {
        return longitud;
    }

    void vaciar() {
        Nodo<T>* temp = cabeza;
        while (temp != nullptr) {
            Nodo<T>* siguiente = temp->siguiente;
            delete temp;
            temp = siguiente;
        }
        cabeza = nullptr;
        cola = nullptr;
        actual = nullptr;
        longitud = 0;
    }

    void imprimirLista() const {
        if (estaVacia()) {
            std::cout << "Lista vacía\n";
            return;
        }
        Nodo<T>* temp = cabeza;
        int contador = 1;
        while (temp != nullptr) {
            std::cout << contador << ". ";
            temp->dato.imprimirDatos();
            temp = temp->siguiente;
            contador++;
        }
    }

    // Método para saltar a una canción específica
    bool saltarA(int posicion) {
        if (posicion < 1 || posicion > longitud) {
            return false;
        }

        Nodo<T>* temp = cabeza;
        for (int i = 1; i < posicion; ++i) {
            temp = temp->siguiente;
        }

        actual = temp;
        return true;
    }

    // Método para buscar por ID
    T* buscarPorId(int id) {
        Nodo<T>* temp = cabeza;
        while (temp != nullptr) {
            if (temp->dato.getId() == id) {
                return &(temp->dato);
            }
            temp = temp->siguiente;
        }
        return nullptr;
    }

    // Método para eliminar por posición
    bool eliminarPorPosicion(int posicion) {
        if (posicion < 1 || posicion > longitud) {
            return false;
        }

        Nodo<T>* temp = cabeza;
        for (int i = 1; i < posicion; ++i) {
            temp = temp->siguiente;
        }

        if (temp == actual) {
            actual = (temp->siguiente != nullptr) ? temp->siguiente : temp->anterior;
        }

        if (temp == cabeza) {
            cabeza = temp->siguiente;
            if (cabeza != nullptr) cabeza->anterior = nullptr;
        } else {
            temp->anterior->siguiente = temp->siguiente;
        }

        if (temp == cola) {
            cola = temp->anterior;
            if (cola != nullptr) cola->siguiente = nullptr;
        } else {
            temp->siguiente->anterior = temp->anterior;
        }

        delete temp;
        longitud--;
        return true;
    }

    // Método para mezclar la lista (modo aleatorio)
    void mezclar() {
        if (longitud <= 1) return;

        // Crear un array temporal para mezclar
        T* elementos = new T[longitud];
        Nodo<T>* temp = cabeza;

        for (int i = 0; i < longitud; i++) {
            elementos[i] = temp->dato;
            temp = temp->siguiente;
        }


        for (int i = longitud - 1; i > 0; i--) {
            int j = rand() % (i + 1);
            T tempElemento = elementos[i];
            elementos[i] = elementos[j];
            elementos[j] = tempElemento;
        }

        // Reconstruir la lista con los elementos mezclados
        vaciar();
        for (int i = 0; i < longitud + 1; i++) {  // Nota: necesitamos reconstruir
            // Esta implementación necesita ajuste según el contexto
        }

        delete[] elementos;
    }

    // Método para obtener IDs como string (para guardar en archivo)
    std::string obtenerIdsComoString() const {
        if (estaVacia()) return "VACIA";

        std::stringstream ss;
        Nodo<T>* temp = cabeza;
        bool first = true;

        while (temp != nullptr) {
            if (!first) ss << ",";
            ss << temp->dato.getId();
            first = false;
            temp = temp->siguiente;
        }

        return ss.str();
    }

    // Metodo para ir al inicio de la lista
    void irAlInicio() {
        actual = cabeza;
    }

    // Metodo para ir al final de la lista
    void irAlFinal() {
        actual = cola;
    }

    // Metodo para clonar desde otra lista
    void clonarDesde(const SongList<T>& otra) {
        vaciar();
        Nodo<T>* temp = otra.cabeza;
        while (temp != nullptr) {
            agregarAlFinal(temp->dato);
            temp = temp->siguiente;
        }
    }
};

#endif