
#include "logistica/ArchivoCSV.hpp"
#include "logistica/RegistroPedidos.hpp"
#include <chrono>
#include <exception>
#include <fstream>
#include <iostream>
#include <limits>
#include <string>

using namespace logistica;

namespace {
bool leer(const char* mensaje, std::string& valor) {
    std::cout << mensaje;
    return static_cast<bool>(std::getline(std::cin, valor));
}

void mostrarMonto(std::int64_t centavos) {
    std::cout << centavos / 100 << '.' << (centavos % 100 < 10 ? "0" : "") << centavos % 100;
}

class SesionAVL {
public:
    bool agregar(const std::string& fila, std::string& error) {
        DatosPedido datos;
        if (!interpretarFilaCSV(fila, datos, error)) return false;
        if (datos.montoCentavos > std::numeric_limits<std::int64_t>::max() - monto_) {
            error = "El monto acumulado excederia el limite numerico";
            return false;
        }
        Pedido* pedido = registro_.crear(datos);
        bool insertado;
        try { insertado = avl_.insertar(pedido); }
        catch (...) { registro_.liberar(pedido); throw; }
        if (!insertado) {
            registro_.liberar(pedido);
            error = "Ya existe esa combinacion de fecha, hora e ID";
            return false;
        }
        monto_ += datos.montoCentavos;
        return true;
    }

    void cargar(const std::string& ruta) {
        std::ifstream archivo(ruta, std::ios::binary);
        if (!archivo) { std::cout << "No se pudo abrir el archivo\n"; return; }
        std::string fila;
        std::getline(archivo, fila);
        if (!fila.empty() && fila.back() == '\r') fila.pop_back();
        if (fila.compare(0, 3, "\xEF\xBB\xBF") == 0) fila.erase(0, 3);
        if (fila != "idPedido,dniCliente,tipoEnvio,nivelPrioridad,montoDolares,fecha,hora,estado") {
            std::cout << "Cabecera incorrecta\n"; return;
        }
        const auto inicio = std::chrono::steady_clock::now();
        std::size_t aceptados = 0, rechazados = 0, linea = 1;
        while (std::getline(archivo, fila)) {
            ++linea;
            if (!fila.empty() && fila.back() == '\r') fila.pop_back();
            std::string error;
            if (agregar(fila, error)) ++aceptados;
            else {
                if (rechazados == 0) std::cout << "Primer rechazo, linea " << linea << ": " << error << '\n';
                ++rechazados;
            }
        }
        const double ms = std::chrono::duration<double, std::milli>(
            std::chrono::steady_clock::now() - inicio).count();
        std::cout << "Aceptados: " << aceptados << "; rechazados: " << rechazados
                  << "; lectura e insercion en AVL: " << ms << " ms\n";
        if (archivo.bad()) std::cout << "La lectura se interrumpio por un error\n";
    }

    bool ejecutar() {
        std::string opcion;
        std::cout << "\nMENU MANUAL DEL AVL\n"
                     "Usa el AVL real y su registro, sin Hash ni Heap\n"
                     "La unicidad global por ID se comprobara en el sistema completo\n";
        while (true) {
            std::cout << "\n1 Cargar CSV\n2 Agregar pedido como fila CSV\n"
                         "3 Mostrar primeros 30 por fecha y hora\n4 Consultar rango de fechas\n"
                         "5 Eliminar por fecha, hora e ID\n6 Ver cantidad y balance\n"
                         "7 Vaciar pedidos de esta sesion\n0 Salir\n";
            if (!leer("Opcion: ", opcion) || opcion == "0") return true;
            if (opcion == "1") {
                std::string ruta;
                if (!leer("Ruta CSV (Enter = datos/pedidos_pequeno.csv): ", ruta)) return true;
                cargar(ruta.empty() ? "datos/pedidos_pequeno.csv" : ruta);
            } else if (opcion == "2") {
                std::cout << "Ejemplo: PD-020001,00123456,Express,10,45.50,2026-06-10,12:00:00,Pendiente\n";
                std::string fila, error;
                if (!leer("Fila: ", fila)) return true;
                std::cout << (agregar(fila, error) ? "Pedido agregado" : error) << '\n';
            } else if (opcion == "3") {
                const Pedido* pedidos[30];
                const auto cantidad = avl_.obtenerPrimeros(pedidos, 30);
                for (std::size_t i = 0; i < cantidad; ++i) {
                    escribirPedidoCSV(std::cout, *pedidos[i]);
                    std::cout << '\n';
                }
                std::cout << "Mostrados: " << cantidad << '\n';
            } else if (opcion == "4") {
                std::string desde, hasta;
                if (!leer("Desde (YYYY-MM-DD): ", desde) || !leer("Hasta (YYYY-MM-DD): ", hasta)) return true;
                if (!fechaValida(desde) || !fechaValida(hasta) || desde > hasta) {
                    std::cout << "Rango invalido\n"; continue;
                }
                const auto inicio = std::chrono::steady_clock::now();
                const auto resumen = avl_.consultarRango(desde, hasta);
                const double ms = std::chrono::duration<double, std::milli>(
                    std::chrono::steady_clock::now() - inicio).count();
                std::cout << "Pedidos: " << resumen.cantidad << "; monto USD: ";
                mostrarMonto(resumen.montoTotalCentavos);
                std::cout << "; tiempo: " << ms << " ms\n";
            } else if (opcion == "5") {
                std::string fecha, hora, id;
                if (!leer("Fecha: ", fecha) || !leer("Hora: ", hora) || !leer("ID: ", id)) return true;
                Pedido* pedido = avl_.buscar(fecha, hora, id);
                if (!pedido) { std::cout << "Pedido no encontrado\n"; continue; }
                avl_.eliminar(*pedido);
                monto_ -= pedido->datos().montoCentavos;
                registro_.liberar(pedido);
                std::cout << "Pedido eliminado del AVL\n";
            } else if (opcion == "6") {
                std::cout << "Pedidos: " << avl_.cantidad() << "; AVL valido: "
                          << (avl_.verificarInvariantes() ? "si" : "no") << '\n';
            } else if (opcion == "7") {
                avl_.vaciar();
                registro_.vaciar();
                monto_ = 0;
                std::cout << "Sesion vaciada, los archivos CSV no cambian\n";
            } else std::cout << "Opcion invalida\n";
        }
    }

private:
    RegistroPedidos registro_;
    ArbolAVL avl_;
    std::int64_t monto_ = 0;
};
}

int main() {
    try {
        SesionAVL sesion;
        sesion.ejecutar();
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
