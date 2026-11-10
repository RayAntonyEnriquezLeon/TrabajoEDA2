[CmdletBinding()]
param(
    [switch]$ConIndicesEquipo,
    [switch]$MenuAVL,
    [string]$Python = ''
)

$ErrorActionPreference = 'Stop'
$raizProyecto = Split-Path -Parent $PSScriptRoot
$cmakeDetectado = Get-Command cmake -ErrorAction SilentlyContinue
if ($cmakeDetectado) {
    $ejecutableCMake = $cmakeDetectado.Source
} else {
    $vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
    if (-not (Test-Path -LiteralPath $vswhere)) {
        throw 'No se encontro CMake. Instalar CMake y un compilador C++17 o Visual Studio con Desarrollo de escritorio con C++.'
    }
    $instalacionVS = & $vswhere -latest -products '*' -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
    if (-not $instalacionVS) { throw 'No se encontro el compilador C++ de Visual Studio.' }
    $ejecutableCMake = Join-Path $instalacionVS 'Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe'
    if (-not (Test-Path -LiteralPath $ejecutableCMake)) { throw 'Instalar el componente CMake de Visual Studio o agregar CMake al PATH.' }
}

$carpetaBuild = Join-Path $raizProyecto 'build'
$argumentos = @('-S', $raizProyecto, '-B', $carpetaBuild, '-DBUILD_TESTING=ON')
if ($ConIndicesEquipo) { $argumentos += '-DLOGISTICA_CONSTRUIR_APP=ON' }
else { $argumentos += '-DLOGISTICA_CONSTRUIR_APP=OFF' }
if ($Python) { $argumentos += "-DPython3_EXECUTABLE=$Python" }

& $ejecutableCMake @argumentos
if ($LASTEXITCODE -ne 0) { throw 'Fallo la configuracion de CMake.' }
& $ejecutableCMake --build $carpetaBuild --config Debug
if ($LASTEXITCODE -ne 0) { throw 'Fallo la compilacion.' }
if ($MenuAVL) {
    $menuEjecutable = Join-Path $carpetaBuild 'Debug\menu_avl.exe'
    if (-not (Test-Path -LiteralPath $menuEjecutable)) {
        $menuEjecutable = Join-Path $carpetaBuild 'menu_avl.exe'
    }
    Push-Location $raizProyecto
    try { & $menuEjecutable }
    finally { Pop-Location }
    if ($LASTEXITCODE -ne 0) { throw 'El menu AVL termino con un error.' }
    return
}
$ejecutableCTest = Join-Path (Split-Path -Parent $ejecutableCMake) 'ctest.exe'
if (-not (Test-Path -LiteralPath $ejecutableCTest)) {
    $comandoCTest = Get-Command ctest -ErrorAction Stop
    $ejecutableCTest = $comandoCTest.Source
}
& $ejecutableCTest --test-dir $carpetaBuild -C Debug --output-on-failure
if ($LASTEXITCODE -ne 0) { throw 'Hay pruebas fallidas.' }
Write-Host 'Compilacion y pruebas correctas.'
