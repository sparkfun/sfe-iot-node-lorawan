
/*
 *---------------------------------------------------------------------------------
 *
 * Copyright (c) 2024-2025, SparkFun Electronics Inc.
 *
 * SPDX-License-Identifier: MIT
 *
 *---------------------------------------------------------------------------------
 */
#include "sfeIoTNodeLoRaWAN.h"
#include "sfeNLBoard.h"
#include "sfeNLCommands.h"
#include "sfeNLLed.h"
#include "sfeNLVersion.h"
#include <Arduino.h>

#include <Flux/flxDevButton.h>
#include <Flux/flxSerial.h>

static const char *kProductName = "SparkFun IoT Node LoRaWAN";

// Startup/Timeout for serial connection to init...
#define kSerialStartupDelayMS 5000

// delay used in loop during startup
const uint32_t kStartupLoopDelayMS = 70;

// Button event increment
#define kButtonPressedIncrement 5

//---------------------------------------------------------------------------
// LoRaWAN Receive message ids
//
// Set the on-board LED to the RGB value in the message
const uint8_t kLoRaWANMsgLEDRGB = 0x01;

// Turn off the on-board LED
const uint8_t kLoRaWANMsgLEDOff = 0x02;

// Set the on-board LED to blink with the given RGB color
const uint8_t kLoRaWANMsgLEDBlink = 0x03;

// Set the on-board LED to fast blink with the given RGB color
const uint8_t kLoRaWANMsgLEDFastBlink = 0x04;

// Set the on-board LED to flash with the given RGB color
const uint8_t kLoRaWANMsgLEDFlash = 0x05;

// Set the brightness for the  on-board LED
const uint8_t kLoRaWANMsgLEDBrightness = 0x06;

// For finding the firmware files on SD card
#define kLoRaWANFirmwareFilePrefix "sfeIoTNodeLoRaWAN_"
//---------------------------------------------------------------------------

// The default Soil Sensor pins
const uint8_t kSoilSensorVCCPin = 28;
const uint8_t kSoilSensorSensorPin = 29;
//---------------------------------------------------------------------------

// Application keys - used to encrypt runtime secrets for the app.
//
// NOTE: Gen a base 64 key  % openssl rand -base64 32
//       Convert into ASCII ints in python %    data = [ord(c) for c in ss]
//       Jam into the below array

// If a key array is passed in via a #define, use that, otherwise use a default, dev key
#ifdef IOT_NODE_LORAWAN_APP_KEY
static const uint8_t _app_jump[] = IOT_NODE_LORAWAN_APP_KEY;
#else
static const uint8_t _app_jump[] = {104, 72, 67, 51,  74,  67,  108, 99, 104, 112, 77,  100, 55,  106, 56,
                                    78,  68, 69, 108, 98,  118, 51,  65, 90,  48,  51,  82,  111, 120, 56,
                                    52,  49, 70, 76,  103, 77,  84,  49, 85,  99,  117, 66,  111, 61};
#endif

// Our ID/App class prefix
#define kAppClassPrefix "INLW"
//---------------------------------------------------------------------------
//
sfeIoTNodeLoRaWAN::sfeIoTNodeLoRaWAN()
    : _logTypeSD{kAppLogTypeNone}, _logTypeSer{kAppLogTypeNone}, _opFlags{0}, _hasOnBoardFlashFS{false}
{
    // Constructor
}
//---------------------------------------------------------------------------
void sfeIoTNodeLoRaWAN::getStartupProperties(uint32_t &baudRate, uint32_t &startupDelay)
{
    // Do we have this block in storage? And yes, a little hacky with name :)
    flxStorageBlock *stBlk = _sysStorage.getBlock(((flxObject *)this)->name());

    if (!stBlk)
    {
        baudRate = kDefaultTerminalBaudRate;
        startupDelay = kStartupMenuDefaultDelaySecs;
        return;
    }

    baudRate = 0;
    bool status = stBlk->read(serialBaudRate.name(), baudRate);

    if (!status)
        baudRate = kDefaultTerminalBaudRate;

    status = stBlk->read(startupDelaySecs.name(), startupDelay);
    if (!status)
        startupDelay = kStartupMenuDefaultDelaySecs;

    // // Startup Output message level
    // uint8_t uTmp;
    // status = stBlk->read(startupOutputMode.name(), uTmp);
    // startupOutputMode = (status ? uTmp : kAppStartupMsgNormal);

    _sysStorage.endBlock(stBlk);
}
///
/// @brief Checks for any before startup commands from the user
///

