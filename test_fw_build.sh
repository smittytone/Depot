#!/bin/bash

boards=(pico pico2 adafruit_qtpy_rp2040 adafruit_trinkey_qt2040 adafruit_feather_rp2040 sparkfun_promicro pimoroni_tiny2040 arduino_nano_rp2040_connect)
count=0

for board in "${boards[@]}"; do
    ((count+=1))
    echo "${count}. Building for ${board}"

    [[ -d build && ! "${1}" == "keep" ]] && rm -rf build

    echo "   Configuring..."
    export PICO_BOARD="${board}"
    cmake -S . -B build > /dev/null 2>&1

    echo "   Building..."
    if ! cmake --build build > /dev/null 2>&1 ; then
        echo "   [ERROR] Build failed for ${board}"
        exit 1
    fi

    echo "   ...done - firmware built for ${board}"
done

if [[ ${count} -eq ${#boards[@]} ]]; then
    echo Depot firmware successfully built for all supported boards
fi

[[ -d build && ! "${1}" == "keep" ]] && rm -rf build

exit 0
