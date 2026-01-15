import hid
import struct
import time
import sys
import threading

# =============================================================================
# Configuration
# =============================================================================

VID = 0xCafe  # Default TinyUSB VID
PID = 0x4004  # Default TinyUSB PID (Generic HID)
# Adjust PID if needed based on how it enumerates

# Protocol Constants
CMD_PING            = 0x01
CMD_GET_PUBKEY      = 0x10
CMD_SIGN_TX         = 0x11
CMD_GET_DEVICE_INFO = 0x20
CMD_INIT_DEVICE     = 0x30

STATUS_OK           = 0x00
STATUS_ERROR        = 0x01
STATUS_BUSY         = 0x02
STATUS_NEED_CONFIRM = 0x03

PACKET_SIZE = 64

# =============================================================================
# CRC16-CCITT Implementation
# =============================================================================

def crc16_ccitt(data):
    crc = 0xFFFF
    for byte in data:
        crc ^= (byte << 8)
        for _ in range(8):
            if (crc & 0x8000):
                crc = (crc << 1) ^ 0x1021
            else:
                crc <<= 1
            crc &= 0xFFFF
    return crc

# =============================================================================
# HID Wrapper
# =============================================================================

class LibreCryptWallet:
    def __init__(self, vid=VID, pid=PID):
        self.device = None
        try:
            self.device = hid.device()
            self.device.open(vid, pid)
            print(f"[+] Connected to LibreCrypt Wallet ({hex(vid)}:{hex(pid)})")
        except Exception as e:
            print(f"[-] Failed to open device: {e}")
            print("    Make sure the device is plugged in and you have permissions.")
            sys.exit(1)

    def send_command(self, cmd, payload=b""):
        # Construct Packet: CMD(1) + LEN(2) + PAYLOAD(N)
        # Note: Report ID 0 is prepended by hidapi write usually, but we construct the 64 byte buffer
        
        # Header
        packet = struct.pack("<BH", cmd, len(payload)) + payload
        
        # Padding
        pad_len = PACKET_SIZE - 2 - len(packet) # -2 for CRC
        if pad_len > 0:
            packet += b'\x00' * pad_len
            
        # Calculate CRC over the first 62 bytes (Packet - CRC slot)
        crc = crc16_ccitt(packet[:62])
        packet += struct.pack("<H", crc)
        
        # Send (prepend \x00 for Report ID 0 on Windows/Linux)
        # HIDAPI expects the Report ID as the first byte of the data buffer passed to write().
        # If the device uses Report IDs, this first byte is the Report ID.
        # If not, it should be 0x00.
        self.device.write(b'\x00' + packet)
        
    def receive_response(self, timeout_ms=2000):
        try:
            # Read 64 bytes
            data = self.device.read(PACKET_SIZE, timeout_ms)
            if not data:
                return None, b""
            
            # Parse
            data = bytes(data)
            cmd = data[0]
            length = struct.unpack("<H", data[1:3])[0]
            status = data[3]
            
            payload_len = length - 1 # Length includes status byte
            payload = data[4 : 4 + payload_len]
            
            # Verify CRC (optional but good practice)
            # stored_crc = struct.unpack("<H", data[62:64])[0]
            # ...
            
            return status, payload
        except Exception as e:
            print(f"[-] Receive error: {e}")
            return None, b""

    def close(self):
        if self.device:
            self.device.close()

# =============================================================================
# Tests
# =============================================================================

def test_flow():
    wallet = LibreCryptWallet()
    
    # 1. Ping
    print("\n--- Test 1: PING ---")
    wallet.send_command(CMD_PING)
    status, payload = wallet.receive_response()
    if status == STATUS_OK:
        print(f"[+] SUCCESS: PING replied '{payload.decode()}'")
    else:
        print(f"[-] FAILED: Status {hex(status)}")

    # 2. Init Device (Might fail if already init)
    print("\n--- Test 2: INIT DEVICE ---")
    print("Sending init request (generates mnemonic)...")
    wallet.send_command(CMD_INIT_DEVICE)
    status, payload = wallet.receive_response(timeout_ms=5000) # Takes time
    if status == STATUS_OK:
        print(f"[+] SUCCESS: Device Initialized!")
        print(f"    Mnemonic: {payload.decode()}")
    elif status == STATUS_ERROR:
        print(f"[*] NOTE: Device returned ERROR (likely already initialized)")
    else:
        print(f"[-] FAILED: Status {hex(status)}")
        
    # 3. Get Public Key
    print("\n--- Test 3: GET PUBKEY ---")
    path = "m/44'/0'/0'/0/0"
    print(f"Requesting pubkey for: {path}")
    wallet.send_command(CMD_GET_PUBKEY, path.encode())
    status, payload = wallet.receive_response()
    if status == STATUS_OK:
        print(f"[+] SUCCESS: Pubkey={payload.hex()}")
    else:
        print(f"[-] FAILED: Status {hex(status)}")

    # 4. Sign Transaction (Button Test)
    print("\n--- Test 4: SIGN TX (Button Required) ---")
    dummy_hash = b"\xAB" * 32
    path = "m/44'/0'/0'/0/0"
    payload = dummy_hash + path.encode()
    
    print("[-] Sending Sign Request...")
    wallet.send_command(CMD_SIGN_TX, payload)
    
    # Poll for confirmation
    while True:
        status, resp_payload = wallet.receive_response()
        
        if status == STATUS_OK:
            print(f"[+] SUCCESS: Transaction Signed!")
            print(f"    Signature: {resp_payload.hex()}")
            break
        elif status == STATUS_NEED_CONFIRM:
            sys.stdout.write("\r[*] Status: WAITING FOR BUTTON PRESS... (Please press button on device)   ")
            sys.stdout.flush()
            # Retry sending the command (or the device might stay in a polling state? 
            # Protocol usually requires re-sending the command or polling for response.
            # Our firmware implementation returns once. So we must loop sending the command.)
            time.sleep(0.5)
            wallet.send_command(CMD_SIGN_TX, payload)
        elif status is None:
            print("\n[-] Timeout waiting for response")
            break
        else:
            print(f"\n[-] FAILED: Status {hex(status)}")
            break

    wallet.close()

if __name__ == "__main__":
    test_flow()
