# Sistema de gestión logística — EDA II

Universidad de Lima · Ingeniería de Sistemas · Estructura de Datos y Algoritmos.

**Integrantes:** Brigitte Arvorco, Ray Antony y Emilio (completar nombres).

Sistema en C++17 con Tabla Hash, Min-Heap y AVL propios. Permite cargar pedidos desde CSV, registrar, buscar, despachar, consultar por fechas, actualizar estados, cancelar y obtener estadísticas.

## Requisitos

- Compilador C++17 y CMake 3.16 o posterior.
- Python 3.9 o posterior para generar datos y ejecutar la prueba del generador.
- Windows: Visual Studio/Build Tools con Desarrollo de escritorio con C++.
- macOS: herramientas de línea de comandos de Xcode.

## Compilar y probar

Desde la carpeta que contiene `CMakeLists.txt`:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON -DLOGISTICA_CONSTRUIR_APP=ON
cmake --build build --config Release --parallel
ctest --test-dir build -C Release --output-on-failure
```

Se ejecutan **7 pruebas**: AVL, Heap, Hash, integración, datos, consola y generador. Si solo aparecen 6, indicar `-DPython3_EXECUTABLE=/ruta/a/python3` al configurar CMake.

En Windows también puede usarse `.\herramientas\verificar.ps1 -ConIndicesEquipo`; ese script compila y prueba en **Debug**, no en Release.

## Ejecutar

```sh
# macOS/Linux
./build/sistema_logistico
```

```powershell
# Windows con Visual Studio
.\build\Release\sistema_logistico.exe
```

Elegir `1` y cargar uno de estos archivos:

- `datos/pedidos_pequeno.csv`: 30 pedidos para revisión manual.
- `datos/pedidos_masivo.csv`: 10000 pedidos para pruebas de volumen.

Usar una sesión nueva para probar cada archivo por separado. Cargar el mismo archivo dos veces debe rechazar los IDs repetidos. La sesión no se guarda al salir ni modifica los CSV.

El menú permite despachar lotes de 10 o 100, mostrar los primeros 30 pedidos y consultar rangos. Para auditoría pueden usarse `2026-06-01` a `2026-06-07`, el mes completo de junio y un rango vacío de 2027.

Después de cada operación, presionar **Enter para volver al menú**. Los resultados permanecen en el historial de la terminal. La búsqueda muestra campos con etiquetas; los listados usan una tabla. Se eliminan espacios accidentales al principio y al final de las entradas de consola, sin modificar espacios internos ni las validaciones del archivo CSV.

Para entrada automatizada, incluir una línea vacía de pausa después de cada operación. Los tiempos siguen midiendo únicamente la operación, no la impresión ni la pausa. Los estados se introducen como `EnTransito`, `Entregado` o `Cancelado`; seleccionar `Cancelado` elimina el pedido.

## Organización y entrega

- `include/logistica/` y `src/`: estructuras y sistema.
- `tests/`: pruebas automáticas; sus colecciones auxiliares solo sirven como referencia.
- `datos/`: CSV utilizados.
- `herramientas/`: generador reproducible y verificación en Windows.
- [INTEGRACION.md](INTEGRACION.md): diseño y manejo de memoria.
- `doc/`: documentación; el PDF del enunciado no reemplaza el informe final.

El informe IEEE se prepara por separado para la revisión del docente; debe adjuntarse en PDF a la entrega. Los tiempos recopilados son de una sesión manual, no promedios de mediciones repetidas. El formato de ID admite hasta `PD-999999` y el generador hasta 20000 pedidos por archivo; no se ha validado el sistema con millones.

No incluir `build/` ni ejecutables en GitHub. La entrega final es un ZIP con informe PDF, fuentes y datos de prueba.
