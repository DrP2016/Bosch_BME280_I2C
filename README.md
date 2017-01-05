## BOSCH BME280 Library for I2C Bus to use with Arduino IDE
Tested with ESP8266-12/-07 and Arduino IDE 1.6.13 including ESP8266 Core v2.3.0

#### 1 - Create BME280 Sensor Node
```c++
BME280_I2C BME280;
```

#### 2 - Open I2C Bus on defined Pins
```c++
Wire.begin(GPIO_I2C_SDA, GPIO_I2C_SCL);
```

#### 3 - Init BME280 on I2C
```c++
BME280.begin(BME280ADDR);
```

#### 4.1 - Data in 'Burst Mode' from BME280
Use EITHER 'Burst' OR 'Single' Read on BME280 in one cycle! NOT BOTH !
```c++
BME280.read_adc_burst();
```

#### 4.2 - Data in 'Single Mode' from BME280
Use EITHER 'Burst' OR 'Single' Read on BME280 in one cycle! NOT BOTH !
```c++
BME280.read_adc_single();
```

#### 5 - Print compensated BME280 ADC Data to Serial Output
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

#### Use DoxyGen (doxy/html/index.html) and Examples for further information
