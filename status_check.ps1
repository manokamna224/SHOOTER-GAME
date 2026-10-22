$log = "d:\SHOOTER GAME\status_check.txt"

"Time: $(Get-Date)" | Out-File $log

# Check running processes
"=== Running processes ===" | Out-File $log -Append
$procs = Get-Process -ErrorAction SilentlyContinue | Where-Object {
    $_.ProcessName -match "setup|vs_installer|vs_Community|msiexec|winget|vs_setup"
}
if ($procs) {
    foreach ($p in $procs) {
        "PID: $($p.Id) | Name: $($p.ProcessName)" | Out-File $log -Append
    }
} else {
    "No installer processes running" | Out-File $log -Append
}

# vswhere check
"=== vswhere ===" | Out-File $log -Append
$vswhere = "C:\Program Files (x86)\Microsoft Visual Studio\Installer\vswhere.exe"
if (Test-Path $vswhere) {
    $vsResult = & $vswhere -latest -format value -property displayName 2>&1
    "displayName: $vsResult" | Out-File $log -Append
    $vsResult2 = & $vswhere -latest -format value -property installationPath 2>&1
    "installationPath: $vsResult2" | Out-File $log -Append
} else {
    "vswhere: NOT FOUND" | Out-File $log -Append
}

# Latest setup log
"=== Latest setup logs ===" | Out-File $log -Append
$vsLogs = Get-ChildItem $env:TEMP -Filter "dd_setup_*.log" -ErrorAction SilentlyContinue | Sort-Object LastWriteTime -Descending | Select-Object -First 5
if ($vsLogs) {
    foreach ($l in $vsLogs) {
        "  $($l.Name) - $($l.LastWriteTime)" | Out-File $log -Append
    }
} else {
    "No setup logs" | Out-File $log -Append
}

# Epic launcher
"=== Epic Launcher ===" | Out-File $log -Append
$epicLauncher = "C:\Program Files (x86)\Epic Games\Launcher\Portal\Binaries\Win64\EpicGamesLauncher.exe"
if (Test-Path $epicLauncher) {
    "Epic Launcher: INSTALLED at $epicLauncher" | Out-File $log -Append
} else {
    $epicLauncher32 = "C:\Program Files (x86)\Epic Games\Launcher\Portal\Binaries\Win32\EpicGamesLauncher.exe"
    if (Test-Path $epicLauncher32) {
        "Epic Launcher: INSTALLED at $epicLauncher32" | Out-File $log -Append
    } else {
        "Epic Launcher: NOT FOUND" | Out-File $log -Append
    }
}

# cl.exe
"=== cl.exe ===" | Out-File $log -Append
$clResults = Get-ChildItem "C:\Program Files\Microsoft Visual Studio\2022" -Recurse -Filter "cl.exe" -ErrorAction SilentlyContinue | Select-Object -First 2
if ($clResults) {
    foreach ($r in $clResults) { "Found: $($r.FullName)" | Out-File $log -Append }
} else {
    "cl.exe: NOT FOUND (VS install may still be in progress)" | Out-File $log -Append
}

"=== DONE ===" | Out-File $log -Append
