#!/usr/bin/env python3

import signal
from subprocess import run, PIPE
from sys import exit, argv
from time import sleep

app_sensor = "cli2c"
app_display = "segment"
device = None
i2c_address_sensor = "0x18"
i2c_address_display = "0x70"

def handler(signum, frame):
    # Reset the host's I2C bus
    sleep(0.5)
    run([app_sensor, device, "x"], check=True)
    print("\nDone")
    exit(0)

signal.signal(signal.SIGINT, handler)

if len(argv) > 1:
    device = argv[1]

if len(argv) > 2:
    i2c_address = argv[2]

if device:
    # Activate I2C on the host
    run([app_sensor, device, "z"], check=True)
    run([app_display, device, i2c_address_display, "w", "a", "on", "b", "4"], check=True)

    # Loop and read the temperature from the MCP9808
    while True:
        # Read the MCP9808's ambient temperature measurement (two bytes from register 0x05)
        result = run([app_sensor, device, "w", i2c_address_sensor, "0x05", "r", i2c_address_sensor, "2"], check=True, stdout=PIPE)

        # Convert the raw value to a Celsius reading and format for display
        temp_raw = (int(result.stdout[0:2], 16) << 8) | int(result.stdout[2:4], 16)
        temp_col = (temp_raw & 0x0FFF) / 16.0
        if temp_raw & 0x10000: temp_col -= 256.0
        temp_base = f"{temp_col:.2f}"
        temp_disp = temp_base[0:2]+temp_base[3:]
        run([app_display, device, i2c_address_display, "n", temp_disp, "d", "1"], check=True)
        sleep(0.5)
else:
    print("Usage: python mcp9809_temp_cli2c.py device [I2C address]")
