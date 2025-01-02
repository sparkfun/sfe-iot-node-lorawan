/*
 *---------------------------------------------------------------------------------
 *
 * Copyright (c) 2024-2025, SparkFun Electronics Inc.
 *
 * SPDX-License-Identifier: MIT
 *
 *---------------------------------------------------------------------------------
 */
#pragma once

#include "sfeIoTNodeLoRaWAN.h"
#include <ArduinoJson.h>
#include <Flux/flxCoreLog.h>

#include <Flux/flxPlatform.h>
#include <Flux/flxSerialField.h>
#include <Flux/flxUtils.h>
#include <time.h>

class sfeNLCommands
{
    typedef bool (sfeNLCommands::*commandCB_t)(sfeIoTNodeLoRaWAN *);
    typedef std::map<std::string, commandCB_t> commandMap_t;

    //---------------------------------------------------------------------
    // Command Callbacks
    //---------------------------------------------------------------------

    //---------------------------------------------------------------------
    bool resetDevice(sfeIoTNodeLoRaWAN *theApp)
    {
        if (!theApp)
            return false;

        theApp->_sysSystem.resetDevicePrompt();

        // should never get here
        return true;
    }
    //---------------------------------------------------------------------
    bool resetDeviceForced(sfeIoTNodeLoRaWAN *theApp)
    {
        if (!theApp)
            return false;

        theApp->_sysSystem.resetDevice();

        return true;
    }
    //---------------------------------------------------------------------
    bool clearDeviceSettings(sfeIoTNodeLoRaWAN *theApp)
    {
        if (!theApp)
            return false;

        // Need to prompt for an a-okay ...
        Serial.printf("\n\rClear Device Saved Settings? [Y/n]? ");
        uint8_t selected = theApp->_serialSettings.getMenuSelectionYN();
        flxLog_N("");

        if (selected != 'y' || selected == kReadBufferTimeoutExpired || selected == kReadBufferExit)
        {
            flxLog_I(F("Aborting..."));
            return false;
        }
        return clearDeviceSettingsForced(theApp);

        return true;
    }

    //---------------------------------------------------------------------
    bool clearDeviceSettingsForced(sfeIoTNodeLoRaWAN *theApp)
    {
        if (!theApp)
            return false;

        theApp->_sysStorage.resetStorage();
        flxLog_I(F("Settings Cleared"));

        return true;
    }
    // //---------------------------------------------------------------------
    bool restartDevice(sfeIoTNodeLoRaWAN *theApp)
    {
        if (theApp)
            theApp->_sysSystem.restartDevicePrompt();

        return true;
    }
    //---------------------------------------------------------------------
    bool restartDeviceForced(sfeIoTNodeLoRaWAN *theApp)
    {

        if (theApp)
            theApp->_sysSystem.restartDevice();

        return true;
    }
    //---------------------------------------------------------------------
    bool aboutDevice(sfeIoTNodeLoRaWAN *theApp)
    {
        if (theApp)
            theApp->displayAppAbout();
        return true;
    }
    //---------------------------------------------------------------------
    bool helpDevice(sfeIoTNodeLoRaWAN *theApp)
    {
        flxLog_N(F("Available Commands:"));
        for (auto it : _commandMap)
            flxLog_N(F("   !%s"), it.first.c_str());

        return true;
    }
    //---------------------------------------------------------------------
    ///
    /// @brief Reads JSON from the serial console - uses as input into the settings system
    ///
    /// @param theApp Pointer to the DataLogger App
    /// @retval bool indicates success (true) or failure (!true)
    ///
    // bool loadJSONSettings(sfeIoTNodeLoRaWAN *theApp)
    // {
    //     if (!theApp)
    //         return false;

    //     flxLog_I(F("Load JSON Settings - Not Implemented"));

    //     // // Create a JSON prefs serial object and read in the settings
    //     // flxStorageJSONPrefSerial prefsSerial(flxSettings.fallbackBuffer() > 0 ? flxSettings.fallbackBuffer() :
    //     2000);

