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
#include <Flux/flxDeviceValueTypes.h>

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
                flxParameterOutArray *pArray = (flxParameterOutArray *)param->accessor();
                // look for known values - if not, skip
                switch (pArray->valueType())
                {
                case kParamValueLocation: {

                    flxDataArrayFloat *parrData = (flxDataArrayFloat *)pArray->get();

                    // is the data array sane?
                    if (parrData->size() != 2)
                    {
                        flxLog_W("Location array size is not 2. Size: %d", parrData->size());
                        continue;
                    }

                    // location is a two element float array, that we want to send in the same packet.
                    // So first, flush the buffer, then send the data
                    _pLoRaWAN->flushBuffer();

                    // trick the system into thinking we have a float array, which the LoRaWAN driver
                    // can handle. We need to do this because the LoRaWAN driver is expecting a float array[2]
                    float *pfData = parrData->get();
                    float tmparr[2] = {pfData[0], pfData[1]};

                    if (flxIsLoggingVerbose())
                    {
                        // dump out details for the parameter. Note - for packed value, this depends on
                        // verbose output from the packer.
                        flxLog_V_("LoRa Packing [%s::%s]  Type: Location  Value ID: 0x%02X Value: (%f,%f)",
                                  pDevice->name(), param->name(), pArray->valueType(), tmparr[0], tmparr[1]);
                    }

                    // send the data
                    status = _pLoRaWAN->sendData(pArray->valueType(), tmparr);
                    break;
                }
                default:
                    flxLog_V("Array parameters not supported by LoRaWAN driver. Parameter: %s", param->name());
                    break;
                }
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
