#include "https_server.h"
#include "config.h"
#include "logger.h"
#include "sdcard.h"
#include "Meteo.h"
#include <ArduinoJson.h>
#include <SD.h>
#include <WiFi.h>
#include <mbedtls/base64.h>

// Глобальный сервер
HTTPSServer* httpsServer = nullptr;

// Глобальные буферы для сертификатов в DER формате (бинарные)
std::vector<uint8_t> g_ssl_cert_der;
std::vector<uint8_t> g_ssl_key_der;

// Функция преобразования PEM в DER
static bool pem_to_der(const String& pem, std::vector<uint8_t>& der) {
    int start = pem.indexOf("-----BEGIN");
    if (start < 0) return false;
    
    int header_end = pem.indexOf("-----", start + 10);
    if (header_end < 0) return false;
    
    int end_start = pem.indexOf("-----END", header_end);
    if (end_start < 0) return false;
    
    String b64 = pem.substring(header_end + 5, end_start);
    b64.trim();
    b64.replace(" ", "");
    b64.replace("\n", "");
    b64.replace("\r", "");
    
    size_t olen = 0;
    der.resize(b64.length());
    int ret = mbedtls_base64_decode(der.data(), der.size(), &olen, 
                                     (const unsigned char*)b64.c_str(), b64.length());
    
    if (ret != 0) {
        return false;
    }
    
    der.resize(olen);
    return true;
}

// Получить MIME тип по расширению файла
static const char* getContentType(const String& path) {
    if (path.endsWith(".html")) return "text/html";
    if (path.endsWith(".css")) return "text/css";
    if (path.endsWith(".js")) return "application/javascript";
    if (path.endsWith(".json")) return "application/json";
    if (path.endsWith(".png")) return "image/png";
    if (path.endsWith(".jpg") || path.endsWith(".jpeg")) return "image/jpeg";
    if (path.endsWith(".gif")) return "image/gif";
    if (path.endsWith(".ico")) return "image/x-icon";
    return "application/octet-stream";
}

// Вспомогательная функция для отдачи файлов с SD (оптимизированная)
void serveStaticFile(HTTPRequest* req, HTTPResponse* res, const char* path, const char* contentType) {
    if (!SD.exists(path)) {
        res->setStatusCode(404);
        res->setStatusText("Not Found");
        res->setHeader("Content-Type", "text/plain");
        res->print("File Not Found");
        return;
    }

    File file = SD.open(path, FILE_READ);
    if (!file) {
        res->setStatusCode(500);
        res->setStatusText("Internal Server Error");
        res->print("Error opening file");
        return;
    }

    // Оптимизированные заголовки для быстрой загрузки
    res->setHeader("Content-Type", contentType);
    res->setHeader("Content-Length", std::to_string(file.size()));
    
    // Кэширование: 1 час для статики, без кэша для HTML
    bool isHtml = String(contentType) == "text/html";
    if (isHtml) {
        res->setHeader("Cache-Control", "no-cache, no-store, must-revalidate");
        res->setHeader("Pragma", "no-cache");
    } else {
        res->setHeader("Cache-Control", "public, max-age=3600, immutable");
    }
    
    // Безопасность
    res->setHeader("X-Content-Type-Options", "nosniff");
    res->setHeader("Connection", "keep-alive");

    // Буфер 512 байт - оптимально для SSL + SD (баланс скорость/RAM)
    uint8_t buf[512];
    size_t totalSent = 0;
    while (file.available()) {
        size_t len = file.read(buf, sizeof(buf));
        if (len > 0) {
            if (res->write(buf, len) == 0) {
                LOG_WARNING(WEBSERVER, "Client disconnected during file transfer");
                file.close();
                return;
            }
            totalSent += len;
        }
    }
    file.close();
    
    LOG_DEBUG_FMT(WEBSERVER, "Sent %s (%d bytes)", path, totalSent);
}

void handleHTTPSRoot(HTTPRequest* req, HTTPResponse* res) {
    serveStaticFile(req, res, "/www/index.html", "text/html");
}

