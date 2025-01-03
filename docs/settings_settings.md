# IoT Node - LoRaWAN Settings

The following sections provide an overview of the available settings and functions within the settings menu of the IoT Node - LoRaWAN firmware.

The settings system is divided into two sections - ***Settings*** and ***Device Settings*** as shown in the following image:

![Settings Top Menu](assets/img/settings_top_menu.png)

## Settings - System Settings and Operations

When this entry is selected, the following entries are presented:

![All Settings](assets/img/settings_all_settings.png)

### Application Settings

The ***Application Settings*** menu has the following entries:

![Settings > Application Settings](assets/img/settings_application.png)

#### Color Output

When enabled (the default), output to the console will use colors to highlight values and functionality. When disabled, all output is plain text. When using a limited serial console application, set this value to false.

#### Board Name

Set this value any name desired for the particular board. By default this value is ***empty***.

#### Serial Console Format

This property sets the output format for sensor/log data for the serial console. Available values include CSV, JSON and Disabled. The default value is ***CSV***.

#### JSON Buffer Size 

This is the size - in bytes - the application should use when building JSON formatted output. The default value is ***1600*** bytes. This value should only be adjusted if logging a large number of values to the serial console. 

The maximum bytes used by the JSON system is displayed on the ***About*** page of the application (try using `!about` console command to see this value).

#### Terminal Baud Rate 

Use this value to change the baud rate settings for the serial console. Once changed this value might require a system restart to  take effect. The default value is ***115200***.

#### Device Names

When enabled (default is ***disabled***), attached devices have their address appended to the name. This is helpful to identify a particular board when two or more of the same board type are attached to the IoT Node board.

#### Startup Delay

When the IoT Node board starts up, an startup menu is displayed. This settings determines how long to display this menu. Setting this value disables the menu. 

The startup menu has the following options:

![Startup Menu](assets/img/settings_startupmenu.png)

Pressing the indicted menu key selects that option. If the startup delay expires, a *normal* startup sequence occurs.

##### Startup Menu Entries

* 'n' = Normal startup sequence 
* 'v' = Verbose output messaging is enabled at startup
* 'a' = Disable qwiic device autoload 
* 'l' = List the available qwiic/i2c drivers at startup (this list is deleted after startup)
* 's' = Disable settings restore on startup 

#### Verbose Messages

When enabled, verbose output is enabled. This value is ***disabled*** by default.

#### About...

When selected, the system displays the ***About*** page for the application. This reflects the current status of system. This page is also displayed using the `!about` console command.

### LoRaWAN Network 

The LoRaWAN Network page has the following settings:

![LoRaWAN Network](assets/img/settings_lorawan.png)

#### Enabled

Used to enable/disable the LoRaWAN network functionality. This value is *enabled* by default. 

#### Application EUI 

The Application EUI for the LoRaWAN connection

#### Application Key

The Application Key for the LoRaWAN connection. This value is *secret*, and while editable, it is not visible when editing. 

#### Network Key 

The Network Key for the LoRaWAN connection. This value is *secret*, and while editable, it is not visible when editing. 

#### LoRaWAN Class 

The operating class for the LoRaWAN module. By default this value is set to ***C***.

#### LoRaWAN Region 

The operating region for the LoRaWAN module. By default this value is ***US915***. 

#### Reset

Calls the *reset* function on the module. 

### LoRaWAN Logger 

This pages has no entires currently.

### Logger

The logger system is used to output values to the Serial Console. 

The logger has the following settings:

![Settings Logger](assets/img/settings_logger.png)


#### Timestamp Mode

Enables output of a timestamp to the logged information and sets the format of the timestamp. By default this is ***disabled***. 

#### Sample Numbering

If enabled, a sample number is included in the console output of logged data. This value is ***disabled*** by default.

#### Numbering Increment 

The amount to increment each sample number when sample numbering is enabled. The default value is ***1***.

#### Output ID

When enabled, the board ID is included in the logged output. This value is ***disabled*** by default. 

#### Output Name

When enabled, the board Name is included in the logged output. This value is ***disabled*** by default.

#### Rate Metic

When enabled, metrics are recorded for the logging system. This value is ***disabled*** by default.

#### Reset Sample Counter

Resets the data sample number to a user provided value. 

### Logging Timer

The timer has the following properties:

![Logging Timer](assets/img/settings_timer.png)

#### Interval

The interval for the logging timer. For every *interval* period, a logging event occurs - sending data to the LoRaWAN and serial console (if enabled).  The default value is ***90000 ms***.

### System Control

The System Control page includes the following entries:

![System Control](assets/img/settings_system.png)

#### Device Restart

When selected, the user is prompted to restart the device/system. This is also available using the `!restart` console command.

#### Device Reset

When selected, the user is prompted to reset the device/system. A device reset erases the current saved settings values and restarts the board/system. This is also available using the `!reset-device` console command.