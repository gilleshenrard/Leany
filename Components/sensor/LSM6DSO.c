/**
 * @file LSM6DSO.c
 * @brief Implement the LSM6DSO MEMS sensor communication
 * @author Gilles Henrard
 * @date 05/08/2024
 *
 * @note Additional information can be found in :
 *   - Datasheet : https://www.st.com/resource/en/datasheet/lsm6dso.pdf
 *   - AN5192 (always-on 3-axis accelerometer and 3-axis gyroscope) : https://www.st.com/resource/en/application_note/an5192-lsm6dso-alwayson-3axis-accelerometer-and-3axis-gyroscope-stmicroelectronics.pdf
 *   - AN5226 (Finite State Machine) : https://www.st.com/resource/en/application_note/an5226-lsm6dso-finite-state-machine-stmicroelectronics.pdf
 *   - DT0058 (Design tip) : https://www.st.com/resource/en/design_tip/dt0058-computing-tilt-measurement-and-tiltcompensated-ecompass-stmicroelectronics.pdf
 */
#include "LSM6DSO.h"
#include <math.h>
#include <stdint.h>
#include "LSM6DSO_registers.h"
#include "errorstack.h"
#include "main.h"
#include "stm32f103xb.h"
#include "stm32f1xx_ll_gpio.h"
#include "stm32f1xx_ll_spi.h"
#include "systick.h"

#define ANGLE_DELTA_MINIMUM       0.05F        ///< Minimum value for angle differences to be noticed
#define RADIANS_TO_DEGREES_TENTHS 572.957795F  ///< Ratio between radians and tenths of degrees (= 10 * (180°/PI))
#define BASE_TEMPERATURE          25.0F        ///< Temperature at which the LSM6DSO temperature reading will give 0
enum {
    BOOT_TIME_MS         = 10U,                         ///< Number of milliseconds to wait for the MEMS to boot
    SPI_TIMEOUT_MS       = 10U,                         ///< Number of milliseconds beyond which SPI is in timeout
    TIMEOUT_MS           = 1000U,                       ///< Max number of milliseconds to wait for the device ID
    REGISTER_VALUE_ALIGN = 8,                           ///< Alignment of the registerValue_t struct
    NB_REGISTERS_TO_READ = LSM6_NB_OUT_REGISTERS + 2U,  ///< Numbers of data registers to read
    NB_INIT_REG          = 9U,                          ///< Number of initialisation registers
};

/**
 * @brief Enumeration of all the function ID used in errors
 */
typedef enum {
    READ_REGISTERS = 1,  ///< readRegisters() function
    WRITE_REGISTER,      ///< writeRegister() function
    CHECK_DEVICE_ID,     ///< stateWaitingDeviceID() state
    CONFIGURING,         ///< stateConfiguring() state
    DROPPING,            ///< stateIgnoringSamples() state
    MEASURING,           ///< stMeasuring() state
} LSM6DSOfunction_e;

/**
 * @brief Structure representing a value to write at a specific register
 */
typedef struct {
    LSM6DSOregister_e registerID;  ///< Register ID to which write the value
    uint8_t           value;       ///< Value to write
} __attribute__((aligned(REGISTER_VALUE_ALIGN))) registerValue_t;

/**
 * @brief Union regrouping 8-bits and 16-bits arrays
 * @details
 *  This allows reading all the accelerometer and gyroscope values at once, and convert them to 16 bit instantly
 */
typedef union {
    uint8_t registers8bits[NB_REGISTERS_TO_READ];                   ///< 8-bits registers array
    int16_t values16bits[((uint8_t)(NB_REGISTERS_TO_READ) >> 1U)];  ///< 16-bits values array
} rawValues_u;

/**
 * @brief State machine state prototype
 *
 * @return Error code of the state
 */
typedef errorCode_u (*lsm6dsoState)();

//machine state
static errorCode_u stateWaitingBoot();
static errorCode_u stateWaitingDeviceID();
static errorCode_u stateConfiguring();
static errorCode_u stateIgnoringSamples();
static errorCode_u stateMeasuring();
static errorCode_u stateHoldingValues();
static errorCode_u stateError();

//registers read/write functions
static errorCode_u writeRegister(LSM6DSOregister_e registerNumber, uint8_t value);
static errorCode_u readRegisters(LSM6DSOregister_e firstRegister, uint8_t value[], uint8_t size);

