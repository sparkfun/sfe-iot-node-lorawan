/*
 *---------------------------------------------------------------------------------
 *
 * Copyright (c) 2024, SparkFun Electronics Inc.
 *
 * SPDX-License-Identifier: MIT
 *
 *---------------------------------------------------------------------------------
 */

/*
 *---------------------------------------------------------------------------------
 *
 *---------------------------------------------------------------------------------
 */

#include "flxLoRaWANLogger.h"

//---------------------------------------------------------------------------
// flxLoRaWANLogger Class
//---------------------------------------------------------------------------

flxLoRaWANLogger::flxLoRaWANLogger() : _pLoRaWAN{nullptr}
{
    setName("LoRaWAN Logger", "Output data to the LoRaWAN ");

    _devicesToLog.setName("Device Objects");

    flux_add(this);
}

//----------------------------------------------------------------------------
void flxLoRaWANLogger::logObservation(void)
{

    // no LoRaWAN object - nothing to do
    if (!_pLoRaWAN)
        return;

    // loop over our devices
    for (auto pDevice : _devicesToLog)
    {
        // call execute if the device needs to do anything - like get the latest value
        pDevice->execute();

        // flxLog_I("device: %s", pDevice->name());

        // Do we know about this device  - if so call it's handler
        // TODO - this is a placeholder - we need to add a handler to the device

        // now loop over the device's output parameters - if we know the type, call it's handler
        for (auto param : pDevice->getOutputParameters())
        {
            // Is this parameter enabled? Does it have a value Type set?
            if (!param->enabled() || param->valueType() == kParamValueNone)
                continue;

            // we don't process arrays. We only process scalar values
            if ((param->flags() & kParameterOutFlagArray) == kParameterOutFlagArray)
            {
                // flxLog_W("Array type for parameter: %s", param->name());
                continue;
            }
            flxParameterOutScalar *pScalar = (flxParameterOutScalar *)param->accessor();

            // key off type and make the call to the

            // okay, we have a value type we know, lets process it and send it to the network
            // TODO -- there has to be a cleaner way to do this ....
            switch (param->type())
            {
            case flxTypeBool:
                if (!_pLoRaWAN->sendData(pScalar->valueType(), pScalar->getBool()))
                    flxLog_W(F("LoRaWAN send failed for parameter: %s"), pScalar->name());
                break;
            case flxTypeUInt8:
                if (!_pLoRaWAN->sendData(pScalar->valueType(), pScalar->getUInt8()))
                    flxLog_W(F("LoRaWAN send failed for parameter: %s"), pScalar->name());
                break;
            case flxTypeInt8:
                if (!_pLoRaWAN->sendData(pScalar->valueType(), pScalar->getInt8()))
                    flxLog_W(F("LoRaWAN send failed for parameter: %s"), pScalar->name());
                break;

            case flxTypeUInt16:
                if (!_pLoRaWAN->sendData(pScalar->valueType(), pScalar->getUInt16()))
                    flxLog_W(F("LoRaWAN send failed for parameter: %s"), pScalar->name());
                break;
            case flxTypeInt16:
                if (!_pLoRaWAN->sendData(pScalar->valueType(), pScalar->getInt16()))
                    flxLog_W(F("LoRaWAN send failed for parameter: %s"), pScalar->name());
                break;
            case flxTypeUInt32:
                if (!_pLoRaWAN->sendData(pScalar->valueType(), pScalar->getUInt32()))
                    flxLog_W(F("LoRaWAN send failed for parameter: %s"), pScalar->name());
                break;
            case flxTypeInt32:
                if (!_pLoRaWAN->sendData(pScalar->valueType(), pScalar->getInt32()))
                    flxLog_W(F("LoRaWAN send failed for parameter: %s"), pScalar->name());
                break;
            case flxTypeFloat:
            case flxTypeDouble:
                if (!_pLoRaWAN->sendData(pScalar->valueType(), pScalar->getFloat()))
                    flxLog_W(F("LoRaWAN send failed for parameter: %s"), pScalar->name());
                break;
            default:
                break;
            }
            // TODO - look for a handler for this parameter
        }
    }
    _pLoRaWAN->flushBuffer();
}