void sfeIoTNodeLoRaWAN::onInitStartupCommands(uint32_t delaySecs)
{

    // // Waking up from sleep?
    // if (boot_count > 0)
    //     return;

    uint32_t delayMS = delaySecs * 1000;

    uint32_t nIterations = delayMS / kStartupLoopDelayMS;

    // setup our commands
    //
    // may a simple command table
    typedef struct
    {
        char key;
        uint16_t mode;
        const char *name;
    } startupCommand_t;
    startupCommand_t commands[] = {{'n', kAppOpNone, "normal-startup"},
                                   {'v', kAppOpStartVerboseOutput, "verbose-output-enabled"},
                                   {'a', kAppOpStartNoAutoload, "device-auto-load-disabled"},
                                   {'l', kAppOpStartListDevices, "i2c-driver-listing-enabled"},
                                   {'s', kAppOpStartNoSettings, "settings-restore-disabled"}};

    // Default
    int iCommand = 0;

    Serial.printf("\n\r");

    if (nIterations > 0)
    {
        // clear buffer
        while (Serial.available() > 0)
            Serial.read();

        bool menuSelected = false;
        Serial.printf("Startup Options:\n\r");
        for (int n = 0; n < sizeof(commands) / sizeof(startupCommand_t); n++)
            Serial.printf("   '%c' = %s\n\r", commands[n].key, commands[n].name);
        Serial.printf("Select Option[%2us]:", delaySecs);

        char chBS = 8; // backspace
        char chCodeBell = 7;

        for (int i = 0; i < nIterations && !menuSelected; i++)
        {
            delayMS -= kStartupLoopDelayMS;
            if (delayMS / 1000 != delaySecs)
            {
                Serial.write(chBS);
                Serial.write(chBS);
                Serial.write(chBS);
                Serial.write(chBS);
                Serial.write(chBS);
                delaySecs = delayMS / 1000;
                Serial.printf("%2us]:", delaySecs);
            }

            delay(kStartupLoopDelayMS);

            if (Serial.available() > 0)
            {
                // code /key pressed
                uint8_t chIn = Serial.read();

                // any match
                for (int n = 0; n < sizeof(commands) / sizeof(startupCommand_t); n++)
                {
                    if (chIn == commands[n].key)
                    {
                        iCommand = n;
                        menuSelected = true;
                        break;
                    }
                }
                if (!menuSelected)
                    Serial.write(chCodeBell); // bad char
            }
        }
    }
    // set the op mode
    setOpMode(commands[iCommand].mode);
    Serial.printf(" %s\n\r", commands[iCommand].name);
}
//---------------------------------------------------------------------------
//
void sfeIoTNodeLoRaWAN::onInit()
{

    // Add a title for this section - the application level  - of settings
    setTitle("General");
    flxRegister(colorTextOutput, "Color Output", "Use color output with the Serial console");
    // user defined board name
    flxRegister(localBoardName, "Board Name", "A specific name for this IoT Node");

    // Update timer object string
    _timer.setName("Logging Timer", "Set the internal between log entries");
    // flxLog_I("in onInit()");

    _logTypeSer = kAppLogTypeNone;
    _logTypeSD = kAppLogTypeNone;
    serialLogType.setTitle("Output");
    flxRegister(serialLogType, "Serial Console Format", "Enable and set the output format");
    flxRegister(sdCardLogType, "SD Card Format", "Enable and set the output format");
    flxRegister(jsonBufferSize, "JSON Buffer Size", "Output buffer size in bytes");

    // Terminal Serial Baud Rate
    flxRegister(serialBaudRate, "Terminal Baud Rate", "Update terminal baud rate. Changes take effect on restart");
    _terminalBaudRate = kDefaultTerminalBaudRate;

    enableSoilSensor.setTitle("Devices");
    flxRegister(enableSoilSensor, "Soil Moisture Sensor", "Enable GPIO attached Soil Moisture Sensor");
    // Advanced settings
    verboseDevNames.setTitle("Advanced");
    flxRegister(verboseDevNames, "Device Names", "Name always includes the device address");
    flxRegister(startupDelaySecs, "Startup Delay", "Startup Menu Delay in Seconds");
    flxRegister(verboseEnabled, "Verbose Messages", "Enable verbose messages");

    // about?
    flxRegister(aboutApplication, "About...", "Details about the system");
    aboutApplication.prompt = false; // no prompt needed before execution

    // setup storage for our settings
    const uint32_t kSegmentSize = 4096;
    uint8_t *preStart = (uint8_t *)(XIP_BASE + PICO_FLASH_SIZE_BYTES - (kSegmentSize * 10));

    _sysStorageDevice.initialize(preStart, kSegmentSize, 10);
    _sysStorage.setStorageDevice(&_sysStorageDevice);
    flxSettings.setStorage(&_sysStorage);
    flxSettings.setFallback(&_jsonStorage);

    _jsonStorage.setFileSystem(&_theSDCard);
    _jsonStorage.setFilename("iot-node-lorawan.json");

    _theSDCard.setCSPin(kNLBoardSDCardCSPin);

    // Did the user set a serial value?
    uint32_t theRate;
    uint32_t theDelay;
    getStartupProperties(theRate, theDelay);

    // setup Serial
    Serial.begin(theRate);

    // Wait on serial - not sure if a timeout is needed ... but added for safety
    for (uint32_t startMS = millis(); !Serial && millis() - startMS <= kSerialStartupDelayMS;)
        delay(250);

    // flush serial - needed for initial output to work
    Serial.println();
    Serial.flush();

    // setup the editor - to keep  a look at flux
    _serialSettings.setSystemRoot(&flux);
    sfeLED.initialize();
    sfeLED.on(sfeLED.Green);

    // app key
    flux.setAppToken(_app_jump, sizeof(_app_jump));

    // we don't want to start connecting to the LoRaWAN right away - the module needs to initialize
    _loraWANConnection.setDelayedStartup(true);

    setOpMode(kAppOpStartup);
    startupDelaySecs = theDelay;
    onInitStartupCommands(theDelay);
}
//---------------------------------------------------------------------------
//

