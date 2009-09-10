@echo off

.\build\win32\ReleaseStatic\Extern.Lemon.exe -q .\code\Ddc\Parser.y

del .\code\Ddc\Parser.cpp
rename .\code\Ddc\Parser.c Parser.cpp
