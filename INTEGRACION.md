# Diseño e integración

Las tres estructuras están implementadas y conectadas mediante `SistemaLogistico`. Para compilar, ejecutar y probar, consultar [README.md](README.md).

## Componentes

| Componente | Responsabilidad |
| --- | --- |
| `SistemaLogistico` | Coordinar operaciones y mantener los índices consistentes. |
| `RegistroPedidos` | Crear y liberar cada objeto `Pedido`. |
| `TablaHash` | Buscar por ID y rechazar duplicados mediante FNV-1a y encadenamiento separado. |
| `ArbolAVL` | Ordenar por `(fecha, hora, idPedido)` y consultar rangos inclusivos. |
| `MinHeap` | Despachar por prioridad numérica mayor, fecha/hora más antiguas e ID. |

La Hash comienza con 1024 cubetas y crece antes de superar una carga de 0.75. El Heap usa un arreglo dinámico propio y guarda la posición de cada pedido para eliminarlo sin búsqueda lineal.

## Punteros y contratos

- Cada pedido existe una sola vez; los índices comparten el mismo `Pedido*`.
- Solo `RegistroPedidos` destruye los pedidos. Cada índice libera únicamente sus nodos o arreglos.
- `IIndicePedidos` e `IColaDespacho`, en `ContratosIndices.hpp`, definen las interfaces.
- Una inserción fallida no debe dejar el pedido insertado parcialmente. `eliminar` y `vaciar` no reservan memoria ni lanzan excepciones.
- Los intercambios del Heap actualizan ambas posiciones; al retirar un pedido, su posición pasa a `-1`.
- Los índices deben estar vacíos al conectar el sistema, sobrevivir a este y no modificarse externamente durante su uso.
- Cancelar, vaciar o destruir el sistema invalida los punteros correspondientes.

## Operaciones

- **Registro:** validar, comprobar ID, crear pedido e insertarlo en Hash y AVL; también en Heap si está pendiente. Si falla, deshacer las inserciones realizadas.
- **Despacho:** extraer del Heap y cambiar a `EnTransito`; conservar el pedido en Hash y AVL.
- **Actualización:** `Pendiente -> EnTransito -> Entregado`. Salir de pendiente lo retira del Heap.
- **Cancelación:** retirar de los índices antes de liberar el objeto. Actualizar a `Cancelado` ejecuta esta misma operación.
- **Estadísticas:** contadores y monto acumulado para consultas O(1). Cancelar descuenta el monto; despachar no.

## Datos y compilación

Los montos se almacenan en centavos enteros. El CSV utiliza cabecera y ocho columnas, sin campos entrecomillados:

```text
idPedido,dniCliente,tipoEnvio,nivelPrioridad,montoDolares,fecha,hora,estado
```

Fechas `YYYY-MM-DD`, horas `HH:MM:SS` y montos como `45.50`. Los CSV entregados contienen pendientes; los pedidos importados como `EnTransito` o `Entregado` no ingresan al Heap. No se importan cancelados.

CMake compila las estructuras una sola vez en la biblioteca `logistica`, compartida por menús y pruebas. `integracion/main.cpp.in` conecta los índices con la consola. Los archivos `TablaHash.hpp.in` y `MinHeap.hpp.in` son plantillas históricas: no se compilan ni deben reemplazar las implementaciones actuales. `menu_avl` es una herramienta de prueba del árbol, no el sistema completo.