static inline uint8_t dataReady(void);
static void           complementaryFilter(const float accelerometer_mG[], const float gyroscope_radps[],
                                          float filteredAngles_rad[]);

//global variables
static systick_t lsm6dsoTimer_ms = 0;  ///< Timer used in various states of the LSM6DSO (in ms)

//state variables
static SPI_TypeDef* spiHandle                   = (void*)0;          ///< SPI handle used by the LSM6DSO device
static lsm6dsoState state                       = stateWaitingBoot;  ///< State machine current state
static uint8_t     accelerometerSamplesToIgnore = 0;  ///< Number of samples to ignore after change of ODR or power mode
static errorCode_u result;                            ///< Variables used to store error codes
static float       anglesAtZeroing_rad[NB_AXIS];      ///< Accelerometer values at time of zeroing in [m/s²]
static float       latestAngles_rad[NB_AXIS - 1] = {0.0F, 0.0F};      ///< Latest angles measured and filtered in [rad]
float              temperature_degC              = BASE_TEMPERATURE;  ///< Temperature of the LSM6DSO in [°C]

/********************************************************************************************************************************************/
/********************************************************************************************************************************************/

/**
 * @brief Initialise the LSM6DSO
 *
 * @param handle	SPI handle used
 * @returns 		Success
 */
errorCode_u lsm6dsoInitialise(const SPI_TypeDef* handle) {
    spiHandle = (SPI_TypeDef*)handle;
    LL_SPI_Disable(spiHandle);

    return (ERR_SUCCESS);
}

/**
 * @brief Run the LSM6DSO state machine
 * @returns Current state return code
 */
errorCode_u lsm6dsoUpdate() {
    return ((*state)());
}

/**
 * @brief Read several registers on the LSM6DSO
 *
 * @param firstRegister Number of the first register to read
 * @param[out] value Registers value array
 * @param size Number of registers to read
 * @return   Success
 * @retval 1 SPI handle or value buffer NULL
 * @retval 2 Timeout
 */
static errorCode_u readRegisters(LSM6DSOregister_e firstRegister, uint8_t value[], uint8_t size) {
    static const uint8_t SPI_RX_FILLER = 0xFFU;  ///< Value to send as a filler while receiving multiple bytes

    //if no bytes to read, success
    if(!size) {
        return ERR_SUCCESS;
    }

    //make sure neither the handle nor the buffer are NULL
    if(!spiHandle || !value) {
        return (createErrorCode(READ_REGISTERS, 1, ERR_CRITICAL));
    }

    //set timeout timer and enable SPI
    systick_t lsm6dsoSPITimer_ms = getSystick();
    LL_SPI_Enable(spiHandle);
    uint8_t* iterator = value;

    //send the read request and ignore the first byte received (reply to the write request)
    LL_SPI_TransmitData8(spiHandle, LSM6_READ | (uint8_t)firstRegister);
    while((!LL_SPI_IsActiveFlag_RXNE(spiHandle)) && !isTimeElapsed(lsm6dsoSPITimer_ms, SPI_TIMEOUT_MS)) {};
    *iterator = LL_SPI_ReceiveData8(spiHandle);

    //receive the bytes to read
    do {
        //send a filler byte to keep the SPI clock running, to receive the next byte
        LL_SPI_TransmitData8(spiHandle, SPI_RX_FILLER);

        //wait for data to be available, and read it
        while((!LL_SPI_IsActiveFlag_RXNE(spiHandle)) && lsm6dsoSPITimer_ms) {};
        *iterator = LL_SPI_ReceiveData8(spiHandle);

        iterator++;
        size--;
    } while(size && !isTimeElapsed(lsm6dsoSPITimer_ms, SPI_TIMEOUT_MS));

    //wait for transaction to be finished and clear Overrun flag
    while(LL_SPI_IsActiveFlag_BSY(spiHandle) && !isTimeElapsed(lsm6dsoSPITimer_ms, SPI_TIMEOUT_MS)) {};
    LL_SPI_ClearFlag_OVR(spiHandle);

    //disable SPI
    LL_SPI_Disable(spiHandle);

    //if timeout, error
    if(isTimeElapsed(lsm6dsoSPITimer_ms, SPI_TIMEOUT_MS)) {
        return (createErrorCode(READ_REGISTERS, 2, ERR_WARNING));
    }

    return (ERR_SUCCESS);
}

