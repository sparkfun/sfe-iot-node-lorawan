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

// Header file for the Digi LoRaWAN communication connection - works with the XBee LR module

#include <Flux/flxCoreJobs.h>
#include <Flux/flxFlux.h>

#include <XBeeArduino.h>

// Setup an event
flxDefineEventID(kLoRaWANSendStatus);
// Event for received messages
flxDefineEventID(kLoRaWANReceivedMessage);

// Our Development App EUI
#define kDevelopmentAppEUI "37D56A3F6CDCF0A5"

class flxLoRaWANDigi : public flxActionType<flxLoRaWANDigi>
{
  private:
    void set_isEnabled(bool bEnabled);
    bool get_isEnabled(void);

    void reset_module(void);

    void set_app_eui(std::string appEUI);
    std::string get_app_eui(void);
    std::string _app_eui;

    void set_app_key(std::string appKey);
    std::string get_app_key(void);

    std::string _app_key;

    void set_network_key(std::string networkKey);
    std::string get_network_key(void);
    std::string _network_key;

    void update_system_config(void);

    void set_lora_class(uint8_t);
    uint8_t get_lora_class(void);
    uint8_t _lora_class;

    void set_lora_region(uint8_t);
    uint8_t get_lora_region(void);
    uint8_t _lora_region;

  public:
    // ctor
    flxLoRaWANDigi()
        : _app_key{""}, _network_key{""}, _lora_class{2}, _lora_region{kLoRaWANRegionIDs[0]}, _wasConnected{false},
          _isEnabled{true}, _delayedStartup{false}, _moduleInitialized{false}, _pXBeeLR{nullptr}, _devEUI{'\0'},
          _currentOffset{0}
    {
        setName("LoRaWAN Network", "Digi LoRaWAN connection for the system");
        flux_add(this);

        // default app EUI
#if defined(FLX_SPARKFUN_LORAWAN_APP_EUI)
        _app_eui = FLX_SPARKFUN_LORAWAN_APP_EUI;
#else
        _app_eui = kDevelopmentAppEUI;
#endif
    }

    // dtor
    ~flxLoRaWANDigi()
    {
        if (_pXBeeLR)
        {
            _pXBeeLR->disconnect();
            delete _pXBeeLR;
            _pXBeeLR = nullptr;
        }
    }

    // Properties
    flxPropertyRWString<flxLoRaWANDigi, &flxLoRaWANDigi::get_app_eui, &flxLoRaWANDigi::set_app_eui> appEUI;
    flxPropertyRWSecureString<flxLoRaWANDigi, &flxLoRaWANDigi::get_app_key, &flxLoRaWANDigi::set_app_key> appKey;
    flxPropertyRWSecureString<flxLoRaWANDigi, &flxLoRaWANDigi::get_network_key, &flxLoRaWANDigi::set_network_key>
        networkKey;

    flxPropertyRWBool<flxLoRaWANDigi, &flxLoRaWANDigi::get_isEnabled, &flxLoRaWANDigi::set_isEnabled> enabled;

    flxPropertyRWUInt8<flxLoRaWANDigi, &flxLoRaWANDigi::get_lora_class, &flxLoRaWANDigi::set_lora_class> loraWANClass =
        {2, {{kLoRaWANClasses[0], 0}, {kLoRaWANClasses[1], 1}, {kLoRaWANClasses[2], 2}}};

    flxPropertyRWUInt8<flxLoRaWANDigi, &flxLoRaWANDigi::get_lora_region, &flxLoRaWANDigi::set_lora_region>
        loraWANRegion = {
            kLoRaWANRegionIDs[0],
            {{kLoRaWANRegionNames[0], kLoRaWANRegionIDs[0]}, {kLoRaWANRegionNames[1], kLoRaWANRegionIDs[1]}}};

    flxPropertyHiddenBool<flxLoRaWANDigi> _moduleConfigured = {false};

    // input params/functions
    flxParameterInVoid<flxLoRaWANDigi, &flxLoRaWANDigi::reset_module> resetModule;

    bool connect(void);
    void disconnect(void);

    bool isConnected();

    bool initialize(void);

    void startReconnectMode(void);

    // used to prevent auto connect when init called - rare case
    void setDelayedStartup(bool bDelay = true)
    {
        _delayedStartup = bDelay;
    }

    const char *deviceEUI(void)
    {
        return _devEUI;
    }

    // methods to send data to the LoRaWAN module - based on data size.
    bool sendData(uint8_t tag, bool data);
    bool sendData(uint8_t tag, int8_t data);
    bool sendData(uint8_t tag, int16_t data);
    bool sendData(uint8_t tag, int32_t data);
    bool sendData(uint8_t tag, uint8_t data);
    bool sendData(uint8_t tag, uint16_t data);
    bool sendData(uint8_t tag, uint32_t data);
    bool sendData(uint8_t tag, float data);
    bool sendData(uint8_t tag, float data[2]);
    bool sendData(uint8_t tag, const uint8_t *data, size_t len);
    bool flushBuffer(void);

    static constexpr const char *kLoRaWANClasses[3] = {"A", "B", "C"};
    static constexpr const char *kLoRaWANRegionNames[2] = {"US915", "EU868"};
    static constexpr uint8_t kLoRaWANRegionIDs[2] = {8, 5};

    const char *getRegionName(void);

  private:
    void connectionStatusCB(void);
    bool setupModule(void);
    bool configureModule(void);
    void reconnectJobCB(void);
    void processMessagesCB(void);
    bool setupLoRaWANClass(void);

    // send our payload buffer
    bool sendPayload(const uint8_t *payload, size_t len);

    bool checkBuffer(size_t len);

    static constexpr uint8_t kLoRaBufferLen = 11;

    // flag used to help with connection changes.
    bool _wasConnected;
    bool _isEnabled;
    bool _delayedStartup;

    bool _moduleInitialized;

    // our job for the connection status
    flxJob _connectionJob;

    // our job for reconnection
    flxJob _reconnectJob;

    // The XBee processing messages (incoming) job
    flxJob _processJob;

    // Our XBee object for the LoRaWAN module
    XBeeArduino *_pXBeeLR;

    char _devEUI[18];

    // for data transmission
    uint16_t _currentOffset; // offset into the current packet
    uint8_t _packetBuffer[kLoRaBufferLen];
};
