$ErrorActionPreference = 'Stop'

$Root = Resolve-Path (Join-Path $PSScriptRoot '..')
$InnoScript = Join-Path $Root 'installer\upkun-hmi.iss'

Push-Location $Root
try {
    & (Join-Path $PSScriptRoot 'package_release.ps1')

    $iscc = Get-Command 'ISCC.exe' -ErrorAction SilentlyContinue
    if (-not $iscc) {
        Write-Host 'Inno Setup ISCC.exe not found. Portable package is ready under dist\upkun-hmi and dist\upkun-hmi.zip.'
        Write-Host 'Install Inno Setup and rerun this script to create an installer.'
        return
    }

    & $iscc.Source $InnoScript
    Write-Host 'Installer package created under dist.'
} finally {
    Pop-Location
}
