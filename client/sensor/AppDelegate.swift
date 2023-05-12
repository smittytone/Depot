/*
 * macOS/Linux 1-Wire DS18B20 readout GUI app
 *
 * Version 1.3.0
 * Copyright © 2023, Tony Smith (@smittytone)
 * Licence: MIT
 *
 */
import Foundation
import Cocoa


// MARK: - Globals accessed from C code
var board: SerialDriver = SerialDriver()
var modems: [String] = []
var modemCount: UInt32 = 0


@main
class AppDelegate: NSObject, NSApplicationDelegate, NSAnimationDelegate {

    // MARK: - UI Properties

    @IBOutlet var window: NSWindow!
    @IBOutlet var outputLabel: NSTextField!
    @IBOutlet var connectedLabel: NSTextField!
    @IBOutlet var connectedImage: NSImageView!
    @IBOutlet var startButton: NSButton!
    @IBOutlet var devicesMenu: NSMenu!


    // MARK: - Private Properties

    private var current_device: String = ""
    private var board_info: String? = nil
    private var reading: [UInt8] = [0, 0];
    private var cmd_bytes_convert_temp: Data? = nil
    private var cmd_bytes_read_scratch: Data? = nil
    private var sensorTimer: Timer? = nil
    private var sensorInterval: Double = 2.0
    private var celsiusTemp: Float = 0.0
    private var hasStarted: Bool = false
    private var alphaRampUp: NSViewAnimation? = nil
    private var alphaRampDown: NSViewAnimation? = nil
    private var redImage: NSImage? = nil
    private var amberImage: NSImage? = nil
    private var greenImage: NSImage? = nil
    private var backImage: NSImage? = nil
    private var parentApp: NSApplication? = nil

    // Set up a computed property that's used for
    // the result of actions that return `false` on
    // failed serial or 1-Wire ops. On fail we can
    // reset the connection and app state
    private var checkBacking: Bool = false
    public var check: Bool {
        get {
            return self.checkBacking
        }
        set(v) {
            self.checkBacking = v
            if v == false {
                reset()
            }
        }
    }


    // MARK: - App Lifecycle Functions

    func applicationDidFinishLaunching(_ aNotification: Notification) {

        // Disable the Help menu Spotlight features
        let dummyHelpMenu: NSMenu = NSMenu.init(title: "Dummy")
        let theApp = NSApplication.shared
        theApp.helpMenu = dummyHelpMenu

        // Initialise the UI
        self.outputLabel.stringValue = "--.--°C"

        // Set start-up values
        initValues()

        // Get the parent app
        self.parentApp = theApp

        // Centre the main window and display
        self.window.center()
        self.window.makeKeyAndOrderFront(self)
    }


    func applicationWillTerminate(_ aNotification: Notification) {

        // Halt the timer and any animations still playing
        haltTimerAndAnimation()

        // Close the board port if it's still open
        if board.file_descriptor != -1 {
            serial_flush_and_close_port(&board)
        }
    }


    func applicationShouldTerminateAfterLastWindowClosed(_ sender: NSApplication) -> Bool {

        // When the main window closed, shut down the app
        return true
    }


    func applicationSupportsSecureRestorableState(_ app: NSApplication) -> Bool {

        // Don't need this
        return false
    }


    /**
     Initialise key variables.
     */
    private func initValues() {

        // Prepare board store
        board.file_descriptor = -1
        board.is_connected = false

        // Prepare the command byte sequences
        self.cmd_bytes_convert_temp = Data.init(count: 2)
        self.cmd_bytes_convert_temp![0] = 0xCC
        self.cmd_bytes_convert_temp![1] = 0x44

        self.cmd_bytes_read_scratch = Data.init(count: 2)
        self.cmd_bytes_read_scratch![0] = 0xCC
        self.cmd_bytes_read_scratch![1] = 0xBE

        // Pre-load images
        self.redImage = NSImage.init(named: NSImage.Name(stringLiteral: "NSStatusUnavailable"))
        self.amberImage = NSImage.init(named: NSImage.Name(stringLiteral: "NSStatusPartiallyAvailable"))
        self.greenImage = NSImage.init(named: NSImage.Name(stringLiteral: "NSStatusAvailable"))
        self.backImage = NSImage.init(named: NSImage.Name(stringLiteral: "icon_back"))

        // Set up pulse animations
        // Fade the view in from the background
        self.alphaRampUp = NSViewAnimation.init(viewAnimations: [[NSViewAnimation.Key.target: self.connectedImage!, NSViewAnimation.Key.effect: NSViewAnimation.EffectName.fadeIn]])

        self.alphaRampUp!.duration = 1.0
        self.alphaRampUp!.animationBlockingMode = .nonblockingThreaded
        self.alphaRampUp!.delegate = self

        // Fade the view to the background
        self.alphaRampDown = NSViewAnimation.init(viewAnimations: [[NSViewAnimation.Key.target: self.connectedImage!, NSViewAnimation.Key.effect: NSViewAnimation.EffectName.fadeOut]])

        self.alphaRampDown!.duration = 1.0
        self.alphaRampDown!.animationBlockingMode = .nonblockingThreaded
        self.alphaRampDown!.delegate = self

        // Referesh the devices menu
        refreshDevicesMenu(false)
    }


