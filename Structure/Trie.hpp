#ifndef TRIE_HPP
#define TRIE_HPP

#include <string>
#include <cctype>

// Arbol Trie (con sufijos) para busqueda de canciones/artistas.
// No utiliza contenedores STL (vector, array, list, etc.): los hijos se
// guardan en un arreglo crudo de punteros y los valores en una lista
// enlazada simple implementada a mano, igual que el resto del proyecto.
//
// Para poder encontrar coincidencias que EMPIEZAN, TERMINAN o CONTIENEN
// el texto buscado (y no solo prefijos), al insertar una palabra se
// insertan TODOS sus sufijos. De esa forma, buscar "is" en "This is a test"
// funciona porque el sufijo "is a test" (y "is") quedan indexados desde
// la raiz.
template <typename Value>
class Trie {
private:
    static const int ALFABETO = 256;

    // Lista enlazada simple manual para los valores asociados a un nodo.
    struct NodoValor {
        Value valor;
        NodoValor* siguiente;
        NodoValor(const Value& v) : valor(v), siguiente(nullptr) {}
    };

    struct Nodo {
        Nodo* hijos[ALFABETO];
        NodoValor* valores;

        Nodo() : valores(nullptr) {
            for (int i = 0; i < ALFABETO; ++i) {
                hijos[i] = nullptr;
            }
        }
    };

    Nodo* raiz;

    static unsigned char normalizar(char c) {
        return static_cast<unsigned char>(std::tolower(static_cast<unsigned char>(c)));
    }

    // Evita insertar el mismo valor dos veces en el mismo nodo (puede pasar
    // porque varios sufijos de una misma palabra comparten nodos).
    static bool contieneValor(NodoValor* cabeza, const Value& valor) {
        NodoValor* actual = cabeza;
        while (actual != nullptr) {
            if (actual->valor == valor) return true;
            actual = actual->siguiente;
        }
        return false;
    }

    static void agregarValorUnico(Nodo* nodo, const Value& valor) {
        if (contieneValor(nodo->valores, valor)) return;
        NodoValor* nuevo = new NodoValor(valor);
        nuevo->siguiente = nodo->valores;
        nodo->valores = nuevo;
    }

    static void liberarValores(NodoValor* cabeza) {
        while (cabeza != nullptr) {
            NodoValor* siguiente = cabeza->siguiente;
            delete cabeza;
            cabeza = siguiente;
        }
    }

    static void liberarNodo(Nodo* nodo) {
        if (nodo == nullptr) return;
        for (int i = 0; i < ALFABETO; ++i) {
            liberarNodo(nodo->hijos[i]);
        }
        liberarValores(nodo->valores);
        delete nodo;
    }

    // Inserta el sufijo de "clave" que comienza en "inicio", asociando
    // "valor" a cada nodo del camino recorrido.
    void insertarDesde(const std::string& clave, size_t inicio, const Value& valor) {
        Nodo* nodo = raiz;
        for (size_t i = inicio; i < clave.size(); ++i) {
            unsigned char idx = normalizar(clave[i]);
            if (nodo->hijos[idx] == nullptr) {
                nodo->hijos[idx] = new Nodo();
            }
            nodo = nodo->hijos[idx];
            agregarValorUnico(nodo, valor);
        }
    }

public:
    Trie() : raiz(new Nodo()) {}

    ~Trie() {
        liberarNodo(raiz);
    }

    Trie(const Trie&) = delete;
    Trie& operator=(const Trie&) = delete;

    // Reinicia el arbol (se usa al recargar canciones desde archivo).
    void vaciarTodo() {
        liberarNodo(raiz);
        raiz = new Nodo();
    }

    // Inserta la palabra completa junto con TODOS sus sufijos, para que la
    // busqueda posterior encuentre coincidencias en cualquier posicion.
    void insertar(const std::string& clave, const Value& valor) {
        for (size_t inicio = 0; inicio < clave.size(); ++inicio) {
            insertarDesde(clave, inicio, valor);
        }
    }

    // Recorre el arbol siguiendo "texto" caracter a caracter y entrega
    // (via callback) cada valor asociado al nodo final, si existe.
    // No retorna un contenedor STL: el llamador decide donde acumular
    // los resultados.
    template <typename Func>
    void buscarCoincidencias(const std::string& texto, Func callback) const {
        if (texto.empty()) return;

        Nodo* nodo = raiz;
        for (char ch : texto) {
            unsigned char idx = normalizar(ch);
            nodo = nodo->hijos[idx];
            if (nodo == nullptr) return;
        }

        NodoValor* actual = nodo->valores;
        while (actual != nullptr) {
            callback(actual->valor);
            actual = actual->siguiente;
        }
    }
};

#endif // TRIE_HPP
