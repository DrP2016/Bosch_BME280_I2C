/**
 *  \file BME280_I2C.cpp
 *  \brief BOSCH BME280 Sensor Library. I2C ONLY !
 *  
 *  \details Library C Code File
 *  \details 
 *  \details Written by Pascal Droege (GER) for private use.  
 *  \details BSD license, all text above must be included in any redistribution
 */
 
#include "Arduino.h"
#include <Wire.h>
#include "BME280_I2C.h"

BME280_I2C::BME280_I2C(){}

/**
 *  \brief Start Sensor
 *  
 *  \return Success Flag
 *  
 *  \details Read Factory Calibration Data , then write custom Configuration and Mode to BME280
 */
bool BME280_I2C::begin(uint8_t address) {
	if ( !read_chip_id(address) ){
		if ( !read_chip_id(BME280_ADDRESS) ){
			if ( !read_chip_id(BME280_ADDRESS_2) ){
				return false;
			}
		}
	}
	_inited = true;
	read_coeff();
	filter_config();
	filter_write();	
	osrs_config();
	forced();	
	return true;
}

/**
 *  \brief Read BME280 ChipID "0x60" from given I2C Address
 *  
 *  \param [in] address I2C Address for BME280
 *  
 *  \details Set '_i2caddr' to given I2C Address and read ChipID from BME280_REGISTER_CHIPID
 */
bool BME280_I2C::read_chip_id( uint8_t address ){
	_i2caddr = address;
	return (bool) ( readU8(BME280_REGISTER_CHIPID) == 0x60 );
}

/**
 *  \brief Read BME280 Run State
 *  
 *  \return BME280 Run State as Signed 8Bit Integer 
 *  
 *  \details Read Sensor State from BME280_REGISTER_STATE and splice the Repsonse.
 *  \details -1 > Sensor not inited
 *  \details 0 > Sensor in Pause
 *  \details 1 > NVM data are being copied
 *  \details 2 > Conversion and Storing Results is running
 *  
 */
int8_t BME280_I2C::state( void ){
	int8_t retval = 0;
	uint8_t bme280_state = readU8(BME280_REGISTER_STATE);
	uint8_t bme280_measuring = (bme280_state & 0b00001000) >> 3;
	uint8_t bme280_im_update = bme280_state & 0b00000001;
	if ( _inited == true ) {
		retval = bme280_measuring<< 1 + bme280_im_update;
	} else {
		retval = -1;
	}
	return (int8_t) retval;
}

/**
 *  \brief Set BME280 to 'Sleep Mode'
 *  
 *  \details Set 'Sleep Mode' by writing 'BME280_REGISTER_CONTROL' with '0b00' on [1:0]
 */
void BME280_I2C::sleep(void) {
	osrs_mode_write(0b00);
}

/**
 *  \brief Set BME280 to 'Forced Mode'
 *  
 *  \details Set 'Forced Mode' by writing 'BME280_REGISTER_CONTROL' with '0b01' OR '0b10' on [1:0]
 */
void BME280_I2C::forced(void) {
	osrs_mode_write(0b01);
}

/**
 *  \brief Set BME280 to 'Normal Mode'
 *  
 *  \details Set 'Normal Mode' by writing 'BME280_REGISTER_CONTROL' with '0b11' on [1:0]
 */
void BME280_I2C::normal(void) {
	osrs_mode_write(0b11);
}

/**
 *  \brief Set Filter and StandBy Time for BME280
 *  
 *  \param [in] t_sb See Description 'BME280 T_SB Settings | 35' in .h File
 *  \param [in] filter See Description 'BME280 FILTER Settings | 52' in .h File
 *  
 *  \details Set '_t_sb' and '_filter' before writing Configuration Data to BME280
 */
void BME280_I2C::filter_config(uint8_t t_sb,uint8_t filter){
	_t_sb = t_sb;
	_filter = filter;
}

/**
 *  \brief Write Filter and StandBy Time to BME280
 *  
 *  \details Build the Byte, that contains Filter and StandBy Time, then write it to 'BME280_REGISTER_CONFIG'
 *  \details BME280_REGISTER_CONFIG [7:5] = 't_sb'
 *  \details BME280_REGISTER_CONFIG [4:2] = 'filter'
 *  \details BME280_REGISTER_CONFIG [1:0] = 'spi3w_en' (Ignored for I2C)
 */
