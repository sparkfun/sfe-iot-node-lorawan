
/*
 *---------------------------------------------------------------------------------
 *
 * Copyright (c) 2024, SparkFun Electronics Inc.
 *
 * SPDX-License-Identifier: MIT
 *
 *---------------------------------------------------------------------------------
 */
#include <Arduino.h>

#include "sfeIoTNodeLoRaWAN.h"
#include "sfeNLLed.h"

// our LoRaWAN application

sfeIoTNodeLoRaWAN theNodeLoRaWAN;

void setup()
{

    // Startup the framework
    flux.setApplication(theNodeLoRaWAN);
    flux.start();
}

void loop()
{
    // Run the framework
    if (flux.loop())
        sfeLED.flash(sfeLED.Blue);

    delay(10);
}