/**
 * @brief Implement the ADXL345 accelerometer communication
 * @author Gilles Henrard
 * @date 29/09/2023
 *
 * @note Additional information can be found in :
 *   - ADXL345 datasheet : https://www.analog.com/media/en/technical-documentation/data-sheets/ADXL345.pdf
 *   - AN-1025 (FIFO application note) document : https://www.analog.com/media/en/technical-documentation/application-notes/AN-1025.pdf
 *
 */
#include "ADXL345.h"
#include "ADXL345registers.h"
#include "main.h"

#define ADXL_TIMEOUT_MS	10			///< SPI direct transmission timeout span in milliseconds

#define ENABLE_SPI	HAL_GPIO_WritePin(ADXL_CS_GPIO_Port, ADXL_CS_Pin, GPIO_PIN_RESET);	///< Macro used to enable the SPI communication towards the accelerometer
#define DISABLE_SPI	HAL_GPIO_WritePin(ADXL_CS_GPIO_Port, ADXL_CS_Pin, GPIO_PIN_SET);	///< Macro used to disable the SPI communication towards the accelerometer

HAL_StatusTypeDef ADXL345readRegister(adxl345Registers_e registerNumber, uint8_t* value);
HAL_StatusTypeDef ADXL345writeRegister(adxl345Registers_e registerNumber, uint8_t value);
HAL_StatusTypeDef ADXL345readRegisters(adxl345Registers_e firstRegister, uint8_t* value, uint8_t size);

SPI_HandleTypeDef* ADXL_spiHandle = NULL;	///< SPI handle used with the ADXL345
volatile uint8_t adxlINT1occurred = 0;
uint8_t buffer[6];
int16_t castedX;
int16_t castedY;
int16_t castedZ;

/**
 * @brief Initialise the ADXL345
 *
 * @param[in] handle SPI handle used
 */
HAL_StatusTypeDef ADXL345initialise(const SPI_HandleTypeDef* handle){
	ADXL_spiHandle = (SPI_HandleTypeDef*)handle;

	ADXL345writeRegister(DATA_FORMAT, ADXL_SPI_4WIRE | ADXL_INT_ACTIV_LOW | ADXL_RANGE_2G);

	//configure the FIFO as to wait for 16 samples before triggering INT1
	ADXL345writeRegister(FIFO_CONTROL, ADXL_MODE_FIFO | ADXL_TRIGGER_INT1 | ADXL_SAMPLES_16);
	ADXL345writeRegister(INTERRUPT_ENABLE, ADXL_INT_WATERMARK);

	//set the ADXL in the measurement mode (to be done last)
	return (ADXL345writeRegister(POWER_CONTROL, ADXL_MEASURE_MODE));
}

uint16_t ADXL345update(){
//	if(adxlINT1occurred){
		adxlINT1occurred = 0;
		ADXL345readRegisters(DATA_X0, buffer, 6);
		castedX = ((uint16_t)(buffer[1]) << 8) | (uint16_t)(buffer[0]);
		castedY = ((uint16_t)(buffer[3]) << 8) | (uint16_t)(buffer[2]);
		castedZ = ((uint16_t)(buffer[5]) << 8) | (uint16_t)(buffer[4]);
//	}
	return (0);
}

/**
 * @brief Read a single register on the ADXL345
 *
 * @param[in] registerNumber Register number
 * @param[out] value Register value
 * @return Return value of SPI transmissions
 */
HAL_StatusTypeDef ADXL345readRegister(adxl345Registers_e registerNumber, uint8_t* value){
	HAL_StatusTypeDef result;
	uint8_t instruction = ADXL_READ | ADXL_SINGLE | registerNumber;

	//if handle not set, error
	if(ADXL_spiHandle == NULL)
		return (HAL_ERROR);

	//if register number above known, error
	if(registerNumber > ADXL_NB_REGISTERS)
		return (HAL_ERROR);

	//if register number between 0x01 and 0x1C included, error
	if((uint8_t)(registerNumber - 1) < ADXL_HIGH_RESERVED_REG)
		return (HAL_ERROR);

	//transmit the read instruction and receive the reply
	ENABLE_SPI
	result = HAL_SPI_Transmit(ADXL_spiHandle, &instruction, 1, ADXL_TIMEOUT_MS);
	if(result == HAL_OK)
		result = HAL_SPI_Receive(ADXL_spiHandle, value, 1, ADXL_TIMEOUT_MS);
	DISABLE_SPI

	return (result);
}

/**
 * @brief Write a single register on the ADXL345
 *
 * @param[in] registerNumber Register number
 * @param[in] value Register value
 * @return Return value of SPI transmissions
 */
HAL_StatusTypeDef ADXL345writeRegister(adxl345Registers_e registerNumber, uint8_t value){
	HAL_StatusTypeDef result;
	uint8_t instruction = ADXL_WRITE | ADXL_SINGLE | registerNumber;

	//if handle not set, error
	if(ADXL_spiHandle == NULL)
		return (HAL_ERROR);

	//if register number above known, error
	if(registerNumber > ADXL_NB_REGISTERS)
		return (HAL_ERROR);

	//if register number between 0x01 and 0x1C included, error
	if((uint8_t)(registerNumber - 1) < ADXL_HIGH_RESERVED_REG)
		return (HAL_ERROR);

	//transmit the read instruction and receive the reply
	ENABLE_SPI
	result = HAL_SPI_Transmit(ADXL_spiHandle, &instruction, 1, ADXL_TIMEOUT_MS);
	if(result == HAL_OK)
		result = HAL_SPI_Transmit(ADXL_spiHandle, &value, 1, ADXL_TIMEOUT_MS);
	DISABLE_SPI

	return (result);
}

/**
 * @brief Read several registers on the ADXL345
 *
 * @param[in] firstRegister Number of the first register to read
 * @param[out] value Registers value array
 * @param[in] size Number of registers to read
 * @return Return value of SPI transmissions
 */
HAL_StatusTypeDef ADXL345readRegisters(adxl345Registers_e firstRegister, uint8_t* value, uint8_t size){
	HAL_StatusTypeDef result;
	uint8_t instruction = ADXL_READ | ADXL_MULTIPLE | firstRegister;

	//if handle not set, error
	if(ADXL_spiHandle == NULL)
		return (HAL_ERROR);

	//if register numbers above known, error
	if(firstRegister > ADXL_NB_REGISTERS)
		return (HAL_ERROR);

	//transmit the read instruction and receive the reply
	ENABLE_SPI
	result = HAL_SPI_Transmit(ADXL_spiHandle, &instruction, 1, ADXL_TIMEOUT_MS);
	if(result == HAL_OK)
		result = HAL_SPI_Receive(ADXL_spiHandle, value, size, ADXL_TIMEOUT_MS);
	DISABLE_SPI

	return (result);
}
