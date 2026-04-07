param(
    [string]$DotNetVersion = "9.0.14",
    [string]$DotNetRoot = "C:\Program Files\dotnet"
)

$HostPackPath = Join-Path $DotNetRoot "packs\Microsoft.NETCore.App.Host.win-x64\$DotNetVersion\runtimes\win-x64\native"

if (-Not (Test-Path $HostPackPath)) {
    Write-Error "Host pack not found at: $HostPackPath"
    Write-Error "Install the .NET $DotNetVersion runtime or adjust -DotNetVersion parameter."
    Write-Error "Available versions:"
    Get-ChildItem (Join-Path $DotNetRoot "packs\Microsoft.NETCore.App.Host.win-x64") | ForEach-Object { Write-Host "  $_" }
    exit 1
}

$ProjectRoot = Split-Path -Parent $PSScriptRoot
$DestDir = Join-Path $ProjectRoot "External\dotnet"
$LibDir = Join-Path $DestDir "lib"

New-Item -ItemType Directory -Force -Path $LibDir | Out-Null

# Headers
Copy-Item "$HostPackPath\nethost.h" -Destination $DestDir -Force
Copy-Item "$HostPackPath\hostfxr.h" -Destination $DestDir -Force
Copy-Item "$HostPackPath\coreclr_delegates.h" -Destination $DestDir -Force

# Libraries
Copy-Item "$HostPackPath\nethost.lib" -Destination $LibDir -Force
Copy-Item "$HostPackPath\nethost.dll" -Destination $LibDir -Force

Write-Host "Successfully copied .NET $DotNetVersion hosting files to External/dotnet/"
Write-Host "  Headers: nethost.h, hostfxr.h, coreclr_delegates.h"
Write-Host "  Libs:    nethost.lib, nethost.dll"
