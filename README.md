# Sistema de gestión logística — EDA II

Universidad de Lima · Ingeniería de Sistemas · Estructura de Datos II.

**Integrantes:** Brigitte Zarella Arvorco Leon, Ray Antony Enriquez Leon y Jose Emilio Huari Vivar.

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
- `integracion/main.cpp.in`: entrada del sistema completo, necesaria para compilar.
- `tests/`: pruebas automáticas; sus colecciones auxiliares solo sirven como referencia.
- `datos/`: CSV utilizados.
- `herramientas/`: generador reproducible y verificación en Windows.
- [INTEGRACION.md](INTEGRACION.md): diseño y manejo de memoria.
- `doc/`: documentación; el PDF del enunciado no reemplaza el informe final.

## Identificadores y pruebas de escala

Los IDs admitidos van de `PD-000001` a `PD-9999999`. Cuando coinciden fecha y hora, AVL y Heap comparan numéricamente el ID; Hash utiliza el identificador completo.

Sin `--salida`, el generador produce los CSV de entrega: 30 pedidos y un archivo masivo de entre 10000 y 20000. Para una prueba adicional de dos millones sin reemplazar esos archivos:

```sh
python3 herramientas/generar_csv.py --semilla 652 --cantidad 2000000 --salida datos/pedidos_escala.csv
```

En Windows puede usarse `python` en lugar de `python3`. La opción `--salida` admite de 1 a 9999999 pedidos y reemplaza únicamente el archivo indicado. El volumen que puede cargarse depende de la memoria disponible; el máximo de IDs no garantiza que cualquier equipo pueda cargar ese volumen.

El informe del equipo documenta una ejecución adicional de 2000000 de pedidos en Windows. Los tiempos de esa prueba y los de la sesión de 10000 en Mac corresponden a equipos y volúmenes distintos; no son promedios de repeticiones ni una comparación directa de rendimiento.

## ZIP final

Incluir el informe IEEE final en PDF, `CMakeLists.txt`, `src/`, `include/`, `integracion/`, `datos/`, `tests/`, `herramientas/` y las guías. Conservar y adjuntar los archivos y evidencias de prueba utilizados, incluida la prueba de escala descrita en el informe.

No incluir `build/`, ejecutables ni entornos de Python. El ZIP descargado de GitHub no incluye automáticamente el informe que se edita en Overleaf: agregar su PDF final antes de entregar.