void BME280_I2C::filter_write(void) {
	uint8_t config = 0;
	config = _t_sb;
	config = config << 3;
	config += _filter;
	config = config << 2;
	config += 0b00;
	
    writeU8(BME280_REGISTER_CONFIG, config);
}

/**
 *  \brief Set Oversampling Rate for T, P, H for BME280
 *  
 *  \param [in] osrs_p See Description 'BME280 OSRS Settings | 18' in .h File
 *  \param [in] osrs_t See Description 'BME280 OSRS Settings | 18' in .h File
 *  \param [in] osrs_h See Description 'BME280 OSRS Settings | 18' in .h File
 *  
 *  \details Set 'osrs_t','osrs_p' and 'osrs_h' before writing Configuration Data to BME280
 */
void BME280_I2C::osrs_config(uint8_t osrs_p,uint8_t osrs_t,uint8_t osrs_h){
	_osrs_p = osrs_p;
	_osrs_t = osrs_t;
	_osrs_h = osrs_h;
}

/**
 *  \brief Write Oversampling Rate for T, P, H and Mode to BME280
 *  
 *  \param [in] mode 0b00-Sleep; 0b01-Forced; 0b11-Normal
 *  
 *  \details Build the Byte, that contains Oversampling for T, P and Mode, then write it to 'BME280_REGISTER_CONTROL'
 *  \details BME280_REGISTER_CONTROL [7:5] = 'osrs_t'
 *  \details BME280_REGISTER_CONTROL [4:2] = 'osrs_p'
 *  \details BME280_REGISTER_CONTROL [1:0] = 'mode'
 *  \details Oversampling for Humidity is written directly to 'BME280_REGISTER_CONTROLHUMID'
 *  \details BME280_REGISTER_CONTROLHUMID [2:0] = 'osrs_h'
 */
void BME280_I2C::osrs_mode_write(uint8_t mode) {
	uint8_t ctrl_meas = 0;
	ctrl_meas = _osrs_t;								
	ctrl_meas = ctrl_meas << 3;
	ctrl_meas += _osrs_p;
	ctrl_meas = ctrl_meas << 2;
	
    writeU8(BME280_REGISTER_CONTROLHUMID, _osrs_h);
    writeU8(BME280_REGISTER_CONTROL, ctrl_meas + mode);
}


void BME280_I2C::read_data_burst(void){
	uint8_t value;
	Wire.beginTransmission((uint8_t)_i2caddr);
	Wire.write((uint8_t)0xF7);
	Wire.endTransmission();
	Wire.requestFrom((uint8_t)_i2caddr, (uint8_t)8);
	_adc_P = (Wire.read() << 16) | (Wire.read() << 8) | Wire.read();
	_adc_P = _adc_P >> 4;
	_adc_T = (Wire.read() << 16) | (Wire.read() << 8) | Wire.read();
	_adc_T = _adc_T >> 4;
	_adc_H = (Wire.read() << 8) | Wire.read() ;
	value = 0;
}

/**
 *  \brief Read adc_T, adc_P, adc_H in 'Burst Mode'
 *  
 *  \details Before reading, change BME280 Mode from 'Sleep' to 'Forced or 'Normal'
 *  \details Reads adc_T, adc_P, adc_H by doing a very fast 'Burst Read' on 0xF7 to 0xFE
 *  \details After reading, change BME280 Mode to 'Sleep'
 */
void BME280_I2C::read_adc_burst(void){
	read_data_burst();
	compensate_T_int32(_adc_T);						// calculate current '_t_fine' with double precision
}

/**
 *  \brief Read adc_T, adc_P, adc_H in 'Single Mode'
 *  
 *  \details Before reading, change BME280 Mode from 'Sleep' to 'Forced or 'Normal'
 *  \details Reads adc_T, adc_P, adc_H by polling every single register by its address
 *  \details After reading, change BME280 Mode to 'Sleep'
 */
void BME280_I2C::read_adc_single(void){
	read_adc_P();
	read_adc_T();
	read_adc_H();
	compensate_T_int32(_adc_T);						// calculate current '_t_fine' with double precision
}

/**
 *  \brief Read adc_P in 'Single Mode'
 *  
 *  \details Build the _adc_P Value from Register 0xFA, 0xFB and 0xFC
 *  \details _adc_P is expected to be received in 20 bit format, positive, stored in a 32 bit signed integer
 *  \details Humidity is expected to be received in 16 bit format, positive, stored in a 32 bit signed integer
 */
