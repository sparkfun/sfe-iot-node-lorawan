
# LoRaWAN Data Encoding

To achieve incredible low-power and long-range performance, a LoRaWAN end node minimizes the size of data packets sent across the LoRa network. While the size of a data packet is adjustable based on specific performance needs and network deployment topology, for maximum flexibility, the SparkFun IoT Node - LoRaWAN firmware uses a packet payload size of 11 bytes. This relativity small payload length is a challenge for a general data-logging application and requires that the firmware develop a unique data packing methodology to meet the general data logging goals of the implementation.

## Value Packing Structure

The IoT Node - LoRaWAN firmware employs a simple tagged format to pack and send sensor data across the LoRaWAN network. The key element to this methodology is the ***tag***, which is defined a "Value Type" for the implementation used for sending data. The Value Type tag is followed by the data value, which is formatted for network transport.

The general structure of a packed value is:

 \[ **Value Type** *{1 byte}* ][**Data Value** *{n bytes - network byte order }*]

### Value Type

Key attributes of a "Value Type" tag/code:

* There is a common set of "Value Type" code IDs - each code identifies a value, which is defined as a specific measured value/phenomenology and its associated units.
* The "Value Type" codes are known by both sender and receiver of a data value
* The length of a data value is defined by it's Value Type.
  * Data types supported: *unit8, uint16, unit32, int8, int16, int32, float and double*

### Data Formatting

When a data value is "packed", for multi-byte values, the data is encoded for network endianness (big-endian format).

The following table outlines how a data value is packed, based on it's data type size:

| Data Type | Length (bytes) | Format Used |
|--|--|--|
| int8   | 1 | No format applied |
| int16  | 2 | Network Byte Order (2 bytes) |
| int32  | 4 | Network Byte Order (4 bytes) |
| uint8   | 1 | No format applied |
| uint16  | 2 | Network Byte Order (2 bytes) |
| uint32  | 4 | Network Byte Order (4 bytes) |
| float  | 4 | Network Byte Order (4 bytes) |
| double  | 8 | \[ Network Byte Order (4 bytes) ][ Network Byte Order (4 bytes) ] |

## Packet Encoding

When packing a data value in the LoRaWAN packet payload, which has a total of 11 bytes available, data values are formatted as described in the previous section and added to the payload until no further space is available. When the payload is *full* (no free space available for the next data value), it is sent to the LoRaWAN network.

So for a 11 byte LoRaWAN Payload buffer, the contents could be (note a Value Type is 1 byte):

* \[ **Value Type**][**Data Value** *{4 bytes}*]\[ **Value Type**][**Data Value** *{4 bytes}*]\[**empty** {1 byte}]  = **10 bytes used**
* \[ **Value Type**][**Data Value** *{2 bytes}*]\[ **Value Type**][**Data Value** *{4 bytes}*]\[**empty** {3 byte}]  = **8 bytes used***
* \[ **Value Type**][**Data Value** *{2 bytes}*]\[ **Value Type**][**Data Value** *{4 bytes}*]\[ **Value Type**][**Data Value** *{2 bytes}*] =  **11 bytes used**

The general payload packing is as follows:

### *Initial Condition*

* Set the current position in the payload to position 0

### *Operation*

1) Data Packet is formatted
1) If no room is available in the payload
    * The data payload is sent to the LoRaWAN
    * The payload buffer set to position 0
1) The data packet is added to the payload buffer
1) The current position in in the payload is incremented by the size of the data packet
1) Repeat step 1) until all data is sent

### *End Condition*

* Flush the payload buffer - send any pending data to the LoRaWAN

## Sensor and Data Value Encodings

The following table outlines the sensors and data encoding used by the IoT Node - LoRaWAN firmware:

| Sensor | Parameter | Value Type | Value Type Code | Data Type |
| -- | -- | -- | -- | -- |
| BME688  | |  |  ||
|  | Humidity | Humidity_F |  | float|
| | Temperature C | TempC | | float|
| | Pressure | Pressure_F | | float|
| BME280 | ||||
|| Humidity |Humidity_F | | float|
|| Temperature F | TempF || float|
|| Temperature C | TempC || float|
|BMP384|||||
|| Temperature C | TempC_D ||double|
|| Presure (Pa)| Pressure_D || double|
|BMP581|||||
|| Pressure (Pa) | Pressure_F || float|
|| Temperature C | TempC || float|
|CCS811|||||
|| CO2 | CO2_F || float|
|| VOC| TVOC || float|
|ENS160|||||
|| Equivalent CO2 | CO2 || uint16|
|| TVOC| VOC || unit16|
||Ethanol Concentration | ETOH || uint16 |
|| Air Quality Index | AQI || uint8 |
|FS3000|||||
|| Flow (MPS) | MPS || float|
|| FLow (MPH) | MPH || float|
|GNSS|||||
|| Latitude in Degrees | Latitude || double|
|| Longitude in Degrees| Longitude || double|
|| Altitude (meters) | Altitude || double|
|ISM330|||||
|| Accelerometer X (milli-g)| AccelX || float|
|| Accelerometer Y (milli-g)| AccelY || float|
|| Accelerometer Z (milli-g)| AccelZ || float|
|| Gyro X (milli-dps)| GyroX || float|
|| Gyro Y (milli-dps)| GyroY || float|
|| Gyro Z (milli-dps)| GyroZ || float|
|LPS25HB|||||
|| Pressure (hPa) | Pressure_F || float|
|| Temperature C | TempC || float|
|MAX17048|||||
|| Voltage (V)| BatteryVoltage || float|
|| State Of Charge (%) | BatteryCharge || float|
|| Change Rate (%/hr) | BatteryChargeRate || float|
|Micro Pressure|||||
|| Pressure (Pa)| Pressure_F || float|
|MS5637|||||
|| Pressure (mBar) | Pressure_mBar || float|
|| Temperature C | TempC || float|
|NAU7802|||||
|| Weight| WeightUserUnits|| float|
|OPT4048|||||
|| CIEx| CIE_X || double|
|| CIEy | CIE_Y || double|
|| CIET | CCT || double|

