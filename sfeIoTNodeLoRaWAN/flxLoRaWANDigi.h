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

class flxLoRaWANDigi : public flxActionType<flxLoRaWANDigi>
{
  private:
    void set_isEnabled(bool bEnabled);
    bool get_isEnabled(void);

    void reset_module(void);

  public:
    // ctor
    flxLoRaWANDigi()
        : _wasConnected{false}, _isEnabled{true}, _delayedStartup{false}, _moduleInitialized{false}, _pXBeeLR{nullptr},
          _devEUI{'\0'}, _currentOffset{0}
    {
        setName("LoRaWAN Network", "Digi LoRaWAN connection for the system");
        flux_add(this);
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
    flxPropertyString<flxLoRaWANDigi> appEUI;
    flxPropertySecureString<flxLoRaWANDigi> appKey;
    flxPropertySecureString<flxLoRaWANDigi> networkKey;

    flxPropertyRWBool<flxLoRaWANDigi, &flxLoRaWANDigi::get_isEnabled, &flxLoRaWANDigi::set_isEnabled> enabled;
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

  private:
    void connectionStatusCB(void);
    bool setupModule(void);
    bool configureModule(void);
    void reconnectJobCB(void);
    void processMessagesCB(void);

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
