#include <iostream>
#include "logistica/MinHeap.hpp"
#include "logistica/Pedido.hpp"

using namespace std;
using namespace logistica;

int main() {
    MinHeap miHeap;
    
    DatosPedido d1{"PD-1", "123", TipoEnvio::Normal, 5, 100, "2026-11-15", "10:00", EstadoPedido::Pendiente};
    DatosPedido d2{"PD-2", "456", TipoEnvio::Express, 2, 200, "2026-11-15", "11:00", EstadoPedido::Pendiente};

    cout << "Prueba de compilacion del Heap exitosa. Capacidad actual de nodos: " << miHeap.cantidad() << endl;
    
    return 0;
}