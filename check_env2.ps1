$log = "d:\SHOOTER GAME\env_check2.txt"
"=== Check 2 ===" | Out-File $log

# VS Community via vswhere
$vswhere = "C:\Program Files (x86)\Microsoft Visual Studio\Installer\vswhere.exe"
if (Test-Path $vswhere) {
    $vsResult = & $vswhere -all -property displayName 2>&1
    "vswhere: $vsResult" | Out-File $log -Append
} else {
    "vswhere: NOT FOUND" | Out-File $log -Append
}

# VS paths
$vsPaths = @("C:\Program Files\Microsoft Visual Studio", "C:\Program Files (x86)\Microsoft Visual Studio")
foreach ($p in $vsPaths) {
    if (Test-Path $p) {
        "VS FOUND: $p" | Out-File $log -Append
        Get-ChildItem $p -ErrorAction SilentlyContinue | ForEach-Object {
            "  Edition: $($_.Name)" | Out-File $log -Append
        }
    }
}

# Windows SDK
$winSdk = "C:\Program Files (x86)\Windows Kits\10\Include"
if (Test-Path $winSdk) {
    $sdks = Get-ChildItem $winSdk -ErrorAction SilentlyContinue
    "Windows SDK versions:" | Out-File $log -Append
    foreach ($s in $sdks) { "  $($s.Name)" | Out-File $log -Append }
} else {
    "Windows SDK: NOT FOUND" | Out-File $log -Append
}

# Epic Games Launcher
$epicLauncher = "C:\Program Files (x86)\Epic Games\Launcher\Portal\Binaries\Win32\EpicGamesLauncher.exe"
$epicLauncher64 = "C:\Program Files (x86)\Epic Games\Launcher\Portal\Binaries\Win64\EpicGamesLauncher.exe"
if (Test-Path $epicLauncher) { "Epic Launcher: $epicLauncher" | Out-File $log -Append }
elseif (Test-Path $epicLauncher64) { "Epic Launcher: $epicLauncher64" | Out-File $log -Append }
else { "Epic Launcher: NOT FOUND" | Out-File $log -Append }

# UE5 anywhere
$ueRoot = "C:\Program Files\Epic Games"
if (Test-Path $ueRoot) {
    "UE installations:" | Out-File $log -Append
    Get-ChildItem $ueRoot -ErrorAction SilentlyContinue | ForEach-Object {
        "  $($_.Name)" | Out-File $log -Append
    }
} else { "UE5: NOT FOUND in default location" | Out-File $log -Append }

# Disk space
$d = Get-PSDrive D
$c = Get-PSDrive C
"D: free: $([math]::Round($d.Free / 1GB, 2)) GB" | Out-File $log -Append
"C: free: $([math]::Round($c.Free / 1GB, 2)) GB" | Out-File $log -Append

"=== DONE ===" | Out-File $log -Append
