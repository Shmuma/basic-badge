"""
Utility to flash Atari ROMs into Hackaday Badge flash via UART3 interface
"""
import pathlib
import argparse
import serial

ROMFILE = "kernel_01.bin"


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("port", help="Port name to use")
    args = parser.parse_args()

    with serial.Serial(args.port, 19200, timeout=1) as ser:
        # write handshake byte
        ser.write(b'\x42')
        b = ser.read(1)
        if b != b'\x42':
            print("Handshake failed, expected 0x42, got %x" % ord(b))
        else:
            path = pathlib.Path(ROMFILE)
            size = path.stat().st_size
            print("Sending len %d" % size)
            ser.write(bytes([size & 0xFF, size >> 8]))
            data = path.read_bytes()
            for ofs, b in enumerate(data):
                ser.write(b)
                if ofs % 128 == 0:
                    v = ser.read(1)
                    if v != b'\x40':
                        print("Got wrong ACK value, stopping...")
                        break
            print("Sent file %s, size %d" % (ROMFILE, len(data)))
