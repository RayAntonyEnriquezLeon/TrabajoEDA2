# Como conectar las partes

Para ejecutar el proyecto y validarlo vean [README.md](README.md)

## Estado actual de Emilio-Heap

Hash, Heap y AVL ya tienen implementaciones concretas en `include/logistica/` y `src/`. El ejecutable `sistema_logistico` conecta las tres estructuras. Las instrucciones de copia de plantillas siguientes se conservan como referencia del contrato inicial: **no sobrescribir las implementaciones actuales con las plantillas**.

CTest incluye pruebas del Hash y del sistema integrado, ademas de las pruebas del AVL, Heap, datos y generador. Para ejecutar las seis pruebas y compilar en Release, consultar el README. El informe y las mediciones finales siguen pendientes.

La idea es usar un solo `SistemaLogistico`, asi cada uno hace su estructura pero todos trabajan con los mismos pedidos

## Que hace cada uno

- `SistemaLogistico` coordina registro, despacho, estados y cancelaciones
- `RegistroPedidos` crea y libera los pedidos
- `TablaHash` busca por ID y evita duplicados
- `ArbolAVL` ordena por fecha y hora y consulta rangos
- `MinHeap` guarda pendientes y saca primero los mas urgentes
- El menu muestra resultados y el lector CSV manda cada fila a `registrarPedido`

Las estructuras no se llaman entre ellas ni crean otro sistema logistico

## Archivos de sus partes de hash y heap

Para Hash, copiar `integracion/TablaHash.hpp.in` a `include/logistica/TablaHash.hpp` y crear `src/TablaHash.cpp`

Para Heap, copiar `integracion/MinHeap.hpp.in` a `include/logistica/MinHeap.hpp` y crear `src/MinHeap.cpp`

Las plantillas describen los metodos del contrato inicial; las variables privadas y algoritmos ya estan implementados en los archivos concretos.

Mantengan los nombres `logistica::TablaHash` y `logistica::MinHeap`, ambas deben crearse vacias y sin argumentos

Las interfaces estan en `include/logistica/ContratosIndices.hpp`, si quieren cambiar una firma primero lo conversamos para no romper la coneccion

## Ojo con los punteros

Cada pedido se crea una sola vez y las tres estructuras guardan el mismo `Pedido*` esto para evitar sistema aislado o que puedan no tener comunicacion entre los diferentes bloques

- No creen copias ni hagan `delete pedido`, eso lo maneja el registro
- Cada uno libera solo sus nodos o arreglos
- Los datos se leen con `pedido->datos()` y los estados los cambia el sistema
- Cancelar un pedido o vaciar el sistema invalida los punteros correspondientes

## Hash

Heredar de `IIndicePedidos` y completar los metodos de la plantilla

- `insertar` agrega el puntero y devuelve `true`, rechaza IDs repetidos
- `buscar` devuelve el mismo puntero por ID o `nullptr`, debe ser O(1) promedio
- `eliminar` retira por ID y devuelve `false` si no estaba
- `cantidad` devuelve un contador en O(1)
- `vaciar` deja la tabla vacia y lista para reutilizar

Al resolver colisiones comparen el ID completo

## Heap

Heredar de `IColaDespacho` y usar un arreglo propio de `Pedido*`, solo con pendientes

- `insertar` agrega el puntero y actualiza su posicion
- `verMinimo` devuelve el mas urgente sin retirarlo
- `extraerMinimo` lo retira y lo devuelve, sin cambiar su estado
- Ambos devuelven `nullptr` si la cola esta vacia
- `eliminar` retira por posicion en O(log n), devuelve `false` si no estaba
- `cantidad` devuelve los pendientes en O(1) y `vaciar` deja la cola reutilizable

Usen `precedeEnDespacho(*a, *b)`, ya compara prioridad mayor primero, luego fecha y hora mas antiguas y al final ID

Lean la posicion con `pedido->posicionHeap()` y actualicenla con `fijarPosicion(*pedido, indice)` desde el Heap

Cada intercambio actualiza ambas posiciones, al retirar pedidos o vaciar pongan su posicion en `-1`

## Para mantener todo junto

En ambas estructuras, si `insertar` devuelve `false` o lanza una excepcion no debe dejar una insercion a medias

`eliminar` y `vaciar` son `noexcept`, no deben lanzar excepciones ni reservar memoria porque sirven para deshacer registros fallidos

El sistema hace estos pasos

- Registrar → validar, revisar ID en Hash, crear pedido y agregar el mismo puntero a Hash y AVL, tambien al Heap si esta pendiente
- Si falla → retirar las referencias ya agregadas y liberar el pedido
- Despachar → sacar del Heap y cambiar a `EnTransito`, sigue en Hash y AVL
- Actualizar → `Pendiente -> EnTransito -> Entregado`, pasar a `EnTransito` tambien lo retira del Heap
- Cancelar → quitar del Heap si sigue pendiente, retirar de AVL y Hash y recien liberar el pedido

Cambiar a `Cancelado` hace la cancelacion completa y repetir el mismo estado no cambia nada

No pongan lectura de consola, impresiones ni otro camino de registro dentro de sus estructuras

## Como queda la coneccion

`integracion/main.cpp.in` crea `TablaHash` y `MinHeap` y los pasa a `ejecutarConsola`, que crea un solo `SistemaLogistico` con esos indices

Hash y Heap deben vivir mas tiempo que el sistema y no se modifican por fuera mientras estan conectados

Los archivos de `integracion/` son plantillas, las tres estructuras terminan al mismo nivel en `include/logistica/` y `src/`, el AVL ya esta ahi

CMake compila `src/TablaHash.cpp` y `src/MinHeap.cpp` una sola vez en la biblioteca `logistica`, compartida por el menu y las pruebas. Si agregan otros archivos `.cpp` de las estructuras incluyanlos en esa biblioteca, no los dupliquen en cada ejecutable.

El menu `menu_avl` es solo para probar el AVL manualmente, no es otro sistema logistico ni reemplaza sus estructuras

## Datos compartidos

Los pedidos se leen con `pedido->datos()`, el monto se guarda en centavos enteros para sumar sin errores decimales

El AVL compara `(fecha, hora, idPedido)`, los rangos incluyen ambas fechas completas y el menu del sistema muestra hasta 30 pedidos ordenados

El generador crea todos como `Pendiente`, asi los CSV de prueba se insertan en las tres estructuras

El cargador tambien acepta `EnTransito` y `Entregado`, esos solo van a Hash y AVL porque no esperan despacho, no se importan cancelados

El CSV tiene cabecera y ocho columnas en este orden, sin comillas ni comas dentro de los campos

```text
idPedido,dniCliente,tipoEnvio,nivelPrioridad,montoDolares,fecha,hora,estado
```

Usen fechas `YYYY-MM-DD`, horas `HH:MM:SS` y montos como `45.50`
