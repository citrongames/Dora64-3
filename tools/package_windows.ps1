param(
    [Parameter(Mandatory=$true)][string]$Destination,
    [Parameter(Mandatory=$true)][string]$CrtDirectory,
    [string]$BuildId = 'test-017'
)
$ErrorActionPreference = 'Stop'
if ($BuildId -notmatch '^[a-zA-Z0-9-]+$') { throw 'Invalid build ID.' }
$projectRoot = Split-Path -Parent $PSScriptRoot
$buildDirectory = Join-Path $projectRoot 'build-local/windows'
$items = [ordered]@{}
$items["Dora64-3-$BuildId.exe"] = Join-Path $buildDirectory 'Dora64-3.exe'
foreach ($name in @('SDL2.dll', 'dxcompiler.dll', 'dxil.dll')) {
    $items[$name] = Join-Path $buildDirectory $name
}
foreach ($name in @('msvcp140.dll', 'msvcp140_atomic_wait.dll', 'vcruntime140.dll', 'vcruntime140_1.dll')) {
    $items[$name] = Join-Path $CrtDirectory $name
}
$items["README-$BuildId.md"] = Join-Path $projectRoot 'docs/windows-test-build.md'
foreach ($source in $items.Values) {
    if (-not (Test-Path -LiteralPath $source -PathType Leaf)) { throw "Missing input: $source" }
}
$receipt = Get-Content -LiteralPath (Join-Path $projectRoot 'research/windows-build-check.json') -Raw | ConvertFrom-Json
if ((Get-FileHash -LiteralPath (Join-Path $buildDirectory 'Dora64-3.exe') -Algorithm SHA256).Hash.ToLowerInvariant() -ne $receipt.executable_sha256) { throw 'Executable differs from compile receipt.' }
foreach ($entry in $receipt.source_sha256.PSObject.Properties) {
    if ((Get-FileHash -LiteralPath (Join-Path $projectRoot $entry.Name) -Algorithm SHA256).Hash.ToLowerInvariant() -ne $entry.Value) { throw "Compiled input changed: $($entry.Name)" }
}
$stage = Get-Content -Raw -LiteralPath (Join-Path $projectRoot 'stage-manifest.json') | ConvertFrom-Json
# Refuse packaging an executable with source metadata from another tree state.
foreach ($entry in $stage.source_sha256.PSObject.Properties) {
    $actual = (Get-FileHash -LiteralPath (Join-Path $projectRoot $entry.Name) -Algorithm SHA256).Hash.ToLowerInvariant()
    if ($actual -ne $entry.Value) { throw "Source changed since staging: $($entry.Name)" }
}
New-Item -ItemType Directory -Force -Path $Destination | Out-Null
if (Test-Path -LiteralPath (Join-Path $Destination "Dora64-3-$BuildId.exe")) {
    throw 'Build ID already exists; use a new ID to preserve previous builds.'
}
$hashes = [ordered]@{}
foreach ($entry in $items.GetEnumerator()) {
    $target = Join-Path $Destination $entry.Key
    if ((Test-Path -LiteralPath $target) -and ([IO.Path]::GetExtension($target) -eq '.dll')) {
        if ((Get-FileHash -LiteralPath $target -Algorithm SHA256).Hash -ne (Get-FileHash -LiteralPath $entry.Value -Algorithm SHA256).Hash) {
            throw "Shared DLL differs; preserve existing test builds: $target"
        }
    } else {
        Copy-Item -LiteralPath $entry.Value -Destination $target
    }
    $hashes[$entry.Key] = (Get-FileHash -LiteralPath $target -Algorithm SHA256).Hash.ToLowerInvariant()
}
$report = [ordered]@{
    build_id = $BuildId
    platform = 'Windows x64'
    api = 'RT64 Automatic (Dora64 policy)'
    compiled = $true
    locally_launched = $false
    visual_input_audible_validation = 'pending user test'
    compiler = 'MSVC 19.44; RelWithDebInfo /O2 /Zi /EHsc; dynamic CRT'
    previous_user_test = 'test-014: available mini-games user-tested without observed crashes; progression-locked variants untested'
    current_test_scope = 'Frame light snapshots; CPU framebuffer/RT64 exclusion; bounded periodic presentation; freed redundant ROM copy; minute memory telemetry. ASan/LSan components passed; GPU/full playthrough pending. Shared saves/settings; no session limit.'
    rom = 'external; exact filename beside EXE only; never copied by this script'
    files_sha256 = $hashes
    source = $stage
}
$report | ConvertTo-Json -Depth 20 | Set-Content -Encoding utf8 -LiteralPath (Join-Path $Destination "build-info-$BuildId.json")
Write-Output "Packaged $Destination\Dora64-3-$BuildId.exe; ROM untouched; application not launched."