    //     // // restore the settings from serial
    //     // bool status = flxSettings.restoreObjectFromStorage(&flux, &prefsSerial);
    //     // if (!status)
    //     //     return false;

    //     // flxLog_I_(F("Settings restored from serial..."));

    //     // // now save the new settings in primary storage
    //     // status = flxSettings.save(&flux, true);
    //     // if (status)
    //     //     flxLog_N(F("saved locally"));

    //     // return status;
    //     return true;
    // }
    //---------------------------------------------------------------------
    ///
    /// @brief Saves the current system to preferences/Settings
    ///
    /// @param theApp Pointer to the DataLogger App
    /// @retval bool indicates success (true) or failure (!true)
    ///
    bool saveSettings(sfeIoTNodeLoRaWAN *theApp)
    {
        if (!theApp)
            return false;

        // Just call save
        bool status = flxSettings.save(&flux);
        if (status)
            flxLog_I(F("Saving System Settings."));
        else
            flxLog_E(F("Error saving settings"));

        return status;
    }
    //---------------------------------------------------------------------
    ///
    /// @brief Dumps out the current heap size/stats
    ///
    /// @param theApp Pointer to the DataLogger App
    /// @retval bool indicates success (true) or failure (!true)
    ///
    bool heapStatus(sfeIoTNodeLoRaWAN *theApp)
    {

        // just dump out the current heap
        flxLog_I("System Heap - Total: %dB Free: %dB (%.1f%%)", flxPlatform::heap_size(), flxPlatform::heap_free(),
                 (float)flxPlatform::heap_free() / (float)flxPlatform::heap_size() * 100.);

        return true;
    }

    //---------------------------------------------------------------------
    ///
    /// @brief Dumps out the current logging rate metric
    ///
    /// @param theApp Pointer to the DataLogger App
    /// @retval bool indicates success (true) or failure (!true)
    ///
    bool logRateStats(sfeIoTNodeLoRaWAN *theApp)
    {
        if (!theApp)
            return false;

        // Run rate metric
        flxLog_N_(F("Logging Rate - Set Interval: %u (ms)  Measured: "), theApp->_timer.interval());
        if (!theApp->_logger.enabledLogRate())
            flxLog_N("%s", "<disabled>");
        else
            flxLog_N("%.2f (ms)", theApp->_logger.getLogRate());

        return true;
    }

    //---------------------------------------------------------------------
    ///
    /// @brief Toggles the state of current logging rate metric
    ///
    /// @param theApp Pointer to the DataLogger App
    /// @retval bool indicates success (true) or failure (!true)
    ///
    bool logRateToggle(sfeIoTNodeLoRaWAN *theApp)
    {
        if (!theApp)
            return false;

        theApp->_logger.logRateMetric = !theApp->_logger.logRateMetric();
        // Run rate metric
        flxLog_N(F("Logging Rate Metric %s"), theApp->_logger.enabledLogRate() ? "Enabled" : "Disabled");

        return true;
    }

    //---------------------------------------------------------------------
    ///
    /// @brief Dumps out the current sd card stats
    ///
    /// @param theApp Pointer to the DataLogger App
    /// @retval bool indicates success (true) or failure (!true)
    ///
    // bool sdCardStats(sfeIoTNodeLoRaWAN *theApp)
    // {
    //     if (!theApp)
    //         return false;

    //     flxLog_I(F("SD Stats - Not Implemented"));

    //     // if (theApp->_theSDCard.enabled())
    //     // {

    //     //     char szSize[32];
    //     //     char szCap[32];
    //     //     char szAvail[32];

    //     //     flx_utils::formatByteString(theApp->_theSDCard.size(), 2, szSize, sizeof(szSize));
    //     //     flx_utils::formatByteString(theApp->_theSDCard.total(), 2, szCap, sizeof(szCap));
    //     //     flx_utils::formatByteString(theApp->_theSDCard.total() - theApp->_theSDCard.used(), 2, szAvail,
    //     //                                 sizeof(szAvail));

