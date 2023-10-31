/**
 * @brief Implement the ADXL345 accelerometer communication
 * @author Gilles Henrard
 * @date 31/10/2023
 *
 * @note Additional information can be found in :
 *   - ADXL345 datasheet : https://www.analog.com/media/en/technical-documentation/data-sheets/ADXL345.pdf
 *   - AN-1025 (FIFO application note) document : https://www.analog.com/media/en/technical-documentation/application-notes/AN-1025.pdf
 *
 */
#include "ADXL345.h"
#include "ADXL345registers.h"
#include "main.h"
#include <math.h>

//definitions
#define ADXL_SPI_TIMEOUT_MS	10U		///< SPI direct transmission timeout span in milliseconds
#define ADXL_INT_TIMEOUT_MS	1000U	///< Maximum number of milliseconds before watermark int. timeout
#define ADXL_ST_WAIT_MS		25U		///< Maximum number of milliseconds before watermark int. timeout
#define ADXL_BYTE_OFFSET	8U		///< Number of bits to offset a byte
#define ADXL_X_INDEX_MSB	1U		///< Index of the X MSB in the measurements
#define ADXL_X_INDEX_LSB	0U		///< Index of the X LSB in the measurements
#define ADXL_Y_INDEX_MSB	3U		///< Index of the Y MSB in the measurements
#define ADXL_Y_INDEX_LSB	2U		///< Index of the Y LSB in the measurements
#define ADXL_Z_INDEX_MSB	5U		///< Index of the Z MSB in the measurements
#define ADXL_Z_INDEX_LSB	4U		///< Index of the Z LSB in the measurements
#define ADXL_NB_REG_INIT	5U		///< Number of registers configured at initialisation
#define ADXL_180_DEG		180.0f	///< Value representing a flat angle

//integration sampling
#define ADXL_AVG_SAMPLES	ADXL_SAMPLES_32
#define ADXL_AVG_SHIFT		5U
#if (ADXL_AVG_SAMPLES >> ADXL_AVG_SHIFT) != 1
#error TADXL_AVG_SHIFT does not divide all the samples configured with ADXL_AVG_NB
#endif

//macros
#define ENABLE_SPI		HAL_GPIO_WritePin(ADXL_CS_GPIO_Port, ADXL_CS_Pin, GPIO_PIN_RESET);	///< Macro used to enable the SPI communication towards the accelerometer
#define DISABLE_SPI		HAL_GPIO_WritePin(ADXL_CS_GPIO_Port, ADXL_CS_Pin, GPIO_PIN_SET);	///< Macro used to disable the SPI communication towards the accelerometer

//type definitions
/**
 * @brief Enumeration of the function IDs of the ADXL345
 */
typedef enum _ADXLfunctionCodes_e{
	INIT = 0,      		///< ADXL345initialise()
	SELF_TESTING_OFF,	///< stSelfTestingOFF()
	SELF_TEST_WAIT,		///< stWaitingForSTchange()
	SELF_TESTING_ON,	///< stSelfTestingON()
	MEASURE,        	///< stMeasuring()
	CHK_MEASURES,  		///< ADXL345hasNewMeasurements()
	WRITE_REGISTER,		///< ADXL345writeRegister()
	READ_REGISTERS,		///< ADXL345readRegisters()
	GET_X_ANGLE,		///< ADXL345getXangleDegrees()
	GET_Y_ANGLE,		///< ADXL345getYangleDegrees()
	STARTUP				///< stStartup()
}ADXLfunctionCodes_e;

/**
 * @brief State machine state prototype
 *
 * @return Error code of the state
 */
typedef errorCode_u (*adxlState)();

//machine state
static errorCode_u stStartup();
static errorCode_u stConfiguring();
static errorCode_u stSelfTestingOFF();
static errorCode_u stWaitingForSTchange();
static errorCode_u stSelfTestingON();
static errorCode_u stMeasuring();
static errorCode_u stError();

//manipulation functions
static errorCode_u ADXL345writeRegister(adxl345Registers_e registerNumber, uint8_t value);
static errorCode_u ADXL345readRegisters(adxl345Registers_e firstRegister, uint8_t* value, uint8_t size);

//tool functions
static inline void divideBy16(int16_t* sum);
static inline float atanDegrees(int16_t direction, int16_t axisZ);

/**
 * @brief Array of all the registers/values to write at initialisation
 * @note Two values are written in FIFO_CONTROL to clear the FIFO at startup
 */
