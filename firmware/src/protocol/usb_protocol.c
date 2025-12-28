/**
 * USB Protocol Layer - Implementação
 */

#include "usb_protocol.h"
#include "librecipher.h"
#include "wallet.h"
#include <stdio.h>
#include <string.h>


// Frame format: [SOF][LEN][CMD][DATA...][CRC16]
#define SOF_BYTE 0xAA
#define MAX_FRAME_SIZE 256

// Buffers
static uint8_t rx_buffer[MAX_FRAME_SIZE];
static uint8_t tx_buffer[MAX_FRAME_SIZE];
static size_t rx_pos = 0;

// Versão do firmware
static const uint8_t VERSION[] = {0, 1, 0}; // Major.Minor.Patch

/**
 * CRC-16 simples
 */
static uint16_t crc16(const uint8_t *data, size_t len) {
  uint16_t crc = 0xFFFF;
  for (size_t i = 0; i < len; i++) {
    crc ^= data[i];
    for (int j = 0; j < 8; j++) {
      if (crc & 1) {
        crc = (crc >> 1) ^ 0xA001;
      } else {
        crc >>= 1;
      }
    }
  }
  return crc;
}

/**
 * Envia resposta
 */
static void send_response(uint8_t status, const uint8_t *data, size_t len) {
  tx_buffer[0] = SOF_BYTE;
  tx_buffer[1] = (uint8_t)(len + 1); // +1 para status
  tx_buffer[2] = status;

  if (data && len > 0) {
    memcpy(&tx_buffer[3], data, len);
  }

  uint16_t crc = crc16(&tx_buffer[1], len + 2);
  tx_buffer[3 + len] = crc & 0xFF;
  tx_buffer[4 + len] = (crc >> 8) & 0xFF;

  // Enviar via stdio (USB CDC)
  for (size_t i = 0; i < 5 + len; i++) {
    putchar(tx_buffer[i]);
  }
}

/**
 * Processa comando recebido
 */
static void process_command(uint8_t cmd, const uint8_t *data, size_t len) {
  switch (cmd) {
  case CMD_PING:
    send_response(STATUS_OK, (const uint8_t *)"PONG", 4);
    break;

  case CMD_GET_VERSION:
    send_response(STATUS_OK, VERSION, sizeof(VERSION));
    break;

  case CMD_GET_STATUS: {
    uint8_t status = wallet_get_status();
    send_response(STATUS_OK, &status, 1);
    break;
  }

  case CMD_CREATE_WALLET:
    if (len < 32) {
      send_response(STATUS_ERROR, NULL, 0);
      break;
    }
    if (wallet_create(data, len)) {
      send_response(STATUS_OK, NULL, 0);
    } else {
      send_response(STATUS_ERROR, NULL, 0);
    }
    break;

  case CMD_UNLOCK:
    if (len < 1) {
      send_response(STATUS_ERROR, NULL, 0);
      break;
    }
    if (wallet_unlock(data, len)) {
      send_response(STATUS_OK, NULL, 0);
    } else {
      send_response(STATUS_ERROR, NULL, 0);
    }
    break;

  case CMD_LOCK:
    wallet_lock();
    send_response(STATUS_OK, NULL, 0);
    break;

  case CMD_GET_ADDRESS: {
    if (len < 4) {
      send_response(STATUS_ERROR, NULL, 0);
      break;
    }
    uint32_t index =
        data[0] | (data[1] << 8) | (data[2] << 16) | (data[3] << 24);
    char address[64];
    size_t addr_len = wallet_get_address(index, address, sizeof(address));
    if (addr_len > 0) {
      send_response(STATUS_OK, (uint8_t *)address, addr_len);
    } else {
      send_response(STATUS_ERROR, NULL, 0);
    }
    break;
  }

  default:
    send_response(STATUS_INVALID_CMD, NULL, 0);
    break;
  }
}

/**
 * Inicializa protocolo
 */
void usb_protocol_init(void) {
  rx_pos = 0;
  memset(rx_buffer, 0, sizeof(rx_buffer));
}

/**
 * Task do protocolo
 */
void usb_protocol_task(void) {
  int c = getchar_timeout_us(0);
  if (c == PICO_ERROR_TIMEOUT) {
    return;
  }

  rx_buffer[rx_pos++] = (uint8_t)c;

  // Verificar frame completo
  if (rx_pos >= 4) {
    if (rx_buffer[0] != SOF_BYTE) {
      // Resync
      rx_pos = 0;
      return;
    }

    uint8_t frame_len = rx_buffer[1];
    size_t expected_total = 2 + frame_len + 2; // SOF + LEN + DATA + CRC

    if (rx_pos >= expected_total) {
      // Verificar CRC
      uint16_t received_crc =
          rx_buffer[expected_total - 2] | (rx_buffer[expected_total - 1] << 8);
      uint16_t calc_crc = crc16(&rx_buffer[1], frame_len + 1);

      if (received_crc == calc_crc) {
        uint8_t cmd = rx_buffer[2];
        process_command(cmd, &rx_buffer[3], frame_len - 1);
      }

      rx_pos = 0;
    }
  }

  // Overflow protection
  if (rx_pos >= MAX_FRAME_SIZE) {
    rx_pos = 0;
  }
}
