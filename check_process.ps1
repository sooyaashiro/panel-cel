$procs = Get-Process -ErrorAction SilentlyContinue | Where-Object { $_.Id -eq 15144 }
if ($procs) {
    Write-Output "PID 15144 SIGUE VIVO: $($procs.ProcessName)"
} else {
    Write-Output "PID 15144 YA NO EXISTE (murio)"
}

# Buscar cualquier scrcpy corriendo
$scrcpy = Get-Process -ErrorAction SilentlyContinue | Where-Object { $_.ProcessName -eq 'main' -or $_.Path -like '*scrcpy*' }
if ($scrcpy) {
    foreach ($p in $scrcpy) {
        Write-Output "Encontrado: PID=$($p.Id) Name=$($p.ProcessName) Path=$($p.Path)"
    }
} else {
    Write-Output "No hay procesos scrcpy corriendo"
}