static const uint8_t initialisationArray[ADXL_NB_REG_INIT][2] = {
	{BANDWIDTH_POWERMODE,	ADXL_POWER_NORMAL | ADXL_RATE_200HZ},
	{FIFO_CONTROL,			ADXL_MODE_BYPASS},
	{FIFO_CONTROL,			ADXL_MODE_FIFO | ADXL_TRIGGER_INT1 | (ADXL_AVG_SAMPLES - 1)},
	{INTERRUPT_ENABLE,		ADXL_INT_WATERMARK},
	{POWER_CONTROL,			ADXL_MEASURE_MODE},
};

static const uint8_t dataFormatDefault = ADXL_NO_SELF_TEST | ADXL_SPI_4WIRE | ADXL_INT_ACTIV_LOW | ADXL_RANGE_16G;

//global variables
volatile uint8_t			adxlINT1occurred = 0;			///< Flag used to indicate the ADXL triggered an interrupt
volatile uint16_t			adxlTimer_ms = 0;				///< Timer used in various states of the ADXL (in ms)

//state variables
static SPI_HandleTypeDef*	ADXL_spiHandle = NULL;			///< SPI handle used with the ADXL345
static adxlState			state = stStartup;				///< State machine current state
static uint8_t				adxlMeasurementsUpdated = 0;	///< Flag used to indicate new integrated measurements are ready within the ADXL345
static uint8_t 				buffer[ADXL_NB_DATA_REGISTERS];
static int16_t				finalX = 0;						///< X value obtained after integration
static int16_t				finalY = 0;						///< Y value obtained after integration
static int16_t				finalZ = 0;						///< Z value obtained after integration
static int16_t				finalXSTon = 0;					///< X value obtained after integration
static int16_t				finalYSTon = 0;					///< Y value obtained after integration
static int16_t				finalZSTon = 0;					///< Z value obtained after integration


/********************************************************************************************************************************************/
/********************************************************************************************************************************************/


/**
 * @brief Initialise the ADXL345
 *
 * @param handle SPI handle used
 * @retval 0 Success
 */
errorCode_u ADXL345initialise(const SPI_HandleTypeDef* handle){
	ADXL_spiHandle = (SPI_HandleTypeDef*)handle;
	return (ERR_SUCCESS);
}

/**
 * @brief Run the ADXL state machine
 *
 * @return Current machine state return value
 */
errorCode_u ADXL345update(){
	return ( (*state)() );
}

/**
 * @brief Check if new measurements have been updated
 *
 * @retval 0 No new values available
 * @retval 1 New values are available
 */
uint8_t ADXL345hasNewMeasurements(){
	uint8_t tmp = adxlMeasurementsUpdated;
	adxlMeasurementsUpdated = 0;

	return (tmp);
}

/**
 * @brief Write a single register on the ADXL345
 *
 * @param registerNumber Register number
 * @param value Register value
 * @retval 0 Success
 * @retval 1 No SPI handle set
 * @retval 2 Register number out of range
 * @retval 3 Attempted to access a reserved register
 * @retval 4 Error while writing the command
 * @retval 5 Error while writing the value
 */
errorCode_u ADXL345writeRegister(adxl345Registers_e registerNumber, uint8_t value){
	HAL_StatusTypeDef HALresult;
	errorCode_u result = ERR_SUCCESS;
	uint8_t instruction = ADXL_WRITE | ADXL_SINGLE | registerNumber;

	//if handle not set, error
	if(ADXL_spiHandle == NULL)
		return (createErrorCode(WRITE_REGISTER, 1, ERR_CRITICAL));

	//if register number above known, error
	if(registerNumber > ADXL_NB_REGISTERS)
		return (createErrorCode(WRITE_REGISTER, 2, ERR_WARNING));

	//if register number between 0x01 and 0x1C included, error
	if((uint8_t)(registerNumber - 1) < ADXL_HIGH_RESERVED_REG)
		return (createErrorCode(WRITE_REGISTER, 3, ERR_WARNING)); 	// @suppress("Avoid magic numbers")

	ENABLE_SPI

	//transmit the read instruction
	HALresult = HAL_SPI_Transmit(ADXL_spiHandle, &instruction, 1, ADXL_SPI_TIMEOUT_MS);
	if(HALresult != HAL_OK){
		DISABLE_SPI
		return (createErrorCodeLayer1(WRITE_REGISTER, 4, HALresult, ERR_ERROR)); 	// @suppress("Avoid magic numbers")
	}

	//receive the reply
	HALresult = HAL_SPI_Transmit(ADXL_spiHandle, &value, 1, ADXL_SPI_TIMEOUT_MS);
	if(HALresult != HAL_OK)
		result = createErrorCodeLayer1(WRITE_REGISTER, 5, HALresult, ERR_ERROR); 	// @suppress("Avoid magic numbers")

	DISABLE_SPI
	return (result);
}

