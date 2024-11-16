
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
    // put your setup code here, to run once:

    // delay(2000);
    // Serial.begin(115200);

    // Startup the framework
    flux.setApplication(theNodeLoRaWAN);
    flux.start();
}

void loop()
{

    // Serial.printf("Loop - testing\n\r");
    if (flux.loop())
        sfeLED.flash(sfeLED.Blue);

    delay(100);
}