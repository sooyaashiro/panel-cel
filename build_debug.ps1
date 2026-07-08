$ErrorActionPreference = "Stop"
$root = "C:\Users\Rize\panel-cel"
$env:PATH = "C:\Users\Rize\dev\w64devkit\bin;" + $env:PATH

$src = "$root\src\main_debug.cpp"
$out = "$root\main_debug.exe"

Write-Output "Compilando CON CONSOLA $src ..."
# Sin -mwindows para que se vea la consola
g++ $src -o $out -O2 -std=c++17 -municode -lgdi32 -luser32 -lshell32
if ($LASTEXITCODE -ne 0) { throw "g++ fallo con codigo $LASTEXITCODE" }
if (Test-Path $out) { Write-Output "OK -> $out" } else { throw "No se genero $out" }
