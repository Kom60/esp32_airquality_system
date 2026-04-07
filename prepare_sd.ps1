# Скрипт для копирования файлов из data/ на SD карту
# Использование: .\prepare_sd.ps1 [-SdLetter "F"]
# Параметр: -SdLetter - буква SD карты (по умолчанию F)

param(
    [string]$SdLetter = "F"
)

$ErrorActionPreference = "Stop"

# Пути
$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$DataDir = Join-Path $ScriptDir "data"
$SdRoot = "${SdLetter}:\"

Write-Host "==================================================" -ForegroundColor Cyan
Write-Host "  Подготовка SD карты для ESP32 Air Quality Monitor" -ForegroundColor Cyan
Write-Host "==================================================" -ForegroundColor Cyan
Write-Host ""

# Проверка существования директории data
if (-not (Test-Path $DataDir)) {
    Write-Host "ОШИБКА: Директория 'data' не найдена!" -ForegroundColor Red
    exit 1
}

# Проверка существования SD карты
if (-not (Test-Path $SdRoot)) {
    Write-Host "ОШИБКА: SD карта не найдена на диске ${SdLetter}:" -ForegroundColor Red
    Write-Host "Вставьте SD карту и укажите правильную букву диска." -ForegroundColor Yellow
    Write-Host "Пример: .\prepare_sd.ps1 -SdLetter G" -ForegroundColor Yellow
    exit 1
}

Write-Host "SD карта найдена на диске ${SdLetter}:" -ForegroundColor Green
Write-Host ""

# Создание структуры директорий на SD карте
Write-Host "Создание структуры директорий..." -ForegroundColor Yellow

$directories = @("www", "config", "logs", "firmware")
foreach ($dir in $directories) {
    $dirPath = Join-Path $SdRoot $dir
    if (-not (Test-Path $dirPath)) {
        New-Item -ItemType Directory -Path $dirPath -Force | Out-Null
        Write-Host "  Создана директория: /$dir" -ForegroundColor Gray
    } else {
        Write-Host "  Директория уже существует: /$dir" -ForegroundColor Gray
    }
}

Write-Host ""

# Копирование веб-файлов из data/ в /www на SD карте
Write-Host "Копирование веб-файлов в /www..." -ForegroundColor Yellow

$webFiles = Get-ChildItem -Path $DataDir -File
$copiedCount = 0

foreach ($file in $webFiles) {
    $destPath = Join-Path (Join-Path $SdRoot "www") $file.Name
    Copy-Item -Path $file.FullName -Destination $destPath -Force
    Write-Host "  Скопирован: $($file.Name)" -ForegroundColor Gray
    $copiedCount++
}

Write-Host ""
Write-Host "Скопировано файлов: $copiedCount" -ForegroundColor Green

# Копирование config.json в /config на SD карте
Write-Host "Копирование конфигурации в /config..." -ForegroundColor Yellow

$configSource = Join-Path $DataDir "config.json"
$configDest = Join-Path (Join-Path $SdRoot "config") "config.json"

if (Test-Path $configSource) {
    Copy-Item -Path $configSource -Destination $configDest -Force
    Write-Host "  Скопирован config.json в /config/" -ForegroundColor Green
    
    # Проверка содержимого
    $content = Get-Content $configSource -Raw
    if ($content -match '"ssid"\s*:\s*"([^"]*)"') {
        Write-Host "  → WiFi SSID: $($matches[1])" -ForegroundColor Cyan
    }
} else {
    Write-Host "  ОШИБКА: config.json не найден в data/!" -ForegroundColor Red
}

# Копирование SSL сертификатов если существуют
$sslDir = Join-Path $DataDir "ssl"
if (Test-Path $sslDir) {
    Write-Host "Копирование SSL сертификатов..." -ForegroundColor Yellow
    
    $sslDest = Join-Path $SdRoot "ssl"
    if (-not (Test-Path $sslDest)) {
        New-Item -ItemType Directory -Path $sslDest -Force | Out-Null
    }
    
    $sslFiles = Get-ChildItem -Path $sslDir -File
    $sslCopied = 0
    foreach ($file in $sslFiles) {
        $destPath = Join-Path $sslDest $file.Name
        Copy-Item -Path $file.FullName -Destination $destPath -Force
        Write-Host "  Скопирован SSL: $($file.Name)" -ForegroundColor Gray
        $sslCopied++
    }
    
    if ($sslCopied -gt 0) {
        Write-Host "  SSL сертификатов скопировано: $sslCopied" -ForegroundColor Green
    } else {
        Write-Host "  SSL сертификаты не найдены" -ForegroundColor Yellow
        Write-Host "  Запустите: .\generate_ssl.ps1" -ForegroundColor Gray
    }
}

Write-Host ""
Write-Host "==================================================" -ForegroundColor Cyan
Write-Host "  SD карта готова!" -ForegroundColor Green
Write-Host "==================================================" -ForegroundColor Cyan
Write-Host ""
Write-Host "Структура на SD карте:" -ForegroundColor Yellow
Write-Host "  /www/          - Веб-файлы (HTML, CSS, JS, изображения)" -ForegroundColor Gray
Write-Host "  /config/       - Файлы конфигурации" -ForegroundColor Gray
Write-Host "  /logs/         - Логи данных с датчиков (создаются автоматически)" -ForegroundColor Gray
Write-Host "  /firmware/     - Прошивки для OTA обновления" -ForegroundColor Gray
Write-Host ""
Write-Host "Следующие шаги:" -ForegroundColor Yellow
Write-Host "  1. Извлеките SD карту из ПК" -ForegroundColor Gray
Write-Host "  2. Вставьте SD карту в ESP32" -ForegroundColor Gray
Write-Host "  3. Загрузите прошивку через PlatformIO" -ForegroundColor Gray
Write-Host "  4. Откройте Serial Monitor для проверки" -ForegroundColor Gray
Write-Host ""