/**
 * @brief Write a single register on the LSM6DSO
 *
 * @param registerNumber Register number
 * @param value Register value
 * @return	 Success
 * @retval 1 No SPI handle specified
 * @retval 2 Register number out of range
 * @retval 3 Timeout
 */
static errorCode_u writeRegister(LSM6DSOregister_e registerNumber, uint8_t value) {
    //if handle not specified, error
    if(!spiHandle) {
        return (createErrorCode(WRITE_REGISTER, 1, ERR_WARNING));
    }

    //if register number above known or within the reserved range, error
    if(registerNumber > MAX_REGISTER) {
        return (createErrorCode(WRITE_REGISTER, 2, ERR_WARNING));
    }

    //set timeout timer and enable SPI
    systick_t lsm6dsoSPITimer_ms = getSystick();
    LL_SPI_Enable(spiHandle);

    //send the write instruction
    LL_SPI_TransmitData8(spiHandle, LSM6_WRITE | (uint8_t)registerNumber);

    //wait for TX buffer to be ready and send value to write
    while(!LL_SPI_IsActiveFlag_TXE(spiHandle) && !isTimeElapsed(lsm6dsoSPITimer_ms, SPI_TIMEOUT_MS)) {};
    if(!isTimeElapsed(lsm6dsoSPITimer_ms, SPI_TIMEOUT_MS)) {
        LL_SPI_TransmitData8(spiHandle, value);
    }

    //wait for transaction to be finished and clear Overrun flag
    while(LL_SPI_IsActiveFlag_BSY(spiHandle) && !isTimeElapsed(lsm6dsoSPITimer_ms, SPI_TIMEOUT_MS)) {};
    LL_SPI_ClearFlag_OVR(spiHandle);

    //disable SPI
    LL_SPI_Disable(spiHandle);

    //if timeout, error
    if(isTimeElapsed(lsm6dsoSPITimer_ms, SPI_TIMEOUT_MS)) {
        return (createErrorCode(WRITE_REGISTER, 3, ERR_WARNING));
    }

    return (ERR_SUCCESS);
}

/**
 * @brief Check if angle measurements have changed
 *
 * @param axis Axis to check for a change
 * @retval 0 No new values available
 * @retval 1 New values are available
 */
uint8_t lsm6dsoHasChanged(axis_e axis) {
    static float previousAngles_rad[NB_AXIS - 1] = {0.0F, 0.0F};
    uint8_t      comparison                      = 0;

    if(fabsf(latestAngles_rad[axis] - previousAngles_rad[axis]) > ANGLE_DELTA_MINIMUM) {
        previousAngles_rad[axis] = latestAngles_rad[axis];
        comparison               = 1;
    }

    return (comparison);
}

/**
 * @brief Transpose a measurement to an angle in tenths of degrees with the Z axis
 *
 * @param axis Axis for which get the angle with the Z axis
 * @return Angle with the Z axis
 */
int16_t getAngleDegreesTenths(axis_e axis) {
    return ((int16_t)((latestAngles_rad[axis] + anglesAtZeroing_rad[axis]) * RADIANS_TO_DEGREES_TENTHS));
}

/**
 * @brief Set the measurements in relative mode and zero down the values
 */
void lsm6dsoZeroDown(void) {
    anglesAtZeroing_rad[X_AXIS] = -latestAngles_rad[X_AXIS];
    anglesAtZeroing_rad[Y_AXIS] = -latestAngles_rad[Y_AXIS];
}

/**
 * @brief Set the measurements in absolute mode (no zeroing compensation)
 */
void lsm6dsoCancelZeroing(void) {
    for(uint8_t axis = 0; axis < (uint8_t)NB_AXIS; axis++) {
        anglesAtZeroing_rad[axis] = 0;
    }
}

/**
 * @brief Hold/release the MEMS by either turning the accelerometer/gyroscope off or by reconfiguring them
 * 
 * @param toHold Either 1 to hold the latest values or 0 to start taking measurements again
 * @return Success
 * @retval 1 Error while sending shut down instructions
 */