bool sfeIoTNodeLoRaWAN::onSetup()
{
    // flxLog_I("in onSetup()");
    setName(kProductName);
    setDescription(kDLVersionBoardDesc);

    // Version info
    setVersion(kDLVersionNumberMajor, kDLVersionNumberMinor, kDLVersionNumberPoint, kDLVersionDescriptor, BUILD_NUMBER);

    // Have settings saved when editing via serial console is complete.
    flxRegisterEventCB(flxEvent::kOnEdit, this, &sfeIoTNodeLoRaWAN::onSettingsEdit);
    flxRegisterEventCB(flxEvent::kOnEditFinished, &flxSettings, &flxSettingsSave::saveEvent_CB);

    // setup our event callbacks for system/framework events;
    flxRegisterEventCB(flxEvent::kOnSystemActivity, this, &sfeIoTNodeLoRaWAN::onSystemActivity);
    flxRegisterEventCB(flxEvent::kOnSystemActivityLow, this, &sfeIoTNodeLoRaWAN::onSystemActivityLow);

    // Event Callback for a "onLogEvent"
    flxRegisterEventCB(flxEvent::kOnLogObservation, this, &sfeIoTNodeLoRaWAN::onLogEvent);

    // Event Callback for lorawan send status
    flxRegisterEventCB(flxEvent::kLoRaWANSendStatus, this, &sfeIoTNodeLoRaWAN::onLoRaWANSendEvent);

    // Event Callback for lorawan receive status
    flxRegisterEventCB(flxEvent::kLoRaWANReceivedMessage, this, &sfeIoTNodeLoRaWAN::onLoRaWANReceiveEvent);

    // for system reset event
    flxRegisterEventCB(flxEvent::kOnSystemReset, this, &sfeIoTNodeLoRaWAN::onSystemResetEvent);

    // for needs restart event
    flxRegisterEventCB(flxEvent::kSystemNeedsRestart, this, &sfeIoTNodeLoRaWAN::onNeedsRestart);

    // was device auto load disabled by startup commands?
    if (inOpMode(kAppOpStartNoAutoload))
        flux.setAutoload(false);

    // was settings restore disabled by startup commands?
    if (inOpMode(kAppOpStartNoSettings))
        flux.setLoadSettings(false);

    // Verbose output
    if (inOpMode(kAppOpStartVerboseOutput))
        set_verbose(true);

    // was list device divers set by startup commands?
    if (inOpMode(kAppOpStartListDevices))
        flux.dumpDeviceAutoLoadTable();
    // setup SD card. Do this before calling start - so prefs can be read off SD if needed
    if (!setupSDCard())
    {
        flxLog_W(F("Unable to initialize the SD Card. Is an SD card installed on the board?"));
    }

    // Filesystem to read firmware from
    _sysUpdate.setFileSystem(&_theSDCard);

    // Serial UX - used to list files to select off the filesystem
    _sysUpdate.setSerialSettings(_serialSettings);

    _sysUpdate.setFirmwareFilePrefix(kLoRaWANFirmwareFilePrefix);

    flxRegisterEventCB(flxEvent::kOnFirmwareLoad, this, &sfeIoTNodeLoRaWAN::onFirmwareLoad);
    flux_add(&_sysUpdate);

    // check our on-board flash file system
    _hasOnBoardFlashFS = checkOnBoardFS();

    // Button events we're listening on
    _boardButton.on_momentaryPress.call(this, &sfeIoTNodeLoRaWAN::onLogEvent);

    flux_add(_boardButton);

    // We want an event every 5 seconds
    _boardButton.setPressIncrement(kButtonPressedIncrement);

    // Button events we're listening on
    _boardButton.on_buttonRelease.call(this, &sfeIoTNodeLoRaWAN::onButtonReleased);
    _boardButton.on_buttonPressed.call(this, &sfeIoTNodeLoRaWAN::onButtonPressed);

    // titles for menu arrangement
    _loraWANConnection.setTitle("LoRaWAN");
    _logger.setTitle("Logging");
    _sysSystem.setTitle("System");

    return true;
}
//---------------------------------------------------------------------------
//
void sfeIoTNodeLoRaWAN::onDeviceLoad()
{

    // quick check on fuel gauge
    auto fuelGauge = flux_get().get<flxDevMAX17048>();
    if (fuelGauge->size() > 0)
        _fuelGauge = fuelGauge->at(0);

    // Was a button attached? If it is, on press event, we trigger a log event.
    // To do this, we need to *wire up* the events
    // Identify any buttons
    // Add their on_clicked events as logger triggers
    auto buttons = flux_get().get<flxDevButton>();

    for (auto b : *buttons)
        b->on_clicked.call(this, &sfeIoTNodeLoRaWAN::onQwiicButtonEvent);

    // setup our soil sensor device
    _soilSensor.vccPin = kSoilSensorVCCPin;
    _soilSensor.sensorPin = kSoilSensorSensorPin;
}
//---------------------------------------------------------------------------
//
void sfeIoTNodeLoRaWAN::onRestore()
{
    // flxLog_I("in onRestore()");
    setAppClassID(kDLAppClassNameID, (char *)kAppClassPrefix);
    // Set the default timer interval, before restore of settings
    _timer.interval = kDefaultLogInterval;
}

