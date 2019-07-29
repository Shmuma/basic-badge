"""
Utility to flush/dump I2C eeprom using hydra bus
"""
import serial
import pathlib
import argparse


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--port", required=True, help="Serial port to use")
    parser.add_argument("--input", help="Data file to flush into the port")
    parser.add_argument("--output", help="Output file to write data")
    parser.add_argument("--size", type=int, help="Size of data to read (for --output)")
    args = parser.parse_args()

    ser = serial.Serial(args.port, 115200)
    ser.write(b'i2c\n')

    if args.output:
        ser.write(b'[ 0xA0 0x00 0x00 ] [ 0xA1 hd:%d ]\n' % args.size)
        dump = []
        dump_mode = False
        while True:
            l = ser.readline()
            l = str(l, encoding='utf-8').strip()

#            if dump_mode and l != 'NACK':
            print(l)

            if l == 'WRITE: 0xA1 ACK':
                dump_mode = True
            elif l == 'NACK':
                dump_mode = False
                break
    elif args.input:
        path = pathlib.Path(args.input)
        ofs = 0
        with path.open('rb') as fd:
            while True:
                dat = fd.read(16)
                if len(dat) == 0:
                    break
                dat_str = ["0x%02X" % b for b in dat]
                cmd = "[ 0xA0 0x%02X 0x%02X %s ]\n" % (
                    ofs // 256, ofs % 256, " ".join(dat_str)
                )
                ofs += len(dat)
#                print(cmd)
                cmd = bytes(cmd, encoding='utf-8')
                ser.write(cmd)
                ser.flush()
                while True:
                    l = ser.readline()
                    l = str(l, encoding='utf-8').strip()
                    print(l)
                    if l == 'I2C STOP':
                        break
