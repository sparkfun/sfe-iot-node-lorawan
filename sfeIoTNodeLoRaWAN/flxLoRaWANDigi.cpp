/*
 *---------------------------------------------------------------------------------
 *
 * Copyright (c) 2024, SparkFun Electronics Inc.
 *
 * SPDX-License-Identifier: MIT
 *
 *---------------------------------------------------------------------------------
 */

#include "flxLoRaWANDigi.h"
#include <Flux/flxNetwork.h>
#include <Flux/flxSerial.h>
#include <Flux/flxSettings.h>

// for network data endian conversion
#include <lwip/def.h>

#include <xbee_lr.h>
#define kXBeeLRSerial Serial1
#define kXBeeLRBaud 9600

// Define a connection iteration value - exceed this, skip the connection

#define kMaxConnectionTries 3

// How often is the connection status checked ... in MS
const uint16_t kLoRaWANUpdateHandlerTimeMS = 15000;

// Define times (in ms) for the reconnected job
const uint32_t kReconnectInitialTime = 30000;
// 1 hour
const uint32_t kReconnectMaxTime = 3600000;
//----------------------------------------------------------------
// Callbacks for the XBee LR module - these are static functions
//
// TODO - Relay this call back into the object/system and move to SDK API/logic
//
static void OnReceiveCallback(void *data)
{
    XBeeLRPacket_t *packet = (XBeeLRPacket_t *)data;
    Serial.print("Received Packet: ");
    for (int i = 0; i < packet->payloadSize; i++)
    {
        Serial.print("0x");
        if (packet->payload[i] < 0x10)
        {
            Serial.print("0");
        }
        Serial.print(packet->payload[i], HEX);
        Serial.print(" ");
    }
    Serial.println();
    Serial.print("Ack: ");
    Serial.println(packet->ack);
    Serial.print("Port: ");
    Serial.println(packet->port);
    Serial.print("RSSI: ");
    Serial.println(packet->rssi);
    Serial.print("SNR: ");
    Serial.println(packet->snr);
    Serial.print("Downlink Counter: ");
    Serial.println(packet->counter);
}

static void OnSendCallback(void *data)
{
    XBeeLRPacket_t *packet = (XBeeLRPacket_t *)data;
    flxSendEvent(flxEvent::kLoRaWANSendStatus, packet->status == 0);

    if (packet->status != 0)
    {
        flxLog_W_(F("%s: Data send failed. Frame ID: 0X%x Reason: "), packet->frameId);
        switch (packet->status)
        {
        case 0x01:
            flxLog_N(F("ACK Failed"));
            break;
        case 0x022:
            flxLog_N(F("Not Connected)"));
            break;
        default:
            flxLog_N_(F("code 0x%X"), packet->status);
            break;
        }
    }
}
//----------------------------------------------------------------
// Enabled Property setter/getters
void flxLoRaWANDigi::set_isEnabled(bool bEnabled)
{
    // Any changes?
    if (_isEnabled == bEnabled)
        return;

    // changing state ...
    // TODO - Should this control connection state?

    _isEnabled = bEnabled;

    if (_isEnabled)
    {
        // Try to connect ..
        if (!connect())
        {
            // failed to connect - set the reconnect job
            _reconnectJob.setPeriod(kReconnectInitialTime);
            flxAddJobToQueue(_reconnectJob);
        }
    }
    else
        disconnect();
}

//----------------------------------------------------------------
bool flxLoRaWANDigi::get_isEnabled(void)
{
    return _isEnabled;
}

//----------------------------------------------------------------
// Config the settings on the module. These settings are persistent, so only need to set once.

