#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
// Bibliotecas para envio de dados via POST
#include "lwip/pbuf.h"
#include "lwip/udp.h"
#include "lwip/tcp.h"
#include "lwip/api.h"
#include "lwip/inet.h"
#include "wifi_config.h"

// Tamanho máximo do buffer para os dados JSON
#define JSON_BUF_SIZE 128

volatile bool is_connected = false;
static bool busy = false;

char json_data[JSON_BUF_SIZE];

// Função para inicializar o Wi-Fi e conectar-se à rede
bool wifi_connect()
{
    // Inicializa o chip Wi-Fi
    if (cyw43_arch_init())
    {
        char *fail_msg[] = {"Wi-Fi", "init failed"};
    }

    // Modo estação (conecta a um AP)
    cyw43_arch_enable_sta_mode();

    // Mensagem de conexão no OLED
    printf("Conectando ao Wi-Fi...\n");

    // Tenta conectar à rede Wi-Fi
    if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASS, CYW43_AUTH_WPA2_AES_PSK, 30000))
    {
        printf("Failed to connect to Wi-Fi\n");

        return false;
    }
    else
    {
        // Conexão bem-sucedida
        printf("Connected to Wi-Fi\n");
        is_connected = true;
        sleep_ms(1500);
    }
    return true;
}

// Função para obter o IP atribuído
uint8_t *get_my_ip()
{
    // Obtém o IP atribuído
    uint8_t *ip_address = (uint8_t *)&(cyw43_state.netif[0].ip_addr.addr);

    // Exibe o IP no console (para debug)
    printf("IP obtido: %d.%d.%d.%d\n", ip_address[0], ip_address[1], ip_address[2], ip_address[3]);

    return ip_address;
}

// Função chamada quando a conexão TCP com o servidor é estabelecida
static err_t tcp_client_connected(void *arg, struct tcp_pcb *tpcb, err_t err)
{
    if (err != ERR_OK)
    {
        printf("Erro de conexão: %d\n", err);
        busy = false;
        return err;
    }

    // Monta a requisição POST completa
    char request[512];
    snprintf(request, sizeof(request),
             "POST / HTTP/1.1\r\n"
             "Host: %s\r\n"
             "Content-Type: application/json\r\n"
             "Content-Length: %d\r\n"
             "\r\n"
             "%s",
             SERVER_IP,
             (int)strlen(json_data),
             json_data);

    // Envia os dados via TCP
    err_t wr_err = tcp_write(tpcb, request, strlen(request), TCP_WRITE_FLAG_COPY);
    tcp_output(tpcb);
    tcp_close(tpcb);
    busy = false;
    printf("POST enviado.\n");
    return wr_err;
}

// Função chamada periodicamente para enviar os dados JSON via POST
bool send_data_to_access_point(float temperature)
{
    // Verifica se já está ocupado enviando dados
    // Se já estiver ocupado, não faz nada
    if (busy)
        return true;

    busy = true;
    
    // Variável para armazenar o IP
    char my_ip[16];

    uint8_t *ip_address = get_my_ip();
        
    snprintf(my_ip, 16, "%d.%d.%d.%d", ip_address[0], ip_address[1], ip_address[2], ip_address[3]);
    

    snprintf(json_data, JSON_BUF_SIZE,
             "{\n  \"IP\": \"%s\",\n  \"Temp\": %.2f\n}",
             my_ip, temperature);

    struct tcp_pcb *pcb = tcp_new();
    if (!pcb)
    {
        printf("Erro ao criar PCB TCP.\n");
        busy = false;
        return true;
    }

    ip_addr_t server_ip;
    ip4addr_aton(SERVER_IP, &server_ip);

    // Inicia a conexão com o servidor
    err_t err = tcp_connect(pcb, &server_ip, SERVER_PORT, tcp_client_connected);
    if (err != ERR_OK)
    {
        printf("Erro ao conectar: %d\n", err);
        tcp_abort(pcb);
        busy = false;
    }
    return true;
}