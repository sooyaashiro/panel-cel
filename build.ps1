# Compila panel-cel\src\<nombre>.cpp -> panel-cel\<nombre>.exe
# Uso:  .\build.ps1            (launcher, sin consola)
#       .\build.ps1 main debug (con consola)
param([string]$name = "main")

$ErrorActionPreference = "Stop"
$root = "C:\Users\Rize\panel-cel"
$env:PATH = "C:\Users\Rize\dev\w64devkit\bin;" + $env:PATH

$src = "$root\src\$name.cpp"
$out = "$root\$name.exe"

$mwin = "-mwindows"
if ($args -contains "debug") { $mwin = "" }

Write-Output "Compilando $src ..."
g++ $src -o $out -O2 -std=c++17 -municode $mwin `
    -lgdi32 -luser32 -lshell32
if ($LASTEXITCODE -ne 0) { throw "g++ fallo con codigo $LASTEXITCODE" }
if (Test-Path $out) { Write-Output "OK -> $out" } else { throw "No se genero $out" }
