
#include "Touch_FT5x06.h"

/***************************************************************************************
** Function name:           Touch_FT5x06
** Description:             Constructor
***************************************************************************************/
Touch_FT5x06::Touch_FT5x06(void) {
}

/***************************************************************************************
** Function name:           begin
** Description:             Initialise with selected parameters, maxPoints optional
***************************************************************************************/
void Touch_FT5x06::begin(uint8_t sda, uint8_t scl, uint8_t intPin, uint8_t maxPoints) {
    detected = false;
    pointCount = 0;

    Wire.setPins(sda, scl);
    Wire.begin();
    Wire.setClock(FT5X06_I2C_FREQ);

    writeReg(FT5X06_DEVICE_MODE, 0);

    interruptPin = intPin;
    attachInterrupt(digitalPinToInterrupt(interruptPin), serviceInterrupt, FALLING);

    this->maxPoints = maxPoints;

    for (uint8_t i = 0; i < maxPoints; i++) {
        pointX[i] = -1;
        pointY[i] = -1;
    }
}

/***************************************************************************************
** Function name:           serviceInterrupt
** Description:             Touch detect interrupt service routine
***************************************************************************************/
void Touch_FT5x06::serviceInterrupt(void) {
    detected = true;
    detachInterrupt(digitalPinToInterrupt(interruptPin));
}

/***************************************************************************************
** Function name:           getPointX
** Description:             Get X coordinate of touch point 1-5
***************************************************************************************/
int16_t Touch_FT5x06::getPointX(uint8_t point) {
    if (point > pointCount || point == 0) return -1;
    return pointX[point];
}

/***************************************************************************************
** Function name:           getPointY
** Description:             Get Y coordinate of touch point 1-5
***************************************************************************************/
int16_t Touch_FT5x06::getPointY(uint8_t point) {
    if (point > pointCount || point == 0) return -1;
    return pointY[point];
}

uint8_t Touch_FT5x06::getGesture(void) {
    uint8_t gestureID = reg[FT5X06_GEST_ID];
    uint8_t gesture = 0;         //  No gesture default

    switch (gestureID) {
      case FT5X06_GEST_ID_ZOOM_IN:
        gesture = ZOOM_IN;       //  1
        break;
      case FT5X06_GEST_ID_ZOOM_OUT:
        gesture = ZOOM_OUT;      //  2
        break;
/*
      // These are not always supported
      case FT5X06_GEST_ID_MOVE_RIGHT:
        gesture = SWIPE_RIGHT;   //  3 = to  3 o'clock
        break;
      case FT5X06_GEST_ID_MOVE_DOWN:
        gesture = SWIPE_DOWN;    //  6 = to  6 o'clock
        break;
      case FT5X06_GEST_ID_MOVE_LEFT:
        gesture = SWIPE_LEFT;    //  9 = to  9 o'clock
        break;
      case FT5X06_GEST_ID_MOVE_UP:
        gesture = SWIPE_UP;      // 12 = to  12 o'clock
        break;
*/
    }

    return gesture;
}

/***************************************************************************************
** Function name:           getPointCount
** Description:             Get multi-touch point count (0 = none yet or 1-5 touches)
***************************************************************************************/
uint8_t  Touch_FT5x06::getPointCount(void) {
    return pointCount;
}

/***************************************************************************************
** Function name:           maxPointCount
** Description:             Set/get maximum points (returns last value if count = 0)
***************************************************************************************/
uint8_t Touch_FT5x06::maxPointCount(int8_t count) {
  if (count > 0 && count <= 5) maxPoints = count;
  return maxPoints;
}

/***************************************************************************************
** Function name:           pointDetected
** Description:             If touch detected, read points and return true, else false
***************************************************************************************/
bool Touch_FT5x06::pointDetected(void) {
    if (detected) {
      readPoints();
      detected = false;
      attachInterrupt(digitalPinToInterrupt(interruptPin), serviceInterrupt, FALLING);
      if ( lastPointCount == 1
           && pointCount == 1
           && abs(lastPointX - pointX[0]) < 5 
           && abs(lastPointY - pointY[0]) < 5)
        return false;
      lastPointCount = pointCount;
      lastPointX = pointX[0];
      lastPointY = pointY[0];
      return true;
    }
    return false;
}

/***************************************************************************************
** Function name:           readPoints
** Description:             Read the points from the sensor and save them
***************************************************************************************/
void Touch_FT5x06::readPoints(void) {
    uint8_t n = 0;
    Wire.requestFrom(FT5X06_I2C_ADDRESS, FT5X06_REG_COUNT);
    while (Wire.available() && n < FT5X06_REG_COUNT) reg[n++] = Wire.read();
    Wire.endTransmission(FT5X06_I2C_ADDRESS);

    pointCount = reg[FT5X06_TD_STATUS] & 0x7; // 0 - 5
    if (pointCount > 5) pointCount = 5;

    if (pointCount > 0) {
        pointX[0] = (reg[FT5X06_TOUCH1_XH] & 0x0f)*256 + reg[FT5X06_TOUCH1_XL];
        pointY[0] = (reg[FT5X06_TOUCH1_YH] & 0x0f)*256 + reg[FT5X06_TOUCH1_YL];
    }
    if (pointCount > 1) {
        pointX[1] = (reg[FT5X06_TOUCH2_XH] & 0x0f)*256 + reg[FT5X06_TOUCH2_XL];
        pointY[1] = (reg[FT5X06_TOUCH2_YH] & 0x0f)*256 + reg[FT5X06_TOUCH2_YL];
    }
    if (pointCount > 2) {
        pointX[2] = (reg[FT5X06_TOUCH3_XH] & 0x0f)*256 + reg[FT5X06_TOUCH3_XL];
        pointY[2] = (reg[FT5X06_TOUCH3_YH] & 0x0f)*256 + reg[FT5X06_TOUCH3_YL];
    }
    if (pointCount > 3) {
        pointX[3] = (reg[FT5X06_TOUCH4_XH] & 0x0f)*256 + reg[FT5X06_TOUCH4_XL];
        pointY[3] = (reg[FT5X06_TOUCH4_YH] & 0x0f)*256 + reg[FT5X06_TOUCH4_YL];
    }
    if (pointCount > 4) {
        pointX[4] = (reg[FT5X06_TOUCH5_XH] & 0x0f)*256 + reg[FT5X06_TOUCH5_XL];
        pointY[4] = (reg[FT5X06_TOUCH5_YH] & 0x0f)*256 + reg[FT5X06_TOUCH5_YL];
    }

    if (pointCount > maxPoints) pointCount = maxPoints;
}

/***************************************************************************************
** Function name:           jitterMargin
** Description:             Jitter allowance for a change in touch position
***************************************************************************************/
int8_t Touch_FT5x06::jitterMargin(int8_t margin) {
    if (margin > 0) this->margin = margin;
    return this->margin;
}
 
/***************************************************************************************
** Function name:           writeReg
** Description:             Write data or command to sensor
***************************************************************************************/
void Touch_FT5x06::writeReg(uint8_t reg, uint8_t val) {
    Wire.beginTransmission(FT5X06_I2C_ADDRESS);
    Wire.write(reg);
    Wire.write(val);
    Wire.endTransmission(FT5X06_I2C_ADDRESS);
}
