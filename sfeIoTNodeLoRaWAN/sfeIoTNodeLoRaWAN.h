
/*
 *---------------------------------------------------------------------------------
 *
 * Copyright (c) 2024, SparkFun Electronics Inc.
 *
 * SPDX-License-Identifier: MIT
 *
 *---------------------------------------------------------------------------------
 */
#pragma once

#include <Flux.h>

#include <Flux/flxFmtCSV.h>
#include <Flux/flxFmtJSON.h>
#include <Flux/flxLogger.h>
#include <Flux/flxTimer.h>

#include <Flux/flxDevMAX17048.h>
#include <Flux/flxKVPStoreDeviceRP2.h>
#include <Flux/flxSettings.h>
#include <Flux/flxSettingsSerial.h>
#include <Flux/flxStorageKVPPref.h>
// #include <Flux/flxSysFirmware.h>

#include "flxLoRaWANDigi.h"
#include "flxLoRaWANLogger.h"
#include "sfeNLButton.h"

// Buffer size of our JSON document output
const uint16_t kAppJSONDocSize = 1600;
// Default log interval in milli secs
const uint16_t kDefaultLogInterval = 15000;

// What is the out of the box baud rate ..
const uint32_t kDefaultTerminalBaudRate = 115200;

// General startup delay (secs) for the startup menu
const uint32_t kStartupMenuDefaultDelaySecs = 2;

// Battery check interval (90 seconds)
const uint32_t kBatteryCheckInterval = 90000;

// Operation mode flags
#define kAppOpNone (0)
#define kAppOpEditing (1 << 0)
#define kAppOpStartup (1 << 1)
#define kAppOpPendingRestart (1 << 2)

// startup things
#define kAppOpStartNoAutoload (1 << 3)
#define kAppOpStartListDevices (1 << 4)
#define kAppOpStartNoSettings (1 << 5)

#define kAppOpStartAllFlags (kAppOpStartNoAutoload | kAppOpStartListDevices | kAppOpStartNoSettings)

#define inOpMode(__mode__) ((_opFlags & __mode__) == __mode__)
#define setOpMode(__mode__) _opFlags |= __mode__
#define clearOpMode(__mode__) _opFlags &= ~__mode__

// Define an event that triggers a log event -- long something
flxDefineEventID(kOnLogObservation);

// forward declare of our system info class
class sfeDLSystemOp;
class sfeIoTNodeLoRaWAN : public flxApplication
{
  private:
    //------------------------------------------
    // For controlling the log output types

    static constexpr uint8_t kAppLogTypeNone = 0x0;
    static constexpr uint8_t kAppLogTypeCSV = 0x1;
    static constexpr uint8_t kAppLogTypeJSON = 0x2;

    static constexpr const char *kLogFormatNames[] = {"Disabled", "CSV Format", "JSON Format"};
    uint8_t _logTypeSer; // type of serial log output format
    //---------------------------------------------------------------------------
    uint8_t get_logTypeSer(void);

    //---------------------------------------------------------------------------
    void set_logTypeSer(uint8_t logType);

    // color text
    bool get_color_text(void);
    void set_color_text(bool);

    // For the terminal baud rate setting
    uint32_t _terminalBaudRate;
    uint32_t get_termBaudRate(void);
    void set_termBaudRate(uint32_t rate);

    bool get_verbose_dev_name(void);
    void set_verbose_dev_name(bool);

    uint32_t get_jsonBufferSize(void);
    void set_jsonBufferSize(uint32_t);

    void about_app_status(void)
    {
        displayAppAbout();
    }

    std::string get_local_name(void);
    void set_local_name(std::string name);

    void onSettingsEdit(bool bLoading);
    void onSystemActivity(void);
    void onSystemActivityLow(void);

    void onErrorMessage(uint8_t);
    void onLogEvent(void);
    void onQwiicButtonEvent(bool);
    void onLoRaWANSendEvent(bool);

