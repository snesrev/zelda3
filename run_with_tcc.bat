@echo off

set SDL2=third_party\SDL2-2.26.3

IF NOT EXIST "third_party\tcc\tcc.exe" (
  ECHO:
  ECHO ERROR: third_party\tcc\tcc.exe doesn't exist. Please verify that you have put it in the right location.
  ECHO   Download it from https://github.com/FitzRoyX/tinycc/releases/download/tcc_20221020/tcc_20221020.zip
  ECHO   It needs to be the 64-bit version.
  ECHO:
  PAUSE
  EXIT /B 1
) ELSE (
  REM
)

IF NOT EXIST "%SDL2%\lib\x64\SDL2.dll" (
  ECHO:
  ECHO ERROR: SDL is not unzipped properly into %SDL2%
  ECHO   Download it from https://github.com/libsdl-org/SDL/releases/download/release-2.26.3/SDL2-devel-2.26.3-VC.zip
  ECHO:
  PAUSE
  EXIT /B 1
) ELSE (
  REM
)

IF NOT EXIST "zelda3_assets.dat" (
  ECHO:
  ECHO ERROR: zelda3_assets.dat was not found.
  ECHO You need to extract assets from the ROM first, or get this file from a friend. Please see README.md
  ECHO:
  PAUSE
  EXIT /B 1
) ELSE (
  REM
)


echo Building with TCC...
third_party\tcc\tcc.exe -ozelda3.exe -DCOMPILER_TCC=1 -DSTBI_NO_SIMD=1 -DHAVE_STDINT_H=1 -D_HAVE_STDINT_H=1 -DSYSTEM_VOLUME_MIXER_AVAILABLE=0 -I%SDL2%/include -L%SDL2%/lib/x64 -lSDL2 -I. src/*.c snes/*.c third_party/gl_core/gl_core_3_1.c third_party/opus-1.3.1-stripped/opus_decoder_amalgam.c
IF ERRORLEVEL 1 goto GETOUT

copy %SDL2%\lib\x64\SDL2.dll .

echo Running...
zelda3.exe


:GETOUT
pause
