$log = "d:\SHOOTER GAME\proc_check.txt"

"=== Running VS/setup processes ===" | Out-File $log
$procs = Get-Process -ErrorAction SilentlyContinue | Where-Object {
    $_.ProcessName -match "setup|vs_installer|vs_Community|msiexec|winget|Epic" -or
    $_.MainWindowTitle -match "Visual Studio|Epic|Installer"
}
if ($procs) {
    foreach ($p in $procs) {
        "PID: $($p.Id) | Name: $($p.ProcessName) | Title: $($p.MainWindowTitle)" | Out-File $log -Append
    }
} else {
    "No matching processes found" | Out-File $log -Append
}

# Check VS installer folder
"=== VS Installer folder ===" | Out-File $log -Append
$installerDir = "C:\Program Files (x86)\Microsoft Visual Studio\Installer"
if (Test-Path $installerDir) {
    "Installer dir exists" | Out-File $log -Append
    Get-ChildItem $installerDir -ErrorAction SilentlyContinue | ForEach-Object {
        "  $($_.Name)" | Out-File $log -Append
    }
} else {
    "Installer dir NOT found" | Out-File $log -Append
}

# Check VS 2022 installation folder
"=== VS 2022 folder ===" | Out-File $log -Append
$vs2022Path = "C:\Program Files\Microsoft Visual Studio\2022"
if (Test-Path $vs2022Path) {
    "VS 2022 dir exists" | Out-File $log -Append
    Get-ChildItem $vs2022Path -ErrorAction SilentlyContinue | ForEach-Object {
        "  $($_.Name)" | Out-File $log -Append
    }
} else {
    "VS 2022 dir NOT found" | Out-File $log -Append
}

# Check VS setup logs
"=== VS Setup Logs ===" | Out-File $log -Append
$logDir = "$env:TEMP"
$setupLogs = Get-ChildItem $logDir -Filter "dd_setup_*.log" -ErrorAction SilentlyContinue | Sort-Object LastWriteTime -Descending | Select-Object -First 5
if ($setupLogs) {
    foreach ($l in $setupLogs) {
        "  $($l.Name) - $($l.LastWriteTime) - $($l.Length) bytes" | Out-File $log -Append
    }
} else {
    "No dd_setup logs in TEMP" | Out-File $log -Append
}

$vsLogs = Get-ChildItem $logDir -Filter "dd_*.log" -ErrorAction SilentlyContinue | Sort-Object LastWriteTime -Descending | Select-Object -First 10
if ($vsLogs) {
    foreach ($l in $vsLogs) {
        "  $($l.Name) - $($l.LastWriteTime)" | Out-File $log -Append
    }
} else {
    "No dd_ logs at all" | Out-File $log -Append
}

"=== DONE ===" | Out-File $log -Append
