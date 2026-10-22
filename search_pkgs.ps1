$log = "d:\SHOOTER GAME\pkg_search.txt"

"=== Visual Studio Build Tools ===" | Out-File $log
winget search "Visual Studio Build Tools" --source winget 2>&1 | Out-File $log -Append

"" | Out-File $log -Append
"=== Epic Games ===" | Out-File $log -Append
winget search "Epic Games" --source winget 2>&1 | Out-File $log -Append

"" | Out-File $log -Append
"=== Unreal Engine ===" | Out-File $log -Append
winget search "Unreal Engine" --source winget 2>&1 | Out-File $log -Append

"" | Out-File $log -Append
"=== Windows SDK ===" | Out-File $log -Append
winget search "Windows SDK" --source winget 2>&1 | Out-File $log -Append

"=== DONE ===" | Out-File $log -Append