void BME280_I2C::read_adc_P(void){
	_adc_P = 0;
	_adc_P = readU24(BME280_REGISTER_PRESSUREDATA);
	_adc_P = _adc_P >> 4;
}

/**
 *  \brief Read adc_T in 'Single Mode'
 *  
 *  \details Build the _adc_T Value from Register 0xF7, 0xF8 and 0xF9
 *  \details _adc_T is expected to be received in 20 bit format, positive, stored in a 32 bit signed integer
 */
void BME280_I2C::read_adc_T(void){
	_adc_T = 0;
	_adc_T = readU24(BME280_REGISTER_TEMPDATA);
	_adc_T = _adc_T >> 4;
}

/**
 *  \brief Read adc_H in 'Single Mode'
 *  
 *  \details Build the _adc_H Value from Register 0xFD, 0xFE
 *  \details _adc_H is expected to be received in 16 bit format, positive, stored in a 32 bit signed integer
 */
void BME280_I2C::read_adc_H(void){
	_adc_H = 0;
	_adc_H = readU16(BME280_REGISTER_HUMIDDATA);
}	

/**
 *  \brief Compensate given 'adc_P' with Factory Calibration Data
 *  
 *  \param [in] adc_P 20 bit format, positive, stored in a 32 bit signed integer
 *  \return Pressure in Pa as unsigned 32 bit integer. Output value of “96386” equals 96386 Pa = 963.86 hPa
 *  
 *  \details Formula was taken from official BOSCH BME280 Datasheet | 50
 */
uint32_t BME280_I2C::compensate_P_int32(int32_t adc_P){
	int32_t var1, var2;
	uint32_t P;
	
	var1 = (((int32_t)_t_fine)>>1) - (int32_t)64000;
	var2 = (((var1>>2) * (var1>>2)) >> 11 ) * ((int32_t)_bme280_calib.dig_P6);
	var2 = var2 + ((var1*((int32_t)_bme280_calib.dig_P5))<<1);
	var2 = (var2>>2)+(((int32_t)_bme280_calib.dig_P4)<<16);
	var1 = (((_bme280_calib.dig_P3 * (((var1>>2) * (var1>>2)) >> 13 )) >> 3) + ((((int32_t)_bme280_calib.dig_P2) * var1)>>1))>>18;
	var1 =((((32768+var1))*((int32_t)_bme280_calib.dig_P1))>>15);
	if (var1 == 0){
		return 0; // avoid exception caused by division by zero
	}
	P = (((uint32_t)(((int32_t)1048576) -adc_P) -(var2>>12)))*3125;
	if (P < 0x80000000){
		P = (P << 1) / ((uint32_t)var1);
	} else {
		P = (P / (uint32_t)var1) * 2;
	}
	var1 = (((int32_t)_bme280_calib.dig_P9) * ((int32_t)(((P>>3) * (P>>3))>>13)))>>12;
	var2 = (((int32_t)(P>>2)) * ((int32_t)_bme280_calib.dig_P8))>>13;
	P = (uint32_t)((int32_t)P + ((var1 + var2 + _bme280_calib.dig_P7) >> 4));
	
	return P;
}

/**
 *  \brief Compensate given 'adc_P' with Factory Calibration Data
 *  
 *  \param [in] adc_P 20 bit format, positive, stored in a 32 bit signed integer
 *  \return Pressure in Pa as unsigned 32 bit integer. Output value of “96386” equals 96386 Pa = 963.86 hPa
 *  
 *  \details Formula was taken from official BOSCH BME280 Datasheet | 50
 */
uint32_t BME280_I2C::compensate_P_int64(int32_t adc_P){
	int64_t var1, var2, P;
		
	var1 = ((int64_t)_t_fine) - 128000;
	var2 = var1 * var1 * (int64_t)_bme280_calib.dig_P6;
	var2 = var2 + ((var1*(int64_t)_bme280_calib.dig_P5)<<17);
	var2 = var2 + (((int64_t)_bme280_calib.dig_P4)<<35);
	var1 = ((var1 * var1 * (int64_t)_bme280_calib.dig_P3)>>8) + ((var1 * (int64_t)_bme280_calib.dig_P2)<<12);
	var1 = (((((int64_t)1)<<47)+var1))*((int64_t)_bme280_calib.dig_P1)>>33;
	if (var1 == 0){
		return 0; // avoid exception caused by division by zero
	}
	P = 1048576-adc_P;
	P = (((P<<31) -var2)*3125)/var1;
	var1 = (((int64_t)_bme280_calib.dig_P9) * (P>>13) * (P>>13)) >> 25;
	var2 = (((int64_t)_bme280_calib.dig_P8) * P) >> 19;
	P = ((P + var1 + var2) >> 8) + (((int64_t)_bme280_calib.dig_P7)<<4);
	return (uint32_t)P/256;
}

