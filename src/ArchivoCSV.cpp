#include "logistica/ArchivoCSV.hpp"
#include <chrono>
#include <fstream>
#include <limits>
#include <ostream>

namespace logistica {
namespace {
const char* cabecera = "idPedido,dniCliente,tipoEnvio,nivelPrioridad,montoDolares,fecha,hora,estado";

bool enteroPositivo(const std::string& texto, std::int64_t& valor) {
    if (texto.empty()) return false;
    valor = 0;
    for (char caracter : texto) {
        if (caracter < '0' || caracter > '9') return false;
        const int digito = caracter - '0';
        if (valor > (std::numeric_limits<std::int64_t>::max() - digito) / 10) return false;
        valor = valor * 10 + digito;
    }
    return true;
}
bool montoExacto(const std::string& texto, std::int64_t& centavos) {
    const auto punto = texto.find('.');
    if (punto == std::string::npos || punto == 0 || texto.size() - punto != 3) return false;
    std::int64_t dolares = 0, fraccion = 0;
    if (!enteroPositivo(texto.substr(0, punto), dolares) ||
        !enteroPositivo(texto.substr(punto + 1), fraccion)) return false;
    if (dolares > (std::numeric_limits<std::int64_t>::max() - fraccion) / 100) return false;
    centavos = dolares * 100 + fraccion;
    return true;
}
void anotarError(ReporteCarga& reporte, std::size_t linea, const std::string& mensaje) {
    if (reporte.primerError.empty()) {
        reporte.primeraLineaError = linea;
        reporte.primerError = mensaje;
    }
}
}

bool interpretarFilaCSV(const std::string& fila, DatosPedido& datos, std::string& error) {
    datos = {};
    error.clear();
    std::string campos[8];
    std::size_t inicio = 0;
    for (std::size_t i = 0; i < 8; ++i) {
        const auto fin = fila.find(',', inicio);
        if ((i < 7 && fin == std::string::npos) || (i == 7 && fin != std::string::npos)) {
            error = "Se esperaban exactamente ocho columnas.";
            return false;
        }
        campos[i] = fila.substr(inicio, fin == std::string::npos ? fin : fin - inicio);
        if (campos[i].empty() || campos[i].find('"') != std::string::npos) {
            error = "No se admiten campos vacios ni entrecomillados.";
            return false;
        }
        if (i < 7) inicio = fin + 1;
    }
    datos.idPedido = campos[0];
    datos.dniCliente = campos[1];
    if (!interpretarTipoEnvio(campos[2], datos.tipoEnvio)) { error = "Tipo de envio invalido."; return false; }
    std::int64_t prioridad = 0;
    if (!enteroPositivo(campos[3], prioridad) || prioridad < 1 || prioridad > 10) {
        error = "Prioridad invalida."; return false;
    }
    datos.nivelPrioridad = static_cast<int>(prioridad);
    if (!montoExacto(campos[4], datos.montoCentavos)) {
        error = "Monto invalido; usar dolares y exactamente dos decimales."; return false;
    }
    datos.fecha = campos[5];
    datos.hora = campos[6];
    if (!interpretarEstado(campos[7], datos.estado)) { error = "Estado desconocido."; return false; }
    error = validarNuevoPedido(datos);
    return error.empty();
}

ReporteCarga cargarCSV(const std::string& ruta, SistemaLogistico& sistema) {
    const auto inicio = std::chrono::steady_clock::now();
    ReporteCarga reporte;
    const auto terminar = [&]() {
        reporte.tiempoTotalMs = std::chrono::duration<double, std::milli>(
            std::chrono::steady_clock::now() - inicio).count();
        return reporte;
    };
    std::ifstream archivo(ruta, std::ios::binary);
    if (!archivo) { anotarError(reporte, 0, "No se pudo abrir el archivo."); return terminar(); }
    reporte.archivoAbierto = true;
    std::string fila;
    if (!std::getline(archivo, fila)) { anotarError(reporte, 1, "Falta la cabecera."); return terminar(); }
    if (!fila.empty() && fila.back() == '\r') fila.pop_back();
    if (fila.compare(0, 3, "\xEF\xBB\xBF") == 0) fila.erase(0, 3);
    if (fila != cabecera) { anotarError(reporte, 1, "Cabecera CSV incorrecta."); return terminar(); }
    reporte.cabeceraValida = true;
    std::size_t linea = 1;
    while (std::getline(archivo, fila)) {
        ++linea;
        if (!fila.empty() && fila.back() == '\r') fila.pop_back();
        DatosPedido datos;
        std::string error;
        if (!interpretarFilaCSV(fila, datos, error)) {
            ++reporte.rechazados;
            anotarError(reporte, linea, error);
            continue;
        }
        TiemposInsercion tiempos;
        const auto resultado = sistema.registrarPedido(datos, &tiempos);
        if (!resultado.correcto()) {
            ++reporte.rechazados;
            if (resultado.codigo == CodigoResultado::IdDuplicado) ++reporte.duplicados;
            anotarError(reporte, linea, resultado.detalle);
            if (resultado.codigo == CodigoResultado::SinMemoria || resultado.codigo == CodigoResultado::ErrorIndice) break;
            continue;
        }
        ++reporte.aceptados;
        reporte.tiemposIndices.hashMs += tiempos.hashMs;
        reporte.tiemposIndices.avlMs += tiempos.avlMs;
        reporte.tiemposIndices.heapMs += tiempos.heapMs;
    }
    if (archivo.bad()) anotarError(reporte, linea + 1, "Fallo de lectura del archivo.");
    return terminar();
}

void escribirPedidoCSV(std::ostream& salida, const Pedido& pedido) {
    const auto& d = pedido.datos();
    const auto centavos = d.montoCentavos % 100;
    salida << d.idPedido << ',' << d.dniCliente << ',' << texto(d.tipoEnvio) << ','
           << d.nivelPrioridad << ',' << d.montoCentavos / 100 << '.'
           << (centavos < 10 ? "0" : "") << centavos << ',' << d.fecha << ',' << d.hora
           << ',' << texto(d.estado);
}
}
