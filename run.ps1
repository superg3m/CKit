# This build file has been generated by C-Build
$ErrorActionPreference = "Stop"
Set-StrictMode -Version Latest
$PSNativeCommandUseErrorActionPreference = $true

$configPath = "c_build_config.json"
$jsonData = Get-Content -Path $configPath -Raw | ConvertFrom-Json

$project_name = $jsonData.'$project_name'

Write-Host "running [$project_name] run.ps1..." -ForegroundColor Green

Push-Location  ".\C-BUILD"
git stash
git stash drop
git pull
Pop-Location

./build.ps1 -run_exe $true

# run is complicated because what do you want to run?


