#include "logistica/SistemaLogistico.hpp"
#include <algorithm>
#include <chrono>
#include <limits>
#include <new>
#include <stdexcept>

namespace logistica {
namespace {
using Reloj = std::chrono::steady_clock;
double milisegundos(Reloj::time_point inicio) {
    return std::chrono::duration<double, std::milli>(Reloj::now() - inicio).count();
}
}

SistemaLogistico::SistemaLogistico(IIndicePedidos& hash, IColaDespacho& heap)
    : hash_(hash), heap_(heap) {
    if (hash_.cantidad() != 0 || heap_.cantidad() != 0)
        throw std::invalid_argument("Conectar indices vacios al SistemaLogistico.");
}
SistemaLogistico::~SistemaLogistico() { vaciar(); }

ResultadoOperacion SistemaLogistico::registrarPedido(const DatosPedido& datos, TiemposInsercion* tiempos) {
    if (tiempos) *tiempos = {};
    const std::string error = validarNuevoPedido(datos);
    if (!error.empty()) return {CodigoResultado::DatosInvalidos, error};
    if (hash_.buscar(datos.idPedido)) return {CodigoResultado::IdDuplicado, "El ID ya esta registrado."};
    if (datos.montoCentavos > std::numeric_limits<std::int64_t>::max() - montoTotalCentavos_)
        return {CodigoResultado::DatosInvalidos, "El monto acumulado excederia el limite numerico."};

    Pedido* pedido = nullptr;
    bool enHash = false, enAVL = false, enHeap = false;
    TiemposInsercion medicion;
    const auto revertir = [&]() noexcept {
        if (enHeap) heap_.eliminar(pedido);
        if (enAVL) avl_.eliminar(*pedido);
        if (enHash) hash_.eliminar(datos.idPedido);
        if (pedido) registro_.liberar(pedido);
    };
    try {
        pedido = registro_.crear(datos);
        auto inicio = Reloj::now();
        enHash = hash_.insertar(pedido);
        medicion.hashMs = milisegundos(inicio);
        if (!enHash) { revertir(); return {CodigoResultado::ErrorIndice, "Hash rechazo la insercion."}; }

        inicio = Reloj::now();
        enAVL = avl_.insertar(pedido);
        medicion.avlMs = milisegundos(inicio);
        if (!enAVL) { revertir(); return {CodigoResultado::ErrorIndice, "AVL rechazo la insercion."}; }

        if (datos.estado == EstadoPedido::Pendiente) {
            inicio = Reloj::now();
            enHeap = heap_.insertar(pedido);
            medicion.heapMs = milisegundos(inicio);
            if (!enHeap) { revertir(); return {CodigoResultado::ErrorIndice, "Heap rechazo la insercion."}; }
        }
    } catch (const std::bad_alloc&) {
        revertir();
        return {CodigoResultado::SinMemoria, "No se pudo reservar memoria; registro revertido."};
    } catch (...) {
        revertir();
        throw;
    }
    montoTotalCentavos_ += datos.montoCentavos;
    if (tiempos) *tiempos = medicion;
    return {};
}

const Pedido* SistemaLogistico::buscarPedido(const std::string& idPedido) const noexcept {
    return hash_.buscar(idPedido);
}
ResultadoOperacion SistemaLogistico::actualizarEstado(const std::string& idPedido, EstadoPedido estado) {
    if (estado == EstadoPedido::Cancelado) return cancelarPedido(idPedido);
    Pedido* pedido = hash_.buscar(idPedido);
    if (!pedido) return {CodigoResultado::NoEncontrado, "El pedido no existe."};
    const EstadoPedido anterior = pedido->datos_.estado;
    if (anterior == estado) return {};
    const bool inicioEnvio = anterior == EstadoPedido::Pendiente && estado == EstadoPedido::EnTransito;
    const bool entrega = anterior == EstadoPedido::EnTransito && estado == EstadoPedido::Entregado;
    if (!inicioEnvio && !entrega)
        return {CodigoResultado::TransicionInvalida, "Transicion permitida: Pendiente -> EnTransito -> Entregado."};
    if (inicioEnvio && !heap_.eliminar(pedido))
        throw std::logic_error("Contrato Heap incumplido: pendiente ausente de la cola.");
    pedido->datos_.estado = estado;
    return {};
}
ResultadoOperacion SistemaLogistico::cancelarPedido(const std::string& idPedido) {
    Pedido* pedido = hash_.buscar(idPedido);
    if (!pedido) return {CodigoResultado::NoEncontrado, "El pedido no existe."};
    if (pedido->datos_.estado == EstadoPedido::Pendiente && !heap_.eliminar(pedido))
        throw std::logic_error("Contrato Heap incumplido al cancelar.");
    if (!avl_.eliminar(*pedido)) throw std::logic_error("Pedido ausente del AVL.");
    if (!hash_.eliminar(idPedido)) throw std::logic_error("Contrato Hash incumplido al cancelar.");
    montoTotalCentavos_ -= pedido->datos_.montoCentavos;
    registro_.liberar(pedido);
    return {};
}
std::size_t SistemaLogistico::despacharLote(std::size_t solicitados, const Pedido** salida,
                                         std::size_t capacidad) {
    if (!salida) return 0;
    const std::size_t limite = std::min(solicitados, capacidad);
    std::size_t despachados = 0;
    while (despachados < limite && heap_.cantidad() != 0) {
        Pedido* pedido = heap_.extraerMinimo();
        if (!pedido || pedido->datos_.estado != EstadoPedido::Pendiente)
            throw std::logic_error("Contrato Heap incumplido: extraerMinimo debe devolver un pendiente.");
        pedido->datos_.estado = EstadoPedido::EnTransito;
        salida[despachados++] = pedido;
    }
    return despachados;
}
std::size_t SistemaLogistico::consultarOrdenados(const Pedido** salida, std::size_t capacidad) const noexcept {
    return avl_.obtenerPrimeros(salida, std::min(capacidad, std::size_t{30}));
}
ResultadoOperacion SistemaLogistico::consultarRango(const std::string& desde, const std::string& hasta,
                                                   ResumenRango& resultado) const {
    resultado = {};
    if (!fechaValida(desde) || !fechaValida(hasta) || desde > hasta)
        return {CodigoResultado::DatosInvalidos, "Rango invalido: fechas YYYY-MM-DD e inicio <= fin."};
    resultado = avl_.consultarRango(desde, hasta);
    return {};
}
Estadisticas SistemaLogistico::obtenerEstadisticas() const noexcept {
    return {registro_.cantidad(), heap_.cantidad(), montoTotalCentavos_};
}
void SistemaLogistico::vaciar() noexcept {
    heap_.vaciar();
    avl_.vaciar();
    hash_.vaciar();
    registro_.vaciar();
    montoTotalCentavos_ = 0;
}
}
