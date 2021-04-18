
#include "Touch_FT5x06.h"

/***************************************************************************************
** Function name:           serviceInterrupt
** Description:             Touch detect interrupt service routine
***************************************************************************************/
void Touch_FT5x06::serviceInterrupt(void) {
    detected = true;
    detachInterrupt(digitalPinToInterrupt(interruptPin));
}

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
void Touch_FT5x06::begin(int8_t sda, int8_t scl, int8_t intPin, uint8_t maxPoints) {
    detected = false;
    margin = 5;
    pointCount = 0;
    lastPointCount = 0;
    released = 0;
    readToggle = true;

    this->maxPoints = maxPoints;

    for (uint8_t i = 0; i < maxPoints; i++) {
        pointX[i] = -1;
        pointY[i] = -1;
        lastPointX[i] = -1;
        lastPointY[i] = -1;
    }

    Wire.setPins(sda, scl);
    Wire.begin();
    Wire.setClock(FT5X06_I2C_FREQ);

    writeReg(FT5X06_DEVICE_MODE, 0x00);

    interruptPin = intPin;
    if (interruptPin >= 0) {
      attachInterrupt(digitalPinToInterrupt(interruptPin), serviceInterrupt, FALLING);
    }
}

/***************************************************************************************
** Function name:           jitterMargin
** Description:             Jitter allowance for a change in touch position
***************************************************************************************/
int8_t Touch_FT5x06::jitterMargin(int8_t margin) {
    if (margin >= 0) this->margin = margin;
    return this->margin;
}

/***************************************************************************************
** Function name:           maxPointCount
** Description:             Set/get maximum points (returns last value if count = 0)
***************************************************************************************/
uint8_t Touch_FT5x06::maxPointCount(int8_t count) {
  if (count >= 0 && count <= 5) maxPoints = count;
  return maxPoints;
}

/***************************************************************************************
** Function name:           pointDetected
** Description:             If touch detected, read points and return true, else false
***************************************************************************************/
bool Touch_FT5x06::pointDetected(void) {
  bool changed = false;

  if (interruptPin < 0 || detected) {
    readPoints();
    detected = false;
    released = 0;
    if (pointCount < lastPointCount) released = lastPointCount - pointCount;

    if (interruptPin >= 0) {
      attachInterrupt(digitalPinToInterrupt(interruptPin), serviceInterrupt, FALLING);
    }

    // If the positions are different then changed is true
    for (uint8_t i = 0; i < pointCount; i++) {
      if (abs(lastPointX[i] - pointX[i]) > margin
          || abs(lastPointY[i] - pointY[i]) > margin)
        changed = true;
        // Copy points
        lastPointX[i] = pointX[i];
        lastPointY[i] = pointY[i];
        
    }

    // or, if the point count is different then changed is true
    if (lastPointCount != pointCount) changed = true;
    // Copy point count
    lastPointCount = pointCount;
  }

  return changed;
}

/***************************************************************************************
** Function name:           releaseCount
** Description:             Return count if touch points released
***************************************************************************************/
uint8_t Touch_FT5x06::releaseCount(void) {
  if (released) {
    uint8_t r = released;
    released = 0;
    return r;
  }
  return 0;
}
/***************************************************************************************
** Function name:           getPointCount
** Description:             Get multi-touch point count (0 = none yet or 1-5 touches)
***************************************************************************************/
uint8_t  Touch_FT5x06::getPointCount(void) {
    return pointCount;
}

/***************************************************************************************
** Function name:           getPointX
** Description:             Get X coordinate of touch point 1-5
***************************************************************************************/
int16_t Touch_FT5x06::getPointX(uint8_t point) {
    if (point > pointCount || point == 0) return -1;
    return pointX[point - 1];
}

/***************************************************************************************
** Function name:           getPointY
** Description:             Get Y coordinate of touch point 1-5
***************************************************************************************/
int16_t Touch_FT5x06::getPointY(uint8_t point) {
    if (point > pointCount || point == 0) return -1;
    return pointY[point - 1];
}

/***************************************************************************************
** Function name:           getGesture
** Description:             Get gesture (not all supported by controller)
***************************************************************************************/
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

    for (uint8_t i = 0; i < pointCount; i++) {
        uint8_t r = FT5X06_TOUCH_BASE + FT5X06_TOUCH_INCR * i;
        pointX[i] = ((reg[r + 0] & 0x0F) << 8) | reg[r + 1];
        pointY[i] = ((reg[r + 2] & 0x0F) << 8) | reg[r + 3];
        // Check for bad coordinates when interrupt not used
        if (pointX[i] > 1999 || pointY[i] > 1999) {
          pointCount = i;
          break;
        }
    }

    if (pointCount > maxPoints) pointCount = maxPoints;
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
