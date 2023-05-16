#!/usr/bin/env python3


import os
import sys
import re


def update_build_number(a_path, delta=1):
    
    build_number = 1
    file_data = ""
    new_file_data = ""
    cmake_path = os.path.join(a_path, "CMakeLists.txt")
    
    with open(cmake_path, "r") as file: file_data = file.read()
    start = file_data.find("_BUILD_NUMBER ")
    end = file_data.find('"', start + 15)
    
    if end - start - 15 > 0:
        try:
            num = int(file_data[start + 15: end])
            build_number = num + delta
        except:
            # Version is NaN so reset build count
            build_number = 1
        
    print("Incrementing build number to", build_number)
    new_file_data = re.sub(r'_BUILD_NUMBER "[0-9a-zA-Z\s]*"', f"_BUILD_NUMBER \"{build_number}\"", file_data)
    with open(cmake_path, "w") as file: file.write(new_file_data)


if __name__ == '__main__':
    
    # Get where we're being called from
    is_action = (os.getenv('DO_NOT_INCREMENT') != None)
    if not is_action:
        base = os.path.split(os.getcwd())[0]
        if base.endswith("linux") or base.endswith("Depot"):
            update_build_number(base)
