"""
Utility to flash Atari ROMs into Hackaday Badge flash via UART3 interface
"""
import argparse
import serial


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("port", required=True, help="Port name to use")
    args = parser.parse_args()

    print(args)