errorCode_u lsm6dsoHold(uint8_t toHold) {
    const registerValue_t configurationArray[2] = {
        {CTRL1_XL, LSM6_POWER_DOWN}, //set accelerometer in power down mode
        { CTRL2_G, LSM6_POWER_DOWN}, //set gyroscope in power down mode
    };

    //if holding but already held, or releasing but not yet held : nothing to do
    if(!(toHold ^ (uint8_t)(state == stateHoldingValues))) {
        return (ERR_SUCCESS);
    }

    if(toHold) {
        //write all registers values from the configurationArray array
        for(uint8_t i = 0; i < 2; i++) {
            result = writeRegister(configurationArray[i].registerID, configurationArray[i].value);
            if(isError(result)) {
                state = stateError;
                return (pushErrorCode(result, CONFIGURING, 1));
            }
        }

        state = stateHoldingValues;

    } else {
        //back to configuring state
        state = stateConfiguring;
    }

    return (ERR_SUCCESS);
}

/**
 * @brief Compute a complementary filter on accelerometer/gyroscope values
 * 
 * @param[in] accelerometer_mG    Array of acceleration values in [mG] on all axis
 * @param[in] gyroscope_radps       Array of gyroscope values  in [rad/s] on X and Y axis
 * @param[out] filteredAngles_rad     Array of final angle values in [rad] on X and Y axis
 */
//NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
void complementaryFilter(const float accelerometer_mG[], const float gyroscope_radps[], float filteredAngles_rad[]) {
    const float alpha                 = 0.02F;  ///< Proportion applied to the gyro. and accel. in the final result
    const float dtPeriod_sec          = 0.00240385F;  ///< Time period between two updates (LSM6DSO config. at 416Hz)
    const float GRAVITATION_MG        = 1000.0F;      ///< Grativation value in mG
    float       AccelEstimatedX_rad   = 0.0F;         ///< Estimated accelerator angle on the X axis in [rad]
    float       AccelEstimatedY_rad   = 0.0F;         ///< Estimated accelerator angle on the Y axis in [rad]
    float       eulerAngleRateX_radps = 0.0F;  ///< Euler angle rate (with reference to Earth) around X axis in rad/s
    float       eulerAngleRateY_radps = 0.0F;  ///< Euler angle rate (with reference to Earth) around Y axis in rad/s

    //calculate the accelerometer angle estimations in °
    AccelEstimatedX_rad = asinf(accelerometer_mG[X_AXIS] / GRAVITATION_MG);
    AccelEstimatedY_rad = atanf(accelerometer_mG[Y_AXIS] / accelerometer_mG[Z_AXIS]);

    //Transform gyroscope rates (reference is the solid body) to Euler rates (reference is Earth)
    eulerAngleRateX_radps =
        gyroscope_radps[X_AXIS]
        + (sinf(filteredAngles_rad[X_AXIS]) * tanf(filteredAngles_rad[Y_AXIS]) * gyroscope_radps[Y_AXIS])
        + (cosf(filteredAngles_rad[X_AXIS]) * tanf(filteredAngles_rad[Y_AXIS]) * gyroscope_radps[Z_AXIS]);

    eulerAngleRateY_radps = (cosf(filteredAngles_rad[X_AXIS]) * gyroscope_radps[Y_AXIS])
                            - (sinf(filteredAngles_rad[X_AXIS]) * gyroscope_radps[Z_AXIS]);

    //combine accelerometer estimates with Euler angle rates estimates
    filteredAngles_rad[X_AXIS] =
        ((1.0F - alpha) * (filteredAngles_rad[X_AXIS] + (eulerAngleRateX_radps * dtPeriod_sec)))
        + (alpha * AccelEstimatedX_rad);
    filteredAngles_rad[Y_AXIS] =
        ((1.0F - alpha) * (filteredAngles_rad[Y_AXIS] + (eulerAngleRateY_radps * dtPeriod_sec)))
        + (alpha * AccelEstimatedY_rad);
}

/**
 * @brief Check if an INT1 event occurred
 * 
 * @retval 0 INT1 did not occur
 * @retval 1 INT1 occurred
 */
static inline uint8_t dataReady(void) {
    return (uint8_t)LL_GPIO_IsInputPinSet(LSM6DSO_INT1_GPIO_Port, LSM6DSO_INT1_Pin);
}

/********************************************************************************************************************************************/
/********************************************************************************************************************************************/

/**
 * @brief State in which the program waits for the LSM6DSO to boot up
 * 
 * @return Success
 */
static errorCode_u stateWaitingBoot() {
    //if timer elapsed, reset it and get to next state
    if(isTimeElapsed(lsm6dsoTimer_ms, BOOT_TIME_MS)) {
        lsm6dsoTimer_ms = getSystick();
        state           = stateWaitingDeviceID;
    }

    return (ERR_SUCCESS);
}

