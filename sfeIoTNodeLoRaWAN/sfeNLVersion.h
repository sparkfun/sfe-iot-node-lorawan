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

// Board ID/Mode information data

// Major version number
#define kDLVersionNumberMajor 0

// Minor version number
#define kDLVersionNumberMinor 5

// Point version number
#define kDLVersionNumberPoint 01

// Version string description
#define kDLVersionDescriptor "Version 0.5.01 Alpha"

// app name/class ID string
#define kDLAppClassNameID "SFE-IOT-NODE_LORAWAN"

// Build number - should come from the build system. If not, set default

#ifndef BUILD_NUMBER
#define BUILD_NUMBER 0
#endif

#define kDLVersionBoardDesc "(c) 2023-2024 SparkFun Electronics"