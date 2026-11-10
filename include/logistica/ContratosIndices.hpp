#pragma once

#include "logistica/Pedido.hpp"
#include <cstddef>
#include <string>

namespace logistica {

// Reglas de implementacion y conexion: INTEGRACION.md, si van a modificar aglo de aqui avisa por el grupo

class IIndicePedidos {
public:
    virtual ~IIndicePedidos() = default;
    virtual bool insertar(Pedido* pedido) = 0;
    virtual Pedido* buscar(const std::string& idPedido) const noexcept = 0;
    virtual bool eliminar(const std::string& idPedido) noexcept = 0;
    virtual std::size_t cantidad() const noexcept = 0;
    virtual void vaciar() noexcept = 0;
};

class IColaDespacho {
public:
    virtual ~IColaDespacho() = default;
    virtual bool insertar(Pedido* pedido) = 0;
    virtual const Pedido* verMinimo() const noexcept = 0;
    virtual Pedido* extraerMinimo() noexcept = 0;
    virtual bool eliminar(Pedido* pedido) noexcept = 0;
    virtual std::size_t cantidad() const noexcept = 0;
    virtual void vaciar() noexcept = 0;

protected:
    static void fijarPosicion(Pedido& pedido, int posicion) noexcept {
        pedido.posicionHeap_ = posicion;
    }
};

}
