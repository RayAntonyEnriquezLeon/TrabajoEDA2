#include "logistica/Pedido.hpp"

namespace logistica {
namespace {
bool soloDigitos(const std::string& valor, std::size_t inicio, std::size_t longitud) noexcept {
    for (std::size_t i = inicio; i < inicio + longitud; ++i) {
        if (valor[i] < '0' || valor[i] > '9') return false;
    }
    return true;
}
int numero(const std::string& valor, std::size_t inicio, std::size_t longitud) noexcept {
    int resultado = 0;
    for (std::size_t i = inicio; i < inicio + longitud; ++i) resultado = resultado * 10 + valor[i] - '0';
    return resultado;
}
}

const char* texto(TipoEnvio tipo) noexcept {
    switch (tipo) {
        case TipoEnvio::Normal: return "Normal";
        case TipoEnvio::Express: return "Express";
        case TipoEnvio::SameDay: return "SameDay";
    }
    return "Desconocido";
}
const char* texto(EstadoPedido estado) noexcept {
    switch (estado) {
        case EstadoPedido::Pendiente: return "Pendiente";
        case EstadoPedido::EnTransito: return "EnTransito";
        case EstadoPedido::Entregado: return "Entregado";
        case EstadoPedido::Cancelado: return "Cancelado";
    }
    return "Desconocido";
}
bool interpretarTipoEnvio(const std::string& valor, TipoEnvio& tipo) noexcept {
    if (valor == "Normal") tipo = TipoEnvio::Normal;
    else if (valor == "Express") tipo = TipoEnvio::Express;
    else if (valor == "SameDay") tipo = TipoEnvio::SameDay;
    else return false;
    return true;
}
bool interpretarEstado(const std::string& valor, EstadoPedido& estado) noexcept {
    if (valor == "Pendiente") estado = EstadoPedido::Pendiente;
    else if (valor == "EnTransito") estado = EstadoPedido::EnTransito;
    else if (valor == "Entregado") estado = EstadoPedido::Entregado;
    else if (valor == "Cancelado") estado = EstadoPedido::Cancelado;
    else return false;
    return true;
}
bool fechaValida(const std::string& fecha) noexcept {
    if (fecha.size() != 10 || fecha[4] != '-' || fecha[7] != '-' ||
        !soloDigitos(fecha, 0, 4) || !soloDigitos(fecha, 5, 2) || !soloDigitos(fecha, 8, 2)) return false;
    const int anio = numero(fecha, 0, 4);
    const int mes = numero(fecha, 5, 2);
    const int dia = numero(fecha, 8, 2);
    if (anio < 1 || mes < 1 || mes > 12) return false;
    const int diasPorMes[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    int limite = diasPorMes[mes - 1];
    if (mes == 2 && (anio % 400 == 0 || (anio % 4 == 0 && anio % 100 != 0))) ++limite;
    return dia >= 1 && dia <= limite;
}
bool horaValida(const std::string& hora) noexcept {
    return hora.size() == 8 && hora[2] == ':' && hora[5] == ':' &&
        soloDigitos(hora, 0, 2) && soloDigitos(hora, 3, 2) && soloDigitos(hora, 6, 2) &&
        numero(hora, 0, 2) <= 23 && numero(hora, 3, 2) <= 59 && numero(hora, 6, 2) <= 59;
}
std::string validarNuevoPedido(const DatosPedido& datos) {
    const auto& id = datos.idPedido;
    if ((id.size() != 9 && id.size() != 10) || id.compare(0, 3, "PD-") != 0 ||
        !soloDigitos(id, 3, id.size() - 3) || id == "PD-000000" ||
        (id.size() == 10 && id[3] == '0')) return "ID esperado: PD-000001 a PD-9999999.";
    if (datos.dniCliente.size() != 8 || !soloDigitos(datos.dniCliente, 0, 8)) return "El DNI debe contener ocho digitos.";
    if (datos.tipoEnvio != TipoEnvio::Normal && datos.tipoEnvio != TipoEnvio::Express &&
        datos.tipoEnvio != TipoEnvio::SameDay) return "Tipo de envio invalido.";
    if (datos.nivelPrioridad < 1 || datos.nivelPrioridad > 10) return "La prioridad debe estar entre 1 y 10.";
    if (datos.montoCentavos <= 0) return "El monto debe ser positivo.";
    if (!fechaValida(datos.fecha)) return "Fecha invalida; usar YYYY-MM-DD.";
    if (!horaValida(datos.hora)) return "Hora invalida; usar HH:MM:SS.";
    if (datos.estado != EstadoPedido::Pendiente && datos.estado != EstadoPedido::EnTransito &&
        datos.estado != EstadoPedido::Entregado) return "Estado inicial invalido; no se pueden registrar cancelados.";
    return {};
}
int compararCronologicamente(const std::string& fecha, const std::string& hora,
                             const std::string& id, const Pedido& pedido) noexcept {
    const auto& datos = pedido.datos();
    if (fecha != datos.fecha) return fecha < datos.fecha ? -1 : 1;
    if (hora != datos.hora) return hora < datos.hora ? -1 : 1;
    if (id.size() != datos.idPedido.size()) return id.size() < datos.idPedido.size() ? -1 : 1;
    return id.compare(datos.idPedido);
}
int compararCronologicamente(const Pedido& a, const Pedido& b) noexcept {
    const auto& datos = a.datos();
    return compararCronologicamente(datos.fecha, datos.hora, datos.idPedido, b);
}
bool precedeEnDespacho(const Pedido& a, const Pedido& b) noexcept {
    if (a.datos().nivelPrioridad != b.datos().nivelPrioridad)
        return a.datos().nivelPrioridad > b.datos().nivelPrioridad;
    return compararCronologicamente(a, b) < 0;
}
}
