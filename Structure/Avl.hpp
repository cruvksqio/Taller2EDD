#ifndef AVL_HPP
#define AVL_HPP

template <typename T, typename Comparador>
class Avl {
private:
    struct NodoAVL {
        T dato;
        NodoAVL* izquierdo;
        NodoAVL* derecho;
        int altura;

        NodoAVL(const T& valor)
            : dato(valor), izquierdo(nullptr), derecho(nullptr), altura(1) {}
    };

    NodoAVL* raiz;
    Comparador esMenorQue;

    static int altura(NodoAVL* nodo) {
        return (nodo == nullptr) ? 0 : nodo->altura;
    }

    static int maximo(int a, int b) {
        return (a > b) ? a : b;
    }

    static void actualizarAltura(NodoAVL* nodo) {
        nodo->altura = 1 + maximo(altura(nodo->izquierdo), altura(nodo->derecho));
    }

    static int factorBalance(NodoAVL* nodo) {
        return (nodo == nullptr) ? 0 : altura(nodo->izquierdo) - altura(nodo->derecho);
    }

    static NodoAVL* rotarDerecha(NodoAVL* y) {
        NodoAVL* x = y->izquierdo;
        NodoAVL* subarbolT2 = x->derecho;

        x->derecho = y;
        y->izquierdo = subarbolT2;

        actualizarAltura(y);
        actualizarAltura(x);
        return x;
    }

    static NodoAVL* rotarIzquierda(NodoAVL* x) {
        NodoAVL* y = x->derecho;
        NodoAVL* subarbolT2 = y->izquierdo;

        y->izquierdo = x;
        x->derecho = subarbolT2;

        actualizarAltura(x);
        actualizarAltura(y);
        return y;
    }

    static NodoAVL* balancear(NodoAVL* nodo) {
        actualizarAltura(nodo);
        int balance = factorBalance(nodo);

        if (balance > 1) {
            if (factorBalance(nodo->izquierdo) < 0) {
                nodo->izquierdo = rotarIzquierda(nodo->izquierdo);
            }
            return rotarDerecha(nodo);
        }

        if (balance < -1) {
            if (factorBalance(nodo->derecho) > 0) {
                nodo->derecho = rotarDerecha(nodo->derecho);
            }
            return rotarIzquierda(nodo);
        }

        return nodo;
    }

    NodoAVL* insertarEn(NodoAVL* nodo, const T& valor, bool& seInserto) {
        if (nodo == nullptr) {
            seInserto = true;
            return new NodoAVL(valor);
        }

        if (esMenorQue(valor, nodo->dato)) {
            nodo->izquierdo = insertarEn(nodo->izquierdo, valor, seInserto);
        } else if (esMenorQue(nodo->dato, valor)) {
            nodo->derecho = insertarEn(nodo->derecho, valor, seInserto);
        } else {
            seInserto = false;
            return nodo;
        }

        return balancear(nodo);
    }

    NodoAVL* buscarEn(NodoAVL* nodo, const T& clave) const {
        if (nodo == nullptr) return nullptr;
        if (esMenorQue(clave, nodo->dato)) return buscarEn(nodo->izquierdo, clave);
        if (esMenorQue(nodo->dato, clave)) return buscarEn(nodo->derecho, clave);
        return nodo;
    }

    static void liberarNodo(NodoAVL* nodo) {
        if (nodo == nullptr) return;
        liberarNodo(nodo->izquierdo);
        liberarNodo(nodo->derecho);
        delete nodo;
    }

    static int contarEn(NodoAVL* nodo) {
        if (nodo == nullptr) return 0;
        return 1 + contarEn(nodo->izquierdo) + contarEn(nodo->derecho);
    }

    template <typename Func>
    static void inOrdenEn(NodoAVL* nodo, Func callback) {
        if (nodo == nullptr) return;
        inOrdenEn(nodo->izquierdo, callback);
        callback(nodo->dato);
        inOrdenEn(nodo->derecho, callback);
    }

public:
    Avl() : raiz(nullptr) {}

    ~Avl() {
        liberarNodo(raiz);
    }

    Avl(const Avl&) = delete;
    Avl& operator=(const Avl&) = delete;

    void vaciarTodo() {
        liberarNodo(raiz);
        raiz = nullptr;
    }

    bool insertar(const T& valor) {
        bool seInserto = true;
        raiz = insertarEn(raiz, valor, seInserto);
        return seInserto;
    }

    T* buscar(const T& clave) {
        NodoAVL* nodo = buscarEn(raiz, clave);
        return (nodo == nullptr) ? nullptr : &(nodo->dato);
    }

    const T* buscar(const T& clave) const {
        NodoAVL* nodo = buscarEn(raiz, clave);
        return (nodo == nullptr) ? nullptr : &(nodo->dato);
    }

    bool estaVacio() const {
        return raiz == nullptr;
    }

    int obtenerCantidad() const {
        return contarEn(raiz);
    }

    template <typename Func>
    void recorridoInOrden(Func callback) const {
        inOrdenEn(raiz, callback);
    }
};

#endif // AVL_HPP