cmake_minimum_required(VERSION 3.16)
project(LogisticaEDA2 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(logistica
    src/Pedido.cpp
    src/RegistroPedidos.cpp
    src/ArbolAVL.cpp
    src/SistemaLogistico.cpp
    src/ArchivoCSV.cpp
    src/Consola.cpp
    src/MinHeap.cpp
    src/TablaHash.cpp
)
target_include_directories(logistica PUBLIC include)
add_executable(menu_avl src/menu_avl.cpp)
target_link_libraries(menu_avl PRIVATE logistica)
if(MSVC)
    target_compile_options(logistica PRIVATE /W4 /permissive- /utf-8)
else()
    target_compile_options(logistica PRIVATE -Wall -Wextra -Wpedantic)
endif()

include(CTest)
if(BUILD_TESTING)
    add_executable(pruebas_avl tests/pruebas_avl.cpp)
    target_link_libraries(pruebas_avl PRIVATE logistica)
    add_test(NAME pruebas_avl COMMAND pruebas_avl)

    add_executable(pruebas_heap tests/pruebas_heap.cpp)
    target_link_libraries(pruebas_heap PRIVATE logistica)
    add_test(NAME pruebas_heap COMMAND pruebas_heap)

    add_executable(pruebas_hash tests/pruebas_hash.cpp)
    target_link_libraries(pruebas_hash PRIVATE logistica)
    add_test(NAME pruebas_hash COMMAND pruebas_hash)

    add_executable(pruebas_integracion tests/pruebas_integracion.cpp)
    target_link_libraries(pruebas_integracion PRIVATE logistica)
    add_test(NAME pruebas_integracion COMMAND pruebas_integracion "${CMAKE_CURRENT_SOURCE_DIR}/datos/pedidos_masivo.csv")

    add_executable(pruebas_datos tests/pruebas_datos.cpp)
    target_link_libraries(pruebas_datos PRIVATE logistica)
    add_test(NAME pruebas_datos COMMAND pruebas_datos "${CMAKE_CURRENT_SOURCE_DIR}/datos")

    add_executable(pruebas_consola tests/pruebas_consola.cpp)
    target_link_libraries(pruebas_consola PRIVATE logistica)
    add_test(NAME pruebas_consola COMMAND pruebas_consola "${CMAKE_CURRENT_SOURCE_DIR}/datos")

    find_package(Python3 3.9 COMPONENTS Interpreter QUIET)
    if(Python3_Interpreter_FOUND)
        add_test(NAME generador_reproducible
            COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/tests/test_generador.py")
    else()
        message(WARNING "Pruebas del generador omitidas: indicar -DPython3_EXECUTABLE con Python 3.9 o posterior.")
    endif()
endif()

option(LOGISTICA_CONSTRUIR_APP "Compilar el menu con el Hash y el Heap del equipo" ON)
if(LOGISTICA_CONSTRUIR_APP)
    foreach(archivo include/logistica/TablaHash.hpp include/logistica/MinHeap.hpp src/TablaHash.cpp src/MinHeap.cpp)
        if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${archivo}")
            message(FATAL_ERROR "Falta ${archivo}. Consultar include/logistica/ContratosIndices.hpp.")
        endif()
    endforeach()
    configure_file(integracion/main.cpp.in "${CMAKE_CURRENT_BINARY_DIR}/main.cpp" COPYONLY)
    add_executable(sistema_logistico "${CMAKE_CURRENT_BINARY_DIR}/main.cpp")
    target_link_libraries(sistema_logistico PRIVATE logistica)
endif()
