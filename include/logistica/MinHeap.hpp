#pragma once
#include "logistica/ContratosIndices.hpp"

namespace logistica {
class MinHeap final : public IColaDespacho {
public:
    MinHeap();
    ~MinHeap() override;
    MinHeap(const MinHeap&) = delete;
    MinHeap& operator=(const MinHeap&) = delete;

    bool insertar(Pedido* pedido) override;
    const Pedido* verMinimo() const noexcept override;
    Pedido* extraerMinimo() noexcept override;
    bool eliminar(Pedido* pedido) noexcept override;
    std::size_t cantidad() const noexcept override;
    void vaciar() noexcept override;

private:
    Pedido** arr;
    int capacidad;
    int cantidadNodos;

    void siftUp(int actual);
    void siftDownMin(int i, int n);
    void redimensionar();
};
}