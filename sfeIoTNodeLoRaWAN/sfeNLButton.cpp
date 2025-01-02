/*
 *---------------------------------------------------------------------------------
 *
 * Copyright (c) 2024-2025, SparkFun Electronics Inc.
 *
 * SPDX-License-Identifier: MIT
 *
 *---------------------------------------------------------------------------------
 */

#include "sfeNLButton.h"
#include "sfeNLBoard.h"

// What Defines momentary press?
const uint16_t kMomentaryPress = 1001;

// Button Event Types ...
const uint8_t kEventNone = 0;
const uint8_t kEventButtonPress = 1;
const uint8_t kEventButtonRelease = 2;

// --------------------------------------------------------------------------------
// Define a simple queue for button events - if button events happen quickly, this helps
// capture the changes.

// Define a button event type  and create a queue array
const uint8_t kEventQueueSize = 4;
typedef struct buttonEvent
{
    uint32_t ticks;      // ticks when the event happened
    uint8_t buttonEvent; // event type
} buttonEvent_t;

// The queue
static buttonEvent_t buttonEventQueue[kEventQueueSize];

// used to manage the queue
static int8_t buttonEventQueueHead = 0;
static int8_t buttonEventQueueTail = -1;
static int8_t buttonEventQueueCount = 0;

// --------------------------------------------------------------------------------
/// Add an event to the queue
/// @param eventType - the event type
/// @retval true if added, false if not
///
static bool buttonEventEnqueue(uint8_t eventType)
{
    // Same event type as added last time? Just update the time
    if (buttonEventQueueTail != -1 && buttonEventQueue[buttonEventQueueTail].buttonEvent == eventType)
    {
        buttonEventQueue[buttonEventQueueTail].ticks = millis();
        return true;
    }
    // space in the queue?
    if (buttonEventQueueCount < kEventQueueSize)
    {
        buttonEventQueueTail = (buttonEventQueueTail + 1) % kEventQueueSize;
        buttonEventQueue[buttonEventQueueTail].ticks = millis();
        buttonEventQueue[buttonEventQueueTail].buttonEvent = eventType;
        buttonEventQueueCount++;
        return true;
    }
    return false;
}

// --------------------------------------------------------------------------------
/// Remove an event from the queue
/// @param event[out] - An event struct to hold the event
/// @retval true if an event was removed, false if not
///
static bool buttonEventDequeue(buttonEvent_t &event)
{

    //  Any events?
    if (buttonEventQueueCount > 0)
    {
        event = buttonEventQueue[buttonEventQueueHead];
        buttonEventQueue[buttonEventQueueHead].buttonEvent = kEventNone;
        buttonEventQueueHead = (buttonEventQueueHead + 1) % kEventQueueSize;
        buttonEventQueueCount--;
        return true;
    }
    return false;
}
//---------------------------------------------------------------------------
// for the button ISR

static void userButtonISR(void)
{
    // Add our event to our queue
    buttonEventEnqueue(digitalRead(kNLBoardUserButton) == HIGH ? kEventButtonRelease : kEventButtonPress);
}

//---------------------------------------------------------------------------
// Framework initialize call - setup the button and queue
//
bool sfeNLButton::initialize(void)
{
    // setup our event handler
    // setup the button
    pinMode(kNLBoardUserButton, INPUT_PULLUP);
    attachInterrupt(kNLBoardUserButton, userButtonISR, CHANGE);

    // job/timer for when we should check button state
    _jobCheckButton.setup("buttoncheck", 200, this, &sfeNLButton::checkButton);
    flxAddJobToQueue(_jobCheckButton);

    // initialize our button event queue
    buttonEventQueueHead = 0;
    buttonEventQueueTail = -1;
    buttonEventQueueCount = 0;
    return true;
}

//---------------------------------------------------------------------------
// Callback for the job-queue to check the button state
//
void sfeNLButton::checkButton(void)
{
    // Button events are queued up in the ISR
    if (buttonEventQueueCount > 0)
    {
        buttonEvent_t event;
        while (buttonEventDequeue(event))
        {
            // Drain all events out of the event queue
            if (event.buttonEvent == kEventButtonPress)
            {
                // press event - reset our increment counter and set the press ticks
                _currentInc = 0;
                _incEventTime = event.ticks;
                _pressEventTime = event.ticks;

                _userButtonPressed = true;

                on_buttonPressed.emit(0); // event a pressed event - increment 0
            }
            else if (event.buttonEvent == kEventButtonRelease)
            {
                // button released - if tick delta from start is small, it's a momentary press
                _userButtonPressed = false;
                if (event.ticks - _pressEventTime < kMomentaryPress)
                    on_momentaryPress.emit();
                else // else, it's a release event
                    on_buttonRelease.emit(_currentInc);
            }
        }
    }
    // Is the button pressed ?
    else if (_userButtonPressed)
    {
        // go over the increment time?
        if ((millis() - _incEventTime) / 1000 >= _pressIncrement)
        {
            _currentInc++;
            on_buttonPressed.emit(_currentInc);
            _incEventTime = millis();
        }
    }
}
