@echo off

2>nul (del zelda3_assets.dat)
python assets/restool.py --extract-from-rom
IF NOT ERRORLEVEL 0 goto ERROR

IF NOT EXIST "zelda3_assets.dat" (
  ECHO ERROR: The python program didn't generate zelda3_assets.dat successfully.
  goto ERROR  
) ELSE (
  REM
)

goto DONE


:ERROR
ECHO:
ECHO ERROR: Asset extraction failed!
pause
EXIT /B 1

:DONE
echo Complete!
pause