# BOSCH BME280 Library for I2C Bus to use with Arduino IDE
Tested with ESP8266-12/-07 and Arduino IDE 1.6.13 including ESP8266 Core v2.3.0
***
### 1 - Create BME280 Sensor Node
```c++
BME280_I2C BME280;
```
***
### 2 - Open I2C Bus on defined Pins
```c++
Wire.begin(GPIO_I2C_SDA, GPIO_I2C_SCL);
```
***
### 3 - Init BME280 on I2C
```c++
BME280.begin(BME280ADDR);
```
***
### 4.1 - Read Data in 'Burst Mode' from BME280
Use EITHER 'Burst' OR 'Single' Read on BME280 in one cycle! NOT BOTH !
```c++
BME280.read_adc_burst();
```
***
### 4.2 - Read Data in 'Single Mode' from BME280
Use EITHER 'Burst' OR 'Single' Read on BME280 in one cycle! NOT BOTH !
```c++
BME280.read_adc_single();
```
***
### 5 - Print compensated BME280 ADC Data to Serial Output
ADC Values are compensated with formulas from official Bosch BME280 datasheet. Default calculation precision is 32Bit Integer, return values are Unsigned 32Bit Integer. Temperature returns a Signed 32Bit Integer. Pressure features calculation with 64Bit Integer precision, this returns an Unsigned 32Bit Integer, too. Double Precision for Calculation is also available for all ADC values, return datatype is then 'double'. Return values of 32 and 64 Bit precision functions do not carry a decimal point. You have to divide them by 100. Divide by 1000 for Humidity.


The variable Precision is causing differences on Measurements.
* Temperature
```c++
Serial.println( String("BME280 >> TEMP  >> I32 >> ") + (int32_t) BME280.temperature()     + " C" );
Serial.println( String("BME280 >> TEMP  >> DBL >> ") + (double)  BME280.temperature_dbl() + " C" );
```
* Pressure
```c++
Serial.println( String("BME280 >> PRESS >> I32 >> ") + (int32_t) BME280.pressure()        + " Pa" );
Serial.println( String("BME280 >> PRESS >> I64 >> ") + (int32_t) BME280.pressure_i64()    + " Pa" );
Serial.println( String("BME280 >> PRESS >> DBL >> ") + (double)  BME280.pressure_dbl()    + " Pa" );
```
* Humidity
```c++
Serial.println( String("BME280 >> HUMID >> I32 >> ") + (int32_t) BME280.humidity()        + " %rH" );  
Serial.println( String("BME280 >> HUMID >> DBL >> ") + (double)  BME280.humidity_dbl()    + " %rH" );
```
* Output
```c++
BME280 >> TEMP  >> I32 >> 3007 C
BME280 >> TEMP  >> DBL >> 30.07 C

BME280 >> PRESS >> I32 >> 97911 Pa
BME280 >> PRESS >> I64 >> 97911 Pa
BME280 >> PRESS >> DBL >> 97998.85 Pa

BME280 >> HUMID >> I32 >> 25023 %rH
BME280 >> HUMID >> DBL >> 24.43 %rH
```
***
### 6 - Optional Functions
This library offers extended functions to write Oversampling Rates, StandBy Time, IIR Filter Coefficent and Mode to BME280.

#### BME280 T_SB Settings - Controls inactive duration 't_standby' in normal mode. See Table.
| Value (t_sb)    | Time (ms)     |
|:---------------:|--------------:|
|0b000 | 0.5|
|0b001 | 62.5|
|0b010 | 125|
|0b011 | 250|
|0b100 | 500|
|0b101 | 1000|
|0b110 | 10|
|0b111 | 20|

#### BME280 FILTER Settings - Controls the time constant of the IIR filter. See Table.
| Value (filter)  | Coefficent    |
|:---------------:|--------------:|
|0b000 | OFF |
|0b001 | x2 |
|0b010 | x4 |
|0b011 | x8 |
|>0b100 | x16 |

You have to set the StandBy Time for "Normal Mode" and Filter Coefficent before writing it to the BME280 with this function:
```c++
filter_config( uint8_t t_sb, uint8_t filter );
```

You now can write these settings to the Sensor. This is done by using the following function:
```c++
filter_write();
```

#### BME280 OSRS Settings - Controls the oversampling for measurements. See Table.

| Value (osrs_x)  | Rate          |
|:---------------:|--------------:|
|0b000 | SKIPPED|
|0b001 | x1|
|0b010 | x2|
|0b011 | x4|
|0b100 | x8|
|>0b101 | x16|

You have to set the Oversampling Rates for Temperature, Pressure and Humidity before writing it to the BME280 with this function:
```c++
osrs_config(uint8_t osrs_p,uint8_t osrs_t,uint8_t osrs_h);
```
You now can write these settings to the Sensor. This is done by using one of the following functions. Each of these Functions writes the Oversampling Rate AND the wished Sensor Mode: 
```c++
sleep();		// No readings are done, Registers are ok to read/write
forced();		// One reading is done, then the Sensor is in Sleep Mode again
normal();		// Continuous switching between reading and defined StandBy Time
```

***
### Use DoxyGen (doxy/html/index.html) and Examples for further information