//---------------------------------------------------------------------------
//
bool sfeIoTNodeLoRaWAN::onStart()
{
    // flxLog_I("in onStart()");

    // Logging is done at an interval - using an interval timer.
    // connect our logger method to the timer interval
    _timer.on_interval.call(this, &sfeIoTNodeLoRaWAN::onLogEvent);

    //  - Add the JSON and CVS format to the logger
    _logger.add(_fmtJSON);
    _logger.add(_fmtCSV);

    _sysSystem.setSerialSettings(_serialSettings);

    // check our I2C devices
    // Loop over the device list - note that it is iterable.
    flxLog_I_(F("Loading devices ... "));
    flxDeviceContainer loadedDevices = flux.connectedDevices();
    if (loadedDevices.size() == 0)
        flxLog_N(F("no devices detected"));
    else
    {
        flxLog_N(F("%d devices detected"), loadedDevices.size());
        for (auto device : loadedDevices)
        {
            // output the connected devices ... include device type/address
            flxLog_N_(F("    %-20s  - %-40s  {"), device->name(), device->description());
            if (device->getKind() == flxDeviceKindI2C)
                flxLog_N("%s x%x}", "qwiic", device->address());
            else if (device->getKind() == flxDeviceKindSPI)
                flxLog_N("%s p%u}", "SPI", device->address());
            else if (device->getKind() == flxDeviceKindGPIO)
                flxLog_N("%s p%u}", "GPIO", device->address());

            if (device->nOutputParameters() > 0)
            {
                _logger.add(device);
                _loraWANLogger.add(device);
            }
        }
    }

    // setup the ENS160
    setupENS160();

    // Check time devices
    if (!setupTime())
        flxLog_W(F("Time reference setup failed."));

    flxLog_N("");
    // Do we have a fuel gauge ...
    if (_fuelGauge)
    {
        _batteryJob.reset(new flxJob);
        if (_batteryJob != nullptr)
        {
            _batteryJob->setup("Battery", kBatteryCheckInterval, this, &sfeIoTNodeLoRaWAN::checkBatteryLevels);
            flxAddJobToQueue(*_batteryJob);
        }
    }

    // now load our LoRaWAN .. delay for the module to initialize
    if (!_loraWANConnection.connect())
    {
        flxLog_N(" - connectivity not available");
        // put the module in a reconnect state
        _loraWANConnection.startReconnectMode();
    }

    // connect the LoRaWAN logger to the LoRaWAN object
    _loraWANLogger.setLoRaWAN(&_loraWANConnection);

    // Display detailed app info
    displayAppStatus(true);

    flxLog_N("");
    flxSerial.textToWhite();
    flxLog_N_("Startup Complete");
    flxSerial.textToNormal();
    flxLog_N(" - Press any key to enter the settings menu, '!' to enter console commands - '!help' for commands");

    // not in startup now. Clear startup flags
    clearOpMode(kAppOpStartup);
    clearOpMode(kAppOpStartAllFlags);

    sfeLED.off();

    // trigger a log event if we are connected
    if (_loraWANConnection.isConnected())
        _timer.trigger();
    return true;
}

