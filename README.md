# Proyecto EDA II

Para ver como se conectan las partes del equipo esta [INTEGRACION.md](INTEGRACION.md), aca queda como ejecutar y comprobar lo que ya tenemos

## Que ya esta hecho

- AVL con insercion, eliminacion y rotaciones para mantener el balance
- Orden por fecha, hora e ID, primeros 30 pedidos y consultas por rango con cantidad y monto
- Registro unico de los objetos `Pedido` y coordinacion en `SistemaLogistico`
- Lector CSV, generador con semilla y archivos de 30 y 10000 pedidos
- Menu manual del AVL que ya pueden usar
- Menu del sistema completo preparado para recibir Hash y Heap
- Pruebas del AVL, lectura de datos y reproducibilidad del generador

Todavia faltan las implementaciones reales de Hash y Heap, la coordinacion ya esta escrita pero falta validarla con esas partes

## Que se necesita

Para seguir estos pasos en Windows usen PowerShell y tengan instalado

- Visual Studio o Build Tools con la carga de trabajo de desarrollo de escritorio con C++ y las herramientas CMake o cambien la compilacion a la que consideren conveniente pero tengan como referencia el cmake
- Tambien sirve CMake en el PATH junto con un compilador que soporte C++17
- Python 3.9 o posterior para generar CSV y comprobar la semilla, no necesita instalar paquetes y debe tener la misma csv para todos

igual se necesita el compilador y CMake

Todos los comandos de abajo se ejecutan desde la carpeta raiz del proyecto, donde esta `CMakeLists.txt`

## Abrir el menu del AVL

```powershell
.\herramientas\verificar.ps1 -MenuAVL
```

Compila y abre el menu no ejecuta las pruebas automaticas aunque tambien compile sus archivos

Si ya compilaste y no cambiaste el codigo puedes abrirlo directamente

```powershell
.\build\Debug\menu_avl.exe
```

Esa ruta corresponde a Visual Studio, con otros generadores el ejecutable puede quedar en `build/menu_avl.exe`

## Que probar en el menu

1. Elegir `1` y despues Enter sin escribir una ruta para cargar los 30 pedidos de `datos/pedidos_pequeno.csv`
2. Elegir `3` para verlos ordenados por fecha y hora
3. Elegir `4` y poner `2026-06-01` y `2026-06-30`, debe mostrar cantidad y monto del mes
4. Repetir con `2026-06-01` hasta `2026-06-07` para una semana y con fechas de 2027 para obtener cero resultados
5. Elegir `6` para ver la cantidad y comprobar que diga `AVL valido: si`
6. Elegir `5` e ingresar fecha `2026-06-01`, hora `00:00:00` e ID `PD-000001` para eliminar uno de los pedidos generados
7. Volver a `6`, deberian quedar 29 y el AVL seguir valido

La opcion `2` permite agregar un pedido como una fila completa, por ejemplo

```text
PD-020001,00123456,Express,10,45.50,2026-06-10,12:00:00,Pendiente
```

Para probar volumen, usar `7` para vaciar la sesion y luego `1` con la ruta `datos/pedidos_masivo.csv`

Este menu no modifica los CSV ni guarda cambios al salir, tampoco tiene Hash o Heap simulados, rechaza claves iguales de fecha/hora/ID pero la unicidad del ID entre fechas distintas se comprobara con el Hash real, implementamos todo lo demas en conjunto con lo que avanza ya

## Generar los CSV

El generador requiere Python 3.9 o posterior y no usa dependencias externas(ramdon)

```powershell
python herramientas/generar_csv.py --semilla 652 --cantidad 10000
python herramientas/generar_csv.py --semilla 652 --cantidad 20000
```

Elijan uno de los comandos segun la cantidad que quieran, cada ejecucion reemplaza el CSV masivo y tambien genera el pequeño de 30 pedidos

Misma semilla y misma version del generador dan los mismos bytes, al ampliar a 20000 se conservan las primeras 10000 filas, no depende de la fecha de ejecucion



## Validar con las pruebas automaticas

Esto es aparte del menu manual

```powershell
.\herramientas\verificar.ps1
```

Se compila y CTest ejecuta las pruebas disponibles

| Prueba | Que revisa |
| --- | --- |
| `pruebas_avl` | Rotaciones, inserciones y eliminaciones con 20000 pedidos, orden y rangos |
| `pruebas_datos` | Los dos CSV entregados, campos validos y sumas de semana, mes y rango vacio usando el AVL |
| `generador_reproducible` | Misma semilla, mismos bytes y conservacion del prefijo al ampliar los datos |

Debe aparecer `100% tests passsed`, revisen tambien que esten las tres pruebas porque la de Python se omite con un aviso si CMake no encuentra un interprete compatible

Pueden indicarlo manualmente cambiando esta ruta por la de su Python

```powershell
.\herramientas\verificar.ps1 -Python 'C:\ruta\python.exe'
```

En la computadora donde se preparo el proyecto tambien se puede usar el Python incluido con Codex

```powershell
$python = "$env:USERPROFILE\.cache\codex-runtimes\codex-primary-runtime\dependencies\python\python.exe"
.\herramientas\verificar.ps1 -Python $python
```

## Cuando esten el Hash y el Heap

Primero agreguen los archivos que indica `INTEGRACION.md`, despues ejecuten o solo subalo al repo y ya yo lo hago

```powershell
.\herramientas\verificar.ps1 -ConIndicesEquipo
.\build\Debug\sistema_logistico.exe
```

El menu completo tendra carga CSV, registro, busqueda por ID, despacho, orden cronologico, rangos, estados, cancelacion y estadisticas

Las pruebas actuales no validan automaticamente esas estructuras nuevas, prueben en el menu cargar dos veces el mismo archivo, buscar IDs existentes e inventados, despachar lotes de 10 y 100, actualizar 10 pedidos y cancelar otros 10

Al cancelar, el pedido debe desaparecer de Hash y AVL y tambien del Heap si estaba pendiente, revisen que cantidad, monto y pendientes coincidan

El menu completo ya muestra tiempos de operaciones y carga, falta tomar las mediciones finales usando las estructuras reales y preparar el informe y la sustentacion

La carpeta `build` es salida de compilacion, no hace falta compartirla ni subirla al repo, cada uno la genera en su maquina
