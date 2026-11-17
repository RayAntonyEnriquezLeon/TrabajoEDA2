#include "logistica/MinHeap.hpp"
#include <algorithm>
#include <limits>
#include <stdexcept>
using namespace std;

namespace logistica {

MinHeap::MinHeap() {
    capacidad = 10000;
    cantidadNodos = 0;
    arr = new Pedido*[capacidad];
}

MinHeap::~MinHeap() {
    vaciar();
    delete[] arr;
}

void MinHeap::redimensionar() {
    if (capacidad > std::numeric_limits<int>::max() / 2 ||
        static_cast<std::size_t>(capacidad) > std::numeric_limits<std::size_t>::max() / sizeof(Pedido*) / 2)
        throw std::length_error("Heap demasiado grande.");
    int nuevaCapacidad = capacidad * 2;
    Pedido** nuevoArr = new Pedido*[nuevaCapacidad];
    for (int i = 0; i < cantidadNodos; i++) {
        nuevoArr[i] = arr[i];
    }
    delete[] arr;
    arr = nuevoArr;
    capacidad = nuevaCapacidad;
}

void MinHeap::siftUp(int actual) {
    while (actual > 0) {
        int padre = (actual - 1) / 2;
        if (precedeEnDespacho(*arr[actual], *arr[padre])) {
            swap(arr[actual], arr[padre]);
            fijarPosicion(*arr[actual], actual);
            fijarPosicion(*arr[padre], padre);
            actual = padre;
        } else {
            break;
        }
    }
}

void MinHeap::siftDownMin(int i, int n) {
    while (i < n / 2) {
        int menor = 2 * i + 1;
        if (menor + 1 < n && precedeEnDespacho(*arr[menor + 1], *arr[menor])) {
            menor++;
        }
        if (!precedeEnDespacho(*arr[menor], *arr[i])) {
            break;
        }
        swap(arr[i], arr[menor]);
        fijarPosicion(*arr[i], i);
        fijarPosicion(*arr[menor], menor);
        i = menor;
    }
}

bool MinHeap::insertar(Pedido* pedido) {
    if (pedido == nullptr) return false;
    if (pedido->posicionHeap() != -1) return false;

    if (cantidadNodos == capacidad) {
        redimensionar();
    }

    arr[cantidadNodos] = pedido;
    fijarPosicion(*pedido, cantidadNodos);
    int actual = cantidadNodos;
    cantidadNodos++;

    siftUp(actual);
    return true;
}

const Pedido* MinHeap::verMinimo() const noexcept {
    if (cantidadNodos == 0) return nullptr;
    return arr[0];
}

Pedido* MinHeap::extraerMinimo() noexcept {
    if (cantidadNodos == 0) return nullptr;

    Pedido* minVal = arr[0];
    fijarPosicion(*minVal, -1);

    cantidadNodos--;
    if (cantidadNodos > 0) {
        arr[0] = arr[cantidadNodos];
        fijarPosicion(*arr[0], 0);
        siftDownMin(0, cantidadNodos);
    }
    return minVal;
}

bool MinHeap::eliminar(Pedido* pedido) noexcept {
    if (pedido == nullptr) return false;
    int i = pedido->posicionHeap();
    if (i < 0 || i >= cantidadNodos || arr[i] != pedido) return false;

    fijarPosicion(*pedido, -1);
    cantidadNodos--;

    if (i < cantidadNodos) {
        arr[i] = arr[cantidadNodos];
        fijarPosicion(*arr[i], i);
        siftUp(i);
        siftDownMin(i, cantidadNodos);
    }
    return true;
}

size_t MinHeap::cantidad() const noexcept {
    return cantidadNodos;
}

void MinHeap::vaciar() noexcept {
    for (int i = 0; i < cantidadNodos; i++) {
        fijarPosicion(*arr[i], -1);
    }
    cantidadNodos = 0;
}

}
