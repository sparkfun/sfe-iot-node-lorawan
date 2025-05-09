
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

#include <LittleFS.h>

// on board flash file system bounds

extern uint8_t _FS_start;
extern uint8_t _FS_end;

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

//---------------------------------------------------------------------------
// setupSDCard()
//
// Set's up the SD card subsystem and the objects/systems that use it.
bool sfeIoTNodeLoRaWAN::setupSDCard(void)
{

    // setup output to the SD card
    if (_theSDCard.initialize())
    {

        _theOutputFile.setName("Data File", "Output file rotation manager");

        // SD card is available - lets setup output for it
        // Add the filesystem to the file output/rotation object
        _theOutputFile.setFileSystem(_theSDCard);

        // setup our file rotation parameters
        _theOutputFile.filePrefix = "sfe";
        _theOutputFile.startNumber = 1;
        _theOutputFile.rotatePeriod(24); // one day

        // add the file output to the CSV output.
        //_fmtCSV.add(_theOutputFile);

        // have the CSV format driver listen to the new file event. This
        // will cause a header to be written next cycle.
        flxRegisterEventCB(flxEvent::kOnNewFile, &_fmtCSV, &flxFormatCSV::output_header);

        return true;
    }
    return false;
}
//---------------------------------------------------------------------------
// checkOnBoardFS()
//
// Do we have an onboard FS?

bool sfeIoTNodeLoRaWAN::checkOnBoardFS(void)
{

    // Was a filesystem set for the on-board flash?
    if (&_FS_end - &_FS_start <= 0)
    {
        flxLog_W(F("No onboard flash file system detected"));
        return false;
    }
    // Startup little fs
    if (LittleFS.begin() == false)
    {
        flxLog_W(F("Unable to mount flash file system"));
        return false;
    }
    return true;
}