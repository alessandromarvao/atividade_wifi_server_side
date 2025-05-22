#ifndef WIFI_CONFIG_H
#define WIFI_CONFIG_H

// Credenciais da rede Wi-Fi
#define WIFI_SSID ""
#define WIFI_PASS ""

// IP e porta do servidor de destino
#define SERVER_IP "192.168.4.1"
#define SERVER_PORT 80

// Função para inicializar o Wi-Fi e conectar-se à rede
bool wifi_connect();

// Função para obter o IP atribuído
char* get_my_ip();

// Função para enviar os dados JSON via POST
bool send_data_to_access_point(char *temperature);

#endif