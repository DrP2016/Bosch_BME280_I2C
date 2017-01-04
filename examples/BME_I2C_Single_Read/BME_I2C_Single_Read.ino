/*
 * Includes
 */
#include <Wire.h>
#include <BME280_I2C.h>

/*
 * Defines
 */
#define BME280ADDR 0x76
#define GPIO_I2C_SDA 4
#define GPIO_I2C_SCL 5

/*
 * Create BME280 Sensor Node
 */
BME280_I2C BME280;

/*
 * Setup Routine
 */
void setup() {  
  /*
   * Open Serial Port for Debug
   */
  Serial.begin(115200);
  delay(250);  
  /*
   * Print Chip Information
   */
  Serial.println("");
  Serial.println("");
  Serial.println("PROG INFORMATION =========================================================");
  Serial.println("PROG >> INFO >> BOSCH BME280 Testprogram - Read ADC-Data in Burst Mode");  
  Serial.println("PROG >> DATE >> " __DATE__ );
  Serial.println("PROG >> TIME >> " __TIME__ );
  Serial.println("PROG >> GCC  >> " __VERSION__ );
  Serial.println(String("PROG >> IDE  >> ") + IDEString() );
  Serial.println("CHIP INFORMATION =========================================================");
  Serial.printf("CHIP >> CORE  >> ID: %08X\r\n", ESP.getChipId());
  Serial.println(String("CHIP >> CORE  >> Free Heap: ") + ESP.getFreeHeap() / 1024 + " kB");
  Serial.println("CHIP >> CORE  >> Speed: 80 MHz");
  Serial.printf("CHIP >> FLASH >> ID : %08X\r\n", ESP.getFlashChipId());
  Serial.println(String("CHIP >> FLASH >> Size: ") + ESP.getFlashChipRealSize() / 1024 + " kB");
  Serial.println(String("CHIP >> FLASH >> Speed: ") + ESP.getFlashChipSpeed()/1000000 + " MHz");
  Serial.println("==========================================================================");
  /* 
   * Open I2C Bus on defined Pins and give BME280 time to 'boot'
   */
  Wire.begin(GPIO_I2C_SDA, GPIO_I2C_SCL);
  Serial.println("I2C  >> Bus initialized!");
  delay(1500);
  
  /*
   * Init BME280 on I2C
   */
  if (!BME280.begin(BME280ADDR)) {
    Serial.println("I2C  >> No Sensor detected!");
    Serial.println("==========================================================================");
    while (1);
  } else {
    Serial.println("I2C  >> BME280 initialized!");
    Serial.println("==========================================================================");
  }
}

void loop() {
  /*
   * Read Data in 'Single Mode' from BME280
   * 
   * Alternative Method to read Data from BME280, slower and Measurements may get mixed up
   */
  BME280.read_adc_single();
   
  /*
   * Print compensated BME280 Data 
   */
  Serial.println( String("BME280 >> TEMP  >> I32 >> ") + (int32_t) BME280.temperature()     + " C" );
  Serial.println( String("BME280 >> TEMP  >> DBL >> ") + (double)  BME280.temperature_dbl() + " C" );
  
  Serial.println( String("BME280 >> PRESS >> I32 >> ") + (int32_t) BME280.pressure()        + " Pa" );
  Serial.println( String("BME280 >> PRESS >> I64 >> ") + (int32_t) BME280.pressure_i64()    + " Pa" );
  Serial.println( String("BME280 >> PRESS >> DBL >> ") + (double)  BME280.pressure_dbl()    + " Pa" );
  
  Serial.println( String("BME280 >> HUMID >> I32 >> ") + (int32_t) BME280.humidity()        + " %rH" );  
  Serial.println( String("BME280 >> HUMID >> DBL >> ") + (double)  BME280.humidity_dbl()    + " %rH" );
   
  Serial.println( String("BME280 >> ALTIT >> DBL >> ") + (double)  BME280.altitude_dbl(1013.25) + " m" );
  
  Serial.println("==========================================================================");
  
  /*
   * Pause for next reading
   */
  delay(1000);
}

String IDEString(){
  uint16_t IDE = ARDUINO;
  String tmp = "";
  tmp += String(IDE/10000);
  IDE %= 10000;
  tmp += ".";
  tmp += String(IDE/100);
  IDE %= 100;
  tmp += ".";
  tmp += String(IDE);
  return tmp;
}
