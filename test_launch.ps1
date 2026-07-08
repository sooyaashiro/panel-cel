$p = Start-Process -FilePath 'C:\Users\Rize\scrcpy-d3d11\main.exe' -ArgumentList 'screenoff' -WorkingDirectory 'C:\Users\Rize\scrcpy-d3d11' -PassThru
Start-Sleep 3
$id = $p.Id
$ex = $p.HasExited
Write-Output "PID=$id Exited=$ex"
if ($ex) {
    Write-Output "ExitCode=$($p.ExitCode)"
}
