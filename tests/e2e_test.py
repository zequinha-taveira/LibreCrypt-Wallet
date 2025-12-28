import serial
import time
import sys
import struct
import binascii

# Configuration
# Find correct port manually or scan
PORT = "COM5" # Adjust as needed
BAUDRATE = 115200

CMD_PING = 0x01
CMD_GET_VERSION = 0x02
CMD_GET_STATUS = 0x03
SOF = 0xAA

def crc16(data):
    crc = 0xFFFF
    for byte in data:
        crc ^= byte
        for _ in range(8):
            if crc & 1:
                crc = (crc >> 1) ^ 0xA001
            else:
                crc >>= 1
    return crc

def build_frame(cmd, payload=b""):
    frame = bytearray()
    frame.append(SOF)
    frame.append(len(payload) + 1) # Len includes cmd
    frame.append(cmd)
    frame.extend(payload)
    
    # Calculate CRC on [LEN, CMD, PAYLOAD] (Skip SOF)
    # Check protocol.rs implementation: crc is on &frame[1..] i.e. Len, Cmd, Data
    # frame[1:] is exactly that.
    crc = crc16(frame[1:])
    frame.append(crc & 0xFF)
    frame.append((crc >> 8) & 0xFF)
    return frame

def send_command(ser, cmd, payload=b""):
    frame = build_frame(cmd, payload)
    print(f"Sending: {binascii.hexlify(frame)}")
    ser.write(frame)
    time.sleep(0.1)
    
    # Read response
    # Setup simplistic read
    resp = ser.read(512)
    if len(resp) < 5:
        print("No valid response received")
        return None
        
    print(f"Received: {binascii.hexlify(resp)}")
    
    if resp[0] != SOF:
        print("Invalid SOF")
        return None
        
    length = resp[1]
    status = resp[2]
    data = resp[3:3+length-1]
    
    print(f"Status: {status}")
    print(f"Data: {binascii.hexlify(data)}")
    return (status, data)

def main():
    print("LibreCrypt Hardware Test")
    try:
        ser = serial.Serial(PORT, BAUDRATE, timeout=1)
        print(f"Opened {PORT}")
    except Exception as e:
        print(f"Failed to open port {PORT}: {e}")
        # Try scanning
        print("Listing ports not implemented in this simple script. Please edit PORT variable.")
        return

    print("\n--- TEST: PING ---")
    send_command(ser, CMD_PING)
    
    print("\n--- TEST: GET VERSION ---")
    status, data = send_command(ser, CMD_GET_VERSION)
    if status == 0 and len(data) >= 3:
        print(f"Version: {data[0]}.{data[1]}.{data[2]}")
        if len(data) > 3:
            print(f"Crypto: {data[3:].decode('utf-8', errors='ignore')}")

    print("\n--- TEST: GET STATUS ---")
    status, data = send_command(ser, CMD_GET_STATUS)
    if status == 0 and len(data) > 0:
        s = data[0]
        st = "Disconnected"
        if s == 1: st = "Locked"
        if s == 2: st = "Unlocked"
        print(f"Wallet Status: {st}")

    ser.close()

if __name__ == "__main__":
    main()
