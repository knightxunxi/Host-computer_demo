$ErrorActionPreference = 'Stop'

$Root = Resolve-Path (Join-Path $PSScriptRoot '..')
$QtRoot = 'D:\QT\6.10.2\mingw_64'
$QtTools = 'D:\QT\Tools\mingw1310_64'
$env:Path = "$QtRoot\bin;$QtTools\bin;$env:Path"

$BuildDir = Join-Path $Root 'build-release'
$DistDir = Join-Path $Root 'dist\upkun-hmi'
$ZipPath = Join-Path $Root 'dist\upkun-hmi.zip'
$ExePath = Join-Path $DistDir 'upkun-hmi.exe'
$SimulatorExePath = Join-Path $DistDir 'upkun-simulator.exe'

Push-Location $Root
try {
    cmake -S . -B $BuildDir -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=$QtRoot -DCMAKE_CXX_COMPILER="$QtTools\bin\g++.exe"
    cmake --build $BuildDir --parallel

    if (Test-Path $DistDir) {
        Remove-Item -LiteralPath $DistDir -Recurse -Force
    }
    New-Item -ItemType Directory -Force -Path $DistDir | Out-Null
    New-Item -ItemType Directory -Force -Path (Join-Path $DistDir 'config') | Out-Null

    Copy-Item -LiteralPath (Join-Path $BuildDir 'upkun-hmi.exe') -Destination $ExePath
    Copy-Item -LiteralPath (Join-Path $BuildDir 'upkun-simulator.exe') -Destination $SimulatorExePath
    Copy-Item -LiteralPath 'config\app.example.ini' -Destination (Join-Path $DistDir 'config\app.example.ini')

    & "$QtRoot\bin\windeployqt.exe" $ExePath --release --compiler-runtime

    if (Test-Path $ZipPath) {
        Remove-Item -LiteralPath $ZipPath -Force
    }
    Compress-Archive -Path (Join-Path $DistDir '*') -DestinationPath $ZipPath -Force

    Write-Host "Package created: $DistDir"
    Write-Host "Zip created: $ZipPath"
} finally {
    Pop-Location
}
