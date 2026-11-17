#pragma once

#include "logistica/SistemaLogistico.hpp"
#include <cstddef>
#include <iosfwd>
#include <string>

namespace logistica {

struct ReporteCarga {
    bool archivoAbierto = false;
    bool cabeceraValida = false;
    std::size_t aceptados = 0;
    std::size_t rechazados = 0;
    std::size_t duplicados = 0;
    std::size_t primeraLineaError = 0;
    std::string primerError;
    double tiempoTotalMs = 0;
    TiemposInsercion tiemposIndices;
    bool completo() const noexcept {
        return archivoAbierto && cabeceraValida && primerError.empty() && rechazados == 0;
    }
};





bool interpretarFilaCSV(const std::string& fila, DatosPedido& datos, std::string& error);
ReporteCarga cargarCSV(const std::string& ruta, SistemaLogistico& sistema);
void escribirPedidoCSV(std::ostream& salida, const Pedido& pedido);

}
