@echo off
mkdir bin >nul 2>&1
cd bin
rmdir /s /q win >nul 2>&1
mkdir win >nul 2>&1
cd win
@echo on
cmake -G "Visual Studio 15 2017 Win64" ../..
@echo off
cd ../..
@echo on