/**
 * @brief State during which the Who Am I ID is checked
 * @attention This takes the 10ms boot time into account
 * @note If no correct manufacturer ID is read within 1 second, a timeout occurs
 * 
 * @retval 0 Success
 * @retval 1 Timeout while reading the manufacturer ID
 * @retval 2 Error while sending the read request
 */
static errorCode_u stateWaitingDeviceID() {
    uint8_t deviceID = 0;

    //if 1s elapsed without reading the correct vendor ID, go error
    if(isTimeElapsed(lsm6dsoTimer_ms, TIMEOUT_MS)) {
        state = stateError;
        return (createErrorCode(CHECK_DEVICE_ID, 1, ERR_CRITICAL));
    }

    //if unable to read device ID, error
    result = readRegisters(WHO_AM_I, &deviceID, 1);
    if(isError(result)) {
        return (pushErrorCode(result, CHECK_DEVICE_ID, 2));
    }

    //if invalid device ID, exit
    if(deviceID != LSM6_WHOAMI) {
        return (ERR_SUCCESS);
    }

    //reset timeout timer and get to next state
    state = stateConfiguring;
    return (ERR_SUCCESS);
}

/**
 * @brief State in which the registers are configured in the LSM6DSO
 * 
 * @retval 0 Success
 * @retval 1 Error while writing a register
 */
static errorCode_u stateConfiguring() {
    const uint8_t         AXL_SAMPLES_TO_IGNORE = 2U;  ///< Number of samples to drop (see stateIgnoringSamples())
    const registerValue_t initialisationArray[NB_INIT_REG] = {
        {   CTRL3_C, LSM6_SOFTWARE_RESET | LSM6_INT_ACTIVE_LOW}, //reboot MEMS memory and reset software
        {FIFO_CTRL4,                          FIFO_MODE_BYPASS}, //disable the FIFO (bypass mode)
        { INT1_CTRL,                         INT1_AXL_DATA_RDY}, //enable the accelerometer DATA READY interrupt on INT1
        {  CTRL8_XL,         AXL_NO_HP_FILTER | AXL_LPF2_ODR_4}, //disable accererometer HP filter and set LP2 cutoff to ODR/4
        {  CTRL1_XL,     LSM6_ODR_416HZ | LSM6_AXL_LPF2_ENABLE}, //set accelerometer in high-perf. mode + enable LPF 2
        {   CTRL7_G,     GYR_HPF_ENABLE | GYR_HPF_CUTOFF_65MHZ}, //enable the gyroscope HP filter with 16mHz cutoff freq.
        {   CTRL4_C,                           GYR_LPF1_ENABLE}, //enable the gyroscope LP1 filter
        {   CTRL6_C,                   GYR_LPF1_CUTOFF_120_3HZ}, //set the gyroscope LPF1 cutoff frequency to 136.6Hz
        {   CTRL2_G,           LSM6_ODR_416HZ | GYR_FS_125_DPS}, //set the gyroscope in high-performance mode and sens. to 125dps
    };

    //write all registers values from the initialisation array
    for(uint8_t i = 0; i < (uint8_t)NB_INIT_REG; i++) {
        result = writeRegister(initialisationArray[i].registerID, initialisationArray[i].value);
        if(isError(result)) {
            state = stateError;
            return (pushErrorCode(result, CONFIGURING, 1));
        }
    }

    //set the number of samples to ignore after changing ODR and power mode
    accelerometerSamplesToIgnore = AXL_SAMPLES_TO_IGNORE;

    lsm6dsoTimer_ms = getSystick();
    state           = stateIgnoringSamples;
    return (ERR_SUCCESS);
}

/**
 * @brief State in which the first few samples measured are dropped
 * @note This is recommended in the document AN5192, Table 12
 * 
 * @retval 0 Success
 * @retval 1 No measurement received in a timely manner
 * @retval 2 Error while reading a register
 */