/**
 *  \brief Compensate given 'adc_P' with Factory Calibration Data
 *  
 *  \param [in] adc_P 20 bit format, positive, stored in a 32 bit signed integer
 *  \return Pressure in Pa as double. Output value of “96386.2” equals 96386.2 Pa = 963.862 hPa
 *  
 *  \details Formula was taken from official BOSCH BME280 Datasheet | 50
 */
double BME280_I2C::compensate_P_double(int32_t adc_P) {
	double var1, var2, P;
	
	var1 = ((double)_t_fine/2.0) - 64000.0;
	var2 = var1 * var1 * ((double)_bme280_calib.dig_P6) / 32768.0;
	var2 = var2 + var1 * ((double)_bme280_calib.dig_P5) * 2.0;
	var2 = (var2/4.0)+(((double)_bme280_calib.dig_P4) * 65536.0);
	var1 = (((double)_bme280_calib.dig_P3) * var1 * var1 / 524288.0 + ((double)_bme280_calib.dig_P2) * var1) / 524288.0;
	var1 = (1.0 + var1 / 32768.0)*((double)_bme280_calib.dig_P1);
	if (var1 == 0.0){
		return 0; 			// Avoid division by zero !
	}
	P = 1048576.0 - (double)adc_P;
	P = (P - (var2 / 4096.0)) * 6250.0 / var1;
	var1 = ((double)_bme280_calib.dig_P9) * P * P / 2147483648.0;
	var2 = P * ((double)_bme280_calib.dig_P8) / 32768.0;
	P = P + (var1 + var2 + ((double)_bme280_calib.dig_P7)) / 256.0;
	
	return P;
}

/**
 *  \brief Compensate given 'adc_T' with Factory Calibration Data
 *  
 *  \param [in] adc_T 20 bit format, positive, stored in a 32 bit signed integer
 *  \return Temperature in DegC, resolution is 0.01 DegC. Output value of “5123” equals 51.23 DegC
 *  
 *  \details Formula was taken from official BOSCH BME280 Datasheet | 23
 */
int32_t BME280_I2C::compensate_T_int32(int32_t adc_T){
	int32_t var1, var2, T;
	
	var1 = ((((adc_T>>3) - ((int32_t)_bme280_calib.dig_T1<<1))) * ((int32_t)_bme280_calib.dig_T2)) >> 11;
	var2 = (((((adc_T>>4) - ((int32_t)_bme280_calib.dig_T1)) * ((adc_T>>4) - ((int32_t)_bme280_calib.dig_T1))) >> 12) * ((int32_t)_bme280_calib.dig_T3)) >> 14;
	_t_fine = var1 + var2;	
	T = (_t_fine * 5 + 128) >> 8;
	
	return T;
}

/**
 *  \brief Compensate given 'adc_T' with Factory Calibration Data
 *  
 *  \param [in] adc_T 20 bit format, positive, stored in a 32 bit signed integer
 *  \return Temperature in DegC, double precision. Output value of “51.23” equals 51.23 DegC
 *  
 *  \details Formula was taken from official BOSCH BME280 Datasheet | 49
 */
double BME280_I2C::compensate_T_double(int32_t adc_T){
	double var1, var2, T;
	
	var1 = (((double)adc_T)/16384.0 - ((double)_bme280_calib.dig_T1)/1024.0) * ((double)_bme280_calib.dig_T2);
	var2 = ((((double)adc_T)/131072.0 - ((double)_bme280_calib.dig_T1)/8192.0) *(((double)adc_T)/131072.0 - ((double) _bme280_calib.dig_T1)/8192.0)) * ((double)_bme280_calib.dig_T3);
	_t_fine = (int32_t)(var1 + var2);
	T = (var1 + var2) / 5120.0;
	
	return T;
}