    /**
     Reset the app and sensor. Called on comms failure.
     */
    private func reset() {

        // No longer taking readings
        self.hasStarted = false

        // Halt the timer and any animations still playing
        haltTimerAndAnimation()

        // Break the board connection
        serial_flush_and_close_port(&board)
        board.is_connected = false

        // Update the UI and reset app variables
        self.startButton.title = "Start"
        refreshDevicesMenu(false)

        self.connectedImage.image = self.redImage
        self.connectedLabel.stringValue = "Not connected"

        // Warn the user
        showAlert("Error", "Device \(current_device) has been disconnected")
    }


    /**
     An error warrants port closure.

     - Parameters:
        - title:   An alert title.
        - message: The error message.
     */
    private func closePortOnError(_ title: String, _ message: String) {

        // Close the port
        // NOTE This sets `board.is_connected`.
        serial_flush_and_close_port(&board)

        // Update the UI
        self.connectedImage.image = self.redImage
        self.connectedLabel.stringValue = "Not connected"

        // Warn the user
        showAlert(title, message)
    }


    // MARK: - Action Functions

    /**
     The user has clicked the Start/Stop button.

     - Parameters:
        - sender: The button.
     */
    @IBAction private func doStart(sender: Any) {

        // No current device selected, bail
        if self.current_device == "" {
            showAlert("Error", "No board selected. Use the Devices menu")
            return
        }

        // Base the action on the current app state
        if self.hasStarted {
            // The user has clicked the Stop button
            self.hasStarted = false

            // Zap the timer if it's running and halt any animation
            haltTimerAndAnimation()

            // Close the port
            serial_flush_and_close_port(&board)

            // Update the UI
            self.startButton.title = "Start"
            self.connectedImage.image = self.redImage
            self.connectedImage.alphaValue = 1.0
            self.connectedImage.isHidden = false
            self.connectedLabel.stringValue = "Not connected"
        } else {
            // Update the UI #1
            self.connectedImage.image = self.amberImage
            self.connectedLabel.stringValue = "Connecting..."

            // Start talking to the board: connect first
            if !board.is_connected {
                serial_connect(&board, self.current_device)
            }

            if (board.is_connected) {
                // Is the board running the right firmware version?
                if board.fw_version_major == 1 && board.fw_version_minor < 2 {
                    closePortOnError("Error", "This app requires a board with firmware 1.2.0 or above... exiting")
                    return
                }

                // Set the mode to 1-Wire
                if !serial_set_mode(&board, 0x6F) {
                    closePortOnError("Error", "Could not set board mode")
                    return
                }

                // Get info if we need it
                if self.board_info == nil {
                    if let cstring: UnsafeMutablePointer<CChar> = get_board_info(&board) {
                        self.board_info = String.init(cString: cstring)
                        print(self.board_info ?? "NO INFO FOUND")
                        free_string_storage()

                        if (board_info != nil) {
                            let info_parts: [String] = self.board_info!.components(separatedBy: ".")
                            if info_parts.count > 1 {
                                // TODO
                            }
                        }
                    }
                }

                // Turn off the LED
                serial_set_led(&board, false)

                // Start 1-Wire
                if !one_wire_init(&board) {
                    closePortOnError("Error", "Could not initialise 1-Wire")
                    return
                }

                // Update the UI part deux
                self.hasStarted = true
                self.startButton.title = "Stop"
                self.connectedImage.image = self.greenImage
                self.connectedLabel.stringValue = "Connected"

                // Configure the sensor readings timer
                self.sensorTimer = Timer.init(timeInterval: self.sensorInterval,
                                              target: self,
                                              selector: #selector(self.takeReading),
                                              userInfo: nil,
                                              repeats: true)

                // Fire the timer immediately
                if let timer = self.sensorTimer {
                    RunLoop.main.add(timer, forMode: .default)
                    timer.fire()
                }

                // Start the first animation
                if let animation: NSViewAnimation = self.alphaRampDown {
                    animation.start()
                }
            } else {
                // Could not connect to the board - show an error
                showAlert("Error", "Could not connect to the selected board. Is a Bus Host Board connected to your Mac? Have you chosen the correct board from the Devices menu?")

                self.connectedImage.image = self.redImage
                self.connectedLabel.stringValue = "Not connected"
            }
        }
    }


