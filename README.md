# Sistema de gestion logistica — EDA II

Proyecto de Estructura de Datos y Algoritmos, Ingenieria de Sistemas, Universidad de Lima.

**Integrantes:** Brigitte Arvorco, Ray Antony y Emilio (completar nombres).

## Estado de esta rama

La rama `Emilio-Heap` incluye las implementaciones propias de **Tabla Hash, Min-Heap y AVL**, conectadas mediante `SistemaLogistico`. Los indices comparten los mismos objetos `Pedido`; `RegistroPedidos` administra su memoria.

Se dispone de carga CSV, registro manual con control de duplicados, busqueda por ID, despacho por lotes, primeros 30 pedidos cronologicos, auditoria por rango, actualizacion de estados, cancelacion y estadisticas O(1).

**Pendiente para la entrega:** informe IEEE, diagrama, mediciones repetidas en Release y evidencias de sustentacion. El PDF del enunciado esta en `doc/`; no es el informe final.

## Requisitos

- Compilador C++17.
- CMake 3.16 o posterior.
- Python 3.9 o posterior para generar datos y ejecutar la prueba de reproducibilidad. El sistema C++ no requiere Python en ejecucion.
- Windows: Visual Studio/Build Tools con Desarrollo de escritorio con C++ y CMake.
- macOS: herramientas de linea de comandos de Xcode y CMake.
- Linux: compilador C++ y CMake.

## Compilar y probar

Ejecutar desde la raiz del proyecto, donde esta `CMakeLists.txt`:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON -DLOGISTICA_CONSTRUIR_APP=ON
cmake --build build --config Release --parallel
ctest --test-dir build -C Release --output-on-failure
```

Son **6 pruebas** cuando se encuentra Python compatible. Si aparecen solo 5, revisar el aviso de CMake e indicar `-DPython3_EXECUTABLE=/ruta/a/python3` en la configuracion.

| Prueba | Cobertura |
| --- | --- |
| `pruebas_avl` | Rotaciones, orden, rangos e inserciones/eliminaciones con 20000 pedidos. |
| `pruebas_heap` | Prioridades, desempate cronologico y eliminacion basica. |
| `pruebas_hash` | 25000 pedidos, duplicados, colisiones, crecimiento, eliminacion y reutilizacion. |
| `pruebas_integracion` | CSV masivo, duplicados, busqueda, orden, rangos, lotes 10/100, estados, cancelacion, estadisticas y reutilizacion. |
| `pruebas_datos` | Formato, validaciones y sumas independientes de los CSV entregados. |
| `generador_reproducible` | Misma semilla, mismos bytes y conservacion del prefijo al ampliar. |

Las pruebas usan colecciones auxiliares como referencia de verificacion; las estructuras del sistema no se sustituyen por contenedores de la STL.

En Windows tambien puede usarse `.\herramientas\verificar.ps1 -ConIndicesEquipo`, que compila y prueba en **Debug**. Para las mediciones del informe usar Release.

## Ejecutar el sistema completo

Desde la raiz del proyecto:

```sh
# macOS/Linux y generadores de una configuracion
./build/sistema_logistico
```

En PowerShell con Visual Studio:

```powershell
.\build\Release\sistema_logistico.exe
```

1. Elegir `1` y escribir `datos/pedidos_pequeno.csv` para empezar con 30 pedidos.
2. Elegir `5` para ver los primeros 30 en orden cronologico.
3. Elegir `6` para consultar `2026-06-01` a `2026-06-07`, luego el mes completo y un rango de 2027.
4. Elegir `3` para buscar un ID existente y uno inexistente.
5. Elegir `4` para despachar y `7`/`8` para actualizar/cancelar.
6. Elegir `9` para comprobar cantidades y monto.

Para la prueba masiva, **cerrar y abrir el programa** y cargar `datos/pedidos_masivo.csv`. El archivo entregado contiene 10000 pedidos pendientes. Cargarlo otra vez debe rechazar los 10000 duplicados sin alterar las estadisticas.

El menu no guarda modificaciones en los CSV ni conserva la sesion al salir. `menu_avl` permite probar solamente el arbol; no reemplaza el sistema integrado.

## Diseno e integracion

- **Hash:** FNV-1a de 64 bits, encadenamiento separado, 1024 cubetas iniciales y crecimiento antes de superar carga 0.75.
- **AVL:** orden por `(fecha, hora, idPedido)`, rotaciones y poda de ramas en consultas de rango.
- **Heap:** arreglo dinamico propio; prioridad numerica mayor primero, luego fecha/hora mas antiguas e ID.
- **Memoria:** un objeto por pedido; los indices no lo destruyen.
- **Montos:** centavos enteros de 64 bits.
- **Estados:** `Pendiente -> EnTransito -> Entregado`; cancelar retira el pedido de los indices y libera su memoria.

Detalles y contratos en [INTEGRACION.md](INTEGRACION.md).

## Generar datos reproducibles

```sh
python3 herramientas/generar_csv.py --semilla 652 --cantidad 10000
```

En Windows puede usarse `python` en lugar de `python3`. **Este comando reemplaza ambos CSV:** genera el pequeno de 30 pedidos y el masivo de la cantidad solicitada (10000 a 20000). La misma semilla y version del generador producen los mismos bytes.

## Limitaciones y entrega

- IDs admitidos: `PD-000001` a `PD-999999`; el formato debe ampliarse para superar ese espacio de identificadores.
- El generador actual limita cada archivo masivo a 20000 pedidos. No se ha demostrado soporte para millones.
- Las mediciones dependen del equipo y la configuracion; una ejecucion aislada no constituye el resultado final del informe.
- No subir `build/`, ejecutables ni archivos temporales. La entrega debe incluir informe PDF, fuentes y CSV utilizados, no depender solo del enlace a GitHub.
