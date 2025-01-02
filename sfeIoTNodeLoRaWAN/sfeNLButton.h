/*
 *---------------------------------------------------------------------------------
 *
 * Copyright (c) 2024-2025, SparkFun Electronics Inc.
 *
 * SPDX-License-Identifier: MIT
 *
 *---------------------------------------------------------------------------------
 */
#pragma once

#include <cstdint>

#include <Flux/flxCore.h>
#include <Flux/flxCoreJobs.h>

// A class to encapsulate the event logic/handling of the on-board button of the DataLogger.
//
// Note: For button press events, the class will send out "increment" events if the button is pressed for a
// longer that "momentary" period

class sfeNLButton : public flxActionType<sfeNLButton>
{

  public:
    sfeNLButton() : _pressIncrement{5}, _userButtonPressed{false}, _pressEventTime{0}, _currentInc{0}
    {
        setName("Board Button", "Manage Button Events");
        setHidden();
    }

    bool initialize(void);

    // An Pressed event is sent after each increment value.
    void setPressIncrement(uint inc)
    {
        if (inc > 0)
            _pressIncrement = inc;
    }

    // Our events - signals ...
    flxSignalVoid on_momentaryPress;
    flxSignalUInt32 on_buttonRelease;
    flxSignalUInt32 on_buttonPressed;

  private:
    void checkButton(void);
    // How many seconds per increment on a button press
    uint32_t _pressIncrement;

    // button pressed?
    bool _userButtonPressed;

    // ticks when the button was pressed
    uint32_t _pressEventTime;

    // Ticks since last increment event
    uint32_t _incEventTime;

    // the current increment count
    uint16_t _currentInc;

    flxJob _jobCheckButton;
};