#include "logistica/Consola.hpp"
#include "logistica/ArchivoCSV.hpp"
#include <algorithm>
#include <chrono>
#include <exception>
#include <iomanip>
#include <iostream>
#include <limits>
#include <memory>
#include <sstream>
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

void titulo(const char* textoTitulo) {
    std::cout << "\n--- " << textoTitulo << " ---\n";
}

bool leer(const char* mensaje, std::string& valor) {
    std::cout << mensaje;
    if (!std::getline(std::cin, valor)) return false;
    const auto inicio = valor.find_first_not_of(" \t\r\n");
    if (inicio == std::string::npos) valor.clear();
    else valor = valor.substr(inicio, valor.find_last_not_of(" \t\r\n") - inicio + 1);
    return true;
}

bool pausar() {
    std::string entrada;
    std::cout << "\nPresiona Enter para volver al menu...";
    return static_cast<bool>(std::getline(std::cin, entrada));
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

std::string montoTexto(std::int64_t centavos) {
    std::ostringstream salida;
    salida << centavos / 100 << '.' << (centavos % 100 < 10 ? "0" : "") << centavos % 100;
    return salida.str();
}

const char* estadoVisible(EstadoPedido estado) {
    return estado == EstadoPedido::EnTransito ? "En transito" : texto(estado);
}

void mostrarResultado(const ResultadoOperacion& resultado, const char* exito) {
    if (resultado.correcto()) std::cout << exito << '\n';
    else std::cout << "No se pudo completar: " << resultado.detalle << '\n';
}

void mostrarPedido(const Pedido& pedido) {
    const auto& d = pedido.datos();
    std::cout << "Codigo:     " << d.idPedido << '\n'
              << "DNI:        " << d.dniCliente << '\n'
              << "Envio:      " << texto(d.tipoEnvio) << '\n'
              << "Prioridad:  " << d.nivelPrioridad << '\n'
              << "Monto:      USD " << montoTexto(d.montoCentavos) << '\n'
              << "Fecha:      " << d.fecha << '\n'
              << "Hora:       " << d.hora << '\n'
              << "Estado:     " << estadoVisible(d.estado) << '\n';
}

void mostrarTabla(const Pedido* const* pedidos, std::size_t cantidad) {
    if (cantidad == 0) {
        std::cout << "No hay pedidos para mostrar.\n";
        return;
    }
    std::ostringstream salida;
    salida << std::left << std::setw(12) << "ID" << std::setw(10) << "DNI"
           << std::setw(10) << "Envio" << std::setw(6) << "Prior."
           << std::setw(18) << "Monto USD" << std::setw(12) << "Fecha"
           << std::setw(10) << "Hora" << "Estado\n"
           << std::string(92, '-') << '\n';
    for (std::size_t i = 0; i < cantidad; ++i) {
        const auto& d = pedidos[i]->datos();
        salida << std::setw(12) << d.idPedido << std::setw(10) << d.dniCliente
               << std::setw(10) << texto(d.tipoEnvio) << std::setw(6) << d.nivelPrioridad
               << std::setw(18) << montoTexto(d.montoCentavos) << std::setw(12) << d.fecha
               << std::setw(10) << d.hora << estadoVisible(d.estado) << '\n';
    }
    std::cout << salida.str();
}

bool registrarManual(SistemaLogistico& sistema) {
    titulo("REGISTRAR PEDIDO");
    std::cout << "El nuevo pedido se registra como Pendiente.\n";
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
    if (!interpretarFilaCSV(fila, datos, error))
        std::cout << "No se pudo registrar: " << error << '\n';
    else mostrarResultado(medir([&] { return sistema.registrarPedido(datos); }),
                          "Pedido registrado correctamente.");
    return true;
}

void mostrarCarga(const ReporteCarga& reporte) {
    if (!reporte.archivoAbierto || !reporte.cabeceraValida)
        std::cout << "CARGA NO REALIZADA\n";
    else if (!reporte.completo())
        std::cout << "CARGA CON OBSERVACIONES\n";
    else std::cout << "CARGA COMPLETADA\n";
    std::cout << "Pedidos aceptados:  " << reporte.aceptados << '\n'
              << "Pedidos rechazados: " << reporte.rechazados << '\n'
              << "IDs duplicados:     " << reporte.duplicados << " (incluidos en rechazados)\n"
              << "Tiempo total: " << reporte.tiempoTotalMs << " ms\n"
              << "Inserciones por indice: Hash " << reporte.tiemposIndices.hashMs
              << " ms; AVL " << reporte.tiemposIndices.avlMs
              << " ms; Heap " << reporte.tiemposIndices.heapMs << " ms\n";
    if (!reporte.primerError.empty()) {
        std::cout << "Detalle";
        if (reporte.primeraLineaError != 0) std::cout << " en linea " << reporte.primeraLineaError;
        std::cout << ": " << reporte.primerError << '\n';
    }
}
}

