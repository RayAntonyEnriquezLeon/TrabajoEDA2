#include "logistica/Consola.hpp"
#include "logistica/MinHeap.hpp"
#include "logistica/TablaHash.hpp"
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

using namespace logistica;

namespace {
void exigir(bool condicion, const char* mensaje) {
    if (!condicion) throw std::runtime_error(mensaje);
}

// Sustituye los streams solo durante cada escenario y los restaura incluso si falla.
class StreamsTemporales {
public:
    StreamsTemporales(std::istream& entrada, std::ostream& salida)
        : estado_(std::cin.rdstate()), entrada_(std::cin.rdbuf(entrada.rdbuf())),
          salida_(std::cout.rdbuf(salida.rdbuf())) {}
    ~StreamsTemporales() {
        std::cin.rdbuf(entrada_);
        std::cin.clear(estado_);
        std::cout.rdbuf(salida_);
    }
    StreamsTemporales(const StreamsTemporales&) = delete;
    StreamsTemporales& operator=(const StreamsTemporales&) = delete;
private:
    std::ios::iostate estado_;
    std::streambuf* entrada_;
    std::streambuf* salida_;
};

std::string ejecutar(const std::string& instrucciones) {
    std::istringstream entrada(instrucciones);
    std::ostringstream salida;
    TablaHash hash;
    MinHeap heap;
    int codigo = 0;
    {
        StreamsTemporales streams(entrada, salida);
        codigo = ejecutarConsola(hash, heap);
    }
    exigir(codigo == 0, "La consola termino con error.");
    exigir(hash.cantidad() == 0 && heap.cantidad() == 0, "Indices no vacios al salir.");
    return salida.str();
}

void contiene(const std::string& salida, const std::string& texto) {
    exigir(salida.find(texto) != std::string::npos, ("No aparece: " + texto).c_str());
}

void probarEntradasYEstados() {
    const std::string registro =
        " 2 \n PD-020001 \n 00123456 \n Express \n 10 \n 45.50 \n"
        " 2026-06-05 \n 12:00:00 \n\n";
    auto salida = ejecutar(registro +
        " 3 \n PD-020001 \n\n"
        "5\n\n"
        "6\n 2026-06-01 \n 2026-06-07 \n\n"
        "7\n PD-020001 \n Entregado \n\n"
        "4\n 1 \n\n"
        "7\n PD-020001 \n Entregado \n\n"
        "3\nPD-020001\n\n"
        "9\n\n"
        "8\n PD-020001 \n\n"
        "3\nPD-020001\n\n"
        "0\n");
    contiene(salida, "Pedido registrado correctamente.");
    contiene(salida, "PEDIDO ENCONTRADO\nCodigo:     PD-020001");
    contiene(salida, "DNI:        00123456");
    contiene(salida, "Monto:      USD 45.50");
    contiene(salida, "Estado:     Pendiente");
    contiene(salida, "Mostrados: 1 de 1 pedidos registrados.");
    contiene(salida, "Pedidos en el periodo: 1\nMonto acumulado: USD 45.50");
    contiene(salida, "Transicion permitida:");
    contiene(salida, "Pedidos despachados: 1");
    contiene(salida, "Estado actualizado correctamente.");
    contiene(salida, "Estado:     Entregado");
    contiene(salida, "Pendientes por despachar:  0");
    contiene(salida, "Pedido cancelado y eliminado del sistema.");
    contiene(salida, "Pedido no encontrado.");
    contiene(salida, "Presiona Enter para volver al menu...");
    contiene(salida, "Sesion finalizada.");

    salida = ejecutar(registro + registro + "0\n");
    contiene(salida, "El ID ya esta registrado.");
    salida = ejecutar(registro + "7\nPD-020001\n Cancelado \n\n9\n\n0\n");
    contiene(salida, "Pedido cancelado y eliminado del sistema.");
    contiene(salida, "Pedidos registrados:       0");
}

void probarErroresYFinDeEntrada() {
    auto salida = ejecutar("x\n\n4\n0\n\n4\nabc\n\n4\n1\n\n5\n\n"
                          "6\n2027-01-01\n2027-01-31\n\n"
                          "7\nPD-000001\nDesconocido\n\n"
                          "6\n2026-07-01\n2026-06-01\n\n0\n");
    contiene(salida, "Opcion invalida.");
    contiene(salida, "Cantidad invalida:");
    contiene(salida, "Pedidos despachados: 0");
    contiene(salida, "No habia suficientes pendientes");
    contiene(salida, "No hay pedidos para mostrar.");
    contiene(salida, "No hay pedidos en ese rango.");
    contiene(salida, "Estado desconocido.");
    contiene(salida, "Rango invalido:");
    ejecutar("");
    ejecutar("2\nPD-020001\n"); // EOF durante el registro.
    ejecutar("9\n"); // EOF en la pausa.
    salida = ejecutar("2\nPD-020001\n00123456\nExpress\n10\n45.50\n"
                      "2026-06-05\n12: 00:00\n\n9\n\n0\n");
    contiene(salida, "No se pudo registrar:");
    contiene(salida, "Pedidos registrados:       0");
}

void probarCarga(const std::string& directorio) {
    auto salida = ejecutar("1\n " + directorio + "/pedidos_pequeno.csv \n\n"
                           "1\n" + directorio + "/pedidos_pequeno.csv\n\n0\n");
    contiene(salida, "CARGA COMPLETADA");
    contiene(salida, "Pedidos aceptados:  30");
    contiene(salida, "CARGA CON OBSERVACIONES");
    contiene(salida, "IDs duplicados:     30 (incluidos en rechazados)");
    salida = ejecutar("1\n" + directorio + "/no-existe.csv\n\n0\n");
    contiene(salida, "CARGA NO REALIZADA");
    contiene(salida, "No se pudo abrir el archivo.");
    exigir(salida.find("CARGA COMPLETADA") == std::string::npos, "Exito falso de carga.");
}
}

int main(int argc, char** argv) {
    try {
        exigir(argc == 2, "Indicar directorio de datos.");
        probarEntradasYEstados();
        probarErroresYFinDeEntrada();
        probarCarga(argv[1]);
        std::cout << "Consola: mensajes, espacios, pausas, EOF y operaciones correctos.\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
