#include <iostream>
#include <string>
#include "logistica/MinHeap.hpp"
#include "logistica/RegistroPedidos.hpp"

using namespace std;
using namespace logistica;

int main() {
    RegistroPedidos registro;
    MinHeap miHeap;

    DatosPedido d1{"PD-000001", "45879123", TipoEnvio::Normal, 1, 4550, "2026-06-02", "10:00:00", EstadoPedido::Pendiente};
    DatosPedido d2{"PD-000002", "77842159", TipoEnvio::Express, 5, 120000, "2026-06-01", "12:00:00", EstadoPedido::Pendiente};
    DatosPedido d3{"PD-000003", "14578963", TipoEnvio::SameDay, 10, 8599, "2026-06-01", "09:00:00", EstadoPedido::Pendiente};
    DatosPedido d4{"PD-000004", "11223344", TipoEnvio::SameDay, 10, 5000, "2026-06-01", "08:00:00", EstadoPedido::Pendiente};

    Pedido* p1 = registro.crear(d1);
    Pedido* p2 = registro.crear(d2);
    Pedido* p3 = registro.crear(d3);
    Pedido* p4 = registro.crear(d4);

    miHeap.insertar(p1);
    miHeap.insertar(p2);
    miHeap.insertar(p3);
    miHeap.insertar(p4);

    if (miHeap.cantidad() != 4) {
        cout << "Error en cantidad al insertar" << endl;
        return 1;
    }

    miHeap.eliminar(p2);
    if (miHeap.cantidad() != 3 || p2->posicionHeap() != -1) {
        cout << "Error al eliminar pedido del Heap" << endl;
        return 1;
    }

    Pedido* primero = miHeap.extraerMinimo();
    Pedido* segundo = miHeap.extraerMinimo();
    Pedido* tercero = miHeap.extraerMinimo();

    if (primero != p4 || segundo != p3 || tercero != p1) {
        cout << "Error en el orden de extraccion del Heap" << endl;
        return 1;
    }

    cout << "Pruebas del Heap completadas con exito." << endl;
    return 0;
}