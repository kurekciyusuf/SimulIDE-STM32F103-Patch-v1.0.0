@echo off
setlocal EnableExtensions

echo ============================================================
echo   SimulIDE 2.0 (R260501 Win64) - STM32 Patch Installer
echo ============================================================
echo.

set "TARGET=%~1"
if "%TARGET%"=="" (
    set /p "TARGET=SimulIDE_2-R260501_Win64 klasor yolunu girin: "
)

set "TARGET=%TARGET:"=%"
if "%TARGET%"=="" (
    echo [HATA] Klasor yolu bos birakildi.
    exit /b 1
)

if "%TARGET:~-1%"=="\" set "TARGET=%TARGET:~0,-1%"

if not exist "%TARGET%\simulide.exe" (
    echo [HATA] "%TARGET%\simulide.exe" bulunamadi.
    echo Lutfen gecerli bir SimulIDE 2.0 Win64 klasoru secin.
    exit /b 1
)

echo Yama uygulaniyor: "%TARGET%"
echo.

xcopy /E /Y /I /R "%~dp0data\*" "%TARGET%\data\" >nul
if errorlevel 1 (
    echo [HATA] data klasoru kopyalanamadi.
    exit /b 1
)

xcopy /E /Y /I /R "%~dp0examples\STM32\*" "%TARGET%\examples\STM32\" >nul
if errorlevel 1 (
    echo [HATA] examples\STM32 klasoru kopyalanamadi.
    exit /b 1
)

echo [TAMAM] STM32 yamasi basariyla yuklendi!
echo SimulIDE'yi acip Micro/STM32 altindan STM32F103 modellerini
echo veya examples\STM32\blink\blink.sim2 ornegini test edebilirsiniz.
exit /b 0