/**
 * @brief Read several registers on the ADXL345
 *
 * @param firstRegister Number of the first register to read
 * @param[out] value Registers value array
 * @param size Number of registers to read
 * @retval 0 Success
 * @retval 1 No SPI handle set
 * @retval 2 Register number out of range
 * @retval 3 Error while writing the command
 * @retval 4 Error while reading the values
 */
errorCode_u ADXL345readRegisters(adxl345Registers_e firstRegister, uint8_t* value, uint8_t size){
	HAL_StatusTypeDef HALresult;
	errorCode_u result = ERR_SUCCESS;
	uint8_t instruction = ADXL_READ | ADXL_MULTIPLE | firstRegister;

	//if handle not set, error
	if(ADXL_spiHandle == NULL)
		return (createErrorCode(READ_REGISTERS, 1, ERR_CRITICAL));

	//if register numbers above known, error
	if(firstRegister > ADXL_NB_REGISTERS)
		return (createErrorCode(READ_REGISTERS, 2, ERR_WARNING));

	ENABLE_SPI

	//transmit the read instruction
	HALresult = HAL_SPI_Transmit(ADXL_spiHandle, &instruction, 1, ADXL_SPI_TIMEOUT_MS);
	if(HALresult != HAL_OK){
		DISABLE_SPI
		return (createErrorCodeLayer1(READ_REGISTERS, 3, HALresult, ERR_ERROR)); 	// @suppress("Avoid magic numbers")
	}

	//receive the reply
	HALresult = HAL_SPI_Receive(ADXL_spiHandle, value, size, ADXL_SPI_TIMEOUT_MS);
	if(HALresult != HAL_OK)
		result = createErrorCodeLayer1(READ_REGISTERS, 4, HALresult, ERR_ERROR); 	// @suppress("Avoid magic numbers")

	DISABLE_SPI
	return (result);
}

/**
 * @brief Get the latest measured angle between the X axis and the Z axis
 *
 * @return Angle in degrees
 */
float ADXL345getXangleDegrees(){
	return (atanDegrees(finalX, finalZ));
}

/**
 * @brief Get the latest measured angle between the Y axis and the Z axis
 *
 * @return Angle in degrees
 */
float ADXL345getYangleDegrees(){
	return (atanDegrees(finalY, finalZ));
}

/**
 * @brief Divide a sum of values to compute an average
 *
 * @param[out} value Value to divide
 */
void divideBy16(int16_t* sum){
	*sum >>= ADXL_AVG_SHIFT;
}

/**
 * @brief Compute the angle (in degrees) between any axis and the Z axis
 *
 * @param direction Value (in G) of an axis
 * @param axisZ Value (in G) of the Z axis
 * @return Angle between direction and the Z axis
 */
float atanDegrees(int16_t direction, int16_t axisZ){
	return ((atanf((float)direction / (float)axisZ) * ADXL_180_DEG) / (float)M_PI);
}


/********************************************************************************************************************************************/
/********************************************************************************************************************************************/


/**
 * @brief Begin state of the state machine
 *
 * @retval 0 Success
 * @retval 1 No SPI handle has been specified
 * @retval 2 Unable to read device ID
 * @retval 3 Device ID invalid
 */
errorCode_u stStartup(){
	errorCode_u result;
	uint8_t deviceID = 0;

	//if no handle specified, go error
	if(ADXL_spiHandle == NULL){
		state = stError;
		return (createErrorCode(STARTUP, 1, ERR_CRITICAL));
	}

	//if unable to read device ID, go error
	result = ADXL345readRegisters(DEVICE_ID, &deviceID, 1);
	if(IS_ERROR(result)){
		state = stError;
		return (pushErrorCode(result, STARTUP, 2));
	}

	//if invalid device ID, go error
	if(deviceID != ADXL_DEVICE_ID)
		return (createErrorCode(STARTUP, 3, ERR_CRITICAL)); 	// @suppress("Avoid magic numbers")

	state = stConfiguring;
	return (ERR_SUCCESS);
}

/**
 * @brief State in which the registers of the ADXL are configured
 *
 * @retval 0 Success
 * @retval 1 Error while writing a register
 */