    //     //     flxLog_I(F("SD Card - Type: %s Size: %s Capacity: %s Free: %s (%.1f%%)"), theApp->_theSDCard.type(),
    //     //     szSize,
    //     //              szCap, szAvail, 100. - (theApp->_theSDCard.used() / (float)theApp->_theSDCard.total() *
    //     100.));
    //     // }
    //     // else
    //     //     flxLog_I(F("SD card not available"));

    //     return true;
    // }

    //---------------------------------------------------------------------
    ///
    /// @brief Lists loaded devices
    ///
    /// @param theApp Pointer to the DataLogger App
    /// @retval bool indicates success (true) or failure (!true)
    ///
    bool listLoadedDevices(sfeIoTNodeLoRaWAN *theApp)
    {
        if (!theApp)
            return false;

        // connected devices...
        flxDeviceContainer myDevices = flux.connectedDevices();
        flxLog_I(F("Connected Devices [%d]:"), myDevices.size());

        // Loop over the device list - note that it is iterable.
        for (auto device : myDevices)
        {
            flxLog_N_(F("    %-20s  - %s  {"), device->name(), device->description());
            if (device->getKind() == flxDeviceKindI2C)
                flxLog_N("%s x%x}", "qwiic", device->address());
            else
                flxLog_N("%s p%u}", "SPI", device->address());
        }

        return true;
    }
    //---------------------------------------------------------------------
    ///
    /// @brief outputs current time
    ///
    /// @param theApp Pointer to the DataLogger App
    /// @retval bool indicates success (true) or failure (!true)
    ///
    bool outputSystemTime(sfeIoTNodeLoRaWAN *theApp)
    {
        if (!theApp)
            return false;

        char szBuffer[64];
        memset(szBuffer, '\0', sizeof(szBuffer));
        time_t t_now;
        time(&t_now);

        flx_utils::timestampISO8601(t_now, szBuffer, sizeof(szBuffer), true);

        flxLog_I("%s", szBuffer);

        return true;
    }

    //---------------------------------------------------------------------
    ///
    /// @brief outputs uptime
    ///
    /// @param theApp Pointer to the DataLogger App
    /// @retval bool indicates success (true) or failure (!true)
    ///
    bool outputUpTime(sfeIoTNodeLoRaWAN *theApp)
    {
        if (!theApp)
            return false;

        // uptime
        uint32_t days, hours, minutes, secs, mills;

        flx_utils::uptime(days, hours, minutes, secs, mills);

        flxLog_I("Uptime: %u days, %02u:%02u:%02u.%u", days, hours, minutes, secs, mills);

        return true;
    }
    //---------------------------------------------------------------------
    ///
    /// @brief log an observation now!
    ///
    /// @param theApp Pointer to the DataLogger App
    /// @retval bool indicates success (true) or failure (!true)
    ///
    bool logObservationNow(sfeIoTNodeLoRaWAN *theApp)
    {
        // send an event to trigger the log operation
        flxSendEvent(flxEvent::kOnLogObservation);

        return true;
    }

    //---------------------------------------------------------------------
    ///
    /// @brief output the firmware version
    ///
    /// @param theApp Pointer to the DataLogger App
    /// @retval bool indicates success (true) or failure (!true)
    ///
    bool printVersion(sfeIoTNodeLoRaWAN *theApp)
    {

        char szBuffer[128];
        flux.versionString(szBuffer, sizeof(szBuffer), true);

        flxLog_I("%s   %s", flux.name(), flux.description());
        flxLog_I("Version: %s\n\r", szBuffer);

        return true;
    }

