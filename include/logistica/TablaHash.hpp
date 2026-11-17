#pragma once

#include "logistica/ContratosIndices.hpp"

namespace logistica {
class TablaHash final : public IIndicePedidos {
public:
    TablaHash();
    ~TablaHash() override;
    TablaHash(const TablaHash&) = delete;
    TablaHash& operator=(const TablaHash&) = delete;

    bool insertar(Pedido* pedido) override;
    Pedido* buscar(const std::string& idPedido) const noexcept override;
    bool eliminar(const std::string& idPedido) noexcept override;
    std::size_t cantidad() const noexcept override;
    void vaciar() noexcept override;

    std::size_t capacidad() const noexcept { return capacidad_; }
    std::size_t cubetasConColision() const noexcept;
    std::size_t longitudMaximaCadena() const noexcept;

private:
    struct Nodo {
        Pedido* pedido;
        Nodo* siguiente;
    };

    static std::size_t dispersar(const std::string& clave) noexcept;
    void redimensionar();

    Nodo** cubetas_;
    std::size_t capacidad_;
    std::size_t cantidad_;
};
}
