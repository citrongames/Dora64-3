param(
    [Parameter(Mandatory=$true)][string]$Python,
    [string]$CpuDirectory = 'build-local/startup-v58',
    [string]$AudioDirectory = 'build-local/audio-rsp-v1',
    [int]$Jobs = 6,
    [switch]$Fresh
)
$ErrorActionPreference = 'Stop'
$projectRoot = Split-Path -Parent $PSScriptRoot
$vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
$visualStudio = & $vswhere -latest -products '*' -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
if (-not $visualStudio) { throw 'Visual Studio C++ x64 tools are required.' }
$vcvars = Join-Path $visualStudio 'VC\Auxiliary\Build\vcvars64.bat'
$environmentDirectory = Join-Path $projectRoot 'build-local'
New-Item -ItemType Directory -Force -Path $environmentDirectory | Out-Null
$environmentScript = Join-Path $environmentDirectory 'msvc-environment.cmd'
Set-Content -LiteralPath $environmentScript -Encoding ascii -Value @("@call `"$vcvars`" >nul", '@if errorlevel 1 exit /b 1', '@set')
$environmentLines = & $env:ComSpec /d /c $environmentScript
if ($LASTEXITCODE -ne 0) { throw 'MSVC environment setup failed.' }
foreach ($line in $environmentLines) {
    if ($line -match '^([^=]+)=(.*)$') { [Environment]::SetEnvironmentVariable($matches[1], $matches[2], 'Process') }
}
# Some launchers inherit both PATH and Path. vcvars updates uppercase PATH;
# importing the stale mixed-case entry afterward would hide cl.exe again.
$visualStudioPath = $environmentLines | Where-Object { $_ -cmatch '^PATH=' } | Select-Object -First 1
if ($visualStudioPath) { [Environment]::SetEnvironmentVariable('PATH', $visualStudioPath.Substring(5), 'Process') }
$compiler = Join-Path $env:VCToolsInstallDir 'bin/Hostx64/x64/cl.exe'
$cmake = Join-Path $visualStudio 'Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe'
$ninja = Join-Path $visualStudio 'Common7\IDE\CommonExtensions\Microsoft\CMake\Ninja\ninja.exe'
$buildDirectory = Join-Path $projectRoot 'build-local/windows'
$configureOptions = @()
if ($Fresh) { $configureOptions += '--fresh' }
& $cmake @configureOptions -S (Join-Path $projectRoot 'research/runtime') -B $buildDirectory -G Ninja '-DCMAKE_BUILD_TYPE=RelWithDebInfo' '-DDORA3_LIVE_RT64=ON' "-DCMAKE_C_COMPILER=$compiler" "-DCMAKE_CXX_COMPILER=$compiler" "-DCMAKE_MAKE_PROGRAM=$ninja" "-DPython3_EXECUTABLE=$Python" "-DDORA3_GENERATED_DIR=$(Join-Path $projectRoot $CpuDirectory)" "-DDORA3_AUDIO_GENERATED_DIR=$(Join-Path $projectRoot $AudioDirectory)"
if ($LASTEXITCODE -ne 0) { throw 'Windows configure failed.' }
& $cmake --build $buildDirectory --target dora3_startup -j $Jobs
if ($LASTEXITCODE -ne 0) { throw 'Windows build failed.' }
Write-Output "Built $buildDirectory\Dora64-3.exe; application was not launched."
