#pragma once

#include "logistica/Pedido.hpp"
#include <cstddef>
#include <cstdint>
#include <string>

namespace logistica {

struct ResumenRango {
    std::size_t cantidad = 0;
    std::int64_t montoTotalCentavos = 0;
};

class ArbolAVL {
public:
    ArbolAVL() = default;
    ~ArbolAVL();
    ArbolAVL(const ArbolAVL&) = delete;
    ArbolAVL& operator=(const ArbolAVL&) = delete;

    bool insertar(Pedido* pedido);
    bool eliminar(const Pedido& pedido) noexcept;
    Pedido* buscar(const std::string& fecha, const std::string& hora,
                   const std::string& idPedido) const noexcept;

    std::size_t obtenerPrimeros(const Pedido** salida, std::size_t capacidad) const noexcept;

    ResumenRango consultarRango(const std::string& desde, const std::string& hasta) const noexcept;
    std::size_t cantidad() const noexcept { return cantidad_; }
    void vaciar() noexcept;
    bool verificarInvariantes() const noexcept;

private:
    struct Nodo {
        explicit Nodo(Pedido* valor) : pedido(valor) {}
        Pedido* pedido;
        Nodo* izquierdo = nullptr;
        Nodo* derecho = nullptr;
        int altura = 1;
    };

    static int altura(const Nodo* nodo) noexcept;
    static int balance(const Nodo* nodo) noexcept;
    static void actualizarAltura(Nodo* nodo) noexcept;
    static Nodo* rotarDerecha(Nodo* nodo) noexcept;
    static Nodo* rotarIzquierda(Nodo* nodo) noexcept;
    static Nodo* equilibrar(Nodo* nodo) noexcept;


    static Nodo* insertarNodo(Nodo* nodo, Pedido* pedido, bool& insertado);
    static Nodo* eliminarNodo(Nodo* nodo, const Pedido& pedido, bool& eliminado) noexcept;
    static void recorrerPrimeros(const Nodo* nodo, const Pedido** salida,
                                 std::size_t capacidad, std::size_t& escritos) noexcept;
    static void acumularRango(const Nodo* nodo, const std::string& desde,
                             const std::string& hasta, ResumenRango& resultado) noexcept;
    static void liberarNodos(Nodo* nodo) noexcept;
    static bool verificarNodo(const Nodo* nodo, const Pedido* menor, const Pedido* mayor,
                              int& alturaReal, std::size_t& cantidadReal) noexcept;

    Nodo* raiz_ = nullptr;
    std::size_t cantidad_ = 0;
};

}