errorCode_u stateIgnoringSamples() {
    //if 1s elapsed without getting any data ready interrupt, error
    if(isTimeElapsed(lsm6dsoTimer_ms, TIMEOUT_MS)) {
        state = stateError;
        return (createErrorCode(DROPPING, 1, ERR_CRITICAL));
    }

    //if no interrupt occurred, exit
    if(!dataReady()) {
        return (ERR_SUCCESS);
    }

    //read an accelerometer value to reset the latched interrupt
    uint8_t dummyValue = 0;
    result             = readRegisters(OUTX_H_A, &dummyValue, 1);
    if(isError(result)) {
        state = stateError;
        return (pushErrorCode(result, DROPPING, 2));
    }

    //reset the timer
    lsm6dsoTimer_ms = getSystick();

    //decrement the remaining amount to ignore and exit if still remaining
    accelerometerSamplesToIgnore--;
    if(accelerometerSamplesToIgnore) {
        return (ERR_SUCCESS);
    }

    //get to measuring state
    state = stateMeasuring;
    return (ERR_SUCCESS);
}

/**
 * @brief State in which the measurements are pulled from the sensor
 * 
 * @retval 0 Success
 * @retval 1 No measurement received in a timely manner
 * @retval 2 Error while reading the status register value
 */
static errorCode_u stateMeasuring() {
    rawValues_u    LSBvalues = {0};              ///< Buffer in which read values will be stored
    float          accelerometer_mG[NB_AXIS];    ///< Accelerometer values in [mG]
    float          gyroscope_radps[NB_AXIS];     ///< Gyroscope values in [rad/s]
    int16_t*       valueIterator    = (void*)0;  ///< Pointer used to browse through read values
    static int16_t previousTemp_LSB = 0;         ///< Previously read temperature LSB values

    //if 1s elapsed without getting any data ready interrupt, error
    if(isTimeElapsed(lsm6dsoTimer_ms, TIMEOUT_MS)) {
        state = stateError;
        return (createErrorCode(MEASURING, 1, ERR_CRITICAL));
    }

    //if no interrupt occurred, exit
    if(!dataReady()) {
        return (ERR_SUCCESS);
    }

    //reset the timer
    lsm6dsoTimer_ms = getSystick();

    //read all temp/accelerometer/gyroscope values
    result = readRegisters(OUT_TEMP_L, LSBvalues.registers8bits, NB_REGISTERS_TO_READ);
    if(isError(result)) {
        state = stateError;
        return (pushErrorCode(result, MEASURING, 2));
    }

    //prepare iterator values
    uint8_t axis  = 0;
    valueIterator = LSBvalues.values16bits;

    //convert the temperature LSB values to °C
    // Sparingly calculated since the temperature readings max. freq. is 52Hz
    // AN5192 p.113 : temperature sensitivity = 256 [LSB/°C] = 0.00390625 [°C/LSB]
    //                + LSB = 0 @ 25°C
    if(*valueIterator != previousTemp_LSB) {
        const float TEMPERATURE_SENSITIVITY = 0.00390625F;
        temperature_degC                    = BASE_TEMPERATURE + ((float)(*valueIterator) * TEMPERATURE_SENSITIVITY);
        previousTemp_LSB                    = *valueIterator;
    }
    valueIterator++;

    //convert the gyroscope LSB values to rad/s
    //datasheet p.9 : gyroscope sensitivity at 125°/s = 4.375[mdps/LSB]
    //      to rad/s : (sensitivity / 1000[mdps/dps]) * (PI/180°) = 0.000076358155
    const float GYR_SENSITIVITY_125DPS_RPS = 0.000076358155F;
    for(axis = 0; axis < (uint8_t)NB_AXIS; axis++) {
        gyroscope_radps[axis] = (float)(*valueIterator) * GYR_SENSITIVITY_125DPS_RPS;
        valueIterator++;
    }

    //then convert the accelerometer LSB values to mG
    //datasheet p.9 : accelerometer sensitivity at 2G = 0.061 [mG/LSB]
    const float AXL_SENSITIVITY_2G = 0.061F;
    for(axis = 0; axis < (uint8_t)NB_AXIS; axis++) {
        accelerometer_mG[axis] = (float)(*valueIterator) * AXL_SENSITIVITY_2G;
        valueIterator++;
    }

    //apply a complementary filter on read values
    complementaryFilter(accelerometer_mG, gyroscope_radps, latestAngles_rad);

    return (ERR_SUCCESS);
}

/**
 * @brief State in which the MEMS is shut down and the module waits for a user release
 * 
 * @return Success
 */
static errorCode_u stateHoldingValues() {
    return (ERR_SUCCESS);
}

/**
 * @brief State in which the LSM6DSO is in error and no further treatment is done
 * 
 * @return Success
 */
static errorCode_u stateError() {
    return (ERR_SUCCESS);
}