    //---------------------------------------------------------------------
    ///
    /// @brief output the device ID
    ///
    /// @param theApp Pointer to the DataLogger App
    /// @retval bool indicates success (true) or failure (!true)
    ///
    bool printDeviceID(sfeIoTNodeLoRaWAN *theApp)
    {

        flxLog_I("Device ID: %s", flux.deviceId());

        return true;
    }
    //---------------------------------------------------------------------
    ///
    /// @brief Toggle verbose output
    ///
    /// @param theApp Pointer to the DataLogger App
    /// @retval bool indicates success (true) or failure (!true)
    ///
    bool toggleVerboseOutput(sfeIoTNodeLoRaWAN *theApp)
    {

        if (theApp)
            theApp->set_verbose(!theApp->get_verbose());
        flxLog_I("Verbose Output %s", theApp->get_verbose() ? "Enabled" : "Disabled");

        return true;
    }
    //---------------------------------------------------------------------
    ///
    /// @brief output lora status/stats
    ///
    /// @param theApp Pointer to the DataLogger App
    /// @retval bool indicates success (true) or failure (!true)
    ///
    bool loraStatus(sfeIoTNodeLoRaWAN *theApp)
    {

        if (!theApp)
            return false;
        flxLog_I("LoRaWAN Status: '%s'", theApp->_loraWANConnection.isConnected() ? "Connected" : "Disconnected");
        flxLog_I("Device EUI: %s", theApp->_loraWANConnection.deviceEUI());
        flxLog_I("Application EUI: %s", theApp->_loraWANConnection.appEUI().c_str());
        flxLog_I("Operating Class: '%s'",
                 theApp->_loraWANConnection.kLoRaWANClasses[theApp->_loraWANConnection.loraWANClass()]);
        flxLog_I("Operating Region: '%s'", theApp->_loraWANConnection.getRegionName());
        return true;
    }

    //---------------------------------------------------------------------
    // our command map - command name to callback method
    commandMap_t _commandMap = {
        {"reset-device", &sfeNLCommands::resetDevice},
        {"reset-device-forced", &sfeNLCommands::resetDeviceForced},
        {"clear-settings", &sfeNLCommands::clearDeviceSettings},
        {"clear-settings-forced", &sfeNLCommands::clearDeviceSettingsForced},
        {"restart", &sfeNLCommands::restartDevice},
        {"restart-forced", &sfeNLCommands::restartDeviceForced},
        // {"json-settings", &sfeNLCommands::loadJSONSettings},
        {"log-rate", &sfeNLCommands::logRateStats},
        {"log-rate-toggle", &sfeNLCommands::logRateToggle},
        {"log-now", &sfeNLCommands::logObservationNow},
        // {"sdcard", &sfeNLCommands::sdCardStats},
        {"devices", &sfeNLCommands::listLoadedDevices},
        {"save-settings", &sfeNLCommands::saveSettings},
        {"heap", &sfeNLCommands::heapStatus},
        {"systime", &sfeNLCommands::outputSystemTime},
        {"uptime", &sfeNLCommands::outputUpTime},
        {"verbose", &sfeNLCommands::toggleVerboseOutput},
        {"lora-status", &sfeNLCommands::loraStatus},
        {"device-id", &sfeNLCommands::printDeviceID},
        {"version", &sfeNLCommands::printVersion},
        {"about", &sfeNLCommands::aboutDevice},
        {"help", &sfeNLCommands::helpDevice},
    };

  public:
    bool processCommand(sfeIoTNodeLoRaWAN *theApp)
    {
        // The data editor we're using - serial field
        flxSerialField theDataEditor;
        std::string sBuffer;
        bool status = theDataEditor.editFieldString(sBuffer);

        flxLog_N(""); // need to output a CR

        if (!status)
            return false;

        // cleanup string
        sBuffer = flx_utils::strtrim(sBuffer);

        // Find our command
        commandMap_t::iterator it = _commandMap.find(sBuffer);
        if (it != _commandMap.end())
            status = (this->*(it->second))(theApp);
        else
        {
            flxLog_N(F("Unknown Command: `%s`"), sBuffer.c_str());
            status = false;
        }
        return status;
    }
};
