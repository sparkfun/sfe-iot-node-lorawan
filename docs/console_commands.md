# IOT Node - LoRaWAN - Console Commands

The serial console interactive menu system of the IoT Node - LoRaWAN an intuitive, dynamic and flexible methodology to interact with the system configuration.While extremely capable, when performing common admin or debugging tasks the menu system is an impediment to the desired operation. A fast, interactive solution is needed.

To provide rapid access to common administration and debugging commands, the IoT Node - LoRaWAN firmware provides a set of commands that are entered directly into the console. Starting with a "!" (bang), these commands are referred to as ***Console Commands*** or ***Bang Commands***.

## Available Commands

The following commands are available:

|Command | Description|
|:---|:----|
|<nobr>!reset-device-forced</nobr>|Resets the device (clears settings and restarts the device) without a prompt|
|<nobr>!reset-device</nobr>| Resets the device (clears settings and restarts the device) with a [Yes/No] prompt|
|<nobr>!clear-settings</nobr>|Clears the on-device saved settings with a [Yes/No] prompt|
|<nobr>!clear-settings-forced</nobr>|Clears the on-device saved settings without a prompt|
|<nobr>!restart</nobr>|Restarts the device with a [Yes/No] prompt|
|<nobr>!restart-forced</nobr>|Restarts the device without a prompt|
|<nobr>!log-rate</nobr>|Outputs the current log-rate of the device (milliseconds between logging transactions)|
|<nobr>!log-rate-toggle</nobr>|Toggle the on/off state of the log rate data recording by the system. This value is not persisted to on-board settings unless the settings are saved.|
|<nobr>!devices</nobr>|Lists the currently connected devices|
|<nobr>!save-settings</nobr>|Saves the current system settings to the preference system|
|<nobr>!verbose</nobr>|Toggles Verbose output/message mode. This value is not persistent|
|<nobr>!heap</nobr>|Outputs the current statistics of the system heap memory|
|<nobr>!log-now</nobr>|Trigger a data logging event|
|<nobr>!lora-status</nobr>|Display the status and settings of the LoRaWAN|
|<nobr>!about</nobr>|Outputs the full *About* page of the Node Board|
|<nobr>!version</nobr>|Outputs the firmware version|
|<nobr>!help</nobr>|Outputs the available *!* commands|

### Command Usage

To use a *Bang Command*, connect to the target IoT Node - LoRaWAN device via a serial console. and enter the command, starting with a `!` symbol  Entering any other character will launch the standard DataLogger IoT menu system. When the `!` symbol is pressed, a prompt **>** should appear. 

> Note: Since most *Bang Commands* are not interactive, they also enable commanding by another device, such as a computer attached to the IoT Node board.
