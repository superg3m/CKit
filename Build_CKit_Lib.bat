@echo off

if not exist .\build (
    mkdir .\build
) else (
    call clean_CKit.bat
)

powershell -nologo -command "Write-Host 'running CKit build.bat...' -ForegroundColor Green"

if exist compilation_errors.txt (
	del compilation_errors.txt
)

pushd .\build
:: MAKE SURE YOU HAVE AN OPTION FOR DEBUG LIBS
:: cl -DCUSTOM_PLATFORM_IMPL /std:c++20 /c "..\source\*.cpp"
cl /std:c++20 /Zi /FC /c "..\source\*.cpp" >> "..\compilation_errors.txt" 2>nul
lib /OUT:".\CKit.lib" ".\*.obj" > NUL
popd

call normalize_path.bat