bool flxLoRaWANDigi::configureModule(void)
{
    if (!_isEnabled || _pXBeeLR == nullptr)
        return false;

    // Now initialize the module

    flxLog_I("Setting App EUI: %s", appEUI().c_str());
    // App EUI
    if (appEUI().size() == 0)
    {
        flxLog_E(F("%s: No App EUI provided"), name());
        return false;
    }
    if (!_pXBeeLR->setLoRaWANAppEUI(appEUI().c_str()))
    {
        flxLog_E(F("%s: Failed to set the App EUI"), name());
        // return false;
    }
    flxLog_N_(F("."));

    flxLog_I("Setting App Key: %s", appKey().c_str());
    // App Key
    if (appKey().size() == 0)
    {
        flxLog_E(F("%s: No App Key provided"), name());
        return false;
    }
    flxLog_I(F("Setting App Key: %s"), appKey().c_str());
    if (!_pXBeeLR->setLoRaWANAppKey(appKey().c_str()))
    {
        flxLog_E(F("%s: Failed to set the App Key"), name());
        // return false;
    }
    flxLog_N_(F("."));

    // Network Key
    flxLog_I("Setting Network Key: %s", networkKey().c_str());
    if (networkKey().size() == 0)
    {
        flxLog_E(F("%s: No Network Key provided"), name());
        return false;
    }
    flxLog_I(F("Setting Network Key: %s"), networkKey().c_str());
    if (!_pXBeeLR->setLoRaWANNwkKey(networkKey().c_str()))
    {
        flxLog_E(F("%s: Failed to set the Network Key"), name());
        // return false;
    }
    flxLog_N_(F("."));

    // This is probably read only - so just try to set ..
    if (!_pXBeeLR->setLoRaWANClass('C'))
        flxLog_D(F("%s: Failed to set the LoRaWAN Class"), name());

    if (!_pXBeeLR->setApiOptions(0x01))
        flxLog_W_(F("%s: Failed to set the API Options"), name());

    flxLog_N_(F("."));

    if (!_pXBeeLR->writeConfig())
    {
        flxLog_E(F("%s: Failed to write the module configuration"), name());
        // return false;
    }

    flxLog_N_(F("."));
    if (!_pXBeeLR->applyChanges())
    {
        flxLog_E(F("%s: Failed to apply the module configuration"), name());
        // return false;
    }
    flxLog_N_(F("."));

    // At this point, the changes are set in the LoRa module
    return true;
}
//----------------------------------------------------------------
// Setup the LoRaWAN Object

bool flxLoRaWANDigi::setupModule(void)
{
    // If we have a module, reset it
    if (_pXBeeLR)
        _pXBeeLR->reset();
    else
    {
        // Create the module
        _pXBeeLR = new XBeeArduino(&kXBeeLRSerial, kXBeeLRBaud, XBEE_LORA, OnReceiveCallback, OnSendCallback);

        if (!_pXBeeLR)
        {
            flxLog_E(F("%s: Failed to create the XBee LR module"), name());
            return false;
        }
    }
    flxLog_N_(F("."));

    // Initialize the module
    if (!_pXBeeLR->begin())
    {
        flxLog_E_(F("%s: Failed to initialize the XBee LR module"), name());
        return false;
    }
    // This might take a few tries ...
    bool status = false;
    for (int i = 0; i < 3; i++)
    {
        flxLog_N_(F("."));
        if (_pXBeeLR->getLoRaWANDevEUI((uint8_t *)_devEUI, sizeof(_devEUI)))
        {
            status = true;
            break;
        }
        delay(100);
    }
    if (!status)
    {
        flxLog_E_(F("%s: The XBee LR module failed to initialize"), name());
        return false;
    }

    // Do we need to configure the module?
    if (_moduleConfigured() == false)
    {
        if (!configureModule())
            return false;
        _moduleConfigured = true;
        // our setting for the module config has changed - save it
        flxSettings.save(this, true);
    }
    _moduleInitialized = true;

    return true;
}
//----------------------------------------------------------------
// Connection Management

bool flxLoRaWANDigi::connect(void)
{

    if (!_isEnabled)
        return false;

    // are we already connected?
    if (isConnected())
        return true;

    flxLog_I_(F(" Connecting to the LoRaWAN."));

    if (!_moduleInitialized)
    {
        if (!setupModule())
            return false;
    }

    // This might take a few tries ...
    bool status = false;
    for (int i = 0; i < kMaxConnectionTries; i++)
    {
        flxLog_N_(F("."));
        if (_pXBeeLR->connect())
        {
            status = true;
            break;
        }
        delay(200);
    }

    // Let's Connect!
    if (!status)
    {
        flxSerial.textToRed();
        flxLog_N(F("Failed!"));
        flxSerial.textToNormal();

        return false;
    }

    flxSerial.textToGreen();
    flxLog_N(F("Connected!"));
    flxSerial.textToNormal();

    // okay, we're connected.
    _wasConnected = true;
    flxSendEvent(flxEvent::kOnConnectionChange, true);

    // add the connection monitor job to the system
    _connectionJob.setPeriod(kLoRaWANUpdateHandlerTimeMS);
    flxAddJobToQueue(_connectionJob);
    return true;
}

//----------------------------------------------------------------
void flxLoRaWANDigi::disconnect(void)
{
    if (!_pXBeeLR)
        return;

    if (!_pXBeeLR->disconnect())
        flxLog_E(F("LoRaWAN disconnect() - error disconnecting"));

    if (_wasConnected)
        flxSendEvent(flxEvent::kOnConnectionChange, false);

    _wasConnected = false;
    flxRemoveJobFromQueue(_connectionJob);
}

