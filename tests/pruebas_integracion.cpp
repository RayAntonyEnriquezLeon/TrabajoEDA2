#include "logistica/ArchivoCSV.hpp"
#include "logistica/MinHeap.hpp"
#include "logistica/TablaHash.hpp"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <vector>

using namespace logistica;

namespace {
void exigir(bool condicion, const char* mensaje) {
    if (!condicion) throw std::runtime_error(mensaje);
}

bool cronologico(const DatosPedido& a, const DatosPedido& b) {
    if (a.fecha != b.fecha) return a.fecha < b.fecha;
    if (a.hora != b.hora) return a.hora < b.hora;
    return a.idPedido < b.idPedido;
}

void comprobar(SistemaLogistico& sistema, const std::vector<DatosPedido>& referencia) {
    std::size_t cantidad = 0, pendientes = 0;
    std::int64_t monto = 0;
    std::vector<DatosPedido> activos;
    for (const auto& d : referencia) {
        const auto* pedido = sistema.buscarPedido(d.idPedido);
        if (d.estado == EstadoPedido::Cancelado) {
            exigir(!pedido, "Cancelado presente en Hash.");
            continue;
        }
        exigir(pedido && pedido->datos().estado == d.estado, "Busqueda o estado incoherente.");
        exigir((pedido->posicionHeap() >= 0) == (d.estado == EstadoPedido::Pendiente),
               "Pertenencia al Heap incoherente.");
        ++cantidad;
        pendientes += d.estado == EstadoPedido::Pendiente;
        monto += d.montoCentavos;
        activos.push_back(d);
    }
    const auto e = sistema.obtenerEstadisticas();
    exigir(e.totalPedidos == cantidad && e.pendientes == pendientes &&
           e.montoTotalCentavos == monto, "Estadisticas incoherentes.");
    exigir(sistema.verificarAVL(), "AVL invalido.");
    ResumenRango completo;
    exigir(sistema.consultarRango("0001-01-01", "9999-12-31", completo).correcto() &&
           completo.cantidad == cantidad && completo.montoTotalCentavos == monto,
           "AVL y estadisticas no coinciden.");
    std::sort(activos.begin(), activos.end(), cronologico);
    const Pedido* salida[100] = {};
    const auto escritos = sistema.consultarOrdenados(salida, 100);
    exigir(escritos == std::min(std::size_t{30}, activos.size()), "Limite de primeros 30.");
    for (std::size_t i = 0; i < escritos; ++i)
        exigir(salida[i]->datos().idPedido == activos[i].idPedido, "Orden cronologico.");
}

void probarSistema(const std::string& ruta) {
    std::ifstream archivo(ruta);
    exigir(archivo.is_open(), "Abrir CSV de referencia.");
    std::string fila, error;
    std::getline(archivo, fila);
    std::vector<DatosPedido> referencia;
    while (std::getline(archivo, fila)) {
        if (!fila.empty() && fila.back() == '\r') fila.pop_back();
        DatosPedido d;
        exigir(interpretarFilaCSV(fila, d, error), "Leer referencia CSV.");
        exigir(d.estado == EstadoPedido::Pendiente, "Dataset esperado: todos pendientes.");
        referencia.push_back(d);
    }
    exigir(!archivo.bad() && referencia.size() >= 10000, "Dataset masivo insuficiente.");
    TablaHash hash;
    MinHeap heap;
    SistemaLogistico sistema(hash, heap);
    const auto carga = cargarCSV(ruta, sistema);
    exigir(carga.aceptados == referencia.size() && carga.rechazados == 0, "Carga inicial.");
    comprobar(sistema, referencia);
    const auto repetida = cargarCSV(ruta, sistema);
    exigir(repetida.aceptados == 0 && repetida.duplicados == referencia.size() &&
           repetida.rechazados == referencia.size(), "Carga duplicada.");
    exigir(sistema.buscarPedido(referencia.front().idPedido) &&
           sistema.buscarPedido(referencia.back().idPedido) &&
           !sistema.buscarPedido("PD-999999"), "Busqueda inicio, final y ausente.");
    comprobar(sistema, referencia);

    const char* desde[] = {"2026-06-01", "2026-06-01", "2027-01-01"};
    const char* hasta[] = {"2026-06-07", "2026-06-30", "2027-01-31"};
    for (int i = 0; i < 3; ++i) {
        ResumenRango esperado, real;
        for (const auto& d : referencia)
            if (d.fecha >= desde[i] && d.fecha <= hasta[i]) {
                ++esperado.cantidad;
                esperado.montoTotalCentavos += d.montoCentavos;
            }
        exigir(sistema.consultarRango(desde[i], hasta[i], real).correcto() &&
               real.cantidad == esperado.cantidad &&
               real.montoTotalCentavos == esperado.montoTotalCentavos, "Rango y suma independiente.");
    }
    ResumenRango invalido;
    exigir(!sistema.consultarRango("2026-07-01", "2026-06-01", invalido).correcto(),
           "Rechazar rango invertido.");
    auto nuevo = referencia.front();
    nuevo.idPedido = "PD-999998";
    exigir(sistema.registrarPedido(nuevo).correcto(), "Insercion individual.");
    referencia.push_back(nuevo);
    nuevo.fecha = "2026-07-01";
    exigir(sistema.registrarPedido(nuevo).codigo == CodigoResultado::IdDuplicado,
           "ID repetido con fecha distinta.");
    comprobar(sistema, referencia);

    std::sort(referencia.begin(), referencia.end(), [](const auto& a, const auto& b) {
        return a.nivelPrioridad != b.nivelPrioridad ?
               a.nivelPrioridad > b.nivelPrioridad : cronologico(a, b);
    });
    std::size_t enviados = 0;
    for (std::size_t lote : {10u, 100u}) {
        std::vector<const Pedido*> salida(lote);
        exigir(sistema.despacharLote(lote, salida.data(), lote) == lote, "Despacho lote.");
        for (std::size_t i = 0; i < lote; ++i) {
            auto& esperado = referencia[enviados + i];
            exigir(salida[i]->datos().idPedido == esperado.idPedido, "Prioridad y desempates.");
            esperado.estado = EstadoPedido::EnTransito;
        }
        enviados += lote;
        comprobar(sistema, referencia);
    }
    for (std::size_t i = enviados; i < enviados + 10; ++i) {
        auto& d = referencia[i];
        exigir(!sistema.actualizarEstado(d.idPedido, EstadoPedido::Entregado).correcto(),
               "Rechazar salto de estado.");
        exigir(sistema.actualizarEstado(d.idPedido, EstadoPedido::EnTransito).correcto(),
               "Actualizar a transito.");
        exigir(sistema.actualizarEstado(d.idPedido, EstadoPedido::Entregado).correcto(),
               "Actualizar a entregado.");
        exigir(sistema.actualizarEstado(d.idPedido, EstadoPedido::Entregado).correcto(),
               "Repetir estado sin efectos.");
        d.estado = EstadoPedido::Entregado;
    }
    comprobar(sistema, referencia);
    for (std::size_t i = enviados + 10; i < enviados + 20; ++i) {
        auto& d = referencia[i];
        exigir(sistema.cancelarPedido(d.idPedido).correcto(), "Cancelar pendiente.");
        exigir(!sistema.cancelarPedido(d.idPedido).correcto(), "Rechazar cancelacion repetida.");
        d.estado = EstadoPedido::Cancelado;
    }
    comprobar(sistema, referencia);
    for (std::size_t i : {std::size_t{0}, enviados}) {
        auto& d = referencia[i]; // En transito y entregado, respectivamente.
        exigir(sistema.actualizarEstado(d.idPedido, EstadoPedido::Cancelado).correcto(),
               "Cancelar mediante estado.");
        d.estado = EstadoPedido::Cancelado;
    }
    comprobar(sistema, referencia);
    sistema.vaciar();
    sistema.vaciar();
    comprobar(sistema, {});
    nuevo.estado = EstadoPedido::Pendiente;
    exigir(sistema.registrarPedido(nuevo).correcto(), "Reutilizar sistema.");
    comprobar(sistema, {nuevo});
}
}

int main(int argc, char** argv) {
    try {
        exigir(argc == 2, "Indicar ruta del CSV masivo.");
        probarSistema(argv[1]);
        std::cout << "Integracion de Hash, AVL y Heap correcta.\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