errorCode_u stConfiguring(){
	errorCode_u result;

	result = ADXL345writeRegister(DATA_FORMAT, dataFormatDefault);
	if(IS_ERROR(result)){
		state = stError;
		return (pushErrorCode(result, INIT, 1));
	}

	//write all registers values from the initialisation array
	for(uint8_t i = 0 ; i < ADXL_NB_REG_INIT ; i++){
		result = ADXL345writeRegister(initialisationArray[i][0], initialisationArray[i][1]);
		if(IS_ERROR(result)){
			state = stError;
			return (pushErrorCode(result, INIT, 1));
		}
	}

	adxlTimer_ms = ADXL_INT_TIMEOUT_MS;
	state = stSelfTestingOFF;
	return (result);
}

/**
 * @brief State in which the ADXL goes into self-testing mode
 * @note p. 22 of the datasheet
 *
 * @return Success
 */
static errorCode_u stSelfTestingOFF(){
	errorCode_u result;

	//if timeout, go error
	if(!adxlTimer_ms){
		state = stError;
		return (createErrorCode(SELF_TESTING_OFF, 1, ERR_ERROR));
	}

	//if watermark interrupt not fired, exit
	if(!adxlINT1occurred)
		return (ERR_SUCCESS);

	finalX = finalY = finalZ = 0;

	//for eatch of the 16 samples to read
	for(uint8_t i = 0 ; i < ADXL_AVG_SAMPLES ; i++){
		//read all data registers for 1 sample
		result = ADXL345readRegisters(DATA_X0, buffer, ADXL_NB_DATA_REGISTERS);
		if(IS_ERROR(result)){
			state = stError;
			return (pushErrorCode(result, SELF_TESTING_OFF, 2));
		}

		//add the measurements (formatted from a two's complement) to their final value buffer
		finalX += (int16_t)(((uint16_t)(buffer[ADXL_X_INDEX_MSB]) << ADXL_BYTE_OFFSET) | (uint16_t)(buffer[ADXL_X_INDEX_LSB]));
		finalY += (int16_t)(((uint16_t)(buffer[ADXL_Y_INDEX_MSB]) << ADXL_BYTE_OFFSET) | (uint16_t)(buffer[ADXL_Y_INDEX_LSB]));
		finalZ += (int16_t)(((uint16_t)(buffer[ADXL_Z_INDEX_MSB]) << ADXL_BYTE_OFFSET) | (uint16_t)(buffer[ADXL_Z_INDEX_LSB]));
	}

	//divide the buffers by 16
	divideBy16(&finalX);
	divideBy16(&finalY);
	divideBy16(&finalZ);

	result = ADXL345writeRegister(DATA_FORMAT, dataFormatDefault | ADXL_SELF_TEST);
	if(IS_ERROR(result)){
		state = stError;
		return (pushErrorCode(result, SELF_TESTING_OFF, 3)); 	// @suppress("Avoid magic numbers")
	}

	result = ADXL345writeRegister(FIFO_CONTROL, ADXL_MODE_BYPASS);
	if(IS_ERROR(result)){
		state = stError;
		return (pushErrorCode(result, SELF_TESTING_OFF, 4)); 	// @suppress("Avoid magic numbers")
	}

	adxlTimer_ms = ADXL_ST_WAIT_MS;
	state = stWaitingForSTchange;
	return (ERR_SUCCESS);
}

static errorCode_u stWaitingForSTchange(){
	errorCode_u result;

	if(!adxlTimer_ms)
		return (ERR_SUCCESS);

	adxlINT1occurred = 0;
	result = ADXL345writeRegister(FIFO_CONTROL, ADXL_MODE_FIFO | ADXL_TRIGGER_INT1 | (ADXL_AVG_SAMPLES - 1));
	if(IS_ERROR(result)){
		state = stError;
		return (pushErrorCode(result, SELF_TEST_WAIT, 1)); 	// @suppress("Avoid magic numbers")
	}

	adxlTimer_ms = ADXL_INT_TIMEOUT_MS;
	state = stSelfTestingON;
	return (ERR_SUCCESS);
}

/**
 * @brief State in which the ADXL goes into self-testing mode
 * @note p. 22 of the datasheet
 *
 * @return Success
 */
