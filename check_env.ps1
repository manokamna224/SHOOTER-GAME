$log = "d:\SHOOTER GAME\env_check.txt"
"=== Environment Check ===" | Out-File $log

# winget
$wingetVer = winget --version 2>&1
"winget: $wingetVer" | Out-File $log -Append

# choco
$chocoVer = choco --version 2>&1
"choco: $chocoVer" | Out-File $log -Append

# Visual Studio paths
$vsPaths = @(
    "C:\Program Files\Microsoft Visual Studio",
    "C:\Program Files (x86)\Microsoft Visual Studio"
)
foreach ($p in $vsPaths) {
    if (Test-Path $p) {
        "VS Found: $p" | Out-File $log -Append
        Get-ChildItem $p -ErrorAction SilentlyContinue | ForEach-Object {
            "  $($_.Name)" | Out-File $log -Append
        }
    } else {
        "VS Not found: $p" | Out-File $log -Append
    }
}

# vswhere
$vswhere = "C:\Program Files (x86)\Microsoft Visual Studio\Installer\vswhere.exe"
if (Test-Path $vswhere) {
    $vsResult = & $vswhere -all -property displayName 2>&1
    "vswhere results: $vsResult" | Out-File $log -Append
} else {
    "vswhere: not found" | Out-File $log -Append
}

# MSVC compiler
$clPath = Get-Command cl.exe -ErrorAction SilentlyContinue
if ($clPath) { "MSVC cl.exe: $($clPath.Source)" | Out-File $log -Append }
else { "MSVC cl.exe: not found" | Out-File $log -Append }

# MSBuild
$msbuild = Get-Command msbuild -ErrorAction SilentlyContinue
if ($msbuild) { "MSBuild: $($msbuild.Source)" | Out-File $log -Append }
else { "MSBuild: not found" | Out-File $log -Append }

# .NET SDK
$dotnet = dotnet --version 2>&1
"dotnet: $dotnet" | Out-File $log -Append

# Windows SDK
$winSdkPath = "C:\Program Files (x86)\Windows Kits\10\Include"
if (Test-Path $winSdkPath) {
    "Windows SDK:" | Out-File $log -Append
    Get-ChildItem $winSdkPath -ErrorAction SilentlyContinue | ForEach-Object {
        "  $($_.Name)" | Out-File $log -Append
    }
} else {
    "Windows SDK: not found at $winSdkPath" | Out-File $log -Append
}

# Epic Games
$epicPaths = @(
    "C:\Program Files\Epic Games",
    "C:\Program Files (x86)\Epic Games",
    "$env:LOCALAPPDATA\Programs\Epic Games"
)
foreach ($p in $epicPaths) {
    if (Test-Path $p) {
        "Epic Found: $p" | Out-File $log -Append
        Get-ChildItem $p -ErrorAction SilentlyContinue | ForEach-Object {
            "  $($_.Name)" | Out-File $log -Append
        }
    } else {
        "Epic Not found: $p" | Out-File $log -Append
    }
}

# Epic Launcher registry
$launcherReg = Get-ItemProperty "HKLM:\SOFTWARE\WOW6432Node\Epic Games\EpicGamesLauncher" -ErrorAction SilentlyContinue
if ($launcherReg) {
    "Epic Launcher: $($launcherReg.AppPath)" | Out-File $log -Append
} else {
    "Epic Launcher: not in registry" | Out-File $log -Append
}

# Git LFS
$gitlfs = git lfs version 2>&1
"Git LFS: $gitlfs" | Out-File $log -Append

# Disk space
$disk = Get-PSDrive D
"D: drive free space: $([math]::Round($disk.Free / 1GB, 2)) GB" | Out-File $log -Append
$diskC = Get-PSDrive C
"C: drive free space: $([math]::Round($diskC.Free / 1GB, 2)) GB" | Out-File $log -Append

"=== Done ===" | Out-File $log -Append
