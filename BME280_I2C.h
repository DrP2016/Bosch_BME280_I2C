/**
 *  \file BME280_I2C.h
 *  \brief BOSCH BME280 Sensor Library. I2C ONLY !
 *  
 *  \details Library Definition File
 *  \details 
 *  \details Written by Pascal Droege (GER) for private use.  
 *  \details BSD license, all text above must be included in any redistribution
 */

 #ifndef __BME280_H__
#define __BME280_H__

#include "Arduino.h"
#include <Wire.h>

/***********************************************************************
 *  BME280 default I2C Address
 **********************************************************************/
#define BME280_ADDRESS              	0x77

/***********************************************************************
 *  BME280 OSRS Settings
 *  Controls the oversampling for measurements. See Table.
 ***********************************************************************		 
	osrs_X	|	coeff
	--------+--------
	 0b000	|	SKIPPED
	 0b001	|	1
	 0b010	|	2
	 0b011	|	4
	 0b100	|	8
	>0b101	|	16
 **********************************************************************/
#define BME280_OSRS_T					0b010
#define BME280_OSRS_P					0b101
#define BME280_OSRS_H					0b001

/***********************************************************************
 *  BME280 T_SB Settings
 *  Controls inactive duration 't_standby' in normal mode. See Table.
 ***********************************************************************		 
	t_sb	|	ms
	--------+--------
	 0b000	|	0.5
	 0b001	|	62.5
	 0b010	|	125
	 0b011	|	250
	 0b100	|	500
	 0b101	|	1000
	 0b110	|	10
	 0b111	|	20
 **********************************************************************/
#define BME280_T_SB						0b000

/***********************************************************************
 *  BME280 FILTER Settings
 *  Controls the time constant of the IIR filter. See Table.
 ***********************************************************************		 
	filter	|	coeff
	--------+--------
	 0b000	|	OFF
	 0b001	|	2
	 0b010	|	4
	 0b011	|	8
	>0b100	|	16
 **********************************************************************/
#define BME280_FILTER					0b100

/***********************************************************************
 *  BME280 REGISTERS
 **********************************************************************/
enum{
	BME280_REGISTER_DIG_T1             = 0x88,
	BME280_REGISTER_DIG_T2             = 0x8A,
	BME280_REGISTER_DIG_T3             = 0x8C,

	BME280_REGISTER_DIG_P1             = 0x8E,
	BME280_REGISTER_DIG_P2             = 0x90,
	BME280_REGISTER_DIG_P3             = 0x92,
	BME280_REGISTER_DIG_P4             = 0x94,
	BME280_REGISTER_DIG_P5             = 0x96,
	BME280_REGISTER_DIG_P6             = 0x98,
	BME280_REGISTER_DIG_P7             = 0x9A,
	BME280_REGISTER_DIG_P8             = 0x9C,
	BME280_REGISTER_DIG_P9             = 0x9E,

	BME280_REGISTER_DIG_H1             = 0xA1,
	BME280_REGISTER_DIG_H2             = 0xE1,
	BME280_REGISTER_DIG_H3             = 0xE3,
	BME280_REGISTER_DIG_H4             = 0xE4,
	BME280_REGISTER_DIG_H5             = 0xE5,
	BME280_REGISTER_DIG_H6             = 0xE7,

	BME280_REGISTER_CHIPID             = 0xD0,
	BME280_REGISTER_VERSION            = 0xD1,
	BME280_REGISTER_SOFTRESET          = 0xE0,

	BME280_REGISTER_CAL26              = 0xE1,  // R calibration stored in 0xE1-0xF0

	BME280_REGISTER_CONTROLHUMID       = 0xF2,
	BME280_REGISTER_CONTROL            = 0xF4,
	BME280_REGISTER_CONFIG             = 0xF5,
	BME280_REGISTER_PRESSUREDATA       = 0xF7,
	BME280_REGISTER_TEMPDATA           = 0xFA,
	BME280_REGISTER_HUMIDDATA          = 0xFD,
};

/***********************************************************************
 *  BME280 CALIBRATION DATA
 **********************************************************************/
typedef struct{
	uint16_t dig_T1;
	int16_t  dig_T2;
	int16_t  dig_T3;

	uint16_t dig_P1;
	int16_t  dig_P2;
	int16_t  dig_P3;
	int16_t  dig_P4;
	int16_t  dig_P5;
	int16_t  dig_P6;
	int16_t  dig_P7;
	int16_t  dig_P8;
	int16_t  dig_P9;

	uint8_t  dig_H1;
	int16_t  dig_H2;
	uint8_t  dig_H3;
	int16_t  dig_H4;
	int16_t  dig_H5;
	int8_t   dig_H6;
} BME280_CALIB_DATA;

class BME280_I2C{	
	public:

		BME280_I2C(void);

		bool     begin(uint8_t addr = BME280_ADDRESS);
		
		void 	 sleep(void);
		void 	 forced(void);
		void 	 normal(void);
		
		void 	 filter_config(	uint8_t t_sb   = BME280_T_SB,
								uint8_t filter = BME280_FILTER	);
		void 	 filter_write(void);
		
		void 	 osrs_config(	uint8_t osrs_p = BME280_OSRS_P,
								uint8_t osrs_t = BME280_OSRS_T,								
								uint8_t osrs_h = BME280_OSRS_H	);
		void 	 osrs_mode_write(uint8_t mode);
		
		void 	 read_adc_burst(void);
		void 	 read_adc_single(void);
		
		uint32_t compensate_P_int32(int32_t adc_P);
		uint32_t compensate_P_int64(int32_t adc_P);
		double   compensate_P_double(int32_t adc_P);
		
		int32_t  compensate_T_int32(int32_t adc_T);
		double   compensate_T_double(int32_t adc_T);
		
		uint32_t compensate_H_int32(int32_t adc_H);
		double   compensate_H_double(int32_t adc_H);
		
		int32_t  pressure(void);
		int32_t  pressure_i64(void);
		double   pressure_dbl(void);
		
		int32_t  temperature(void);
		double   temperature_dbl(void);
		
		int32_t  humidity(void);
		double 	 humidity_dbl(void);
		
		double 	 altitude_dbl(double seaLevel);

	private:
		void 	  read_coeff(void);
		
		void 	  read_data_burst(void);		
		
		void 	  read_adc_P(void);
		void 	  read_adc_T(void);
		void 	  read_adc_H(void);

		uint8_t   readU8(byte reg);			// Unsigned
		int8_t    readS8(byte reg);			// Signed
		void      writeU8(byte reg, byte value);
		
		uint16_t  readU16(byte reg);		// Unsigned
		int16_t   readS16(byte reg);		// Signed
		uint16_t  readU16_LE(byte reg); 	// little endian
		int16_t   readS16_LE(byte reg); 	// little endian
		
		uint32_t  readU24(byte reg);		// Unsigned
		int32_t   readS24(byte reg);		// Signed
		uint32_t  readU24_LE(byte reg); 	// little endian
		int32_t   readS24_LE(byte reg); 	// little endian

		uint8_t   _i2caddr;
		int32_t   _sensorID;
		
		int32_t  _adc_P;
		int32_t  _adc_T;
		int32_t  _adc_H;
		int32_t   _t_fine;
		
		uint8_t  _osrs_p;
		uint8_t  _osrs_t;
		uint8_t  _osrs_h;
		
		uint8_t  _t_sb;
		uint8_t  _filter;	

		BME280_CALIB_DATA _bme280_calib;
};

#endif
