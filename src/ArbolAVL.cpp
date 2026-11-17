#include "logistica/ArbolAVL.hpp"
#include <algorithm>

namespace logistica {
ArbolAVL::~ArbolAVL() { vaciar(); }
int ArbolAVL::altura(const Nodo* nodo) noexcept { return nodo ? nodo->altura : 0; }
int ArbolAVL::balance(const Nodo* nodo) noexcept {
    return nodo ? altura(nodo->izquierdo) - altura(nodo->derecho) : 0;
}
void ArbolAVL::actualizarAltura(Nodo* nodo) noexcept {
    nodo->altura = 1 + std::max(altura(nodo->izquierdo), altura(nodo->derecho));
}
ArbolAVL::Nodo* ArbolAVL::rotarDerecha(Nodo* nodo) noexcept {
    Nodo* nuevaRaiz = nodo->izquierdo;
    nodo->izquierdo = nuevaRaiz->derecho;
    nuevaRaiz->derecho = nodo;
    actualizarAltura(nodo);
    actualizarAltura(nuevaRaiz);
    return nuevaRaiz;
}
ArbolAVL::Nodo* ArbolAVL::rotarIzquierda(Nodo* nodo) noexcept {
    Nodo* nuevaRaiz = nodo->derecho;
    nodo->derecho = nuevaRaiz->izquierdo;
    nuevaRaiz->izquierdo = nodo;
    actualizarAltura(nodo);
    actualizarAltura(nuevaRaiz);
    return nuevaRaiz;
}
ArbolAVL::Nodo* ArbolAVL::equilibrar(Nodo* nodo) noexcept {
    if (!nodo) return nullptr;
    actualizarAltura(nodo);
    if (balance(nodo) > 1) {
        if (balance(nodo->izquierdo) < 0) nodo->izquierdo = rotarIzquierda(nodo->izquierdo);
        return rotarDerecha(nodo);
    }
    if (balance(nodo) < -1) {
        if (balance(nodo->derecho) > 0) nodo->derecho = rotarDerecha(nodo->derecho);
        return rotarIzquierda(nodo);
    }
    return nodo;
}
ArbolAVL::Nodo* ArbolAVL::insertarNodo(Nodo* nodo, Pedido* pedido, bool& insertado) {
    if (!nodo) {
        Nodo* nuevo = new Nodo(pedido);
        insertado = true;
        return nuevo;
    }
    const int comparacion = compararCronologicamente(*pedido, *nodo->pedido);
    if (comparacion < 0) nodo->izquierdo = insertarNodo(nodo->izquierdo, pedido, insertado);
    else if (comparacion > 0) nodo->derecho = insertarNodo(nodo->derecho, pedido, insertado);
    else return nodo;
    return equilibrar(nodo);
}








bool ArbolAVL::insertar(Pedido* pedido) {
    if (!pedido) return false;
    bool insertado = false;
    raiz_ = insertarNodo(raiz_, pedido, insertado);
    if (insertado) ++cantidad_;
    return insertado;
}
ArbolAVL::Nodo* ArbolAVL::eliminarNodo(Nodo* nodo, const Pedido& pedido, bool& eliminado) noexcept {
    if (!nodo) return nullptr;
    const int comparacion = compararCronologicamente(pedido, *nodo->pedido);
    if (comparacion < 0) nodo->izquierdo = eliminarNodo(nodo->izquierdo, pedido, eliminado);
    else if (comparacion > 0) nodo->derecho = eliminarNodo(nodo->derecho, pedido, eliminado);
    else {
        eliminado = true;
        if (!nodo->izquierdo || !nodo->derecho) {
            Nodo* hijo = nodo->izquierdo ? nodo->izquierdo : nodo->derecho;
            delete nodo;
            return hijo;
        }
        Nodo* sucesor = nodo->derecho;
        while (sucesor->izquierdo) sucesor = sucesor->izquierdo;

        nodo->pedido = sucesor->pedido;
        bool retirado = false;
        nodo->derecho = eliminarNodo(nodo->derecho, *sucesor->pedido, retirado);
    }
    return equilibrar(nodo);
}
bool ArbolAVL::eliminar(const Pedido& pedido) noexcept {
    bool eliminado = false;
    raiz_ = eliminarNodo(raiz_, pedido, eliminado);
    if (eliminado) --cantidad_;
    return eliminado;
}
Pedido* ArbolAVL::buscar(const std::string& fecha, const std::string& hora,
                        const std::string& idPedido) const noexcept {
    const Nodo* nodo = raiz_;
    while (nodo) {
        const int orden = compararCronologicamente(fecha, hora, idPedido, *nodo->pedido);
        if (orden == 0) return nodo->pedido;
        nodo = orden < 0 ? nodo->izquierdo : nodo->derecho;
    }
    return nullptr;
}
void ArbolAVL::recorrerPrimeros(const Nodo* nodo, const Pedido** salida,
                              std::size_t capacidad, std::size_t& escritos) noexcept {
    if (!nodo || escritos == capacidad) return;
    recorrerPrimeros(nodo->izquierdo, salida, capacidad, escritos);
    if (escritos == capacidad) return;
    salida[escritos++] = nodo->pedido;
    recorrerPrimeros(nodo->derecho, salida, capacidad, escritos);
}
std::size_t ArbolAVL::obtenerPrimeros(const Pedido** salida, std::size_t capacidad) const noexcept {
    if (!salida) return 0;
    std::size_t escritos = 0;
    recorrerPrimeros(raiz_, salida, capacidad, escritos);
    return escritos;
}
void ArbolAVL::acumularRango(const Nodo* nodo, const std::string& desde,
                          const std::string& hasta, ResumenRango& resultado) noexcept {
    if (!nodo) return;
    const auto& datos = nodo->pedido->datos();
    if (datos.fecha >= desde) acumularRango(nodo->izquierdo, desde, hasta, resultado);
    if (datos.fecha >= desde && datos.fecha <= hasta) {
        ++resultado.cantidad;
        resultado.montoTotalCentavos += datos.montoCentavos;
    }
    if (datos.fecha <= hasta) acumularRango(nodo->derecho, desde, hasta, resultado);
}
ResumenRango ArbolAVL::consultarRango(const std::string& desde, const std::string& hasta) const noexcept {
    ResumenRango resultado;
    if (desde <= hasta) acumularRango(raiz_, desde, hasta, resultado);
    return resultado;
}
void ArbolAVL::liberarNodos(Nodo* nodo) noexcept {
    if (!nodo) return;
    liberarNodos(nodo->izquierdo);
    liberarNodos(nodo->derecho);
    delete nodo;
}
void ArbolAVL::vaciar() noexcept {
    liberarNodos(raiz_);
    raiz_ = nullptr;
    cantidad_ = 0;
}
bool ArbolAVL::verificarNodo(const Nodo* nodo, const Pedido* menor, const Pedido* mayor,
                           int& alturaReal, std::size_t& cantidadReal) noexcept {
    if (!nodo) { alturaReal = 0; cantidadReal = 0; return true; }
    if (!nodo->pedido || (menor && compararCronologicamente(*menor, *nodo->pedido) >= 0) ||
        (mayor && compararCronologicamente(*nodo->pedido, *mayor) >= 0)) return false;
    int izquierda = 0, derecha = 0;
    std::size_t cantidadIzquierda = 0, cantidadDerecha = 0;
    if (!verificarNodo(nodo->izquierdo, menor, nodo->pedido, izquierda, cantidadIzquierda) ||
        !verificarNodo(nodo->derecho, nodo->pedido, mayor, derecha, cantidadDerecha)) return false;
    alturaReal = 1 + std::max(izquierda, derecha);
    cantidadReal = 1 + cantidadIzquierda + cantidadDerecha;
    return nodo->altura == alturaReal && izquierda - derecha >= -1 && izquierda - derecha <= 1;
}
bool ArbolAVL::verificarInvariantes() const noexcept {
    int alturaReal = 0;
    std::size_t cantidadReal = 0;
    return verificarNodo(raiz_, nullptr, nullptr, alturaReal, cantidadReal) && cantidadReal == cantidad_;
}
}