//----------------------------------------------------------------
///
/// @brief Returns true of the LoRaWAN module is connected
/// @return [true] on connection, [false] otherwise
///
bool flxLoRaWANDigi::isConnected()
{
    // We cache the connection status - so we don't have to check the module, which is *slow*.
    //
    // The connection status is checked ever kLoRaWANUpdateHandlerTimeMS via a job

    return _isEnabled && _pXBeeLR != nullptr && _wasConnected;
}

//----------------------------------------------------------------
bool flxLoRaWANDigi::initialize(void)
{

    // First register all the properties and
    flxRegister(enabled, "Enabled", "Enable or Disable the LoRaWAN Network connection");

    flxRegister(appEUI, "Application EUI", "The LoRaWAN Application EUI");
    flxRegister(appKey, "Application Key", "The LoRaWAN Application Key");
    flxRegister(networkKey, "Network Key", "The LoRaWAN Network Key");

    // our hidden module initialized property
    flxRegister(_moduleConfigured, "mod-config");

    // Register the reset module function/param
    flxRegister(resetModule, "Reset", "Reset the LoRaWAN module");

    // Setup the LoRaWAN job - called when active to monitor connect/disconnect

    _connectionJob.setup("Digi LoRaWAN Connection", kLoRaWANUpdateHandlerTimeMS, this,
                         &flxLoRaWANDigi::connectionStatusCB);

    // our reconnect job
    _reconnectJob.setup("Digi LoRaWAN Reconnect", kReconnectInitialTime, this, &flxLoRaWANDigi::reconnectJobCB);

    // TODO - Fix in the future - before launch
    // ! - These are hard coded for now - fix in the future
    // appEUI = "37D56A3F6CDCF0A5";
    // appKey = "CD32AAB41C54175E9060D86F3A8B7F48";
    // networkKey = "CD32AAB41C54175E9060D86F3A8B7F48";

    // TODO <<END>>
    // Do we connect now?

    // is it desired to delay the startup/connect call?
    if (_delayedStartup)
    {
        _delayedStartup = false;
        return true;
    }
    return connect();
}

//----------------------------------------------------------------
// Reset the module...

void flxLoRaWANDigi::reset_module(void)
{
    flxLog_I_(F("Resetting the LoRaWAN module.."));

    // First, set our pref and persist it
    _moduleConfigured = false;
    flxSettings.save(this, true);

    // Disconnect
    disconnect();

    // setup the module - this process will reconfigure/reset the module
    if (!setupModule())
    {
        flxSerial.textToRed();
        flxLog_N(F("failed"));
        flxSerial.textToNormal();
        return;
    }
    flxSerial.textToGreen();
    flxLog_N(F("success"));
    flxSerial.textToNormal();

    // Is the device enabled/up - call connected?
    if (_isEnabled)
        connect();
}
//----------------------------------------------------------------
bool flxLoRaWANDigi::sendPayload(const uint8_t *payload, size_t len)
{
    if (payload == nullptr || len == 0 || _pXBeeLR == nullptr)
        return false;

    if (!isConnected())
        return false;

    // Okay, build our packet

    flxLog_I_(F("Sending packet: 0x"));
    for (int i = 0; i < len; i++)
    {
        flxLog_N_(F("%02X"), payload[i]);
    }
    flxLog_N("");

    XBeeLRPacket_t packet;
    packet.payload = (uint8_t *)payload;
    packet.payloadSize = len;
    packet.port = 2;
    packet.ack = 0;

    return _pXBeeLR->sendData(packet);
}
//----------------------------------------------------------------
void flxLoRaWANDigi::connectionStatusCB(void)
{
    // Connection change???
    if (!_isEnabled || _pXBeeLR == nullptr)
        return;

    // check the connection status of the module
    bool isConn = _pXBeeLR->isConnected();

    // flxLog_I(F("Connection Status: %s"), isConn ? "Connected" : "Disconnected");

    // changed state?
    if (isConn != _wasConnected)
    {
        _wasConnected = isConn;
        flxSendEvent(flxEvent::kOnConnectionChange, _wasConnected);
    }
    // If we lost connection -  crank up reconnect mode
    if (!_wasConnected)
    {
        // if we lost connection, but are still enabled -- which we are at this point --
        // kick off the reconnect job
        _reconnectJob.setPeriod(kReconnectInitialTime);
        flxAddJobToQueue(_reconnectJob);

        // remove the connection check job -- will be relaunched when we reconnect
        flxRemoveJobFromQueue(_connectionJob);
    }
}

