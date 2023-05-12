/*
 * macOS/Linux 1-Wire CLI DS18B20 readout utility
 *
 * Version 1.3.0
 * Copyright © 2023, Tony Smith (@smittytone)
 * Licence: MIT
 *
 */

import Foundation


// MARK: - Constants

let STD_ERR: FileHandle         = FileHandle.standardError
let STD_OUT: FileHandle         = FileHandle.standardOutput
let STD_IN: FileHandle          = FileHandle.standardInput

let RED: String                 = "\u{001B}[31m"
let RESET: String               = "\u{001B}[0m"
let BOLD: String                = "\u{001B}[1m"

let READING_INTERVAL_S: UInt32  = 10


// MARK: - Global Variables

var doShowMessages: Bool        = true
var board: SerialDriver         = SerialDriver()


// MARK: - Output Functions

/**
 Generic message display routine.
 
 Relies on global `doShowMessage`.
 
 - Parameters:
    - message: The string to output.
 */
func report(_ message: String) {

    if doShowMessages {
        writeToStderr(message)
    }
}


/**
 Generic error display routine.
 
 Exits app on completion.
 
 - Parameters:
    - message: The string to output.
    - code:    The exit code to issue. Default: `EXIT_FAILURE`.
 */
func reportErrorAndExit(_ message: String, _ code: Int32 = EXIT_FAILURE) {

    writeToStderr(RED + BOLD + "ERROR " + RESET + message + " -- exiting")
    exit(code)
}


/**
 Write errors and other messages to `stderr`.
 
 - Parameters:
    - message: The string to output.
 */
func writeToStderr(_ message: String) {

    writeOut(STD_ERR, message)
}


/**
 Write a string to the specified file handle.
 
 - Parameters:
    - fileHandle: The target file handle, eg. `STDERR`.
    - message:    The string to be written.
 */
func writeOut(_ fileHandle: FileHandle, _ message: String) {
    
    let outputString: String = message + "\r\n"
    if let outputData: Data = outputString.data(using: .utf8) {
        fileHandle.write(outputData)
    }
}


// MARK: - Runtime Start

// Process args
// No arguments? Show Help
var args: [String] = CommandLine.arguments
if args.count == 1 {
    report("Usage: ds18b20 /path/to/device")
    exit(EXIT_SUCCESS)
}

// Connect to the board
board.file_descriptor = -1
board.is_connected = false
serial_connect(&board, args[1])

if (board.is_connected) {
    // Running the right firmware version?
    if board.fw_version_major == 1 && board.fw_version_minor < 2 {
        serial_flush_and_close_port(&board)
        reportErrorAndExit("This app requires a board with firmware 1.2.0 or above... exiting")
    }
    
    // Set the mode to 1-Wire
    if !serial_set_mode(&board, 0x6F) {
        serial_flush_and_close_port(&board);
        reportErrorAndExit("Could not set board mode... exiting")
    }
    
    // Prepare the command byte sequences
    var cmd_bytes_convert: Data = Data.init(count: 2);
    cmd_bytes_convert[0] = 0xCC;
    cmd_bytes_convert[1] = 0x44;
    
    var cmd_bytes_read_scratch: Data = Data.init(count: 2);
    cmd_bytes_read_scratch[0] = 0xCC;
    cmd_bytes_read_scratch[1] = 0xBE;
    
    var result: [UInt8] = [0, 0]
    
    // Start 1-Wire
    if !one_wire_init(&board) {
        reportErrorAndExit("Could not initialise 1-Wire... exiting")
    }
    
    report("Starting...")
    
    while true {
        // Send the convert command
        one_wire_reset(&board)
        one_wire_write_bytes(&board, &cmd_bytes_convert, 2)
        
        // Wait 750ms
        usleep(750 * 1000)
        
        // Send the read command
        one_wire_reset(&board)
        one_wire_write_bytes(&board, &cmd_bytes_read_scratch, 2)
        
        // Read back the data
        one_wire_read_bytes(&board, &result, 2)
        
        // Calculate the result
        let raw_temp: Int = (Int(result[1]) * 256) + Int(result[0])
        let celsius_temp: Float = Float((raw_temp << 16) >> 16) * 0.0625

        // Output the result
        // NOTE This doesn't use a convenience function as we don't want
        //      a newline appended, only a carriage return so the text is
        //      overwritten each time
        let outputString: String = String(format: "\rTemperature: %.2f°C   ", celsius_temp)
        if let outputData: Data = outputString.data(using: .utf8) {
            STD_OUT.write(outputData)
        }
        
        // Wait `READING_INTERVAL_S` between readings
        sleep(READING_INTERVAL_S)
    }
} else {
    reportErrorAndExit("Could not connect to the bus host board... exiting")
}

