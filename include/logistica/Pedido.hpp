#pragma once

#include <cstddef>
#include <cstdint>
#include <string>

namespace logistica {

enum class TipoEnvio { Normal, Express, SameDay };
enum class EstadoPedido { Pendiente, EnTransito, Entregado, Cancelado };

struct DatosPedido {
    std::string idPedido;
    std::string dniCliente;
    TipoEnvio tipoEnvio = TipoEnvio::Normal;
    int nivelPrioridad = 1;
    std::int64_t montoCentavos = 0;
    std::string fecha;
    std::string hora;
    EstadoPedido estado = EstadoPedido::Pendiente;
};

class SistemaLogistico;
class RegistroPedidos;
class IColaDespacho;

class Pedido {
public:
    Pedido(const Pedido&) = delete;
    Pedido& operator=(const Pedido&) = delete;

    const DatosPedido& datos() const noexcept { return datos_; }
    int posicionHeap() const noexcept { return posicionHeap_; }

private:
    friend class RegistroPedidos;
    friend class SistemaLogistico;
    friend class IColaDespacho;

    explicit Pedido(const DatosPedido& datos) : datos_(datos) {}
    ~Pedido() = default;

    DatosPedido datos_;
    std::size_t posicionRegistro_ = 0;
    int posicionHeap_ = -1;
};

const char* texto(TipoEnvio tipo) noexcept;
const char* texto(EstadoPedido estado) noexcept;
bool interpretarTipoEnvio(const std::string& valor, TipoEnvio& tipo) noexcept;
bool interpretarEstado(const std::string& valor, EstadoPedido& estado) noexcept;
bool fechaValida(const std::string& fecha) noexcept;
bool horaValida(const std::string& hora) noexcept;

std::string validarNuevoPedido(const DatosPedido& datos);

int compararCronologicamente(const Pedido& a, const Pedido& b) noexcept;


bool precedeEnDespacho(const Pedido& a, const Pedido& b) noexcept;

}