static errorCode_u stSelfTestingON(){
	errorCode_u result;

	//if timeout, go error
	if(!adxlTimer_ms){
		state = stError;
		return (createErrorCode(SELF_TESTING_ON, 1, ERR_ERROR));
	}

	//if watermark interrupt not fired, exit
	if(!adxlINT1occurred)
		return (ERR_SUCCESS);

	adxlINT1occurred = 0;
	finalXSTon = finalYSTon = finalZSTon = 0;

	//for eatch of the 16 samples to read
	for(uint8_t i = 0 ; i < ADXL_AVG_SAMPLES ; i++){
		//read all data registers for 1 sample
		result = ADXL345readRegisters(DATA_X0, buffer, ADXL_NB_DATA_REGISTERS);
		if(IS_ERROR(result)){
			state = stError;
			return (pushErrorCode(result, SELF_TESTING_ON, 2));
		}

		//add the measurements (formatted from a two's complement) to their final value buffer
		finalXSTon += (int16_t)(((uint16_t)(buffer[ADXL_X_INDEX_MSB]) << ADXL_BYTE_OFFSET) | (uint16_t)(buffer[ADXL_X_INDEX_LSB]));
		finalYSTon += (int16_t)(((uint16_t)(buffer[ADXL_Y_INDEX_MSB]) << ADXL_BYTE_OFFSET) | (uint16_t)(buffer[ADXL_Y_INDEX_LSB]));
		finalZSTon += (int16_t)(((uint16_t)(buffer[ADXL_Z_INDEX_MSB]) << ADXL_BYTE_OFFSET) | (uint16_t)(buffer[ADXL_Z_INDEX_LSB]));
	}

	//divide the buffers by 16
	divideBy16(&finalXSTon);
	divideBy16(&finalYSTon);
	divideBy16(&finalZSTon);

	result = ADXL345writeRegister(DATA_FORMAT, dataFormatDefault);
	if(IS_ERROR(result)){
		state = stError;
		return (pushErrorCode(result, SELF_TESTING_ON, 3)); 	// @suppress("Avoid magic numbers")
	}

	finalXSTon -= finalX;
	finalYSTon -= finalY;
	finalZSTon -= finalZ;

	if((finalXSTon <= ADXL_ST_MINX_33_16G) || (finalXSTon >= ADXL_ST_MAXX_33_16G)
		|| (finalYSTon <= ADXL_ST_MINY_33_16G) || (finalYSTon >= ADXL_ST_MAXY_33_16G)
		|| (finalZSTon <= ADXL_ST_MINZ_33_16G) || (finalZSTon >= ADXL_ST_MAXZ_33_16G))
	{
		state = stError;
		return (pushErrorCode(result, SELF_TESTING_ON, 4)); 	// @suppress("Avoid magic numbers")
	}



	adxlTimer_ms = ADXL_INT_TIMEOUT_MS;
	state = stMeasuring;
	return (ERR_SUCCESS);
}

/**
 * @brief State in which the ADXL measures accelerations
 *
 * @retval 0 Success
 * @retval 1 Timeout occurred while waiting for watermark interrupt
 * @retval 2 Error occurred while reading the axis values registers
 */
static errorCode_u stMeasuring(){
	errorCode_u result;

	//if timeout, go error
	if(!adxlTimer_ms){
		state = stError;
		return (createErrorCode(MEASURE, 1, ERR_ERROR));
	}

	//if watermark interrupt not fired, exit
	if(!adxlINT1occurred)
		return (ERR_SUCCESS);

	adxlTimer_ms = ADXL_INT_TIMEOUT_MS;
	adxlINT1occurred = 0;
	finalX = finalY = finalZ = 0;

	//for eatch of the 16 samples to read
	for(uint8_t i = 0 ; i < ADXL_AVG_SAMPLES ; i++){
		//read all data registers for 1 sample
		result = ADXL345readRegisters(DATA_X0, buffer, ADXL_NB_DATA_REGISTERS);
		if(IS_ERROR(result)){
			state = stError;
			return (pushErrorCode(result, MEASURE, 2));
		}

		//add the measurements (formatted from a two's complement) to their final value buffer
		finalX += (int16_t)(((uint16_t)(buffer[ADXL_X_INDEX_MSB]) << ADXL_BYTE_OFFSET) | (uint16_t)(buffer[ADXL_X_INDEX_LSB]));
		finalY += (int16_t)(((uint16_t)(buffer[ADXL_Y_INDEX_MSB]) << ADXL_BYTE_OFFSET) | (uint16_t)(buffer[ADXL_Y_INDEX_LSB]));
		finalZ += (int16_t)(((uint16_t)(buffer[ADXL_Z_INDEX_MSB]) << ADXL_BYTE_OFFSET) | (uint16_t)(buffer[ADXL_Z_INDEX_LSB]));
	}

	//divide the buffers by 16
	divideBy16(&finalX);
	divideBy16(&finalY);
	divideBy16(&finalZ);

	adxlMeasurementsUpdated = 1;
	return (ERR_SUCCESS);
}

/**
 * @brief State in which the ADXL stays in an error state forever
 *
 * @return Success
 */
errorCode_u stError(){
	return (ERR_SUCCESS);
}