//----------------------------------------------------------------
void flxLoRaWANDigi::reconnectJobCB(void)
{
    // If we're enabled, and not connected, try to connect
    if (isConnected())
    {
        // We have reconnected, so remove this job
        flxRemoveJobFromQueue(_reconnectJob);
        return;
    }

    // We are not connected, but want to be - Try to connect
    if (connect())
    {
        // We have reconnected, so remove this job
        flxRemoveJobFromQueue(_reconnectJob);
        return;
    }

    // We failed to connect -  - do we need to adjust the time on this job?

    uint32_t currentPeriod = _reconnectJob.period();
    if (currentPeriod < kReconnectMaxTime)
    {
        // double delay (simple backoff strategy)
        currentPeriod *= 2;
        if (currentPeriod > kReconnectMaxTime)
            currentPeriod = kReconnectMaxTime;
        _reconnectJob.setPeriod(currentPeriod);
        flxUpdateJobInQueue(_reconnectJob);
    }
}

//----------------------------------------------------------------
// startReconnectMode()
//
//
void flxLoRaWANDigi::startReconnectMode(void)
{
    // if we are enabled, start the reconnect job
    if (_isEnabled)
    {
        _reconnectJob.setPeriod(kReconnectInitialTime);
        flxAddJobToQueue(_reconnectJob);
    }
}
//----------------------------------------------------------------

// data / packet transmission things
//----------------------------------------------------------------
// Init the package

bool flxLoRaWANDigi::checkBuffer(size_t len)
{
    // Make sure we have room for the data of the given size
    if (!_isEnabled || _pXBeeLR == nullptr || len == 0 || len > kLoRaBufferLen || !isConnected())
        return false;

    if (len > kLoRaBufferLen)
    {
        flxLog_E(F("LoRaWAN: Buffer overflow. Data size (%d) > packet size (%d)"), len, kLoRaBufferLen);
        return false;
    }

    if (_currentOffset + len > kLoRaBufferLen)
    {
        // send the packet
        if (!sendPayload(_packetBuffer, kLoRaBufferLen))
            flxLog_E(F("LoRaWAN: Error sending packet")); // keep on trucking
        // restart the packet
        _currentOffset = 0;
        memset(_packetBuffer, 0, kLoRaBufferLen);
    }
    return true;
}

// methods to send data to the LoRaWAN module - based on data size.
//------------------------------------------------------------------------------------------
// bool
bool flxLoRaWANDigi::sendData(uint8_t tag, bool data)
{
    uint8_t tmp = data ? 1 : 0;
    return sendData(tag, tmp);
}
//------------------------------------------------------------------------------------------
// uint8_t
bool flxLoRaWANDigi::sendData(uint8_t tag, uint8_t data)
{
    return sendData(tag, (uint8_t *)&data, sizeof(data));
}
//------------------------------------------------------------------------------------------
// int8_t
bool flxLoRaWANDigi::sendData(uint8_t tag, int8_t data)
{
    return sendData(tag, *(uint8_t *)&data);
}
//------------------------------------------------------------------------------------------
// uint16_t
bool flxLoRaWANDigi::sendData(uint8_t tag, uint16_t data)
{
    uint16_t data16 = htons(data);
    return sendData(tag, (uint8_t *)&data16, sizeof(data));
}
//------------------------------------------------------------------------------------------
// int16_t
bool flxLoRaWANDigi::sendData(uint8_t tag, int16_t data)
{
    return sendData(tag, *(uint16_t *)&data);
}
//------------------------------------------------------------------------------------------
// uint32_t
bool flxLoRaWANDigi::sendData(uint8_t tag, uint32_t data)
{
    uint32_t data32 = htonl(data);
    return sendData(tag, (uint8_t *)&data32, sizeof(data));
}
//------------------------------------------------------------------------------------------
// int32_t
bool flxLoRaWANDigi::sendData(uint8_t tag, int32_t data)
{
    return sendData(tag, *(uint32_t *)&data);
}
//------------------------------------------------------------------------------------------
// float
bool flxLoRaWANDigi::sendData(uint8_t tag, float data)
{
    // just send as a uint32_t
    return sendData(tag, *(uint32_t *)&data);
}
//------------------------------------------------------------------------------------------
// float[2] array
bool flxLoRaWANDigi::sendData(uint8_t tag, float data[2])
{
    uint32_t data32[2];
    data32[0] = htonl(*(uint32_t *)&data[0]);
    data32[1] = htonl(*(uint32_t *)&data[1]);

    // just send as a uint32_t
    return sendData(tag, (uint8_t *)data32, sizeof(data32));
}
//------------------------------------------------------------------------------------------
// data array/list
bool flxLoRaWANDigi::sendData(uint8_t tag, const uint8_t *data, size_t len)
{
    if (!checkBuffer(len + 1))
        return false;

    _packetBuffer[_currentOffset++] = tag;
    memcpy(_packetBuffer + _currentOffset, data, len);

    _currentOffset += len;
    return true;
}

//------------------------------------------------------------------------------------------
bool flxLoRaWANDigi::flushBuffer(void)
{
    // just check the buffer with max size requested
    return checkBuffer(kLoRaBufferLen);
}