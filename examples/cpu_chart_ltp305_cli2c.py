#!/usr/bin/env python3

import signal
from subprocess import run, TimeoutExpired, STDOUT
from psutil import cpu_percent
from sys import exit, argv
from time import sleep

app = "cli2c"
device = None
i2c_address = "0x61"
col = 0
cols = [0,0,0,0,0,0,0,0,0,0]

def handler(signum, frame):
    # Reset the host's I2C bus
    sleep(0.5)
    run([app, device, i2c_address, "a", "off"])
    print("Done")
    exit(0)

signal.signal(signal.SIGINT, handler)

if len(argv) > 1:
    device = argv[1]

if len(argv) > 2:
    i2c_address = argv[2]
    
if device:
    # Activate I2C on the host, clear the screen, and turn it on
    run([app, device, "z"])
    run([app, device, "w", i2c_address, "0x00,0x18,0x0D,0x0E,0x19,0x40,0x0C,0x00"])

    out = b'\x21'
    while True:
        # Get the CPU percentage
        cpu = int(cpu_percent())

        # Map the reading to the furthest column on the matrix
        if col > 9:
            del cols[0]
            cols.append(cpu)
        else:
            cols[9 - col] = cpu
        col += 1

        # Set the actual column value
        for i in range(0, 10):
            if i == 0 or i == 5:
                out = bytearray(10)
                out[0] = 0xC9
                if i == 0: out[1] = 0x0E
                if i == 5: out[1] = 0x01

            a = cols[i]
            b = 0
            if a > 89:
                b = 0x7F
            elif a > 74:
                b = 0x7E
            elif a > 59:
                b = 0x7C
            elif a > 44:
                b = 0x78
            elif a > 29:
                b = 0x70
            elif a > 14:
                b = 0x60
            elif a > 0:
                b = 0x40
            
            if i < 5:
                c = 0
                for y in range(0, 7):
                    c |= b & (1 << y)
                out[2 + i] = c
            else:
                inset = i - 5
                for y in range(0, 7):
                    c = ((b & (1 << y)) >> y) << (6 - y)
                    if c > 0:
                        out[y + 2] |= (1 << inset)
                    else:
                        out[y + 2] &= ~(1 << inset)

            if i == 4 or i == 9:
                # Write out L or R matrix buffer
                data_string = ""
                for j in range(1, 10):
                    data_string += "0x{:02x},".format(out[j])
                data_string = data_string[:-1]
                try:
                    run([app, device, "w", i2c_address, data_string], timeout=90.0)
                except TimeoutExpired:
                    print("Attempt to write data timed out")
                    exit(1)
                
        try:
            # Update the displays
            run([app, device, "w", i2c_address, "0x0C,0x01", "p"], timeout=90.0)
        except TimeoutExpired:
            print("Attempt to write data timed out")
            exit(1)
        sleep(1)
else:
    print("Usage: python cpu_chart_matrix.py {path/to/device} [optional i2C address]")
