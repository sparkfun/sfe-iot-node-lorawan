
/*
 *---------------------------------------------------------------------------------
 *
 * Copyright (c) 2024-2025, SparkFun Electronics Inc.
 *
 * SPDX-License-Identifier: MIT
 *
 *---------------------------------------------------------------------------------
 */

/*
 * SparkFun IoT Node LoRaWAN - setup methods
 *
 */
#include "sfeIoTNodeLoRaWAN.h"

#include <Flux/flxDevBME280.h>
#include <Flux/flxDevENS160.h>
#include <Flux/flxDevGNSS.h>
#include <Flux/flxDevRV8803.h>
#include <Flux/flxDevSHTC3.h>

//---------------------------------------------------------------------------
// setupTime()
//
// Setup any time sources/sinks. Called after devices are loaded

bool sfeIoTNodeLoRaWAN::setupTime()
{

    // what is our clock - as setup from init/prefs
    std::string refClock = flxClock.referenceClock();

    // Any GNSS devices attached?
    auto allGNSS = flux.get<flxDevGNSS>();
    for (auto gnss : *allGNSS)
        flxClock.addReferenceClock(gnss, gnss->name());

    // RTC clock?
    auto allRTC8803 = flux.get<flxDevRV8803>();
    for (auto rtc8803 : *allRTC8803)
    {
        flxClock.addReferenceClock(rtc8803, rtc8803->name());
        flxClock.addConnectedClock(rtc8803);
    }

    // Now that clocks are loaded, set the ref clock to what was started with.
    flxClock.referenceClock = refClock;

    // update the system clock to the reference clock
    flxClock.updateClock();

    return true;
}

//---------------------------------------------------------------------------
void sfeIoTNodeLoRaWAN::setupENS160(void)
{

    // do we have one attached?
    auto ens160Devices = flux.get<flxDevENS160>();
    if (ens160Devices->size() == 0)
        return;

    flxDevENS160 *pENS160 = ens160Devices->at(0);

    auto bmeDevices = flux.get<flxDevBME280>();
    if (bmeDevices->size() > 0)
    {
        flxDevBME280 *pBME = bmeDevices->at(0);

        pENS160->setTemperatureCompParameter(pBME->temperatureC);
        pENS160->setHumidityCompParameter(pBME->humidity);

        flxLog_I(F("%s: compensation values applied from %s"), pENS160->name(), pBME->name());
        return;
    }
    // do we have a SHTC3 attached
    auto shtc3Devices = flux.get<flxDevSHTC3>();
    if (shtc3Devices->size() > 0)
    {
        flxDevSHTC3 *pSHTC3 = shtc3Devices->at(0);

        pENS160->setTemperatureCompParameter(pSHTC3->temperatureC);
        pENS160->setHumidityCompParameter(pSHTC3->humidity);

        flxLog_I(F("%s: compensation values applied from %s"), pENS160->name(), pSHTC3->name());
        return;
    }
}