/**
 *  \brief Compensate given 'adc_H' with Factory Calibration Data
 *  
 *  \param [in] adc_H 16 bit format, positive, stored in a 32 bit signed integer
 *  \return Humidity in %RH as unsigned 32 bit integer in Q22.10 format (22 integer and 10 fractional bits)
 *  
 *  \details Formula was taken from official BOSCH BME280 Datasheet | 23
 */
uint32_t BME280_I2C::compensate_H_int32(int32_t adc_H) {
	int32_t v_x1_u32r;

	v_x1_u32r = (_t_fine - ((int32_t)76800));
	v_x1_u32r = (((((adc_H << 14) - (((int32_t)_bme280_calib.dig_H4) << 20) -
			    (((int32_t)_bme280_calib.dig_H5) * v_x1_u32r)) + ((int32_t)16384)) >> 15) *
			    (((((((v_x1_u32r * ((int32_t)_bme280_calib.dig_H6)) >> 10) *
			    (((v_x1_u32r * ((int32_t)_bme280_calib.dig_H3)) >> 11) + ((int32_t)32768))) >> 10) +
			    ((int32_t)2097152)) * ((int32_t)_bme280_calib.dig_H2) + 8192) >> 14));
	v_x1_u32r = (v_x1_u32r - (((((v_x1_u32r >> 15) * (v_x1_u32r >> 15)) >> 7) *
			    ((int32_t)_bme280_calib.dig_H1)) >> 4));

	v_x1_u32r = (v_x1_u32r < 0 ? 0 : v_x1_u32r);
	v_x1_u32r = (v_x1_u32r > 419430400 ? 419430400 : v_x1_u32r);
	
	return v_x1_u32r>>12;
}

/**
 *  \brief Compensate given 'adc_H' with Factory Calibration Data
 *  
 *  \param [in] adc_H 16 bit format, positive, stored in a 32 bit signed integer
 *  \return Humidity in %rH as as double. Output value of “46.332” represents 46.332 %rH
 *  
 *  \details Formula was taken from official BOSCH BME280 Datasheet | 49
 */
double BME280_I2C::compensate_H_double(int32_t adc_H) {
	double H;
	
	H = (((double)_t_fine) - 76800.0);
	H = (adc_H - (((double)_bme280_calib.dig_H4) * 64.0 + ((double)_bme280_calib.dig_H5) / 16384.0 * H)) *(((double)_bme280_calib.dig_H2) / 65536.0 * (1.0 + ((double)_bme280_calib.dig_H6) / 67108864.0 * H *(1.0 + ((double)_bme280_calib.dig_H3) / 67108864.0 * H)));
	H = H * (1.0 - ((double)_bme280_calib.dig_H1) * H / 524288.0);
	if (H > 100.0){
		H = 100.0;
	} else if (H < 0.0){
		H = 0.0;
	}
	
	return H;
}
	
/**
 *  \brief Temperature
 *  
 *  \return Temperature in DegC, resolution is 0.01 DegC. Output value of “5123” equals 51.23 DegC
 *  
 *  \details Calculate Temperature by compensating '_adc_T' with Factory Calibration Data
 */
int32_t BME280_I2C::temperature(void){
	return compensate_T_int32(_adc_T);
}

/**
 *  \brief Temperature with 'Double Precision'
 *  
 *  \return Temperature in DegC, double precision. Output value of “51.23” equals 51.23 DegC
 *  
 *  \details Calculate Temperature by compensating '_adc_T' with Factory Calibration Data
 */
double BME280_I2C::temperature_dbl(void){
	return compensate_T_double(_adc_T);
}

/**
 *  \brief Pressure
 *  
 *  \return Pressure in Pa as unsigned 32 bit integer. Output value of “96386” equals 96386 Pa = 963.86 hPa
 *  
 *  \details Calculate Pressure by compensating '_adc_P' with Factory Calibration Data
 */
int32_t BME280_I2C::pressure(void){
	return compensate_P_int32(_adc_P);
}

/**
 *  \brief Pressure
 *  
 *  \return Pressure in Pa as unsigned 32 bit integer. Output value of “96386” equals 96386 Pa = 963.86 hPa
 *  
 *  \details Calculate Pressure with 64Bit by compensating '_adc_P' with Factory Calibration Data
 */
int32_t BME280_I2C::pressure_i64(void){
	return compensate_P_int64(_adc_P);
}

