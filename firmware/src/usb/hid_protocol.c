/**
 * @file hid_protocol.c
 * @brief LibreCrypt USB HID Protocol Implementation
 */

#include "usb/hid_protocol.h"
#include "pico/unique_id.h"
#include "tusb.h"
#include <string.h>


// =============================================================================
// Internal State
// =============================================================================

static hid_packet_t rx_packet;
static hid_packet_t tx_packet;
static volatile bool packet_pending = false;

// =============================================================================
// CRC16 Implementation (CCITT)
// =============================================================================

uint16_t hid_protocol_crc16(const uint8_t *data, uint16_t len) {
  uint16_t crc = 0xFFFF;
  for (uint16_t i = 0; i < len; i++) {
    crc ^= (uint16_t)data[i] << 8;
    for (uint8_t j = 0; j < 8; j++) {
      if (crc & 0x8000) {
        crc = (crc << 1) ^ 0x1021;
      } else {
        crc <<= 1;
      }
    }
  }
  return crc;
}

// =============================================================================
// Protocol Initialization
// =============================================================================

void hid_protocol_init(void) {
  memset(&rx_packet, 0, sizeof(rx_packet));
  memset(&tx_packet, 0, sizeof(tx_packet));
  packet_pending = false;
}

// =============================================================================
// Send Response
// =============================================================================

bool hid_protocol_send_response(hid_status_t status, const uint8_t *data,
                                uint16_t len) {
  if (len > HID_PAYLOAD_MAX - 1) {
    return false;
  }

  memset(&tx_packet, 0, sizeof(tx_packet));
  tx_packet.cmd = rx_packet.cmd; // Echo command
  tx_packet.len = len + 1;       // Status + payload
  tx_packet.payload[0] = status;

  if (data && len > 0) {
    memcpy(&tx_packet.payload[1], data, len);
  }

  // Calculate CRC
  tx_packet.crc = hid_protocol_crc16((uint8_t *)&tx_packet, 3 + tx_packet.len);

  // Send via TinyUSB
  return tud_hid_report(0, &tx_packet, HID_PACKET_SIZE);
}

// =============================================================================
// Command Handlers
// =============================================================================

static void handle_ping(void) {
  const uint8_t pong[] = "PONG";
  hid_protocol_send_response(STATUS_OK, pong, sizeof(pong) - 1);
}

static void handle_get_device_info(void) {
  device_info_t info = {
      .major = 0,
      .minor = 1,
      .patch = 0,
      .device_state = 0, // TODO: Get from global state
      .capabilities = CAP_BIP32 | CAP_BIP39 | CAP_SECP256K1 | CAP_SHA256_HW |
                      CAP_POST_QUANTUM | CAP_TRUSTZONE,
  };

  // Get unique board ID
  pico_unique_board_id_t board_id;
  pico_get_unique_board_id(&board_id);
  memcpy(info.device_id, board_id.id, 8);

  hid_protocol_send_response(STATUS_OK, (uint8_t *)&info, sizeof(info));
}

static void handle_init_device(void) {
  // TODO: Implement device initialization
  // 1. Generate entropy via TRNG
  // 2. Generate BIP-39 mnemonic
  // 3. Store master seed in OTP
  // 4. Lock OTP pages
  hid_protocol_send_response(STATUS_ERROR, NULL, 0); // Not implemented yet
}

static void handle_get_pubkey(void) {
  // TODO: Implement BIP-32 derivation
  hid_protocol_send_response(STATUS_ERROR, NULL, 0); // Not implemented yet
}

static void handle_sign_tx(void) {
  // TODO: Implement transaction signing
  // MUST require physical button confirmation
  hid_protocol_send_response(STATUS_NEED_CONFIRM, NULL, 0);
}

static void handle_experimental_pq(uint8_t cmd) {
  // Post-quantum cryptography (Kyber/Dilithium)
  // TODO: Implement PQ operations
  (void)cmd;
  hid_protocol_send_response(STATUS_ERROR, NULL, 0); // Not implemented yet
}

// =============================================================================
// Packet Processing
// =============================================================================

void hid_protocol_handle_report(const uint8_t *buffer, uint16_t bufsize) {
  if (bufsize < 5) { // Minimum packet size: CMD + LEN + CRC
    return;
  }

  memcpy(&rx_packet, buffer, sizeof(rx_packet));

  // Verify CRC
  uint16_t expected_crc = hid_protocol_crc16(buffer, 3 + rx_packet.len);
  if (rx_packet.crc != expected_crc) {
    hid_protocol_send_response(STATUS_ERROR, NULL, 0);
    return;
  }

  packet_pending = true;
}

void hid_protocol_process(void) {
  if (!packet_pending) {
    return;
  }
  packet_pending = false;

  switch (rx_packet.cmd) {
  case CMD_PING:
    handle_ping();
    break;

  case CMD_GET_DEVICE_INFO:
    handle_get_device_info();
    break;

  case CMD_INIT_DEVICE:
    handle_init_device();
    break;

  case CMD_GET_PUBKEY:
    handle_get_pubkey();
    break;

  case CMD_SIGN_TX:
    handle_sign_tx();
    break;

  case CMD_EXP_PQ_KEYGEN:
  case CMD_EXP_PQ_SIGN:
  case CMD_EXP_PQ_VERIFY:
    handle_experimental_pq(rx_packet.cmd);
    break;

  default:
    hid_protocol_send_response(STATUS_INVALID_CMD, NULL, 0);
    break;
  }
}