void handleHTTPSConfig(HTTPRequest* req, HTTPResponse* res) {
    Config& cfg = config_get();
    JsonDocument doc;

    doc["version"] = cfg.version;
    doc["wifi"]["ssid"] = cfg.wifi.ssid;
    doc["ntp"]["server"] = cfg.ntp.server;
    doc["ssl"]["https_enabled"] = cfg.ssl.https_enabled;
    doc["ssl"]["https_port"] = cfg.ssl.https_port;

    String json;
    serializeJson(doc, json);

    res->setHeader("Content-Type", "application/json");
    res->print(json);
}

void handleHTTPSSettings(HTTPRequest* req, HTTPResponse* res) {
    res->setHeader("Content-Type", "application/json");
    res->print("{\"status\":\"ok\"}");
}

// Endpoint для получения данных всех датчиков (для polling режима)
void handleHTTPSSensors(HTTPRequest* req, HTTPResponse* res) {
    JsonDocument doc;
    
    // Данные датчиков (последние значения из meteo_buffer)
    doc["bme_temperature"] = meteo_buffer.get_avg_bme_temperature();
    doc["bme_pressure"] = meteo_buffer.get_avg_bme_pressure();
    doc["bme_humidity"] = meteo_buffer.get_avg_bme_humidity();
    doc["htu_temperature"] = meteo_buffer.get_avg_htu_temperature();
    doc["htu_humidity"] = meteo_buffer.get_avg_htu_humidity();
    doc["scd4x_co2"] = (int)meteo_buffer.get_avg_scd4x_co2();
    doc["scd4x_temperature"] = meteo_buffer.get_avg_scd4x_temperature();
    doc["scd4x_humidity"] = meteo_buffer.get_avg_scd4x_humidity();
    doc["pms_pm1"] = (int)meteo_buffer.get_avg_pms_pm1();
    doc["pms_pm2_5"] = (int)meteo_buffer.get_avg_pms_pm2_5();
    doc["pms_pm10"] = (int)meteo_buffer.get_avg_pms_pm10();
    doc["ms5611_pressure"] = meteo_buffer.get_avg_ms5611_pressure();
    doc["ms5611_temperature"] = meteo_buffer.get_avg_ms5611_temperature();
    doc["bh1750_lighting"] = meteo_buffer.get_avg_bh1750_lighting();
    doc["veml_uv"] = (int)meteo_buffer.get_avg_veml_uv();
    doc["ch2o_value"] = meteo_buffer.get_avg_ch2o_value();
    doc["microphone_noise"] = meteo_buffer.get_ema_microphone_noise();
    doc["ina226_voltage"] = meteo_buffer.get_avg_ina226_voltage();
    doc["ina226_current"] = meteo_buffer.get_avg_ina226_current();
    doc["ina226_power"] = meteo_buffer.get_avg_ina226_power();
    
    // Системные данные
    doc["esp32_cpu_temp"] = temperatureRead();
    doc["esp32_free_heap"] = ESP.getFreeHeap() / 1024;  // Переводим в KB
    doc["wifi_rssi"] = WiFi.RSSI();
    
    // Аналитика и расчётные значения
    float bme_temp = meteo_buffer.get_avg_bme_temperature();
    float bme_hum = meteo_buffer.get_avg_bme_humidity();
    float bme_press = meteo_buffer.get_avg_bme_pressure();
    
    // Точка росы
    float a = 17.27, b = 237.7;
    float alpha = (a * bme_temp / (b + bme_temp)) + log(bme_hum / 100.0);
    float dew_point = (b * alpha) / (a - alpha);
    doc["dew_point"] = isnan(dew_point) ? 0 : dew_point;
    
    // Абсолютная влажность
    float abs_humidity = (6.112 * exp((17.67 * bme_temp) / (bme_temp + 243.5)) * bme_hum * 2.1674) / (273.15 + bme_temp);
    doc["absolute_humidity"] = isnan(abs_humidity) ? 0 : abs_humidity;
    
    // Индекс жары (Heat Index)
    float T_F = bme_temp * 9.0/5.0 + 32.0;
    float R = bme_hum;
    float heat_index = -42.379 + 2.04901523*T_F + 10.14333127*R - 0.22475541*T_F*R - 0.00683783*T_F*T_F - 0.05481717*R*R + 0.00122874*T_F*T_F*R + 0.00085282*T_F*R*R - 0.00000199*T_F*T_F*R*R;
    float heat_index_C = (heat_index - 32) * 5.0/9.0;
    doc["heat_index"] = (heat_index_C < bme_temp) ? bme_temp : heat_index_C;
    
    // Ощущаемая температура (Wind Chill для помещений ~0)
    doc["feels_like"] = bme_temp;  // Упрощённо = температура
    
    // Дефицит точки росы
    doc["dew_point_deficit"] = bme_temp - dew_point;
    
    // Индекс комфорта
    float comfort = 100 - abs(bme_temp - 22) * 3 - abs(bme_hum - 50) * 0.5;
    doc["comfort_index"] = max(0.0f, min(100.0f, comfort));
    
    // Индекс плесени
    float mold_risk = 0;
    if (dew_point > 20 && bme_hum > 60) mold_risk = 80;
    else if (dew_point > 15 && bme_hum > 50) mold_risk = 50;
    else if (dew_point > 10 && bme_hum > 40) mold_risk = 20;
    else mold_risk = 5;
    doc["mold_risk"] = mold_risk;
    
    // Рекомендации по вентиляции
    int co2_val = (int)meteo_buffer.get_avg_scd4x_co2();
    String vent_rec = "ok";
    if (co2_val > 1400) vent_rec = "open_window";
    else if (co2_val > 1000) vent_rec = "consider_ventilation";
    else if (co2_val < 600) vent_rec = "close_window";
    doc["ventilation_recommendation"] = vent_rec;
    
    // Качество воздуха по PM2.5
    int pm25 = (int)meteo_buffer.get_avg_pms_pm2_5();
    String air_quality = "good";
    if (pm25 > 75) air_quality = "hazardous";
    else if (pm25 > 50) air_quality = "unhealthy_sensitive";
    else if (pm25 > 35) air_quality = "moderate";
    else if (pm25 > 12) air_quality = "good";
    else air_quality = "excellent";
    doc["air_quality"] = air_quality;
    doc["pm25_aqi"] = pm25;  // Упрощённый AQI = PM2.5
    
    String json;
    serializeJson(doc, json);
    
    res->setHeader("Content-Type", "application/json");
    res->setHeader("Cache-Control", "no-cache, no-store, must-revalidate");
    res->setHeader("Connection", "keep-alive");
    res->print(json);
}