    /**
     The user has clicked the About Sensor menu item.

     - Parameters:
        - sender: The button.
     */
    @IBAction @objc private func doShowAboutPanel(_ sender: Any) {

        guard let app = self.parentApp else { return }

        // Centre the text
        let paraStyle: NSMutableParagraphStyle = NSMutableParagraphStyle.init()
        paraStyle.alignment = .center
        let atts: [NSAttributedString.Key: Any] = [ .paragraphStyle: paraStyle ]

        // Establish the centred text content
        let credits: NSAttributedString = NSAttributedString.init(string: "A sample GUI app written in Swift and which makes use of the Depot client-side driver to connect to an RP2040-based Depot Multi-Bus Host wired to an Analog Devices DS18B20 1-Wire temperature sensor", attributes: atts)

        // Present the standard panel
        app.orderFrontStandardAboutPanel(options: [NSApplication.AboutPanelOptionKey.credits: credits])
    }


    // MARK: - Sensor Reading Functions

    /**
     Request a single reading from the sensor.
     */
    @objc private func takeReading() {

        // Send the convert command
        self.check = one_wire_reset(&board)
        if !self.check { return }
        one_wire_write_bytes(&board, &cmd_bytes_convert_temp, 2)

        // Wait 750ms -- too small to worry about UI?
        usleep(750 * 1000)

        // Send the read command
        self.check = one_wire_reset(&board)
        if !self.check { return }
        one_wire_write_bytes(&board, &cmd_bytes_read_scratch, 2)

        // Read back the data
        one_wire_read_bytes(&board, &reading, 2)

        // Calculate the result
        let raw_temp: Int = (Int(reading[1]) * 256) + Int(reading[0])
        celsiusTemp = Float((raw_temp << 16) >> 16) * 0.0625
        let outputString: String = String(format: "%.2f°C", celsiusTemp)
        self.outputLabel.stringValue = outputString

        // Draw the temperature on the icon
        if let image: NSImage = self.backImage {
            if self.parentApp != nil {
                self.parentApp!.applicationIconImage = image.addTextToImage(drawText: outputString)
            }
        }
    }


    // MARK: - Device Discovery and Management Functions

    /**
     The user has clicked the Start/Stop button.

     - Parameters:
        - deviceSelected: A device has already been selected.
                          Used to maintain the selection.
     */
    @objc private func refreshDevicesMenu(_ deviceSelected: Bool = true) {

        // Assemble the Devices menu
        self.devicesMenu.removeAllItems()

        // Get connected boards
        getBoards()

        // Prepare the device list
        if modemCount > 0 {
            for i: UInt32 in 0..<modemCount {
                let menuItem: NSMenuItem = NSMenuItem.init(title: modems[Int(i)],
                                                           action: #selector(self.doSelectBoard),
                                                           keyEquivalent: "")
                self.devicesMenu.addItem(menuItem)

                if deviceSelected {
                    if let mitem = self.devicesMenu.item(withTitle: self.current_device) {
                        mitem.state = .on
                    }
                } else {
                    if let mitem = self.devicesMenu.item(at: 0) {
                        mitem.state = .on
                        self.current_device = mitem.title
                    }
                }
            }
        } else {
            // No boards available so note this as a dummy menu item
            let menuItem: NSMenuItem = NSMenuItem.init(title: "No boards connected",
                                                       action: nil,
                                                       keyEquivalent: "")
            self.devicesMenu.addItem(menuItem)
            menuItem.isEnabled = false
        }

        // Add a spacer and a re-scan option
        self.devicesMenu.addItem(NSMenuItem.separator())

        let menuItem: NSMenuItem = NSMenuItem.init(title: "Scan for Boards",
                                                   action: #selector(self.refreshDevicesMenu),
                                                   keyEquivalent: "")
        self.devicesMenu.addItem(menuItem)
    }


