#ifndef NODO_HPP
#define NODO_HPP

template <typename T>
class Nodo {
public:
    T dato;
    Nodo* siguiente;
    Nodo* anterior;

    Nodo(T valor) : dato(valor), siguiente(nullptr), anterior(nullptr) {}
};

#endif