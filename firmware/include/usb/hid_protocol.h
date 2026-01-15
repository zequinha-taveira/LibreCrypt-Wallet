/**
 * @file hid_protocol.h
 * @brief LibreCrypt USB HID Protocol
 *
 * Communication protocol between host app and hardware wallet.
 * All sensitive operations require physical button confirmation.
 */

#ifndef LIBRECRYPT_HID_PROTOCOL_H
#define LIBRECRYPT_HID_PROTOCOL_H

#include <stdbool.h>
#include <stdint.h>


// =============================================================================
// Protocol Constants
// =============================================================================

#define HID_PACKET_SIZE 64
#define HID_PAYLOAD_MAX (HID_PACKET_SIZE - 5) // CMD(1) + LEN(2) + CRC(2)

// Command codes
typedef enum {
  CMD_PING = 0x01,
  CMD_GET_DEVICE_INFO = 0x20,
  CMD_INIT_DEVICE = 0x30,
  CMD_GET_PUBKEY = 0x10,
  CMD_SIGN_TX = 0x11,
  CMD_VERIFY_ADDRESS = 0x12,

  // Experimental commands (0xE0-0xEF)
  CMD_EXP_PQ_KEYGEN = 0xE0, // Post-quantum key generation
  CMD_EXP_PQ_SIGN = 0xE1,   // Post-quantum signature
  CMD_EXP_PQ_VERIFY = 0xE2, // Post-quantum verification
} hid_command_t;

// Response status codes
typedef enum {
  STATUS_OK = 0x00,
  STATUS_ERROR = 0x01,
  STATUS_BUSY = 0x02,
  STATUS_NEED_CONFIRM = 0x03,
  STATUS_TIMEOUT = 0x04,
  STATUS_INVALID_CMD = 0x05,
  STATUS_NOT_INIT = 0x06,
  STATUS_LOCKED = 0x07,
} hid_status_t;

// =============================================================================
// Packet Structures
// =============================================================================

typedef struct __attribute__((packed)) {
  uint8_t cmd;
  uint16_t len;
  uint8_t payload[HID_PAYLOAD_MAX];
  uint16_t crc;
} hid_packet_t;

typedef struct __attribute__((packed)) {
  uint8_t major;
  uint8_t minor;
  uint8_t patch;
  uint8_t device_state;
  uint8_t capabilities;
  uint8_t device_id[8];
} device_info_t;

// Capability flags
#define CAP_BIP32 (1 << 0)
#define CAP_BIP39 (1 << 1)
#define CAP_SECP256K1 (1 << 2)
#define CAP_SHA256_HW (1 << 3)
#define CAP_POST_QUANTUM (1 << 4) // Experimental
#define CAP_TRUSTZONE (1 << 5)

// =============================================================================
// API Functions
// =============================================================================

/**
 * @brief Initialize the HID protocol handler
 */
void hid_protocol_init(void);

/**
 * @brief Process pending HID commands (called from main loop)
 */
void hid_protocol_process(void);

/**
 * @brief Handle incoming HID report
 * @param buffer Raw HID report data
 * @param bufsize Size of the report
 */
void hid_protocol_handle_report(const uint8_t *buffer, uint16_t bufsize);

/**
 * @brief Send response to host
 * @param status Response status code
 * @param data Response payload
 * @param len Payload length
 */
bool hid_protocol_send_response(hid_status_t status, const uint8_t *data,
                                uint16_t len);

/**
 * @brief Calculate CRC16 for packet validation
 */
uint16_t hid_protocol_crc16(const uint8_t *data, uint16_t len);

#endif // LIBRECRYPT_HID_PROTOCOL_H