//---------------------------------------------------------------------------
// Color text output
bool sfeIoTNodeLoRaWAN::get_color_text(void)
{
    return flxSerial.colorEnabled();
}
//---------------------------------------------------------------------------

void sfeIoTNodeLoRaWAN::set_color_text(bool enable)
{
    flxSerial.setColorEnabled(enable);
}
//---------------------------------------------------------------------------
// device names
//---------------------------------------------------------------------------
bool sfeIoTNodeLoRaWAN::get_verbose_dev_name(void)
{
    return flux.verboseDevNames();
}

void sfeIoTNodeLoRaWAN::set_verbose_dev_name(bool enable)
{
    flux.setVerboseDevNames(enable);
}
//---------------------------------------------------------------------------
// json Buffer Size

uint32_t sfeIoTNodeLoRaWAN::get_jsonBufferSize(void)
{
    return _fmtJSON.bufferSize();
}

void sfeIoTNodeLoRaWAN::set_jsonBufferSize(uint32_t new_size)
{
    _fmtJSON.setBufferSize(new_size);
}

uint8_t sfeIoTNodeLoRaWAN::get_logTypeSD(void)
{
    return _logTypeSD;
}
//---------------------------------------------------------------------------
void sfeIoTNodeLoRaWAN::set_logTypeSD(uint8_t logType)
{
    if (logType == _logTypeSD)
        return;

    if (_logTypeSD == kAppLogTypeCSV)
        _fmtCSV.remove(&_theOutputFile);
    else if (_logTypeSD == kAppLogTypeJSON)
        _fmtJSON.remove(&_theOutputFile);

    _logTypeSD = logType;

    if (_logTypeSD == kAppLogTypeCSV)
        _fmtCSV.add(&_theOutputFile);
    else if (_logTypeSD == kAppLogTypeJSON)
        _fmtJSON.add(&_theOutputFile);
}

