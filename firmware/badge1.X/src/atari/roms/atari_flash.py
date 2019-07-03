"""
Utility to flash Atari ROMs into Hackaday Badge flash via UART3 interface
"""
import sys
import logging
import pathlib
import argparse
import serial

log = logging.getLogger(__name__)

PROTO_HANDSHAKE = b'\x42'
PROTO_ACK = b'\x40'


if __name__ == "__main__":
    logging.basicConfig(format="%(asctime)-15s %(levelname)s %(message)s", level=logging.INFO)
    parser = argparse.ArgumentParser()
    parser.add_argument("port", help="Port name to use")
    parser.add_argument("roms", nargs='+', help="Up to 10 ROM filenames, currently only 4K roms are supported")
    args = parser.parse_args()

    if len(args.roms) > 10:
        log.error("Currently you can specify only up to 10 rom files")
        sys.exit(1)

    rom_paths = [pathlib.Path(name) for name in args.roms]
    if not all(map(lambda p: p.exists(), rom_paths)):
        log.error("Some of rom files are not found!")
        sys.exit()

    if not all(map(lambda p: p.stat().st_size <= 4096, rom_paths)):
        log.error("Some rom files are larger than 4K!")
        sys.exit()

    with serial.Serial(args.port, 19200, timeout=1) as ser:
        # write handshake byte
        ser.write(PROTO_HANDSHAKE)
        ser.flush()
        b = ser.read(1)
        if len(b) == 0:
            log.warning("Handshake receive timeout")
        elif b != PROTO_HANDSHAKE:
            log.warning("Handshake failed, expected 0x42, got %x", ord(b))
        else:
            log.info("Handshake: %02x", ord(b))
            for path in rom_paths:
                crc_sum = 0
                size = path.stat().st_size
                log.info("Sending file %s, len %d", path, size)
                ser.write(b'\x00\x10')
                ser.flush()
                b = ser.read(1)
                log.info("Len ACK: %02x", ord(b))
                if b != PROTO_ACK:
                    log.error("Got wrong ACK after len: %02d", ord(b))
                    break
                data = path.read_bytes()
                ser.write(bytes([0]*(4096-size)))
                ser.write(data)
                ser.flush()
                crc_sum = sum(data) % 256
                log.info("Sent file %s, size %d, sum %02x", path, len(data), crc_sum)
            ser.write(bytes([0, 0]))
            ser.flush()
            b = ser.read(1)
            if b == PROTO_ACK:
                log.info("Transfer finished")
            else:
                log.error("Got wrong ACK: %02d", ord(b))
