@echo off
REM === pause if there were errors ===
if %errorlevel% neq 0 pause
@echo on
make
cd Decrypt9
mkdir Decrypt9
make
move Decrypt9.3dsx Decrypt9
move Decrypt9.smdh Decrypt9
pause