int ejecutarConsola(IIndicePedidos& hash, IColaDespacho& heap) {
    try {
        SistemaLogistico sistema(hash, heap);
        std::cout << "SISTEMA DE GESTION LOGISTICA\n"
                     "Los cambios duran esta sesion; no modifican los archivos CSV.\n";
        std::string entrada;
        while (true) {
            titulo("MENU PRINCIPAL");
            std::cout << "1 Cargar pedidos desde CSV\n2 Registrar nuevo pedido\n3 Buscar pedido por ID\n"
                         "4 Despachar lote de pedidos urgentes\n5 Ver primeros 30 por fecha y hora\n"
                         "6 Consultar cantidad y monto por rango de fechas\n"
                         "7 Actualizar estado de un pedido\n8 Cancelar y eliminar un pedido\n"
                         "9 Ver estadisticas generales\n0 Salir\n";
            if (!leer("Opcion: ", entrada)) return 0;
            if (entrada == "0") {
                std::cout << "Sesion finalizada. Los archivos CSV permanecen intactos.\n";
                return 0;
            }
            if (entrada == "1") {
                titulo("CARGAR CSV");
                std::cout << "Ejemplos: datos/pedidos_pequeno.csv o datos/pedidos_masivo.csv\n";
                if (!leer("Ruta CSV: ", entrada)) return 0;
                mostrarCarga(cargarCSV(entrada, sistema));
            } else if (entrada == "2") {
                if (!registrarManual(sistema)) return 0;
            } else if (entrada == "3") {
                titulo("BUSQUEDA POR ID");
                if (!leer("ID: ", entrada)) return 0;
                const Pedido* pedido = medir([&] { return sistema.buscarPedido(entrada); });
                if (pedido) {
                    std::cout << "PEDIDO ENCONTRADO\n";
                    mostrarPedido(*pedido);
                } else std::cout << "Pedido no encontrado.\n";
            } else if (entrada == "4") {
                titulo("DESPACHAR PEDIDOS URGENTES");
                std::cout << "Mayor prioridad primero; en empate, fecha, hora e ID.\n";
                if (!leer("Cantidad a despachar: ", entrada)) return 0;
                std::size_t cantidad = 0;
                if (!natural(entrada, cantidad) || cantidad == 0)
                    std::cout << "Cantidad invalida: ingresa un entero positivo.\n";
                else {
                    const auto pendientes = sistema.obtenerEstadisticas().pendientes;
                    const auto capacidad = std::min(cantidad, pendientes);
                    std::unique_ptr<const Pedido*[]> lote(new const Pedido*[capacidad]);
                    const auto escritos = medir([&] {
                        return sistema.despacharLote(cantidad, lote.get(), capacidad);
                    });
                    mostrarTabla(lote.get(), escritos);
                    std::cout << "Pedidos solicitados: " << cantidad << '\n'
                              << "Pedidos despachados: " << escritos << '\n';
                    if (escritos > 0)
                        std::cout << "Nuevo estado: En transito. Permanecen registrados en el sistema.\n";
                    if (cantidad > pendientes)
                        std::cout << "No habia suficientes pendientes para completar el lote solicitado.\n";
                }
            } else if (entrada == "5") {
                titulo("PRIMEROS 30 POR FECHA Y HORA");
                const Pedido* pedidos[30];
                const auto escritos = medir([&] { return sistema.consultarOrdenados(pedidos, 30); });
                mostrarTabla(pedidos, escritos);
                std::cout << "Mostrados: " << escritos << " de "
                          << sistema.obtenerEstadisticas().totalPedidos << " pedidos registrados.\n";
            } else if (entrada == "6") {
                titulo("CONSULTA POR RANGO DE FECHAS");
                std::cout << "Cuenta pedidos registrados y suma sus montos, incluyendo ambas fechas.\n"
                             "Incluye pendientes, en transito y entregados; no modifica pedidos.\n";
                std::string hasta;
                if (!leer("Desde (YYYY-MM-DD): ", entrada) ||
                    !leer("Hasta (YYYY-MM-DD): ", hasta)) return 0;
                ResumenRango resumen;
                const auto resultado = medir([&] { return sistema.consultarRango(entrada, hasta, resumen); });
                if (!resultado.correcto()) mostrarResultado(resultado, "");
                else std::cout << "Pedidos en el periodo: " << resumen.cantidad
                               << "\nMonto acumulado: USD " << montoTexto(resumen.montoTotalCentavos)
                               << (resumen.cantidad == 0 ? "\nNo hay pedidos en ese rango.\n" : "\n");
            } else if (entrada == "7") {
                titulo("ACTUALIZAR ESTADO");
                std::cout << "Transicion: Pendiente -> EnTransito -> Entregado.\n"
                             "Cancelado elimina el pedido de los indices.\n";
                std::string estadoTexto;
                if (!leer("ID: ", entrada) ||
                    !leer("Estado (EnTransito/Entregado/Cancelado): ", estadoTexto)) return 0;
                EstadoPedido estado;
                if (!interpretarEstado(estadoTexto, estado))
                    std::cout << "Estado desconocido. Usa EnTransito, Entregado o Cancelado.\n";
                else mostrarResultado(medir([&] { return sistema.actualizarEstado(entrada, estado); }),
                                      estado == EstadoPedido::Cancelado ?
                                      "Pedido cancelado y eliminado del sistema." : "Estado actualizado correctamente.");
            } else if (entrada == "8") {
                titulo("CANCELAR PEDIDO");
                std::cout << "Esta operacion elimina el pedido y descuenta su monto del total.\n";
                if (!leer("ID a cancelar: ", entrada)) return 0;
                mostrarResultado(medir([&] { return sistema.cancelarPedido(entrada); }),
                                 "Pedido cancelado y eliminado del sistema.");
            } else if (entrada == "9") {
                titulo("ESTADISTICAS GENERALES");
                const auto e = sistema.obtenerEstadisticas();
                std::cout << "Pedidos registrados:       " << e.totalPedidos << '\n'
                          << "Pendientes por despachar:  " << e.pendientes << '\n'
                          << "Monto registrado:          USD " << montoTexto(e.montoTotalCentavos)
                          << "\nDespachar no cambia el monto; cancelar lo descuenta.\n";
            } else std::cout << "Opcion invalida. Elige un numero del 0 al 9.\n";
            if (!pausar()) return 0;
        }
    } catch (const std::exception& error) {
        std::cerr << "No se pudo continuar: " << error.what() << '\n';
        return 1;
    }
}
}
