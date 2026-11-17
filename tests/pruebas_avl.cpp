

#include "logistica/ArbolAVL.hpp"
#include "logistica/RegistroPedidos.hpp"
#include <algorithm>
#include <exception>
#include <iostream>
#include <random>
#include <stdexcept>
#include <string>
#include <vector>

using namespace logistica;

namespace {
void exigir(bool condicion, const char* mensaje) {
    if (!condicion) throw std::runtime_error(mensaje);
}

DatosPedido datosEjemplo(int numero) {
    DatosPedido datos;
    const std::string digitos = std::to_string(numero);
    datos.idPedido = "PD-" + std::string(6 - digitos.size(), '0') + digitos;
    datos.dniCliente = "00123456";
    datos.nivelPrioridad = 5;
    datos.montoCentavos = 4550;
    datos.fecha = "2026-06-01";
    datos.hora = "08:30:00";
    return datos;
}

void probarRotaciones() {
    const int secuencias[4][3] = {{3, 2, 1}, {1, 2, 3}, {3, 1, 2}, {1, 3, 2}};
    for (const auto& secuencia : secuencias) {
        RegistroPedidos registro;
        ArbolAVL avl;
        Pedido* pedidos[4] = {};
        for (int id : secuencia) {
            pedidos[id] = registro.crear(datosEjemplo(id));
            exigir(avl.insertar(pedidos[id]), "Insertar para rotacion.");
            exigir(avl.verificarInvariantes(), "Balance tras rotacion.");
        }
        exigir(!avl.insertar(pedidos[2]), "Rechazar clave duplicada.");
        exigir(avl.buscar("2026-06-01", "08:30:00", "PD-000002") == pedidos[2], "Buscar por clave cronologica.");
        exigir(!avl.buscar("2026-06-02", "08:30:00", "PD-000002"), "Fecha distinta no coincide.");
        for (int id : {2, 1, 3}) {
            exigir(avl.eliminar(*pedidos[id]), "Eliminar nodo.");
            exigir(!avl.eliminar(*pedidos[id]), "Eliminar inexistente sin cambios.");
            exigir(!avl.buscar(pedidos[id]->datos().fecha, pedidos[id]->datos().hora, pedidos[id]->datos().idPedido),
                   "Busqueda posterior a eliminar.");
            exigir(avl.verificarInvariantes(), "Balance tras eliminacion.");
        }
    }
}

void probarVolumenYRangos() {
    RegistroPedidos registro;
    ArbolAVL avl;
    std::vector<Pedido*> referencia;
    for (int i = 1; i <= 20000; ++i) {
        auto datos = datosEjemplo(i);
        datos.fecha = i % 3 == 0 ? "2026-07-01" : "2026-06-30";
        datos.hora = i % 2 == 0 ? "00:00:00" : "23:59:59";
        datos.montoCentavos = i;
        referencia.push_back(registro.crear(datos));
    }
    std::sort(referencia.begin(), referencia.end(), [](auto a, auto b) {
        return compararCronologicamente(*a, *b) < 0;
    });
    for (auto pedido : referencia) exigir(avl.insertar(pedido), "Insertar 20k ordenados.");
    exigir(avl.verificarInvariantes(), "Balance tras 20k inserciones ordenadas.");
    std::vector<const Pedido*> ordenados(20000);
    exigir(avl.obtenerPrimeros(ordenados.data(), ordenados.size()) == 20000, "Recorrido completo.");
    for (std::size_t i = 0; i < referencia.size(); ++i)
        exigir(ordenados[i] == referencia[i], "Orden e identidad de pedidos.");
    const Pedido* primeros[30];
    exigir(avl.obtenerPrimeros(primeros, 30) == 30, "Limite de resultados.");
    for (std::size_t i = 0; i < 30; ++i) exigir(primeros[i] == referencia[i], "Primeros 30.");

    std::size_t cantidad = 0;
    std::int64_t monto = 0;
    for (auto pedido : referencia) {
        if (pedido->datos().fecha == "2026-06-30") {
            ++cantidad;
            monto += pedido->datos().montoCentavos;
        }
    }
    const auto resumen = avl.consultarRango("2026-06-30", "2026-06-30");
    exigir(resumen.cantidad == cantidad && resumen.montoTotalCentavos == monto,
           "Rango incluye todas las horas e IDs de la fecha.");
    exigir(avl.consultarRango("2027-01-01", "2027-01-31").cantidad == 0, "Rango vacio.");
    const auto completo = avl.consultarRango("2026-06-01", "2026-07-31");
    exigir(completo.cantidad == 20000 && completo.montoTotalCentavos == 200010000, "Rango completo.");

    std::mt19937 aleatorio(652);
    std::shuffle(referencia.begin(), referencia.end(), aleatorio);
    for (std::size_t i = 0; i < referencia.size(); ++i) {
        exigir(avl.eliminar(*referencia[i]), "Eliminar aleatoriamente.");
        if (i % 100 == 0 || i + 1 == referencia.size())
            exigir(avl.verificarInvariantes(), "Balance durante eliminaciones.");
        exigir(registro.liberar(referencia[i]), "Liberar despues de retirar referencia del AVL.");
    }
    exigir(avl.cantidad() == 0 && registro.cantidad() == 0, "Sin pedidos restantes.");
}
}

int main() {
    try {
        probarRotaciones();
        probarVolumenYRangos();
        std::cout << "Pruebas del AVL correctas.\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "PRUEBA FALLIDA: " << error.what() << '\n';
        return 1;
    }
}
