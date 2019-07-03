"""
Utility to flash Atari ROMs into Hackaday Badge flash via UART3 interface
"""
import pathlib
import argparse
import serial

ROMFILE = "kernel_01.bin"

PROTO_HANDSHAKE = b'\x42'
PROTO_ACK = b'\x40'


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("port", help="Port name to use")
    args = parser.parse_args()

    with serial.Serial(args.port, 19200, timeout=1) as ser:
        # write handshake byte
        ser.write(PROTO_HANDSHAKE)
        b = ser.read(1)
        print("Handshake: %02x" % ord(b))
        if len(b) == 0:
            print("Handshake receive timeout")
        elif b != PROTO_HANDSHAKE:
            print("Handshake failed, expected 0x42, got %x" % ord(b))
        else:
            crc_sum = 0
            path = pathlib.Path(ROMFILE)
            size = path.stat().st_size
            data = [size & 0xFF, size >> 8]
            print("Sending len %d [%02x, %02x]" % (size, data[0], data[1]))
            ser.write(bytes(data))
            b = ser.read(1)
            print("Len ACK: %02x" % ord(b))
            if b != PROTO_ACK:
                print("Got wrong ACK after len: %s" % b)
#            ser.write(PROTO_ACK)
            data = path.read_bytes()
            ser.write(data)
            ser.flush()
            crc_sum = sum(data) % 256
            # for ofs, b in enumerate(data):
            #     crc_sum += b
            #     crc_sum %= 256
            #     ser.write(b)
            #     print(type(b), b)
                # if ofs % 128 == 0:
                #     v = ser.read(1)
                #     if v != PROTO_ACK:
                #         print("%d: Got wrong ACK value, stopping..." % ofs)
                #         break
                #     else:
                #         print("%d: Got ACK, sum %d" % (ofs, crc_sum))
                #     ser.write(PROTO_ACK)
            print("Sent file %s, size %d, sum %02x" % (ROMFILE, len(data), crc_sum))
