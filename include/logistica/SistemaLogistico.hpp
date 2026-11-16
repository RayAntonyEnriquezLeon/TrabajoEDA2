#pragma once

#include "logistica/ArbolAVL.hpp"
#include "logistica/ContratosIndices.hpp"
#include "logistica/RegistroPedidos.hpp"
#include <cstddef>
#include <cstdint>
#include <string>

namespace logistica {

enum class CodigoResultado {
    Correcto, DatosInvalidos, IdDuplicado, NoEncontrado,
    TransicionInvalida, ErrorIndice, SinMemoria
};

struct ResultadoOperacion {
    CodigoResultado codigo = CodigoResultado::Correcto;
    std::string detalle;
    bool correcto() const noexcept { return codigo == CodigoResultado::Correcto; }
};

struct TiemposInsercion {
    double hashMs = 0;
    double avlMs = 0;
    double heapMs = 0;
};

struct Estadisticas {
    std::size_t totalPedidos = 0;
    std::size_t pendientes = 0;
    std::int64_t montoTotalCentavos = 0;
};

class SistemaLogistico {
public:
    // Los indices deben estar vacios y sobrevivir al sistema que se presenta
    SistemaLogistico(IIndicePedidos& hash, IColaDespacho& heap);
    ~SistemaLogistico();
    SistemaLogistico(const SistemaLogistico&) = delete;
    SistemaLogistico& operator=(const SistemaLogistico&) = delete;

    ResultadoOperacion registrarPedido(const DatosPedido& datos, TiemposInsercion* tiempos = nullptr);
    const Pedido* buscarPedido(const std::string& idPedido) const noexcept;
    ResultadoOperacion actualizarEstado(const std::string& idPedido, EstadoPedido estado);
    ResultadoOperacion cancelarPedido(const std::string& idPedido);

    // Escribe como maximo capacidad referencias; nullptr no despacha.
    std::size_t despacharLote(std::size_t solicitados, const Pedido** salida, std::size_t capacidad);
    std::size_t consultarOrdenados(const Pedido** salida, std::size_t capacidad) const noexcept;
    ResultadoOperacion consultarRango(const std::string& desde, const std::string& hasta,
                                      ResumenRango& resultado) const;
    Estadisticas obtenerEstadisticas() const noexcept;
    bool verificarAVL() const noexcept { return avl_.verificarInvariantes(); }
    void vaciar() noexcept;

    // Cancelar, vaciar o destruir el sistema invalida las referencias correspondientes.

private:
    IIndicePedidos& hash_;
    IColaDespacho& heap_;
    RegistroPedidos registro_;
    ArbolAVL avl_;
    std::int64_t montoTotalCentavos_ = 0;
};

}
