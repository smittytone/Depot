#!/usr/bin/env bash

# Deploy compiled firmware to an RP2040-based board operating
# as a Depot USB-to-I2C hardware bridge
#
# Version 1.3.0
#
# NOTE For this to work, the RP2040 must be running code that uses
#      calls `stdio_usb_init()` (as Depot firmware does). Once that is
#      the case, the RP2040 board will appear under macOS and Linux as
#      a device in `/dev`, e.g., `/dev/cu.usbmodem.1` under macOS.
#
# Usage:
#   ./deploy.sh {path/to/device} {path/to/uf2}
#
# Examples:
#   macOS: ./deploy.sh /dev/cu.usbmodem1.1 /build/firmware/pico/firmware_pico.uf2
#   Linux RPiOS: ./deploy.sh /dev/ttyACMO /build/firmware/pico/firmware_pico.uf2

show_error_and_exit() {
    printf "[ERROR] %s\n" "${1}"
    exit 1
}

if [ -z "${1}" ]; then
    printf "Usage: deploy.sh path/to/device path/to/uf2\n"
    exit 0
fi

if [ -z "${2}" ] || [ "${2##*.}" != uf2 ]; then
    printf "[ERROR] No .uf2 file specified\n"
    exit 1
fi

if [ ! -f "${2}" ]; then
    printf "[ERROR] %s cannot be found\n" "${2}"
    exit 1
fi

# Put the Pico onto BOOTSEL mode
platform=$(uname)
if [ "${platform}" = Darwin ]; then
    # macOS mount path
    pico_path=/Volumes/RPI-RP2
    if ! stty -f "${1}" 1200 ; then
        show_error_and_exit "Could not connect to device ${1}"
    fi
else
    # NOTE This is for Raspberry Pi -- you may need to change it
    #      depending on how you or your OS locate USB drive mount points
    pico_path="/media/$USER/RPI-RP2"
    if ! stty -F "${1}" 1200 ; then
        show_error_and_exit "Couldn't connect to device ${1}"
    fi

    # Allow for command line usage -- i.e., not in a GUI terminal
    # Command line is SHLVL 1, so script is SHLVL 2 (under the GUI we'd be a SHLVL 3)
    if [ $SHLVL -eq 2 ]; then
        # Mount the disk, but allow time for it to appear (not immediate on RPi)
        sleep 5
        rp2_disk=$(sudo fdisk -l | grep FAT16 | cut -f 1 -d ' ')
        if [ -z "${rp2_disk}" ]; then
            show_error_and_exit "Could not see device ${1}"
        fi

        if ! sudo mkdir "${pico_path}" ; then
            show_error_and_exit "Could not make mount point ${pico_path}"
        fi

        if ! sudo mount "${rp2_disk}" "${pico_path}" -o rw ; then
           show_error_and_exit "Could not mount device ${1}"
        fi
    fi
fi

echo "Waiting for Pico to mount..."
count=0
while [ ! -d "${pico_path}" ]; do
    sleep 0.1
    ((count+=1))
    if [ "${count}" -eq 200 ]; then
        show_error_and_exit "Pico mount timed out"
    fi
done
sleep 0.5

# Copy the target file
printf "Copying %s to %s...\n" "${2}" "${1}"
if [ "${platform}" = Darwin ]; then
    cp "${2}" "${pico_path}"
else
    sudo cp "${2}" "${pico_path}"
    if [[ $SHLVL -eq 2 ]]; then
        # We're at the command line, so unmount (RPi GUI does this automatically)
        if sudo umount "${rp2_disk}"; then
            printf "Pico unmounted\n"
            if sudo rm -rf "${pico_path}" ; then
                printf "Mountpoint removed\n"
            fi
        fi
    fi
fi

printf "Done\n"
