

#include "logistica/ArchivoCSV.hpp"
#include "logistica/RegistroPedidos.hpp"
#include <exception>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

using namespace logistica;

namespace {
void exigir(bool condicion, const char* mensaje) {
    if (!condicion) throw std::runtime_error(mensaje);
}

void probarValidaciones() {
    exigir(fechaValida("2024-02-29") && !fechaValida("2026-02-29") &&
           !fechaValida("1900-02-29") && fechaValida("2000-02-29") &&
           !fechaValida("2026-04-31") && !fechaValida("0000-01-01"), "Validacion de fechas.");
    exigir(horaValida("23:59:59") && !horaValida("24:00:00") && !horaValida("12:60:00"), "Validacion de horas.");
    DatosPedido datos;
    std::string error;
    const std::string fila = "PD-000001,00123456,Express,10,0.01,2026-06-01,08:30:00,Pendiente";
    exigir(interpretarFilaCSV(fila, datos, error) && datos.montoCentavos == 1 && datos.dniCliente == "00123456", "Lectura exacta de centavos y DNI.");
    RegistroPedidos registro;
    Pedido* pedido = registro.crear(datos);
    std::ostringstream salida;
    escribirPedidoCSV(salida, *pedido);
    exigir(salida.str() == fila, "Formato CSV de salida.");
    exigir(interpretarFilaCSV("PD-000003,14578963,SameDay,10,85.99,2026-06-02,14:20:15,EnTransito", datos, error) &&
           datos.estado == EstadoPedido::EnTransito, "Aceptar estado del ejemplo del PDF.");
    exigir(interpretarFilaCSV("PD-000004,14578963,Normal,1,85.99,2026-06-02,14:20:15,Entregado", datos, error),
           "Aceptar importacion de entregados.");
    const std::string invalidos[] = {
        "", fila + ",extra", "PD-000001,00123456",
        "PD-000001,00123456,Normal,1,0.001,2026-06-01,00:00:00,Pendiente",
        "PD-000001,00123456,Normal,1,92233720368547758.08,2026-06-01,00:00:00,Pendiente",
        "PD-000001,00123456,Normal,11,1.00,2026-06-01,00:00:00,Pendiente",
        "PD-000001,00123456,Normal,1,1.00,2026-06-01,00:00:00,Desconocido",
        "PD-000001,00123456,Normal,1,1.00,2026-06-01,00:00:00,Cancelado"
    };
    for (const auto& invalido : invalidos)
        exigir(!interpretarFilaCSV(invalido, datos, error) && !error.empty(), "Rechazar datos incorrectos.");
}

void probarArchivo(const std::string& ruta, std::size_t minimo, std::size_t maximo) {
    std::ifstream archivo(ruta, std::ios::binary);
    exigir(archivo.is_open(), "No se pudo abrir un CSV entregado.");
    std::string fila;
    std::getline(archivo, fila);
    exigir(fila == "idPedido,dniCliente,tipoEnvio,nivelPrioridad,montoDolares,fecha,hora,estado", "Cabecera del CSV.");
    RegistroPedidos registro;
    ArbolAVL avl;
    ResumenRango esperado[3];
    const char* desde[] = {"2026-06-01", "2026-06-01", "2027-01-01"};
    const char* hasta[] = {"2026-06-07", "2026-06-30", "2027-01-31"};
    std::size_t cantidad = 0;
    while (std::getline(archivo, fila)) {
        DatosPedido datos;
        std::string error;
        exigir(interpretarFilaCSV(fila, datos, error), "Fila del CSV invalida.");
        ++cantidad;
        const std::string numero = std::to_string(cantidad);
        exigir(datos.idPedido == "PD-" + std::string(6 - numero.size(), '0') + numero, "IDs consecutivos y unicos.");
        exigir(avl.insertar(registro.crear(datos)), "Insertar CSV en AVL.");
        for (int i = 0; i < 3; ++i) {
            if (datos.fecha >= desde[i] && datos.fecha <= hasta[i]) {
                ++esperado[i].cantidad;
                esperado[i].montoTotalCentavos += datos.montoCentavos;
            }
        }
    }
    exigir(!archivo.bad() && cantidad >= minimo && cantidad <= maximo, "Cantidad de pedidos del CSV.");
    exigir(avl.verificarInvariantes(), "Balance tras leer el CSV.");
    for (int i = 0; i < 3; ++i) {
        const auto real = avl.consultarRango(desde[i], hasta[i]);
        exigir(real.cantidad == esperado[i].cantidad && real.montoTotalCentavos == esperado[i].montoTotalCentavos,
               "Rango de semana, mes o vacio no coincide con suma independiente.");
    }
    std::cout << ruta << ": " << cantidad << " filas validas y rangos correctos\n";
}
}

int main(int argc, char** argv) {
    try {
        probarValidaciones();
        const std::string carpeta = argc > 1 ? argv[1] : "datos";
        probarArchivo(carpeta + "/pedidos_pequeno.csv", 20, 50);
        probarArchivo(carpeta + "/pedidos_masivo.csv", 10000, 20000);
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
