$log = "d:\SHOOTER GAME\winget_results.txt"

"=== Epic Games Search ===" | Out-File $log
$result = winget search "Epic" --source winget --accept-source-agreements 2>&1
$result | Out-File $log -Append

"" | Out-File $log -Append
"=== VS Build Tools Search ===" | Out-File $log -Append
$result2 = winget search "Visual Studio" --source winget --accept-source-agreements 2>&1
$result2 | Out-File $log -Append

"=== DONE ===" | Out-File $log -Append
