import argparse
import csv
import random
from datetime import date, timedelta
from pathlib import Path

CABECERA = (
    "idPedido", "dniCliente", "tipoEnvio", "nivelPrioridad",
    "montoDolares", "fecha", "hora", "estado",
)


def generar_filas(semilla, cantidad):
    if not 0 <= semilla < 2**64 or not 1 <= cantidad <= 20000:
        raise ValueError("Usar semilla entre 0 y 2^64 - 1 y cantidad entre 1 y 20000")
    azar = random.Random(semilla)
    bordes = {1: (0, 0), 2: (0, 0), 3: (0, 0), 4: (6, 86399),
              5: (29, 86399), 6: (30, 0), 7: (91, 86399)}

    def entre(minimo, maximo):
        return minimo + int(azar.random() * (maximo - minimo + 1))

    for numero in range(1, cantidad + 1):
        dni = entre(0, 99999999)
        tipo = ("Normal", "Express", "SameDay")[entre(0, 2)]
        prioridad = entre(1, 10)
        monto = entre(100, 500000)
        dia, segundo = entre(0, 91), entre(0, 86399)
        dia, segundo = bordes.get(numero, (dia, segundo))
        if numero <= 3:
            prioridad = 10
        fecha = (date(2026, 6, 1) + timedelta(days=dia)).isoformat()
        hora = f"{segundo // 3600:02d}:{segundo // 60 % 60:02d}:{segundo % 60:02d}"
        yield (f"PD-{numero:06d}", f"{dni:08d}", tipo, str(prioridad),
               f"{monto // 100}.{monto % 100:02d}", fecha, hora, "Pendiente")


def escribir_csv(ruta, semilla, cantidad):
    filas = list(generar_filas(semilla, cantidad))
    ruta = Path(ruta)
    ruta.parent.mkdir(parents=True, exist_ok=True)
    with ruta.open("w", encoding="utf-8", newline="") as archivo:
        escritor = csv.writer(archivo, lineterminator="\n")
        escritor.writerow(CABECERA)
        escritor.writerows(filas)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Genera los CSV reproducibles de pedidos")
    parser.add_argument("--semilla", type=int, default=652)
    parser.add_argument("--cantidad", type=int, default=10000, choices=range(10000, 20001), metavar="10000-20000")
    args = parser.parse_args()
    if not 0 <= args.semilla < 2**64:
        parser.error("La semilla debe estar entre 0 y 2^64 - 1")
    carpeta = Path(__file__).resolve().parents[1] / "datos"
    for nombre, cantidad in (("pedidos_pequeno.csv", 30), ("pedidos_masivo.csv", args.cantidad)):
        escribir_csv(carpeta / nombre, args.semilla, cantidad)
        print(f"{nombre}: {cantidad} pedidos con semilla {args.semilla}")
