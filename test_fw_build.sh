#!/bin/bash

boards=(pico adafruit_qtpy_rp2040 sparkfun_promicro pimoroni_tiny2040 adafruit_trinkey_qt2040 arduino_nano_rp2040_connect)
count=0

for board in "${boards[@]}"; do
    ((count+=1))
    echo "${count}. Building for ${board}"

    [[ -d build ]] && rm -rf build

    echo "   Set up CMake"
    cmake -S . -B build > /dev/null 2>&1

    echo "   Run build"
    result=$(cmake --build build > /dev/null 2>&1 )
    if [[ ${result} -ne 0 ]]; then
        echo "   [ERROR] Build failed for ${board}"
        exit 1
    fi

    echo "   Done"
done

if [[ ${count} -eq ${#boards[@]} ]]; then
    echo Depot firmware successfully built for all supported boards
fi

exit 0
