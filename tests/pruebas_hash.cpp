#include "logistica/RegistroPedidos.hpp"
#include "logistica/TablaHash.hpp"
#include <algorithm>
#include <iostream>
#include <random>
#include <stdexcept>
#include <vector>

using namespace logistica;

namespace {
void exigir(bool condicion, const char* mensaje) {
    if (!condicion) throw std::runtime_error(mensaje);
}

DatosPedido ejemplo(int numero) {
    DatosPedido d;
    const auto digitos = std::to_string(numero);
    d.idPedido = "PD-" + std::string(6 - digitos.size(), '0') + digitos;
    d.dniCliente = "00123456";
    d.fecha = "2026-06-01";
    d.hora = "08:30:00";
    d.montoCentavos = numero + 100;
    return d;
}

void probarHash() {
    RegistroPedidos registro;
    TablaHash hash;
    exigir(!hash.buscar("PD-999999") && !hash.eliminar("PD-999999"), "Hash vacio.");
    exigir(!hash.insertar(nullptr), "Rechazar puntero nulo.");
    std::vector<Pedido*> referencia;
    for (int i = 1; i <= 25000; ++i) {
        Pedido* pedido = registro.crear(ejemplo(i));
        referencia.push_back(pedido);
        exigir(hash.insertar(pedido), "Insercion.");
        exigir(!hash.insertar(pedido), "Rechazar insercion repetida.");
    }
    exigir(hash.cantidad() == 25000 && hash.capacidad() > 1024, "Crecimiento y cantidad.");
    exigir(hash.cubetasConColision() > 0 && hash.longitudMaximaCadena() > 1,
           "Ejercitar colisiones reales.");
    for (auto pedido : referencia)
        exigir(hash.buscar(pedido->datos().idPedido) == pedido, "Identidad tras redistribucion.");

    auto duplicado = ejemplo(1);
    duplicado.fecha = "2026-07-01";
    exigir(!hash.insertar(registro.crear(duplicado)), "Rechazar mismo ID con otros datos.");
    exigir(hash.buscar(duplicado.idPedido) == referencia.front(), "Conservar original.");

    std::mt19937 azar(652);
    std::shuffle(referencia.begin(), referencia.end(), azar);
    for (std::size_t i = 0; i < referencia.size(); ++i) {
        auto pedido = referencia[i];
        const auto id = pedido->datos().idPedido;
        exigir(hash.eliminar(id) && !hash.eliminar(id), "Eliminar una sola vez.");
        exigir(!hash.buscar(id), "Busqueda tras eliminar.");
        exigir(hash.cantidad() == referencia.size() - i - 1, "Contador tras eliminar.");
        exigir(pedido->datos().idPedido == id, "Conservar pedido del registro.");
        if (i + 1 < referencia.size())
            exigir(hash.buscar(referencia[i + 1]->datos().idPedido) == referencia[i + 1],
                   "Conservar otros elementos de las cadenas.");
    }
    exigir(hash.cubetasConColision() == 0 && hash.longitudMaximaCadena() == 0, "Tabla agotada.");
    exigir(hash.insertar(referencia.front()), "Reutilizar tras eliminar todo.");
    hash.vaciar();
    hash.vaciar();
    exigir(hash.cantidad() == 0 && !hash.buscar(referencia.front()->datos().idPedido), "Vaciar.");
    exigir(hash.insertar(referencia.back()), "Reutilizar tras vaciar.");
}
}

int main() {
    try {
        probarHash();
        std::cout << "Hash: 25000 registros, duplicados, colisiones, crecimiento, "
                     "eliminaciones y reutilizacion correctos.\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
