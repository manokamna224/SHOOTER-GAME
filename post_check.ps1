$log = "d:\SHOOTER GAME\post_install.txt"

"=== POST-INSTALL CHECK ===" | Out-File $log
"Time: $(Get-Date)" | Out-File $log -Append

# VS check
"=== Visual Studio ===" | Out-File $log -Append
$vswhere = "C:\Program Files (x86)\Microsoft Visual Studio\Installer\vswhere.exe"
if (Test-Path $vswhere) {
    $vsName = & $vswhere -latest -property displayName 2>&1
    $vsPath = & $vswhere -latest -property installationPath 2>&1
    "vswhere: $vsName" | Out-File $log -Append
    "Path: $vsPath" | Out-File $log -Append
} else {
    "vswhere: NOT FOUND" | Out-File $log -Append
}

# cl.exe check
"=== cl.exe ===" | Out-File $log -Append
$clResults = Get-ChildItem "C:\Program Files\Microsoft Visual Studio" -Recurse -Filter "cl.exe" -ErrorAction SilentlyContinue | Select-Object -First 3
if ($clResults) {
    foreach ($r in $clResults) { "Found: $($r.FullName)" | Out-File $log -Append }
} else {
    $clResults2 = Get-ChildItem "C:\Program Files (x86)\Microsoft Visual Studio" -Recurse -Filter "cl.exe" -ErrorAction SilentlyContinue | Select-Object -First 3
    if ($clResults2) {
        foreach ($r in $clResults2) { "Found: $($r.FullName)" | Out-File $log -Append }
    } else {
        "cl.exe: NOT FOUND" | Out-File $log -Append
    }
}

# Windows SDK
"=== Windows SDK ===" | Out-File $log -Append
$winSdk = "C:\Program Files (x86)\Windows Kits\10\Include"
if (Test-Path $winSdk) {
    $sdks = Get-ChildItem $winSdk -Directory -ErrorAction SilentlyContinue
    foreach ($s in $sdks) { "  $($s.Name)" | Out-File $log -Append }
} else {
    "Windows SDK: NOT FOUND" | Out-File $log -Append
}

# Epic Games Launcher
"=== Epic Games Launcher ===" | Out-File $log -Append
$epicLauncher32 = "C:\Program Files (x86)\Epic Games\Launcher\Portal\Binaries\Win32\EpicGamesLauncher.exe"
$epicLauncher64 = "C:\Program Files (x86)\Epic Games\Launcher\Portal\Binaries\Win64\EpicGamesLauncher.exe"
if (Test-Path $epicLauncher32) { "Found (32): $epicLauncher32" | Out-File $log -Append }
elseif (Test-Path $epicLauncher64) { "Found (64): $epicLauncher64" | Out-File $log -Append }
else {
    # Check if Epic Games folder exists at all
    $epicBase = "C:\Program Files (x86)\Epic Games"
    if (Test-Path $epicBase) {
        "Epic Games folder exists, checking contents:" | Out-File $log -Append
        Get-ChildItem $epicBase -Recurse -Depth 2 -ErrorAction SilentlyContinue | Select-Object FullName | Out-File $log -Append
    } else {
        "Epic Games Launcher: NOT FOUND" | Out-File $log -Append
    }
}

# Disk space
"=== Disk Space ===" | Out-File $log -Append
$d = Get-PSDrive D
$c = Get-PSDrive C
"D: free: $([math]::Round($d.Free / 1GB, 2)) GB" | Out-File $log -Append
"C: free: $([math]::Round($c.Free / 1GB, 2)) GB" | Out-File $log -Append

"=== DONE ===" | Out-File $log -Append