// ============================================================================
// Инициализация
// ============================================================================

void https_server_init() {
    Config& cfg = config_get();

    if (!cfg.ssl.https_enabled) {
        LOG_INFO(WEBSERVER, "HTTPS disabled in config");
        return;
    }

    LOG_INFO(WEBSERVER, "Initializing HTTPS server (meshtastic)...");

    // 1. Проверяем наличие файлов
    bool cert_exists = SD.exists(cfg.ssl.cert_path);
    bool key_exists = SD.exists(cfg.ssl.key_path);
    
    if (!cert_exists || !key_exists) {
        LOG_ERROR(WEBSERVER, "SSL certificates not found on SD");
        cfg.ssl.https_enabled = false;
        return;
    }

    // 2. Читаем PEM сертификаты и конвертируем в DER формат
    {
        File f = SD.open(cfg.ssl.cert_path, FILE_READ);
        if (f) {
            String pem = f.readString();
            f.close();
            if (!pem_to_der(pem, g_ssl_cert_der)) {
                LOG_ERROR(WEBSERVER, "Failed to convert certificate PEM to DER");
                cfg.ssl.https_enabled = false;
                return;
            }
        }
    }
    {
        File f = SD.open(cfg.ssl.key_path, FILE_READ);
        if (f) {
            String pem = f.readString();
            f.close();
            if (!pem_to_der(pem, g_ssl_key_der)) {
                LOG_ERROR(WEBSERVER, "Failed to convert private key PEM to DER");
                cfg.ssl.https_enabled = false;
                return;
            }
        }
    }

    if (g_ssl_cert_der.empty() || g_ssl_key_der.empty()) {
        LOG_ERROR(WEBSERVER, "Failed to read certificate files");
        cfg.ssl.https_enabled = false;
        return;
    }

    LOG_INFO(WEBSERVER, "Certificates loaded and converted to DER format");
    LOG_INFO_FMT(WEBSERVER, "  Certificate: %d bytes DER", g_ssl_cert_der.size());
    LOG_INFO_FMT(WEBSERVER, "  Private key: %d bytes DER", g_ssl_key_der.size());

    // 3. Создаем объект сертификата для библиотеки (передаём DER)
    SSLCert* cert = new SSLCert(
        g_ssl_cert_der.data(), g_ssl_cert_der.size(),
        g_ssl_key_der.data(), g_ssl_key_der.size()
    );
    
    if (cert == nullptr || cert->getCertLength() == 0 || cert->getPKLength() == 0) {
        LOG_ERROR(WEBSERVER, "Failed to create SSLCert object");
        delete cert;
        cfg.ssl.https_enabled = false;
        return;
    }

    LOG_INFO(WEBSERVER, "SSLCert object created");

    // 4. Создаем сервер с 8 соединениями
    // Браузер открывает 6+ соединений для параллельной загрузки ресурсов
    // (HTML + 4 CSS + 2 JS + favicon + polling = до 9 соединений)
    // С bundle HTML нужно минимум соединений
    httpsServer = new HTTPSServer(cert, cfg.ssl.https_port, 8);
    
    LOG_INFO(WEBSERVER, "HTTPSServer created with 8 connections");

    // 5. Регистрируем маршруты
    ResourceNode* nodeRoot = new ResourceNode("/", "GET", &handleHTTPSRoot);
    httpsServer->registerNode(nodeRoot);

    ResourceNode* nodeConfig = new ResourceNode("/api/config", "GET", &handleHTTPSConfig);
    httpsServer->registerNode(nodeConfig);

    ResourceNode* nodeSettings = new ResourceNode("/api/settings", "GET", &handleHTTPSSettings);
    httpsServer->registerNode(nodeSettings);
    
    ResourceNode* nodeSensors = new ResourceNode("/api/sensors", "GET", &handleHTTPSSensors);
    httpsServer->registerNode(nodeSensors);

    // Статические файлы - ВСЕ CSS
    httpsServer->registerNode(new ResourceNode("/style.css", "GET", [](HTTPRequest* req, HTTPResponse* res) {
        serveStaticFile(req, res, "/www/style.css", "text/css");
    }));
    httpsServer->registerNode(new ResourceNode("/main_style.css", "GET", [](HTTPRequest* req, HTTPResponse* res) {
        serveStaticFile(req, res, "/www/main_style.css", "text/css");
    }));
    httpsServer->registerNode(new ResourceNode("/header_style.css", "GET", [](HTTPRequest* req, HTTPResponse* res) {
        serveStaticFile(req, res, "/www/header_style.css", "text/css");
    }));
    httpsServer->registerNode(new ResourceNode("/charts_style.css", "GET", [](HTTPRequest* req, HTTPResponse* res) {
        serveStaticFile(req, res, "/www/charts_style.css", "text/css");
    }));
    httpsServer->registerNode(new ResourceNode("/settings_style.css", "GET", [](HTTPRequest* req, HTTPResponse* res) {
        serveStaticFile(req, res, "/www/settings_style.css", "text/css");
    }));
    httpsServer->registerNode(new ResourceNode("/dungeon.css", "GET", [](HTTPRequest* req, HTTPResponse* res) {
        serveStaticFile(req, res, "/www/dungeon.css", "text/css");
    }));
    
    // ВСЕ JS файлы
    httpsServer->registerNode(new ResourceNode("/logic.js", "GET", [](HTTPRequest* req, HTTPResponse* res) {
        serveStaticFile(req, res, "/www/logic.js", "application/javascript");
    }));
    httpsServer->registerNode(new ResourceNode("/js_logic.js", "GET", [](HTTPRequest* req, HTTPResponse* res) {
        serveStaticFile(req, res, "/www/js_logic.js", "application/javascript");
    }));
    httpsServer->registerNode(new ResourceNode("/charts_logic.js", "GET", [](HTTPRequest* req, HTTPResponse* res) {
        serveStaticFile(req, res, "/www/charts_logic.js", "application/javascript");
    }));
    httpsServer->registerNode(new ResourceNode("/settings_logic.js", "GET", [](HTTPRequest* req, HTTPResponse* res) {
        serveStaticFile(req, res, "/www/settings_logic.js", "application/javascript");
    }));
    
    // Изображения
    httpsServer->registerNode(new ResourceNode("/favicon.png", "GET", [](HTTPRequest* req, HTTPResponse* res) {
        serveStaticFile(req, res, "/www/favicon.png", "image/png");
    }));
    httpsServer->registerNode(new ResourceNode("/esp32_logo.png", "GET", [](HTTPRequest* req, HTTPResponse* res) {
        serveStaticFile(req, res, "/www/esp32_logo.png", "image/png");
    }));
    httpsServer->registerNode(new ResourceNode("/on_bubl.png", "GET", [](HTTPRequest* req, HTTPResponse* res) {
        serveStaticFile(req, res, "/www/on_bubl.png", "image/png");
    }));
    httpsServer->registerNode(new ResourceNode("/off_bubl.png", "GET", [](HTTPRequest* req, HTTPResponse* res) {
        serveStaticFile(req, res, "/www/off_bubl.png", "image/png");
    }));
    
    // HTML страницы
    httpsServer->registerNode(new ResourceNode("/settings.html", "GET", [](HTTPRequest* req, HTTPResponse* res) {
        serveStaticFile(req, res, "/www/settings.html", "text/html");
    }));
    httpsServer->registerNode(new ResourceNode("/config.html", "GET", [](HTTPRequest* req, HTTPResponse* res) {
        serveStaticFile(req, res, "/www/config.html", "text/html");
    }));
    httpsServer->registerNode(new ResourceNode("/charts.html", "GET", [](HTTPRequest* req, HTTPResponse* res) {
        serveStaticFile(req, res, "/www/charts.html", "text/html");
    }));
    httpsServer->registerNode(new ResourceNode("/manifest.json", "GET", [](HTTPRequest* req, HTTPResponse* res) {
        serveStaticFile(req, res, "/www/manifest.json", "application/json");
    }));
    httpsServer->registerNode(new ResourceNode("/config.json", "GET", [](HTTPRequest* req, HTTPResponse* res) {
        serveStaticFile(req, res, "/www/config.json", "application/json");
    }));

    // 404 Handler
    ResourceNode* node404 = new ResourceNode("", "GET", [](HTTPRequest* req, HTTPResponse* res) {
        res->setStatusCode(404);
        res->setStatusText("Not Found");
        res->setHeader("Content-Type", "text/html");
        res->print("<h1>404 Not Found</h1>");
    });
    httpsServer->setDefaultNode(node404);

    // 6. Запускаем!
    httpsServer->start();

    if (httpsServer->isRunning()) {
        LOG_INFO_FMT(WEBSERVER, "✓ HTTPS server started on port %d (max 8 clients)", cfg.ssl.https_port);
        LOG_INFO(WEBSERVER, "  HTTP server is DISABLED - HTTPS only mode");
        LOG_INFO_FMT(WEBSERVER, "  Free heap after server start: %d bytes", ESP.getFreeHeap());
    } else {
        LOG_ERROR(WEBSERVER, "Failed to start HTTPS server");
        LOG_ERROR_FMT(WEBSERVER, "  Free heap: %d bytes", ESP.getFreeHeap());
    }
}

void https_handle_requests() {
    if (httpsServer && httpsServer->isRunning()) {
        httpsServer->loop();
    }
}
