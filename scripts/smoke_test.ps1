$ErrorActionPreference = 'Stop'

$Root = Resolve-Path (Join-Path $PSScriptRoot '..')
$QtRoot = 'D:\QT\6.10.2\mingw_64'
$QtTools = 'D:\QT\Tools\mingw1310_64'
$env:Path = "$QtRoot\bin;$QtTools\bin;$env:Path"

Push-Location $Root
try {
    cmake -S . -B build -G "MinGW Makefiles" -DCMAKE_PREFIX_PATH=$QtRoot -DCMAKE_CXX_COMPILER="$QtTools\bin\g++.exe"
    cmake --build build --parallel

    $process = Start-Process -FilePath '.\build\upkun-hmi.exe' -PassThru -WindowStyle Hidden
    $client = $null

    try {
        $deadline = (Get-Date).AddSeconds(10)
        do {
            try {
                $client = [System.Net.Sockets.TcpClient]::new()
                $client.Connect('127.0.0.1', 1502)
                break
            } catch {
                if ($client) {
                    $client.Close()
                    $client = $null
                }
                Start-Sleep -Milliseconds 250
            }
        } while ((Get-Date) -lt $deadline)

        if (-not $client -or -not $client.Connected) {
            throw 'Modbus TCP simulator did not open 127.0.0.1:1502.'
        }

        $stream = $client.GetStream()

        [byte[]]$readInputRegisters = 0,1,0,0,0,6,1,4,0,0,0,5
        $stream.Write($readInputRegisters, 0, $readInputRegisters.Length)
        $buffer = New-Object byte[] 256
        $read = $stream.Read($buffer, 0, $buffer.Length)
        if ($read -lt 9 -or $buffer[7] -ne 4) {
            throw "Unexpected Modbus read response. bytes=$read function=$($buffer[7])"
        }

        [byte[]]$triggerFault = 0,2,0,0,0,6,1,5,0,9,255,0
        $stream.Write($triggerFault, 0, $triggerFault.Length)
        Start-Sleep -Seconds 1

        [byte[]]$reset = 0,3,0,0,0,6,1,5,0,2,255,0
        $stream.Write($reset, 0, $reset.Length)
        Start-Sleep -Seconds 1

        if (-not (Test-Path 'data\app.sqlite3')) {
            throw 'SQLite database data\app.sqlite3 was not created.'
        }

        Write-Host 'Smoke test passed.'
    } finally {
        if ($client) {
            $client.Close()
        }
        $children = @()
        if ($process) {
            $children = Get-CimInstance Win32_Process -Filter "ParentProcessId=$($process.Id)" -ErrorAction SilentlyContinue |
                Where-Object { $_.Name -eq 'upkun-simulator.exe' }
        }
        if ($process -and -not $process.HasExited) {
            Stop-Process -Id $process.Id -Force
        }
        foreach ($child in $children) {
            Stop-Process -Id $child.ProcessId -Force -ErrorAction SilentlyContinue
        }
    }
} finally {
    Pop-Location
}
