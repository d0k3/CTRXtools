@echo off

make clean
rm Decrypt9/data/payload.bin
rmdir /Q /S .\Decrypt9\Decrypt9
cd Decrypt9
make clean
pause