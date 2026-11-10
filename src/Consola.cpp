#include "logistica/Consola.hpp"
#include "logistica/ArchivoCSV.hpp"
#include <algorithm>
#include <chrono>
#include <exception>
#include <iostream>
#include <limits>
#include <memory>
#include <string>

namespace logistica {
namespace {
template <typename Operacion>
auto medir(Operacion operacion) {
    const auto inicio = std::chrono::steady_clock::now();
    const auto resultado = operacion();
    const double ms = std::chrono::duration<double, std::milli>(
        std::chrono::steady_clock::now() - inicio).count();
    std::cout << "Tiempo de operacion: " << ms << " ms\n";
    return resultado;
}

bool leer(const char* mensaje, std::string& valor) {
    std::cout << mensaje;
    return static_cast<bool>(std::getline(std::cin, valor));
}
bool natural(const std::string& entrada, std::size_t& valor) {
    if (entrada.empty()) return false;
    valor = 0;
    for (char c : entrada) {
        if (c < '0' || c > '9') return false;
        const auto digito = static_cast<std::size_t>(c - '0');
        if (valor > (std::numeric_limits<std::size_t>::max() - digito) / 10) return false;
        valor = valor * 10 + digito;
    }
    return true;
}
void mostrarResultado(const ResultadoOperacion& resultado) {
    std::cout << (resultado.correcto() ? "Operacion completada." : resultado.detalle) << '\n';
}
void mostrarPedido(const Pedido& pedido) {
    escribirPedidoCSV(std::cout, pedido);
    std::cout << '\n';
}
void mostrarMonto(std::int64_t centavos) {
    std::cout << centavos / 100 << '.' << (centavos % 100 < 10 ? "0" : "") << centavos % 100;
}
bool registrarManual(SistemaLogistico& sistema) {
    const char* preguntas[] = {"ID (PD-000001): ", "DNI (8 digitos): ",
        "Tipo (Normal/Express/SameDay): ", "Prioridad (1-10): ", "Monto USD (45.50): ",
        "Fecha (YYYY-MM-DD): ", "Hora (HH:MM:SS): "};
    std::string fila, campo;
    for (const char* pregunta : preguntas) {
        if (!leer(pregunta, campo)) return false;
        if (!fila.empty()) fila += ',';
        fila += campo;
    }
    fila += ",Pendiente";
    DatosPedido datos;
    std::string error;
    if (!interpretarFilaCSV(fila, datos, error)) std::cout << error << '\n';
    else mostrarResultado(medir([&] { return sistema.registrarPedido(datos); }));
    return true;
}
}

int ejecutarConsola(IIndicePedidos& hash, IColaDespacho& heap) {
    try {
        SistemaLogistico sistema(hash, heap);
        std::string entrada;
        while (true) {
            std::cout << "\n1 Cargar CSV\n2 Registrar pedido\n3 Buscar por ID\n"
                         "4 Despachar lote\n5 Primeros 30 por fecha\n6 Rango de fechas\n"
                         "7 Actualizar estado\n8 Cancelar pedido\n9 Estadisticas\n0 Salir\n";
            if (!leer("Opcion: ", entrada) || entrada == "0") return 0;
            if (entrada == "1") {
                if (!leer("Ruta CSV: ", entrada)) return 0;
                const auto reporte = cargarCSV(entrada, sistema);
                std::cout << "Aceptados: " << reporte.aceptados << ", rechazados: " << reporte.rechazados
                          << ", duplicados: " << reporte.duplicados << '\n'
                          << "Carga total: " << reporte.tiempoTotalMs << " ms\n"
                          << "Inserciones aceptadas: Hash " << reporte.tiemposIndices.hashMs
                          << " ms; AVL " << reporte.tiemposIndices.avlMs
                          << " ms; Heap " << reporte.tiemposIndices.heapMs << " ms\n";
                if (!reporte.primerError.empty()) std::cout << "Error en linea " << reporte.primeraLineaError
                                                           << ": " << reporte.primerError << '\n';
            } else if (entrada == "2") {
                if (!registrarManual(sistema)) return 0;
            } else if (entrada == "3") {
                if (!leer("ID: ", entrada)) return 0;
                const Pedido* pedido = medir([&] { return sistema.buscarPedido(entrada); });
                if (pedido) mostrarPedido(*pedido);
                else std::cout << "Pedido no encontrado.\n";
            } else if (entrada == "4") {
                if (!leer("Cantidad a despachar: ", entrada)) return 0;
                std::size_t cantidad = 0;
                if (!natural(entrada, cantidad) || cantidad == 0) {
                    std::cout << "Ingresar una cantidad positiva.\n"; continue;
                }
                const auto capacidad = std::min(cantidad, sistema.obtenerEstadisticas().pendientes);
                std::unique_ptr<const Pedido*[]> lote(new const Pedido*[capacidad]);
                const auto escritos = medir([&] { return sistema.despacharLote(cantidad, lote.get(), capacidad); });
                for (std::size_t i = 0; i < escritos; ++i) mostrarPedido(*lote[i]);
                std::cout << "Despachados: " << escritos << '\n';
            } else if (entrada == "5") {
                const Pedido* pedidos[30];
                const auto escritos = medir([&] { return sistema.consultarOrdenados(pedidos, 30); });
                for (std::size_t i = 0; i < escritos; ++i) mostrarPedido(*pedidos[i]);
                std::cout << "Mostrados: " << escritos << '\n';
            } else if (entrada == "6") {
                std::string hasta;
                if (!leer("Desde (YYYY-MM-DD): ", entrada) || !leer("Hasta (YYYY-MM-DD): ", hasta)) return 0;
                ResumenRango resumen;
                const auto resultado = medir([&] { return sistema.consultarRango(entrada, hasta, resumen); });
                if (!resultado.correcto()) mostrarResultado(resultado);
                else {
                    std::cout << "Pedidos: " << resumen.cantidad << "; monto USD: ";
                    mostrarMonto(resumen.montoTotalCentavos);
                    std::cout << '\n';
                }
            } else if (entrada == "7") {
                std::string estadoTexto;
                if (!leer("ID: ", entrada) || !leer("Estado (EnTransito/Entregado/Cancelado): ", estadoTexto)) return 0;
                EstadoPedido estado;
                if (!interpretarEstado(estadoTexto, estado)) std::cout << "Estado desconocido.\n";
                else mostrarResultado(medir([&] { return sistema.actualizarEstado(entrada, estado); }));
            } else if (entrada == "8") {
                if (!leer("ID a cancelar: ", entrada)) return 0;
                mostrarResultado(medir([&] { return sistema.cancelarPedido(entrada); }));
            } else if (entrada == "9") {
                const auto estadisticas = sistema.obtenerEstadisticas();
                std::cout << "Total: " << estadisticas.totalPedidos << "; pendientes: " << estadisticas.pendientes
                          << "; monto registrado USD: ";
                mostrarMonto(estadisticas.montoTotalCentavos);
                std::cout << '\n';
            } else std::cout << "Opcion invalida.\n";
        }
    } catch (const std::exception& error) {
        std::cerr << "No se pudo continuar: " << error.what() << '\n';
        return 1;
    }
}
}
