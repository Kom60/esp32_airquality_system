# Скрипт генерации самоподписанных SSL сертификатов для ESP32
# Использование: .\generate_ssl.ps1 [-CommonName "esp32.local"]

param(
    [string]$CommonName = "esp32.local",
    [string]$OutputDir = "data\ssl"
)

$ErrorActionPreference = "Stop"

Write-Host "==================================================" -ForegroundColor Cyan
Write-Host "  Генерация SSL сертификатов для ESP32" -ForegroundColor Cyan
Write-Host "==================================================" -ForegroundColor Cyan
Write-Host ""

# Проверка наличия OpenSSL
$openssl = Get-Command openssl -ErrorAction SilentlyContinue
if (-not $openssl) {
    # Пробуем найти в стандартных путях Windows
    $possiblePaths = @(
        "C:\Program Files\OpenSSL\bin\openssl.exe",
        "C:\Program Files (x86)\OpenSSL\bin\openssl.exe",
        "C:\OpenSSL\bin\openssl.exe"
    )
    foreach ($path in $possiblePaths) {
        if (Test-Path $path) {
            $openssl = $path
            break
        }
    }
}

if (-not $openssl) {
    Write-Host "ОШИБКА: OpenSSL не найден!" -ForegroundColor Red
    Write-Host ""
    Write-Host "Установите OpenSSL:" -ForegroundColor Yellow
    Write-Host "  Windows: https://slproweb.com/products/Win32OpenSSL.html" -ForegroundColor Gray
    Write-Host "  Или используйте Git Bash (уже содержит OpenSSL)" -ForegroundColor Gray
    exit 1
}

Write-Host "OpenSSL найден: $openssl" -ForegroundColor Green
Write-Host ""

# Создание директории
if (-not (Test-Path $OutputDir)) {
    New-Item -ItemType Directory -Path $OutputDir -Force | Out-Null
    Write-Host "Создана директория: $OutputDir" -ForegroundColor Gray
}

$certPath = Join-Path $OutputDir "server.crt"
$keyPath = Join-Path $OutputDir "server.key"

# Удаление старых сертификатов
if (Test-Path $certPath) {
    Remove-Item $certPath -Force
    Write-Host "Удалён старый сертификат" -ForegroundColor Gray
}
if (Test-Path $keyPath) {
    Remove-Item $keyPath -Force
    Write-Host "Удалён старый ключ" -ForegroundColor Gray
}

Write-Host ""
Write-Host "Генерация самоподписанного сертификата..." -ForegroundColor Yellow
Write-Host "  Common Name: $CommonName" -ForegroundColor Gray
Write-Host "  Срок действия: 3650 дней (10 лет)" -ForegroundColor Gray
Write-Host "  RSA ключ: 2048 бит" -ForegroundColor Gray
Write-Host ""

# Генерация сертификата
openssl req -x509 `
    -newkey rsa:2048 `
    -keyout $keyPath `
    -out $certPath `
    -days 3650 `
    -nodes `
    -subj "/C=RU/ST=Moscow/L=Moscow/O=ESP32 Air Quality/OU=IoT/CN=$CommonName" `
    -addext "subjectAltName=DNS:$CommonName,DNS:esp32.local,IP:192.168.1.100" 2>&1 | Out-Null

if ($LASTEXITCODE -eq 0) {
    Write-Host "✓ Сертификаты успешно созданы!" -ForegroundColor Green
    Write-Host ""
    Write-Host "Файлы:" -ForegroundColor Yellow
    Write-Host "  Сертификат: $certPath" -ForegroundColor Gray
    Write-Host "  Ключ:       $keyPath" -ForegroundColor Gray
    Write-Host ""
    
    # Информация о сертификате
    Write-Host "Информация о сертификате:" -ForegroundColor Yellow
    openssl x509 -in $certPath -noout -subject -dates 2>&1 | ForEach-Object {
        Write-Host "  $_" -ForegroundColor Gray
    }
    
    Write-Host ""
    Write-Host "==================================================" -ForegroundColor Cyan
    Write-Host "  Следующие шаги:" -ForegroundColor Cyan
    Write-Host "==================================================" -ForegroundColor Cyan
    Write-Host ""
    Write-Host "1. Запустите prepare_sd.ps1 для копирования на SD карту:" -ForegroundColor Yellow
    Write-Host "   .\prepare_sd.ps1 -SdLetter L" -ForegroundColor Gray
    Write-Host ""
    Write-Host "2. Включите HTTPS в config.json:" -ForegroundColor Yellow
    Write-Host '   "ssl": {"https_enabled": true, "wss_enabled": true}' -ForegroundColor Gray
    Write-Host ""
    Write-Host "3. Перезагрузите ESP32" -ForegroundColor Yellow
    Write-Host ""
} else {
    Write-Host "ОШИБКА: Не удалось создать сертификаты!" -ForegroundColor Red
    exit 1
}
