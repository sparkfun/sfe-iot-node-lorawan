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

// Board ID/Mode information data

// Major version number
#define kDLVersionNumberMajor 1

// Minor version number
#define kDLVersionNumberMinor 1

// Point version number
#define kDLVersionNumberPoint 0

// Version string description
#define kDLVersionDescriptor ""

// app name/class ID string
#define kDLAppClassNameID "SFE-IOT-NODE_LORAWAN"

// Build number - should come from the build system. If not, set default

#ifndef BUILD_NUMBER
#define BUILD_NUMBER 0
#endif

#define kDLVersionBoardDesc "(c) 2023-2025 SparkFun Electronics"