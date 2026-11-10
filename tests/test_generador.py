"""Pruebas del contrato de reproducibilidad, no modifican los CSV entregados"""

import csv
import hashlib
import io
import sys
import tempfile
import unittest
from datetime import date, time
from pathlib import Path

RAIZ = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(RAIZ / "herramientas"))
from generar_csv import CABECERA, escribir_csv, generar_filas  # noqa: E402


class PruebasGenerador(unittest.TestCase):
    def test_mismos_bytes_y_prefijo_20k(self):
        with tempfile.TemporaryDirectory() as directorio:
            a, b, grande = (Path(directorio) / nombre for nombre in ("a.csv", "b.csv", "grande.csv"))
            escribir_csv(a, 652, 10000)
            escribir_csv(b, 652, 10000)
            self.assertEqual(a.read_bytes(), b.read_bytes())
            escribir_csv(grande, 652, 20000)
            self.assertEqual(a.read_bytes().splitlines(keepends=True), grande.read_bytes().splitlines(keepends=True)[:10001])
            self.assertNotIn(b"\r", a.read_bytes())
            self.assertFalse(a.read_bytes().startswith(b"\xef\xbb\xbf"))

    def test_semilla_distinta_y_extremos(self):
        self.assertNotEqual(list(generar_filas(1, 30)), list(generar_filas(2, 30)))
        self.assertEqual(list(generar_filas(0, 30)), list(generar_filas(0, 31))[:30])
        self.assertEqual(len(list(generar_filas((1 << 64) - 1, 30))), 30)
        for semilla, cantidad in ((-1, 30), (1 << 64, 30), (652, 0), (652, 20001)):
            with self.assertRaises(ValueError):
                list(generar_filas(semilla, cantidad))

    def test_datos_validos_y_casos_de_borde(self):
        filas = list(generar_filas(652, 20000))
        self.assertEqual(len({fila[0] for fila in filas}), 20000)
        for numero, fila in enumerate(filas, 1):
            self.assertEqual(fila[0], f"PD-{numero:06d}")
            self.assertEqual(len(fila[1]), 8)
            self.assertTrue(fila[1].isdigit())
            self.assertIn(fila[2], ("Normal", "Express", "SameDay"))
            self.assertTrue(1 <= int(fila[3]) <= 10)
            self.assertRegex(fila[4], r"^\d+\.\d{2}$")
            self.assertTrue(date(2026, 6, 1) <= date.fromisoformat(fila[5]) <= date(2026, 8, 31))
            time.fromisoformat(fila[6])
            self.assertEqual(fila[7], "Pendiente")
        self.assertEqual(filas[0][5:7], filas[1][5:7])
        self.assertEqual(filas[0][3], filas[1][3])
        self.assertEqual(filas[4][5:7], ("2026-06-30", "23:59:59"))
        self.assertEqual(filas[5][5:7], ("2026-07-01", "00:00:00"))

    def test_formato_y_firma_version_dos(self):
        # Ancla v2: cambiar accidentalmente algoritmo, formato o reglas rompe esto.
        with tempfile.TemporaryDirectory() as directorio:
            for nombre, cantidad in (("pedidos_pequeno.csv", 30), ("pedidos_masivo.csv", 10000)):
                generado = Path(directorio) / nombre
                escribir_csv(generado, 652, cantidad)
                registros = list(csv.reader(io.StringIO(generado.read_text(encoding="utf-8"))))
                self.assertEqual(tuple(registros[0]), CABECERA)
                self.assertEqual(len(registros), cantidad + 1)
            # Firmas fijas de v2; se verifican en temporales para permitir que el
            # equipo regenere datos/ con otra semilla o con 20000 registros.
            self.assertEqual(hashlib.sha256((Path(directorio) / "pedidos_pequeno.csv").read_bytes()).hexdigest(),
                             "a521e47aac0b22b70bc2c1dce7c7f522e2489995c9f1bb09f58cd8e6b12a5fc4")
            self.assertEqual(hashlib.sha256((Path(directorio) / "pedidos_masivo.csv").read_bytes()).hexdigest(),
                             "672a0e234dbeab572056b8825072235790f3dea13d2c589af1ebfeb154317f2d")


if __name__ == "__main__":
    unittest.main()
