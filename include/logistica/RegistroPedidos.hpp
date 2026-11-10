#pragma once

#include "logistica/Pedido.hpp"
#include <cstddef>

namespace logistica {

// Almacen propietario, no es un indice de busqueda ni reemplaza al Hash
class RegistroPedidos {
public:
    RegistroPedidos() = default;
    ~RegistroPedidos();
    RegistroPedidos(const RegistroPedidos&) = delete;
    RegistroPedidos& operator=(const RegistroPedidos&) = delete;

    Pedido* crear(const DatosPedido& datos);
    bool liberar(Pedido* pedido) noexcept;
    void vaciar() noexcept;
    std::size_t cantidad() const noexcept { return cantidad_; }

private:
    void ampliar();
    Pedido** pedidos_ = nullptr;
    std::size_t cantidad_ = 0;
    std::size_t capacidad_ = 0;
};

}
