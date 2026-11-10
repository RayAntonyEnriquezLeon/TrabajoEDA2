#include "logistica/RegistroPedidos.hpp"
#include <limits>
#include <stdexcept>

namespace logistica {
RegistroPedidos::~RegistroPedidos() {
    vaciar();
    delete[] pedidos_;
}
void RegistroPedidos::ampliar() {
    if (capacidad_ > std::numeric_limits<std::size_t>::max() / 2 / sizeof(Pedido*))
        throw std::length_error("Registro demasiado grande.");
    const std::size_t nuevaCapacidad = capacidad_ == 0 ? 64 : capacidad_ * 2;
    Pedido** nuevo = new Pedido*[nuevaCapacidad];
    for (std::size_t i = 0; i < cantidad_; ++i) nuevo[i] = pedidos_[i];
    delete[] pedidos_;
    pedidos_ = nuevo;
    capacidad_ = nuevaCapacidad;
}
Pedido* RegistroPedidos::crear(const DatosPedido& datos) {
    if (cantidad_ == capacidad_) ampliar();
    Pedido* pedido = new Pedido(datos);
    pedido->posicionRegistro_ = cantidad_;
    pedidos_[cantidad_++] = pedido;
    return pedido;
}
bool RegistroPedidos::liberar(Pedido* pedido) noexcept {
    if (!pedido || pedido->posicionRegistro_ >= cantidad_ ||
        pedidos_[pedido->posicionRegistro_] != pedido) return false;
    const std::size_t posicion = pedido->posicionRegistro_;
    --cantidad_;
    if (posicion != cantidad_) {
        pedidos_[posicion] = pedidos_[cantidad_];
        pedidos_[posicion]->posicionRegistro_ = posicion;
    }
    delete pedido;
    return true;
}
void RegistroPedidos::vaciar() noexcept {
    for (std::size_t i = 0; i < cantidad_; ++i) delete pedidos_[i];
    cantidad_ = 0;
}
}
