#include "logistica/TablaHash.hpp"

#include <cstdint>
#include <limits>
#include <stdexcept>

namespace logistica {

namespace {
constexpr std::size_t kCapacidadInicial = 1024;
}

TablaHash::TablaHash() : cubetas_(nullptr), capacidad_(kCapacidadInicial), cantidad_(0) {
    cubetas_ = new Nodo*[capacidad_];
    for (std::size_t i = 0; i < capacidad_; ++i) cubetas_[i] = nullptr;
}

TablaHash::~TablaHash() {
    vaciar();
    delete[] cubetas_;
}

std::size_t TablaHash::dispersar(const std::string& clave) noexcept {
    std::uint64_t h = 14695981039346656037ULL;
    for (unsigned char c : clave) {
        h ^= c;
        h *= 1099511628211ULL;
    }
    return static_cast<std::size_t>(h);
}

void TablaHash::redimensionar() {
    if (capacidad_ > std::numeric_limits<std::size_t>::max() / sizeof(Nodo*) / 2)
        throw std::length_error("Tabla Hash demasiado grande.");
    const std::size_t nuevaCapacidad = capacidad_ * 2;
    Nodo** nuevas = new Nodo*[nuevaCapacidad];
    for (std::size_t i = 0; i < nuevaCapacidad; ++i) nuevas[i] = nullptr;

    for (std::size_t i = 0; i < capacidad_; ++i) {
        Nodo* actual = cubetas_[i];
        while (actual) {
            Nodo* siguiente = actual->siguiente;
            const std::size_t destino = dispersar(actual->pedido->datos().idPedido) & (nuevaCapacidad - 1);
            actual->siguiente = nuevas[destino];
            nuevas[destino] = actual;
            actual = siguiente;
        }
    }
    delete[] cubetas_;
    cubetas_ = nuevas;
    capacidad_ = nuevaCapacidad;
}

bool TablaHash::insertar(Pedido* pedido) {
    if (!pedido) return false;
    const std::string& id = pedido->datos().idPedido;

    if (buscar(id)) return false;

    if (cantidad_ >= capacidad_ - capacidad_ / 4) redimensionar();

    Nodo* nodo = new Nodo{pedido, nullptr};
    const std::size_t indice = dispersar(id) & (capacidad_ - 1);
    nodo->siguiente = cubetas_[indice];
    cubetas_[indice] = nodo;
    ++cantidad_;
    return true;
}

Pedido* TablaHash::buscar(const std::string& idPedido) const noexcept {
    const std::size_t indice = dispersar(idPedido) & (capacidad_ - 1);
    for (Nodo* actual = cubetas_[indice]; actual; actual = actual->siguiente) {
        if (actual->pedido->datos().idPedido == idPedido) return actual->pedido;
    }
    return nullptr;
}

bool TablaHash::eliminar(const std::string& idPedido) noexcept {
    const std::size_t indice = dispersar(idPedido) & (capacidad_ - 1);
    Nodo* anterior = nullptr;
    for (Nodo* actual = cubetas_[indice]; actual; anterior = actual, actual = actual->siguiente) {
        if (actual->pedido->datos().idPedido == idPedido) {
            if (anterior) anterior->siguiente = actual->siguiente;
            else cubetas_[indice] = actual->siguiente;
            delete actual;
            --cantidad_;
            return true;
        }
    }
    return false;
}

std::size_t TablaHash::cantidad() const noexcept {
    return cantidad_;
}

void TablaHash::vaciar() noexcept {
    for (std::size_t i = 0; i < capacidad_; ++i) {
        Nodo* actual = cubetas_[i];
        while (actual) {
            Nodo* siguiente = actual->siguiente;
            delete actual;
            actual = siguiente;
        }
        cubetas_[i] = nullptr;
    }
    cantidad_ = 0;
}

std::size_t TablaHash::cubetasConColision() const noexcept {
    std::size_t total = 0;
    for (std::size_t i = 0; i < capacidad_; ++i) {
        if (cubetas_[i] && cubetas_[i]->siguiente) ++total;
    }
    return total;
}

std::size_t TablaHash::longitudMaximaCadena() const noexcept {
    std::size_t maxima = 0;
    for (std::size_t i = 0; i < capacidad_; ++i) {
        std::size_t largo = 0;
        for (Nodo* actual = cubetas_[i]; actual; actual = actual->siguiente) ++largo;
        if (largo > maxima) maxima = largo;
    }
    return maxima;
}

}