//---------------------------------------------------------------------------
uint8_t sfeIoTNodeLoRaWAN::get_logTypeSer(void)
{
    return _logTypeSer;
}
/**
 * @brief      Sets the log type for serial output
 *
 * @param[in]  logType  The log type
 */
void sfeIoTNodeLoRaWAN::set_logTypeSer(uint8_t logType)
{
    if (logType == _logTypeSer)
        return;

    if (_logTypeSer == kAppLogTypeCSV)
        _fmtCSV.remove(flxSerial);
    else if (_logTypeSer == kAppLogTypeJSON)
        _fmtJSON.remove(flxSerial);

    _logTypeSer = logType;

    if (_logTypeSer == kAppLogTypeCSV)
        _fmtCSV.add(flxSerial);
    else if (_logTypeSer == kAppLogTypeJSON)
        _fmtJSON.add(flxSerial);
}

//---------------------------------------------------------------------------
// local/board name things
/**
 * @brief      Getter for the  the local name property
 *
 * @return     The local name.
 */

std::string sfeIoTNodeLoRaWAN::get_local_name(void)
{
    return flux.localName();
}
//---------------------------------------------------------------------------
/**
 * @brief      Setter for the local name property
 *
 * @param[in]  name  The name
 */

void sfeIoTNodeLoRaWAN::set_local_name(std::string name)
{
    flux.setLocalName(name);
}

//---------------------------------------------------------------------------
// soil sensor enabled/disable
//---------------------------------------------------------------------------
void sfeIoTNodeLoRaWAN::set_soil_enabled(bool enable)
{
    // Is the soil sensor in the system?

    bool active = flux.contains(_soilSensor);
    if (active == enable)
        return; // same

    if (enable)
    {
        // is the sensor initialized?
        if (!_soilSensor.isInitialized())
        {
            // init the sensor - this adds to the device list
            if (_soilSensor.initialize() == false)
                flxLog_W(F("%s: failed to initialize."), _soilSensor.name());
        }
        else
            flux.add(_soilSensor);
        _loraWANLogger.add(_soilSensor);
        _logger.add(_soilSensor);
    }
    else
    {
        flux.remove(_soilSensor);
        _loraWANLogger.remove(_soilSensor);
        _logger.remove(_soilSensor);
    }

    _soilSensor.isEnabled(enable);
}

bool sfeIoTNodeLoRaWAN::get_soil_enabled(void)
{
    return flux.contains(_soilSensor);
}
//---------------------------------------------------------------------------
// Display things during settings edits
//---------------------------------------------------------------------------
void sfeIoTNodeLoRaWAN::onSettingsEdit(bool bLoading)
{

    if (bLoading)
    {
        setOpMode(kAppOpEditing);
        sfeLED.on(sfeLED.LightGray);
    }
    else
    {
        sfeLED.off();

        // no longer editing
        clearOpMode(kAppOpEditing);

        // did the editing operation set a restart flag? If so see if the user wants to restart
        // the device.
        if (inOpMode(kAppOpPendingRestart))
        {
            flxLog_N("\n\rSome changes required a device restart to take effect...");
            _sysSystem.restartDevice();

            // this shouldn't return unless user aborted
            clearOpMode(kAppOpPendingRestart);
        }
    }
}
//---------------------------------------------------------------------------
void sfeIoTNodeLoRaWAN::onSystemActivity(void)
{
    sfeLED.flash(sfeLED.Gray);
}