    // support for onInit
    void onInitStartupCommands(uint32_t);

  public:
    sfeIoTNodeLoRaWAN();

    bool onSetup();

    void onDeviceLoad();

    void onRestore();
    bool onStart();
    void onInit();

    bool loop();

    // Color Text Output
    flxPropertyRWBool<sfeIoTNodeLoRaWAN, &sfeIoTNodeLoRaWAN::get_color_text, &sfeIoTNodeLoRaWAN::set_color_text>
        colorTextOutput = {true};

    // JSON output buffer size
    flxPropertyRWUInt32<sfeIoTNodeLoRaWAN, &sfeIoTNodeLoRaWAN::get_jsonBufferSize,
                        &sfeIoTNodeLoRaWAN::set_jsonBufferSize>
        jsonBufferSize = {100, 5000};

    // Verbose Device Names
    flxPropertyRWBool<sfeIoTNodeLoRaWAN, &sfeIoTNodeLoRaWAN::get_verbose_dev_name,
                      &sfeIoTNodeLoRaWAN::set_verbose_dev_name>
        verboseDevNames;

    flxPropertyRWUInt8<sfeIoTNodeLoRaWAN, &sfeIoTNodeLoRaWAN::get_logTypeSer, &sfeIoTNodeLoRaWAN::set_logTypeSer>
        serialLogType = {kAppLogTypeCSV,
                         {{kLogFormatNames[kAppLogTypeNone], kAppLogTypeNone},
                          {kLogFormatNames[kAppLogTypeCSV], kAppLogTypeCSV},
                          {kLogFormatNames[kAppLogTypeJSON], kAppLogTypeJSON}}};

    flxParameterInVoid<sfeIoTNodeLoRaWAN, &sfeIoTNodeLoRaWAN::about_app_status> aboutApplication;

    // board user set name
    flxPropertyRWString<sfeIoTNodeLoRaWAN, &sfeIoTNodeLoRaWAN::get_local_name, &sfeIoTNodeLoRaWAN::set_local_name>
        localBoardName;

    // Serial Baud rate setting
    flxPropertyRWUInt32<sfeIoTNodeLoRaWAN, &sfeIoTNodeLoRaWAN::get_termBaudRate, &sfeIoTNodeLoRaWAN::set_termBaudRate>
        serialBaudRate = {1200, 500000};

    // startup delay setting
    flxPropertyUInt32<sfeIoTNodeLoRaWAN> startupDelaySecs = {0, 60};

  private:
    friend class sfeNLCommands;
    void _displayAboutObjHelper(char, const char *, bool);
    void displayAppAbout(void);
    void displayAppStatus(bool useInfo = false);

    void getStartupProperties(uint32_t &baudRate, uint32_t &startupDelay);

    // battery level checks
    void checkBatteryLevels(void);

    // Our LoRaWAN network/connection object
    flxLoRaWANDigi _loraWANConnection;
    flxLoRaWANLogger _loraWANLogger;

    // Our logger
    flxLogger _logger;

    // Timer for event logging
    flxTimer _timer;

    // Create a JSON and CSV output formatters.
    // Note: setting internal buffer sizes using template to minimize alloc calls.
    flxFormatJSON<kAppJSONDocSize> _fmtJSON;
    flxFormatCSV _fmtCSV;

    // Serial Settings editor
    flxSettingsSerial _serialSettings;

    // Settings storage
    flxStorageKVPPref _sysStorage;
    flxKVPStoreDeviceRP2 _sysStorageDevice;

    // Our firmware Update/Reset system
    // flxSysFirmware _sysUpdate;
    // for our button events of the board
    // sfeDLButton _boardButton;

    // Fuel gauge
    flxDevMAX17048 *_fuelGauge;

    // for our button events of the board
    sfeNLButton _boardButton;

    // battery check event
    std::unique_ptr<flxJob> _batteryJob;

    uint32_t _opFlags;
};