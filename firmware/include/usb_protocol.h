/**
 * USB Protocol Layer - Comunicação com App
 */

#ifndef USB_PROTOCOL_H
#define USB_PROTOCOL_H

#include <stdbool.h>
#include <stdint.h>


// Comandos do protocolo
typedef enum {
  CMD_PING = 0x01,
  CMD_GET_VERSION = 0x02,
  CMD_GET_STATUS = 0x03,
  CMD_CREATE_WALLET = 0x10,
  CMD_UNLOCK = 0x11,
  CMD_LOCK = 0x12,
  CMD_GET_ADDRESS = 0x20,
  CMD_SIGN_TX = 0x21,
} usb_command_t;

// Status de resposta
typedef enum {
  STATUS_OK = 0x00,
  STATUS_ERROR = 0x01,
  STATUS_INVALID_CMD = 0x02,
  STATUS_LOCKED = 0x03,
  STATUS_NEED_CONFIRM = 0x04,
} usb_status_t;

/**
 * Inicializa protocolo USB
 */
void usb_protocol_init(void);

/**
 * Task de processamento USB (chamar no loop principal)
 */
void usb_protocol_task(void);

#endif // USB_PROTOCOL_H