/**
 *  \brief Pressure with 'Double Precision'
 *  
 *  \return Pressure in Pa as double. Output value of “96386.2” equals 96386.2 Pa = 963.862 hPa
 *  
 *  \details Calculate Pressure by compensating '_adc_P' with Factory Calibration Data
 */
double BME280_I2C::pressure_dbl(void){
	return compensate_P_double(_adc_P);
}

/**
 *  \brief Humidity
 *  
 *  \return Humidity in %RH as unsigned 32 bit integer in Q22.10 format (22 integer and 10 fractional bits)
 *  
 *  \details Calculate Humidity by compensating '_adc_H' with Factory Calibration Data
 */
int32_t BME280_I2C::humidity(void){
	return compensate_H_int32(_adc_H);
}

/**
 *  \brief Humidity with 'Double Precision'
 *  
 *  \return Humidity in %rH as as double. Output value of “46.332” represents 46.332 %rH
 *  
 *  \details Calculate Humidity by compensating '_adc_H' with Factory Calibration Data
 */
double BME280_I2C::humidity_dbl(void){
	return compensate_H_double(_adc_H);
}

/**
 *  \brief I2C - U8Bit - Write
 *  
 *  \param [in] reg Register Address to write to
 *  \param [in] value Value to write to Register
 *  
 *  \details Start Transmission on I2C, write the Register Address and Value. Close Transmission after writing
 */
void BME280_I2C::writeU8(uint8_t reg, uint8_t value){
	Wire.beginTransmission((uint8_t)_i2caddr);
	Wire.write((uint8_t)reg);
	Wire.write((uint8_t)value);
	Wire.endTransmission();
}

/**
 *  \brief I2C - U8Bit - Read
 *  
 *  \param [in] reg Register Address to read from
 *  
 *  \details Start Transmission on I2C, write the Register Address to BME280, Close Transmission after writing. Then read 1 Byte as Response. 
 */
uint8_t BME280_I2C::readU8(byte reg){
	uint8_t value;
	Wire.beginTransmission((uint8_t)_i2caddr);
	Wire.write((uint8_t)reg);
	Wire.endTransmission();
	Wire.requestFrom((uint8_t)_i2caddr, (byte)1);
	value = Wire.read();
	return value;
}

/**
 *  \brief I2C - S8Bit - Read
 *  
 *  \param [in] reg Register Address to read from
 *  
 *  \details Use 'readU8()' to read from I2C, but return a signed integer
 */
int8_t BME280_I2C::readS8(byte reg){
  return (int8_t)readU8(reg);
}

/**
 *  \brief I2C - U16Bit - Read
 *  
 *  \param [in] reg Register Address to read from
 *  
 *  \details Start Transmission on I2C, write the Register Address to BME280, Close Transmission after writing. Then read 2 Bytes as Response. 
 */
uint16_t BME280_I2C::readU16(byte reg){
	uint16_t value;
	Wire.beginTransmission((uint8_t)_i2caddr);
	Wire.write((uint8_t)reg);
	Wire.endTransmission();
	Wire.requestFrom((uint8_t)_i2caddr, (byte)2);
	value = (Wire.read() << 8) | Wire.read();
	return value;
}

/**
 *  \brief I2C - U16Bit - Read - Little Endian
 *  
 *  \param [in] reg Register Address to read from
 *  
 *  \details Use 'readU16()' to read from I2C, but with swapped MSB and LSB for LE Encoding
 */
uint16_t BME280_I2C::readU16_LE(byte reg){
  uint16_t temp = readU16(reg);
  return (temp >> 8) | (temp << 8);
}

/**
 *  \brief I2C - S16Bit - Read
 *  
 *  \param [in] reg Register Address to read from
 *  
 *  \details Use 'readU16()' to read from I2C, but return a signed integer
 */
int16_t BME280_I2C::readS16(byte reg){
  return (int16_t)readU16(reg);
}

/**
 *  \brief I2C - S16Bit - Read - Little Endian
 *  
 *  \param [in] reg Register Address to read from
 *  
 *  \details Use 'readU16_LE()' to read from I2C, but return a signed integer
 */
int16_t BME280_I2C::readS16_LE(byte reg){
  return (int16_t)readU16_LE(reg);
}

/**
 *  \brief I2C - U24Bit - Read
 *  
 *  \param [in] reg Register Address to read from
 *  
 *  \details Start Transmission on I2C, write the Register Address to BME280, Close Transmission after writing. Then read 3 Bytes as Response. 
 */
