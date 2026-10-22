$log = "d:\SHOOTER GAME\vs_check.txt"

"=== VS Folder Check ===" | Out-File $log
$vsBase = "C:\Program Files (x86)\Microsoft Visual Studio"
if (Test-Path $vsBase) {
    "Base exists: $vsBase" | Out-File $log -Append
    $dirs = Get-ChildItem $vsBase -Directory -ErrorAction SilentlyContinue
    foreach ($d in $dirs) {
        "  $($d.Name)" | Out-File $log -Append
        $subdirs = Get-ChildItem $d.FullName -Directory -ErrorAction SilentlyContinue
        foreach ($sd in $subdirs) {
            "    $($sd.Name)" | Out-File $log -Append
        }
    }
} else {
    "Base NOT found" | Out-File $log -Append
}

# Check for cl.exe
"=== cl.exe Search ===" | Out-File $log -Append
$clResults = Get-ChildItem "C:\Program Files (x86)\Microsoft Visual Studio" -Recurse -Filter "cl.exe" -ErrorAction SilentlyContinue | Select-Object -First 3
if ($clResults) {
    foreach ($r in $clResults) { "Found: $($r.FullName)" | Out-File $log -Append }
} else {
    "cl.exe NOT found" | Out-File $log -Append
}

# Check VS Installer
"=== VS Installer ===" | Out-File $log -Append
$installer = "C:\Program Files (x86)\Microsoft Visual Studio\Installer"
if (Test-Path $installer) {
    "Installer dir exists" | Out-File $log -Append
    Get-ChildItem $installer -ErrorAction SilentlyContinue | Select-Object -First 10 | ForEach-Object {
        "  $($_.Name)" | Out-File $log -Append
    }
} else {
    "Installer dir NOT found" | Out-File $log -Append
}

"=== DONE ===" | Out-File $log -Append
