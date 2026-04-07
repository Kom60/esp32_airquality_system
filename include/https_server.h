#ifndef HTTPS_SERVER_H
#define HTTPS_SERVER_H

#include <Arduino.h>
#include <vector>
#include <HTTPSServer.hpp>
#include <SSLCert.hpp>
#include <HTTPRequest.hpp>
#include <HTTPResponse.hpp>
#include <ResourceNode.hpp>

using namespace httpsserver;

// Глобальный объект сервера
extern HTTPSServer* httpsServer;

// Буферы для сертификатов в DER формате
extern std::vector<uint8_t> g_ssl_cert_der;
extern std::vector<uint8_t> g_ssl_key_der;

// Функция инициализации (вызывается один раз)
void https_server_init();

// Функция обработки запросов (вызывается в loop)
void https_handle_requests();

// Обработчики маршрутов
void handleHTTPSRoot(HTTPRequest* req, HTTPResponse* res);
void handleHTTPSConfig(HTTPRequest* req, HTTPResponse* res);
void handleHTTPSSettings(HTTPRequest* req, HTTPResponse* res);
void serveStaticFile(HTTPRequest* req, HTTPResponse* res, const char* path, const char* contentType);

#endif
