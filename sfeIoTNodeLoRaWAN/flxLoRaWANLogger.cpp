/*
 *---------------------------------------------------------------------------------
 *
 * Copyright (c) 2024-2025, SparkFun Electronics Inc.
 *
 * SPDX-License-Identifier: MIT
 *
 *---------------------------------------------------------------------------------
 */

#include "flxLoRaWANLogger.h"

//---------------------------------------------------------------------------
// flxLoRaWANLogger Class - outputs data to the lorawan during a log event
//---------------------------------------------------------------------------

flxLoRaWANLogger::flxLoRaWANLogger() : _pLoRaWAN{nullptr}
{
    setName("LoRaWAN Logger", "Output data to the LoRaWAN ");

    _devicesToLog.setName("Device Objects");

    flux_add(this);
}

//----------------------------------------------------------------------------
// Called to start a log event
//
void flxLoRaWANLogger::logObservation(void)
{

    // no LoRaWAN object - nothing to do
    if (!_pLoRaWAN || !_pLoRaWAN->isConnected())
        return;

    // loop over our devices
    for (auto pDevice : _devicesToLog)
    {
        // call execute if the device needs to do anything - like get the latest value
        pDevice->execute();

        // flxLog_I("device: %s", pDevice->name());

        bool status = false;
        // now loop over the device's output parameters
        for (auto param : pDevice->getOutputParameters())
        {
            // Is this parameter enabled? Does it have a value Type set?
            if (!param->enabled() || param->valueType() == kParamValueNone)
                continue;

            // we don't process arrays. We only process scalar values
            if ((param->flags() & kParameterOutFlagArray) == kParameterOutFlagArray)
            {
                flxLog_V("Array parameters not supported by LoRaWAN driver. Parameter: %s", param->name());
                continue;
            }
            flxParameterOutScalar *pScalar = (flxParameterOutScalar *)param->accessor();

            // key off type and make the call to the

            if (flxIsLoggingVerbose())
            {
                // dump out details for the parameter. Note - for packed value, this depends on
                // verbose output from the packer.
                flxLog_V_("LoRa Packing [%s::%s]  Type: %s  Value ID: 0x%02X Value: ", pDevice->name(), param->name(),
                          flxGetTypeName(param->type()), pScalar->valueType());
            }
            // okay, we have a value type we know, lets process it and send it to the network

            switch (param->type())
            {
            case flxTypeBool:
                status = _pLoRaWAN->sendData(pScalar->valueType(), pScalar->getBool());
                break;
            case flxTypeUInt8:
                status = _pLoRaWAN->sendData(pScalar->valueType(), pScalar->getUInt8());
                break;
            case flxTypeInt8:
                status = _pLoRaWAN->sendData(pScalar->valueType(), pScalar->getInt8());
                break;
            case flxTypeUInt16:
                status = _pLoRaWAN->sendData(pScalar->valueType(), pScalar->getUInt16());
                break;
            case flxTypeInt16:
                status = _pLoRaWAN->sendData(pScalar->valueType(), pScalar->getInt16());
                break;
            case flxTypeUInt32:
                status = _pLoRaWAN->sendData(pScalar->valueType(), pScalar->getUInt32());
                break;
            case flxTypeInt32:
                status = _pLoRaWAN->sendData(pScalar->valueType(), pScalar->getInt32());
                break;
            case flxTypeFloat:
            case flxTypeDouble:
                status = _pLoRaWAN->sendData(pScalar->valueType(), pScalar->getFloat());
                break;
            default:
                break;
            }
            if (!status)
                flxLog_W(F("LoRaWAN send failed for parameter: %s"), pScalar->name());
        }
    }
    _pLoRaWAN->flushBuffer();
}
