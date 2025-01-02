
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

