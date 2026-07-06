#ifndef HEAP_HPP
#define HEAP_HPP

// Arbol Heap binario (representado con un arreglo, como es tradicional),
// usado para construir los rankings TOP 10 de canciones y artistas.
//
// No utiliza contenedores STL: el arreglo interno se maneja con new/delete
// crudo (igual estilo que SongList::mezclar()).
//
// "Comparador" es un funtor (struct con operator()) que recibe dos
// elementos (a, b) y devuelve true si "a" tiene MAYOR prioridad que "b",
// es decir, si "a" debería quedar más arriba en el heap y salir primero.
// Para un ranking de "más reproducido" eso significa comparar primero por
// contador de reproducciones (descendente) y, en caso de empate, por
// orden alfabético (tal como exige la pauta).
template <typename T, typename Comparador>
class Heap {
private:
    T* datos;
    int capacidad;
    int cantidad;
    Comparador tieneMayorPrioridad;

    void asegurarCapacidad() {
        if (cantidad < capacidad) return;
        int nuevaCapacidad = (capacidad == 0) ? 8 : capacidad * 2;
        T* nuevoArreglo = new T[nuevaCapacidad];
        for (int i = 0; i < cantidad; ++i) {
            nuevoArreglo[i] = datos[i];
        }
        delete[] datos;
        datos = nuevoArreglo;
        capacidad = nuevaCapacidad;
    }

    void intercambiar(int i, int j) {
        T temp = datos[i];
        datos[i] = datos[j];
        datos[j] = temp;
    }

    // Sube el elemento en "indice" mientras tenga mayor prioridad que su padre.
    void subir(int indice) {
        while (indice > 0) {
            int padre = (indice - 1) / 2;
            if (tieneMayorPrioridad(datos[indice], datos[padre])) {
                intercambiar(indice, padre);
                indice = padre;
            } else {
                break;
            }
        }
    }

    // Baja el elemento en "indice" mientras alguno de sus hijos tenga
    // mayor prioridad que él.
    void bajar(int indice) {
        while (true) {
            int izquierdo = 2 * indice + 1;
            int derecho = 2 * indice + 2;
            int mayor = indice;

            if (izquierdo < cantidad && tieneMayorPrioridad(datos[izquierdo], datos[mayor])) {
                mayor = izquierdo;
            }
            if (derecho < cantidad && tieneMayorPrioridad(datos[derecho], datos[mayor])) {
                mayor = derecho;
            }

            if (mayor == indice) break;
            intercambiar(indice, mayor);
            indice = mayor;
        }
    }

public:
    Heap() : datos(nullptr), capacidad(0), cantidad(0) {}

    ~Heap() {
        delete[] datos;
    }

    Heap(const Heap&) = delete;
    Heap& operator=(const Heap&) = delete;

    void insertar(const T& valor) {
        asegurarCapacidad();
        datos[cantidad] = valor;
        subir(cantidad);
        cantidad++;
    }

    bool estaVacio() const {
        return cantidad == 0;
    }

    int obtenerCantidad() const {
        return cantidad;
    }

    // Extrae (quita y devuelve por referencia) el elemento de mayor
    // prioridad actual. Devuelve false si el heap está vacío.
    bool extraerMayor(T& resultado) {
        if (cantidad == 0) return false;

        resultado = datos[0];
        cantidad--;
        datos[0] = datos[cantidad];
        if (cantidad > 0) {
            bajar(0);
        }
        return true;
    }
};

#endif // HEAP_HPP
