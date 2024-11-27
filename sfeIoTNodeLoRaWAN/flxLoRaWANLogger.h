/*
 *---------------------------------------------------------------------------------
 *
 * Copyright (c) 2024, SparkFun Electronics Inc.
 *
 * SPDX-License-Identifier: MIT
 *
 *---------------------------------------------------------------------------------
 */

#pragma once

// #include <ArduinoJson.h>
#include <initializer_list>
#include <vector>

#include "flxLoRaWANDigi.h"
#include <Flux/flxCore.h>

// Define the Logging class
class flxLoRaWANLogger : public flxActionType<flxLoRaWANLogger>
{
  public:
    flxLoRaWANLogger();

    //----------------------------------------------------------------------------
    void logObservation(void);

    // Used to register the event we want to listen to, which will trigger this
    // activity.
    void listen(flxSignalVoid &theEvent)
    {

        // register the logObservation() method on this instance. When an event
        // is triggered, the logObservation method is called
        theEvent.call(this, &flxLoRaWANLogger::logObservation);
    }

    // Used some template magic to support all event types.
    // Need:
    //   - a callback that takes an arg from the signal - any type
    template <typename T> void logObservation(T value)
    {
        // Trigger a log event
        logObservation();
    }
    // And a listen that can take any event type and wire up the callback.
    //
    // Note: Using the defined parameter type of the signal to drive the
    // logObservation template.
    template <typename T> void listen(T &theEvent)
    {
        theEvent.call(this, &flxLoRaWANLogger::logObservation<typename T::value_type>);
    }

    // Add routines with var args. Allows any combo of writer, param or spBase
    // to be added in one method call to this object.
    //
    // Could template first arg, but wanted some upfront type checking.
    //
    // Note Args&& for pass args by ref. Otherwise, copy constructor is used.

    template <typename... Args> void add(flxDevice &a1, Args &&...args)
    {
        va_add(a1, args...);
    }

    template <typename... Args> void add(flxDevice *a1, Args &&...args)
    {
        va_add(a1, args...);
    }

    // small set of removes - for operations

    template <typename... Args> void remove(flxDevice &a1, Args &&...args)
    {
        va_remove(a1, args...);
    }

    template <typename... Args> void remove(flxDevice *a1, Args &&...args)
    {
        va_remove(a1, args...);
    }
    void setLoRaWAN(flxLoRaWANDigi &pLoRaWAN)
    {
        setLoRaWAN(&pLoRaWAN);
    }
    void setLoRaWAN(flxLoRaWANDigi *pLoRaWAN)
    {
        _pLoRaWAN = pLoRaWAN;
    }

  private:
    // The things we're logging
    flxDeviceContainer _devicesToLog;

    // vargs management - how to add things recursively.
    //
    // General pattern for the below methods:
    //	   	- Strip off the first item in the list, based on type
    //      - call internal _add() method - which is typed
    //		- Recurse on remaining args
    //      - when arg list is empty, it calls the noop va_add() method.
    //
    void va_add()
    {
    }

    template <typename T, typename... Args> void va_add(T *a1, Args &&...args)
    {
        _add(a1);
        va_add(args...);
    }
    template <typename T, typename... Args> void va_add(T &a1, Args &&...args)
    {
        _add(a1);
        va_add(args...);
    }

    // removes
    void va_remove()
    {
    }

    template <typename T, typename... Args> void va_remove(T *a1, Args &&...args)
    {
        _remove(a1);
        va_remove(args...);
    }
    template <typename T, typename... Args> void va_remove(T &a1, Args &&...args)
    {
        _remove(a1);
        va_remove(args...);
    }
    //----------------------------------------------------------------------------
    // Internal Adds for final object placement add output writers to the logger.

    void _add(flxDevice &op)
    {
        _add(&op);
    }
    void _add(flxDevice *op)
    {
        if (op != nullptr)
            _devicesToLog.push_back(op);
    }

    // removes

    void _remove(flxDevice &op)
    {
        _remove(&op);
    }
    void _remove(flxDevice *op)
    {
        if (op != nullptr)
            _devicesToLog.remove(op);
    }

  private:
    flxLoRaWANDigi *_pLoRaWAN;
};
