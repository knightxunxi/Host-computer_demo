$ErrorActionPreference = 'Stop'

$Root = Resolve-Path (Join-Path $PSScriptRoot '..')
$QtRoot = 'D:\QT\6.10.2\mingw_64'
$QtTools = 'D:\QT\Tools\mingw1310_64'
$env:Path = "$QtRoot\bin;$QtTools\bin;$env:Path"

Push-Location $Root
try {
    cmake -S . -B build -G "MinGW Makefiles" -DCMAKE_PREFIX_PATH=$QtRoot -DCMAKE_CXX_COMPILER="$QtTools\bin\g++.exe"
    cmake --build build --parallel
    ctest --test-dir build --output-on-failure
} finally {
    Pop-Location
}
