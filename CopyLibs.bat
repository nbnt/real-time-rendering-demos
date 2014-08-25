if "%2" == "x86" copy /y %1\Libs\AntTweakBar\lib\AntTweakBar.dll %3
if "%2" == "x64" copy /y %1\Libs\AntTweakBar\lib\AntTweakBar64.dll %3

copy /y %1\Libs\Assimp\%2\assimp.dll %3
copy /y %1\Libs\D3DCompiler\%2\D3Dcompiler_47.dll %3