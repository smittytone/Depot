#!/usr/bin/env python3

import signal
from subprocess import run, PIPE
from psutil import cpu_percent
from sys import exit, argv
from time import sleep

app = "cli2c"
device = None
i2c_address = "0x18"

def handler(signum, frame):
    # Reset the host's I2C bus
    sleep(0.5)
    run([app, device, "x"])
    print("\nDone")
    exit(0)

signal.signal(signal.SIGINT, handler)

if len(argv) > 1:
    device = argv[1]

if len(argv) > 2:
    i2c_address = argv[2]

if device:
    # Activate I2C on the host
    run([app, device, "z"])
    
    # Loop and read the temperature from the MCP9808
    while True:
        # Read the MCP9808's ambient temperature measurement (two bytes from register 0x05)
        result = run([app, device, "w", i2c_address, "0x05", "r", i2c_address, "2"], stdout=PIPE)
        
        # Convert the raw value to a Celsius reading
        temp_raw = (int(result.stdout[0:2], 16) << 8) | int(result.stdout[2:], 16)
        temp_col = (temp_raw & 0x0FFF) / 16.0
        if temp_raw & 0x10000: temp_col -= 256.0
        print(" Current temperature: {:.2f}°C\r".format(temp_col), end="")
        sleep(1)
else:
    print("Usage: python mcp9809_temp.py {device} {i2C address}")