    /**
     Discover connected devices.
     */
    private func getBoards() {

        // Get a list of connected devices: need to call Core Foundation C functions,
        // so we call an intermediate C function, `find_modems()`, which returns a
        // pointer to a C string comprising a sequence of device paths separated by
        // the `|` character.
        let devices: String = String.init(cString: find_boards(&modemCount))
        modems = devices.components(separatedBy: "|")
        free_string_storage()
    }


    /**
     The user has selected a board from the Devices menu

     - Parameters:
        - sender: A Devices menu menu item for a device.
     */
    @objc private func doSelectBoard(_ sender: Any) {

        // Cast the sender to an NSMenuItem
        let mitem: NSMenuItem = sender as! NSMenuItem

        // Bail if the user is selecting what they have
        // already selected
        if mitem.title == current_device {
            return
        }

        // Can't change horses mid-course, so warn the user and bail
        if self.hasStarted {
            showAlert("Warning", "Please stop reading the temperture before switching devices")
            return
        }

        // Switch off all the menu items...
        for aitem: NSMenuItem in self.devicesMenu.items {
            aitem.state = .off
        }

        // ...then nswitch on the current one...
        mitem.state = .on

        // ...and set the current device accordingly
        current_device = mitem.title
    }


    // MARK: - Alert Functions

    /**
     Generic alert display routine.

     Exits app on completion.

     - Parameters:
        - title:  The alert's title.
        - message: The alert's message.
     */
    private func showAlert(_ title: String, _ message: String) {

        let alert: NSAlert = NSAlert.init()
        alert.messageText = title
        alert.informativeText = message
        alert.addButton(withTitle: "OK")
        alert.beginSheetModal(for: self.window)
    }


    // MARK: - NSAnimation Delegate Functions

    /**
     Delegate function which is called when the current animation
     ends. It is used to trigger the next animation in the loop,
     of which there are just two: making the view visible and making
     the view invisible.

     - Parameters:
        - animation: The animation that has ended.

     - Returns: The constructed alert, ready to be shown.
     */
    func animationDidEnd(_ animation: NSAnimation) {

        // Make sure we are taking readings
        if self.hasStarted {
            // Select an animation based on state
            if animation == self.alphaRampDown! {
                self.alphaRampUp!.start()
            } else {
                self.alphaRampDown!.start()
            }
        }
    }

    /**
     Stop and zap any animations and timers currently running.
     */
    private func haltTimerAndAnimation() {

        if let timer = self.sensorTimer {
            if timer.isValid {
                timer.invalidate()
            }

            self.sensorTimer = nil
        }

        if let ramp = self.alphaRampUp {
            ramp.stop()
        }

        if let ramp = self.alphaRampDown {
            ramp.stop()
        }
    }

}


extension NSImage {

    func addTextToImage(drawText text: String) -> NSImage {

        let targetImage = NSImage(size: self.size, flipped: false) { (destRect: CGRect) -> Bool in

            // Drawing Handler code

            // Draw the base image
            self.draw(in: destRect)

            // Define the text attributes
            let textColor: NSColor = NSColor.white
            let textFont: NSFont = NSFont(name: "Menlo-Bold", size: 94)!
            let paraStyle: NSMutableParagraphStyle = NSMutableParagraphStyle()
            paraStyle.alignment = NSTextAlignment.center

            // Combine the text attributes
            let textFontAtts: [NSAttributedString.Key: Any] = [
                NSAttributedString.Key.font: textFont,
                NSAttributedString.Key.foregroundColor: textColor,
                NSAttributedString.Key.paragraphStyle: paraStyle
            ]

            // Render the text
            let textOrigin: CGPoint = CGPoint(x: 0.0, y: -200.0)
            let drawRect = CGRect(origin: textOrigin, size: self.size)
            text.draw(in: drawRect, withAttributes: textFontAtts)

            // Done!
            return true
        }

        return targetImage
    }
}
