/* This is a library to handling capacitive multitouch sensors using the FocalTech FT5x06
   True Multi-Touch Capacitive Touch Panel Controller
   
   Created by Bodmer 17-04-2021
*/
 
#ifndef Touch_FT5x06_H
#define Touch_FT5x06_H

#include "Arduino.h"
#include "Wire.h"
 
#define FT5X06_I2C_FREQ                     400000UL

// Gestures
#define NO_GESTURE   0
#define ZOOM_IN      1
#define ZOOM_OUT     2

// Controller does not report these gestures...
//#define SWIPE_RIGHT  3 // To  3 o'clock
//#define SWIPE_DOWN   6 // To  6 o'clock
//#define SWIPE_LEFT   9 // To  9 o'clock
//#define SWIPE_UP    12 // To 12 o'clock

/* FT5206 definitions */
#define FT5X06_I2C_ADDRESS                  0x38
#define FT5X06_REG_COUNT                      31 //  registers to get all 5 touch coordinates.
 
#define FT5X06_TOTAL_REG_COUNT              0xFE
 
#define FT5X06_DEVICE_MODE                  0x00
#define FT5X06_GEST_ID                      0x01
#define FT5X06_TD_STATUS                    0x02 // How many points detected (3:0). 1-5 is valid.
 
#define FT5X06_TOUCH1_XH                    0x03 // Event Flag, Touch X Position
#define FT5X06_TOUCH1_XL                    0x04
#define FT5X06_TOUCH1_YH                    0x05 // Touch ID, Touch Y Position
#define FT5X06_TOUCH1_YL                    0x06
 
#define FT5X06_TOUCH2_XH                    0x09 // Event Flag, Touch X Position
#define FT5X06_TOUCH2_XL                    0x0a
#define FT5X06_TOUCH2_YH                    0x0b // Touch ID, Touch Y Position
#define FT5X06_TOUCH2_YL                    0x0c
 
#define FT5X06_TOUCH3_XH                    0x0f // Event Flag, Touch X Position
#define FT5X06_TOUCH3_XL                    0x10
#define FT5X06_TOUCH3_YH                    0x11 // Touch ID, Touch Y Position
#define FT5X06_TOUCH3_YL                    0x12
 
#define FT5X06_TOUCH4_XH                    0x15 // Event Flag, Touch X Position
#define FT5X06_TOUCH4_XL                    0x16
#define FT5X06_TOUCH4_YH                    0x17 // Touch ID, Touch Y Position
#define FT5X06_TOUCH4_YL                    0x18
 
#define FT5X06_TOUCH5_XH                    0x1b // Event Flag, Touch X Position
#define FT5X06_TOUCH5_XL                    0x1c
#define FT5X06_TOUCH5_YH                    0x1d // Touch ID, Touch Y Position
#define FT5X06_TOUCH5_YL                    0x1e
 
#define FT5X06_ID_G_THGROUP                 0x80 // Valid touching detect threshold
#define FT5X06_ID_G_THPEAK                  0x81 // Valid touching peak detect threshold
#define FT5X06_ID_G_THCAL                   0x82 // The threshold when calculating the focus of touching
#define FT5X06_ID_G_THWATER                 0x83 // The threshold when there is surface water
#define FT5X06_ID_G_THTEMP                  0x84 // The threshold of temperature compensation
#define FT5X06_ID_G_CTRL                    0x86 // Power control mode
#define FT5X06_ID_G_TIME_ENTER_MONITOR      0x87 // The timer of entering monitor status
#define FT5X06_ID_G_PERIODACTIVE            0x88 // Period Active
#define FT5X06_ID_G_PERIODMONITOR           0x89 // The timer of entering idle while in monitor status
#define FT5X06_ID_G_AUTO_CLB_MODE           0xA0 // Auto calibration mode
 
#define FT5X06_TOUCH_LIB_VERSION_H          0xA1 // Firmware Library Version H byte
#define FT5X06_TOUCH_LIB_VERSION_L          0xA2 // Firmware Library Version L byte
#define FT5X06_ID_G_CIPHER                  0xA3 // Chip vendor ID
#define FT5X06_G_MODE                       0xA4 // The interrupt status to host
#define FT5X06_ID_G_PMODE                   0xA5 // Power Consume Mode
#define FT5X06_FIRMID                       0xA6 // Firmware ID
#define FT5X06_ID_G_STATE                   0xA7 // Running State
#define FT5X06_ID_G_FT5201ID                0xA8 // CTPM Vendor ID
#define FT5X06_ID_G_ERR                     0xA9 // Error Code
#define FT5X06_ID_G_CLB                     0xAA // Configure TP module during calibration in Test Mode
#define FT5X06_ID_G_B_AREA_TH               0xAE // The threshold of big area
#define FT5X06_LOG_MSG_CNT                  0xFE // The log MSG count
#define FT5X06_LOG_CUR_CHA                  0xFF // Current character of log message, will point to the next
                                               // character when one character is read.
 
#define FT5X06_GEST_ID_MOVE_UP              0x10
#define FT5X06_GEST_ID_MOVE_LEFT            0x14
#define FT5X06_GEST_ID_MOVE_DOWN            0x18
#define FT5X06_GEST_ID_MOVE_RIGHT           0x1c
#define FT5X06_GEST_ID_ZOOM_IN              0x48
#define FT5X06_GEST_ID_ZOOM_OUT             0x49
#define FT5X06_GEST_ID_NO_GESTURE           0x00
 
#define FT5X06_EVENT_FLAG_PUT_DOWN          0x00
#define FT5X06_EVENT_FLAG_PUT_UP            0x01
#define FT5X06_EVENT_FLAG_CONTACT           0x02
#define FT5X06_EVENT_FLAG_RESERVED          0x03
 
#define FT5X06_ID_G_POLLING_MODE            0x00
#define FT5X06_ID_G_TRIGGER_MODE            0x01
 
#define FT5X06_ID_G_PMODE_ACTIVE            0x00
#define FT5X06_ID_G_PMODE_MONITOR           0x01
#define FT5X06_ID_G_PMODE_HIBERNATE         0x03
 
#define FT5X06_ID_G_STATE_CONFIGURE         0x00
#define FT5X06_ID_G_STATE_WORK              0x01
#define FT5X06_ID_G_STATE_CALIBRATION       0x02
#define FT5X06_ID_G_STATE_FACTORY           0x03
#define FT5X06_ID_G_STATE_AUTO_CALIBRATION  0x04

static volatile bool detected;
static uint8_t       interruptPin;

class Touch_FT5x06 {
public:
  Touch_FT5x06(void);
 
  void     begin(uint8_t sda, uint8_t scl, uint8_t intPin, uint8_t maxPoints = 5);
  int16_t  getPointX(uint8_t point);
  int16_t  getPointY(uint8_t point);
  uint8_t  getGesture(void);

  int8_t   jitterMargin(int8_t margin = -1);
  uint8_t  maxPointCount(int8_t count = 5);

  uint8_t  getPointCount(void);
  bool     pointDetected(void);

  uint8_t  pointCount;
  int16_t  pointX[5];
  int16_t  pointY[5];

  uint8_t  lastPointCount;
  int16_t  lastPointX;
  int16_t  lastPointY;

  uint8_t  margin;

private:

  static void serviceInterrupt(void);
  uint8_t  reg[FT5X06_REG_COUNT];
  void     writeReg(uint8_t reg, uint8_t val);
  void     readPoints(void);
  uint8_t  maxPoints;
};
 
#endif
 