//---------------------------------------------------------------------------
void sfeIoTNodeLoRaWAN::onSystemActivityLow(void)
{
    sfeLED.flash(sfeLED.Blue);
}
//---------------------------------------------------------------------------
// Flash led on error/warnings
//---------------------------------------------------------------------------
void sfeIoTNodeLoRaWAN::onErrorMessage(uint8_t msgType)
{
    // send an LED thing
    if (msgType == (uint8_t)flxLogError)
        sfeLED.flash(sfeLED.Red);
    else if (msgType == (uint8_t)flxLogWarning)
        sfeLED.flash(sfeLED.Yellow);
}

//---------------------------------------------------------------------------
// Button Events - general handler
//---------------------------------------------------------------------------
//
// CAlled when the button is pressed and an increment time passed
void sfeIoTNodeLoRaWAN::onButtonPressed(uint32_t increment)
{

    // we need LED on for visual feedback...
    sfeLED.setDisabled(false);

    if (increment == 1)
        sfeLED.blink(sfeLED.Yellow, kLEDFlashSlow);

    else if (increment == 2)
        sfeLED.blink(kLEDFlashMedium);

    else if (increment == 3)
        sfeLED.blink(kLEDFlashFast);

    else if (increment >= 4)
    {
        sfeLED.stop();

        sfeLED.on(sfeLED.Red);
        delay(500);
        sfeLED.off();

        // Reset time !
        _sysSystem.resetDevice();
    }
}
//---------------------------------------------------------------------------
void sfeIoTNodeLoRaWAN::onButtonReleased(uint32_t increment)
{
    if (increment > 0)
        sfeLED.off();
}

// need a restart message
void sfeIoTNodeLoRaWAN::onNeedsRestart(void)
{
    if (inOpMode(kAppOpStartup))
        return;

    // set the need a restart flag
    setOpMode(kAppOpPendingRestart);
}
// ---------------------------------------------------------------------------
// Log event
// ---------------------------------------------------------------------------
void sfeIoTNodeLoRaWAN::onLogEvent(void)
{
    // Log data - triggered from an event
    _logger.logObservation();
    _loraWANLogger.logObservation();
}
//---------------------------------------------------------------------------
// for qwiic button events
void sfeIoTNodeLoRaWAN::onQwiicButtonEvent(bool bPressed)
{
    if (bPressed)
        onLogEvent();
}

void sfeIoTNodeLoRaWAN::onLoRaWANSendEvent(bool bOkay)
{
    if (bOkay)
        sfeLED.flash(sfeLED.Green);
    else
        sfeLED.flash(sfeLED.Red);
}

void sfeIoTNodeLoRaWAN::onSystemResetEvent(void)
{
    // The system is being reset - reset our settings
    flxSettings.reset();
}
//---------------------------------------------------------------------------
void sfeIoTNodeLoRaWAN::onFirmwareLoad(bool bLoading)
{
    if (bLoading)
        sfeLED.on(sfeLED.Yellow);
    else
        sfeLED.off();
}

