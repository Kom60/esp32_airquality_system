@echo off
echo Generating SSL certificates...
echo.

set OPENSSL="C:\Program Files\OpenSSL-Win64\bin\openssl.exe"
set SSL_DIR=data\ssl

if not exist %SSL_DIR% mkdir %SSL_DIR%

%OPENSSL% req -x509 -newkey rsa:2048 -keyout %SSL_DIR%\server.key -out %SSL_DIR%\server.crt -days 3650 -nodes -subj "/C=RU/ST=Moscow/L=Moscow/O=ESP32 Air Quality/OU=IoT/CN=esp32.local"

if %ERRORLEVEL% EQU 0 (
    echo.
    echo SUCCESS! Certificates generated:
    echo   - %SSL_DIR%\server.crt
    echo   - %SSL_DIR%\server.key
    echo.
    echo Now run: prepare_sd.ps1 -SdLetter L
) else (
    echo ERROR! Certificate generation failed.
)

pause
