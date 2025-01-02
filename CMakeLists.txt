#
# Copyright (c) 2024-2025, SparkFun Electronics Inc.
#
# SPDX-License-Identifier: MIT
#
cmake_minimum_required(VERSION 3.13)

# set project name - setting language to NONE disables the default compiler checks
project(IoTNodeLoRaWAN NONE)

# Import the flux-dk cmake system
include($ENV{FLUX_SDK_PATH}/external/flux_sdk_import.cmake)

# set our Arduino Library name to something unique to this project
flux_sdk_set_library_name(SparkFun_IoTNodeLoRaWAN)

# Where is directory that the flux stuff will be added to? This is the relative path from this file
# to the arduino sketch directory this is also used as the name of the cmake project
flux_sdk_set_project_directory(.)

# The node board is rp silicon
flux_sdk_set_platform(platform_rpi)

# what modules/components of the SDK are we using
flux_sdk_add_module(
    flux_base
    flux_clock
    flux_logging
    flux_system
    flux_prefs
    flux_prefs_serial
    flux_network
    device_bme280
    device_bme68x
    device_bmp384
    device_bmp581
    device_button
    device_ccs811
    device_ens160
    device_fs3000
    device_gnss
    device_ism330
    device_lps25hb
    device_max17048
    device_ms5637
    device_micropressure
    device_nau7802
    device_opt4048
    device_rv8803
    device_scd40
    device_sgp30
    device_sgp40
    device_shtc3
    device_stc31
    device_sths34pf80
    device_tmp117
    device_vcnl4040
    device_veml6075
    device_veml7700
    device_vl53l1x)

# now call the init function/macro - this will build the Arduino Library SparkFun_Flux under this
# main directory
flux_sdk_init()