//---------------------------------------------------------------------------
// Callback for LoRaWAN receive events
void sfeIoTNodeLoRaWAN::onLoRaWANReceiveEvent(uint32_t data)
{

    // flxLog_I("LoRaWAN Received Event: 0x%0.8X", data);

    uint8_t *pData = (uint8_t *)&data;

    // We basically update/change the state of the on-board LED
    sfeLEDColor_t color;
    switch (pData[0])
    {
    case kLoRaWANMsgLEDRGB:
        color = pData[1] << 16 | pData[2] << 8 | pData[3];
        sfeLED.on(color);
        break;

    case kLoRaWANMsgLEDOff:
        sfeLED.off();
        break;
    case kLoRaWANMsgLEDBlink:
        color = pData[1] << 16 | pData[2] << 8 | pData[3];
        sfeLED.blink(color, 1000);
        break;
    case kLoRaWANMsgLEDFastBlink:
        color = pData[1] << 16 | pData[2] << 8 | pData[3];
        sfeLED.blink(color, 500);
        break;
    case kLoRaWANMsgLEDFlash:
        color = pData[1] << 16 | pData[2] << 8 | pData[3];
        sfeLED.flash(color);
        break;
    case kLoRaWANMsgLEDBrightness:
        flxLog_I("Brightness: %u", pData[1]);
        sfeLED.brightness(pData[1]);
        break;
    }
}
//---------------------------------------------------------------------------
// checkBatteryLevels()
//
// If a battery is attached, flash the led based on state of charge
//
void sfeIoTNodeLoRaWAN::checkBatteryLevels(void)
{
    if (!_fuelGauge)
        return;

    float batterySOC = _fuelGauge->getSOC();

    sfeLEDColor_t color;

    if (batterySOC > kBatteryNoBatterySOC) // no battery
        return;

    if (batterySOC < 10.)
        color = sfeLED.Red;
    else if (batterySOC < 50.)
        color = sfeLED.Yellow;
    else
        color = sfeLED.Green;

    sfeLED.flash(color);
}

//---------------------------------------------------------------------------
// Terminal Baud-rate things
//---------------------------------------------------------------------------
uint32_t sfeIoTNodeLoRaWAN::get_termBaudRate(void)
{
    return _terminalBaudRate;
}
//---------------------------------------------------------------------------
void sfeIoTNodeLoRaWAN::set_termBaudRate(uint32_t newRate)
{
    // no change?
    if (newRate == _terminalBaudRate)
        return;

    _terminalBaudRate = newRate;

    // Was this done during an edit session?
    if (inOpMode(kAppOpEditing))
    {
        flxLog_N(F("\n\r\n\r\t[The new baud rate of %u takes effect when this device is restarted]"), newRate);
        delay(700);
        setOpMode(kAppOpPendingRestart);
    }
}
//---------------------------------------------------------------------------
// verbose messages
//---------------------------------------------------------------------------
void sfeIoTNodeLoRaWAN::set_verbose(bool enable)
{

    // If disable, but we are in startup mode that enables verbose, don't set disable
    if (enable)
    {

        flxSetLoggingVerbose();

        // if in startup, the verbose mode is being set via pref restore. Note the change to user
        if (inOpMode(kAppOpStartup))
        {
            flxLog_N("");
            flxLog_V(F("Verbose output enabled"));
        }
    }
    else if (!inOpMode(kAppOpStartVerboseOutput))
        flxSetLoggingInfo();
}
bool sfeIoTNodeLoRaWAN::get_verbose(void)
{
    return flxIsLoggingVerbose();
}
//---------------------------------------------------------------------------
// Loop
//---------------------------------------------------------------------------

bool sfeIoTNodeLoRaWAN::loop()
{
    // key press at Serial Console? What to do??
    if (Serial.available())
    {
        // Bang command?
        uint8_t chIn = Serial.read();
        if (chIn == '!')
        {
            flxSerial.textToWhite();
            Serial.write('>');
            flxSerial.textToNormal();
            Serial.write('!');
            Serial.flush();
            sfeNLCommands cmdProcessor;
            bool status = cmdProcessor.processCommand(this);
        }
        else // edit settings
        {
            // start an editing session
            sfeLEDColor_t color;
            int status = _serialSettings.editSettings();

            if (status == -1)
            {
                // clear buffer - if anything is left around in the queue.
                while (Serial.available() > 0)
                    Serial.read();
                color = sfeLED.Red;
            }
            else if (status == 1)
                color = sfeLED.Green;
            else
                color = sfeLED.Yellow;

            sfeLED.flash(color);
        }
    }
    return false;
}