import serial
import argparse


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("-p", "--port", required=True, help="Serial port to use")
    args = parser.parse_args()

    port = serial.Serial(port=args.port, baudrate=9600)
    for _ in range(10):
        port.write(b'AT+BAUD\r\n')
        while port.readable():
            d = port.read()
            print(d)
    port.close()
