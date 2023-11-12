#ifndef INC_ADXL345_H_
#define INC_ADXL345_H_
#include <stm32f1xx.h>
#include "errors.h"

extern volatile uint8_t		adxlINT1occurred;
extern volatile uint16_t	adxlTimer_ms;

/**
 * @brief Enumeration of the axis of which to get measurements
 */
typedef enum{
	X_AXIS,
	Y_AXIS
}axis_e;

extern uint8_t anglesDifferent(float angleA, float angleB);

errorCode_u ADXL345initialise(const SPI_HandleTypeDef* handle);
errorCode_u ADXL345update();
uint8_t ADXL345hasNewMeasurements();
int16_t ADXL345getmeasurement(axis_e axis);
float measureToAngleDegrees(int16_t axisValue);

#endif /* INC_ADXL345_H_ */