uint32_t BME280_I2C::readU24(byte reg){
	uint32_t value;
	Wire.beginTransmission((uint8_t)_i2caddr);
	Wire.write((uint8_t)reg);
	Wire.endTransmission();
	Wire.requestFrom((uint8_t)_i2caddr, (byte)3);
	value = Wire.read();
	value <<= 8;
	value |= Wire.read();
	value <<= 8;
	value |= Wire.read();
	return value;
}

/**
 *  \brief I2C - U24Bit - Read - Little Endian
 *  
 *  \param [in] reg Register Address to read from
 *  
 *  \details Use 'readU24()' to read from I2C, but with swapped MSB and LSB for LE Encoding
 */
uint32_t BME280_I2C::readU24_LE(byte reg){
  uint32_t temp = readU24(reg);
  return (temp >> 8) | (temp << 8);
}

/**
 *  \brief I2C - S24Bit - Read
 *  
 *  \param [in] reg Register Address to read from
 *  
 *  \details Use 'readU24()' to read from I2C, but return a signed integer
 */
int32_t BME280_I2C::readS24(byte reg){
  return (int32_t)readU24(reg);
}

/**
 *  \brief I2C - S24Bit - Read - Little Endian
 *  
 *  \param [in] reg Register Address to read from
 *  
 *  \details Use 'readU24_LE()' to read from I2C, but return a signed integer
 */
int32_t BME280_I2C::readS24_LE(byte reg){
  return (int32_t)readU24_LE(reg);
}

/**************************************************************************/
/*!
    @brief  Reads the factory-set coefficients
*/
/**************************************************************************/
void BME280_I2C::read_coeff(void){
    _bme280_calib.dig_T1 = readU16_LE(BME280_REGISTER_DIG_T1);
    _bme280_calib.dig_T2 = readS16_LE(BME280_REGISTER_DIG_T2);
    _bme280_calib.dig_T3 = readS16_LE(BME280_REGISTER_DIG_T3);

    _bme280_calib.dig_P1 = readU16_LE(BME280_REGISTER_DIG_P1);
    _bme280_calib.dig_P2 = readS16_LE(BME280_REGISTER_DIG_P2);
    _bme280_calib.dig_P3 = readS16_LE(BME280_REGISTER_DIG_P3);
    _bme280_calib.dig_P4 = readS16_LE(BME280_REGISTER_DIG_P4);
    _bme280_calib.dig_P5 = readS16_LE(BME280_REGISTER_DIG_P5);
    _bme280_calib.dig_P6 = readS16_LE(BME280_REGISTER_DIG_P6);
    _bme280_calib.dig_P7 = readS16_LE(BME280_REGISTER_DIG_P7);
    _bme280_calib.dig_P8 = readS16_LE(BME280_REGISTER_DIG_P8);
    _bme280_calib.dig_P9 = readS16_LE(BME280_REGISTER_DIG_P9);

    _bme280_calib.dig_H1 = readU8(BME280_REGISTER_DIG_H1);
    _bme280_calib.dig_H2 = readS16_LE(BME280_REGISTER_DIG_H2);
    _bme280_calib.dig_H3 = readU8(BME280_REGISTER_DIG_H3);
    _bme280_calib.dig_H4 = (readU8(BME280_REGISTER_DIG_H4) << 4) | (readU8(BME280_REGISTER_DIG_H4+1) & 0xF);
    _bme280_calib.dig_H5 = (readU8(BME280_REGISTER_DIG_H5+1) << 4) | (readU8(BME280_REGISTER_DIG_H5) >> 4);
    _bme280_calib.dig_H6 = readS8(BME280_REGISTER_DIG_H6);
}

/**
 *  \brief Altitude from SeaLevel
 *  
 *  \param [in] seaLevel Sea-level pressure in hPa
 *  
 *  \return Altitude in m, double precision
 *  
 *  \details Calculates the altitude (in meters) from the specified atmospheric pressure (in hPa), and sea-level pressure (in hPa).
 *  \details Formula was taken from BMP180 datasheet | 16
 */ 
double BME280_I2C::altitude_dbl(double seaLevel){
  double atmospheric = pressure_dbl() / 100.0F;
  return 44330.0 * (1.0 - pow(atmospheric / seaLevel, 0.1903));
}