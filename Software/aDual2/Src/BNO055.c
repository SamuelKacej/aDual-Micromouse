
#include "BNO055.h"
#include "main.h"
#include "stm32f4xx_hal.h"

uint8_t deviceAddress = 0x28; // alternative address 0x28, 0x29 or 0x40

void _uDelay(uint16_t micros){
	// get system clock
	volatile uint32_t clk_start = MAIN_GetMicros();
	while ((MAIN_GetMicros() - clk_start) < micros);
}

uint8_t bno_test(I2C_HandleTypeDef* hi2c){

	uint8_t addr = 0x28;
	volatile uint8_t buf=0;
	HAL_StatusTypeDef ret;

	buf = bno055_read8(hi2c, BNO055_SELFTEST_RESULT_ADDR);
     HAL_Delay(1);

	return buf;
}



void bno055_write8(I2C_HandleTypeDef* hi2c1, bno055_reg_t reg, uint8_t data)
{

	uint16_t packet = ((uint16_t)data << 8) | reg;

	 HAL_StatusTypeDef ret = HAL_I2C_Master_Transmit(hi2c1, deviceAddress<<1, &packet, 2, HAL_MAX_DELAY);
    if ( ret != HAL_OK ) {
            printf("I2C error.\n\r");
    }
//	  uint16_t packet = ((uint16_t)ret << 8) | BNO055_UNIT_SEL_ADDR;
//	  HAL_I2C_Master_Transmit(&hi2c1, 0x29<<1, &packet, 2, HAL_MAX_DELAY);
}

uint8_t bno055_read8(I2C_HandleTypeDef* hi2c1, bno055_reg_t reg)
{


    uint8_t buf = reg;
    HAL_StatusTypeDef ret;

    ret = HAL_I2C_Master_Transmit(hi2c1, deviceAddress<<1, &buf, 1, HAL_MAX_DELAY);

    _uDelay(200);

    ret = HAL_I2C_Master_Receive(hi2c1, ((deviceAddress)<<1)+1, &buf, 1, HAL_MAX_DELAY);

    _uDelay(200);

    return buf;

}

uint16_t bno055_read16(I2C_HandleTypeDef* hi2c1, bno055_reg_t reg)
{

	// first it receive register then data
	//uint16_t packet = ((uint16_t)(0x02) << 8) | reg;

	 HAL_StatusTypeDef ret = HAL_I2C_Master_Transmit(hi2c1, deviceAddress<<1, &reg, 1, HAL_MAX_DELAY);
	 if(ret != HAL_OK)
		 printf("%d",ret);
    _uDelay(50);
    uint16_t packet = 0;

    ret = HAL_I2C_Master_Receive(hi2c1, (deviceAddress<<1) +1, &packet, 2, HAL_MAX_DELAY);
    if(ret != HAL_OK)
    		 printf("%d",ret);
    _uDelay(100);
    return packet;
}

void bno055_setOpMode(I2C_HandleTypeDef* hi2c1,  bno055_opmode_t mode){

	bno055_write8(hi2c1, (uint8_t) BNO055_OPR_MODE_ADDR, (uint8_t) mode);

}

void bno055_setPwrMode(I2C_HandleTypeDef* hi2c1, bno055_powermode_t mode){

	bno055_write8(hi2c1, (uint8_t) BNO055_PWR_MODE_ADDR, (uint8_t) mode);

}

short bno055_init(I2C_HandleTypeDef* hi2c1, bno055_opmode_t mode){


	//TODO asi niake uistenie ci device naozaj existuje

	HAL_Delay(2);
	// set normal power mode
	bno055_setPwrMode(hi2c1, POWER_MODE_NORMAL);
	HAL_Delay(2);

	//set to page 0
	bno055_write8(hi2c1, BNO055_PAGE_ID_ADDR, 0x00);
	HAL_Delay(2);

	//configure measurement unit  m/s/s ; r/s ; r ; °C
	uint8_t ret = bno055_read8(hi2c1, BNO055_UNIT_SEL_ADDR);
	HAL_Delay(2);
	ret =  (ret & 0b11101110) | 0b00000110;
	bno055_write8(hi2c1, BNO055_UNIT_SEL_ADDR, ret);
	HAL_Delay(2);

	// reset
	//bno055_write8(hi2c1, BNO055_SYS_TRIGGER_ADDR, 0x00);
	//HAL_Delay(10);

	//setup mode
	bno055_setOpMode(hi2c1, mode);
	HAL_Delay(15);

	return 0;

}

float bno055_getEulerYaw(I2C_HandleTypeDef* hi2c1){
	// Yaw or Heading
	// init set value to radians, so retun is also radians
	// page 35 in datasheet

	return (float)((int16_t )bno055_read16(hi2c1, BNO055_EULER_H_LSB_ADDR ))/900;
}

float bno055_getEulerPitch(I2C_HandleTypeDef* hi2c1){
	// init set value to radians, so retun is also radians
	// page 35 in data sheet

	return (float)((int16_t )bno055_read16(hi2c1, BNO055_EULER_P_LSB_ADDR ))/900;
}

float bno055_getEulerRoll(I2C_HandleTypeDef* hi2c1){
	// init set value to radians, so retun is also radians
	// page 35 in data sheet

	return (float)((int16_t )bno055_read16(hi2c1, BNO055_EULER_R_LSB_ADDR ))/900;
}

float bno055_getGyroX(I2C_HandleTypeDef* hi2c1){
	//  return value is in rad/s, gyro unit must be set to rad/s
	return (float)((int16_t )bno055_read16(hi2c1, BNO055_GYRO_DATA_X_LSB_ADDR ))/900;
}
float bno055_getGyroY(I2C_HandleTypeDef* hi2c1){
	//  return value is in rad/s, gyro unit must be set to rad/s
	return (float)((int16_t )bno055_read16(hi2c1, BNO055_GYRO_DATA_Y_LSB_ADDR ))/900;
}
float bno055_getGyroZ(I2C_HandleTypeDef* hi2c1){
	//  return value is in rad/s, unit in UNIT_SEL_ADDR must be set to rad/s
	return (float)((int16_t )bno055_read16(hi2c1, BNO055_GYRO_DATA_Z_LSB_ADDR ))/900;
}
float bno055_getLinAccelX(I2C_HandleTypeDef* hi2c1){
	//  return value is in m/s/s, unit in UNIT_SEL_ADDR must be set to m/s/s
	return (float)((int16_t )bno055_read16(hi2c1, BNO055_LINEAR_ACCEL_DATA_X_LSB_ADDR ))/100;
}
float bno055_getLinAccelY(I2C_HandleTypeDef* hi2c1){
	//  return value is in m/s/s, unit in UNIT_SEL_ADDR must be set to m/s/s
	return (float)((int16_t )bno055_read16(hi2c1, BNO055_LINEAR_ACCEL_DATA_Y_LSB_ADDR ))/100;
}
float bno055_getLinAccelZ(I2C_HandleTypeDef* hi2c1){
	//  return value is in m/s/s, unit in UNIT_SEL_ADDR must be set to m/s/s
	return (float)((int16_t )bno055_read16(hi2c1, BNO055_LINEAR_ACCEL_DATA_Z_LSB_ADDR ))/100;
}
float bno055_getTemp(I2C_HandleTypeDef* hi2c1){
	//  return value is in °C, unit in UNIT_SEL_ADDR must be set to °C
	return (float)((int8_t )bno055_read8(hi2c1, BNO055_LINEAR_ACCEL_DATA_Z_LSB_ADDR ));
}
