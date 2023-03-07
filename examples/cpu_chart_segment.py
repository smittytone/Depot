#!/usr/bin/env python3

import signal
from subprocess import run
from psutil import cpu_percent
from sys import exit, argv
from time import sleep

app = "segment"
device = None
i2c_address = "0x70"

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
    run([app, device, i2c_address, "w", "a", "on", "b", "4"])

    while True:
        # Get the CPU percentage and display it
        cpu = int(cpu_percent() * 10.0)
        run([app, device, i2c_address, "n", str(cpu), "d", "2"])
        sleep(0.5)
else:
    print("Usage: python cpu_chart_segment.py {device} {i2C address}")
