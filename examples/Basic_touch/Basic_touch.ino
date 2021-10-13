// Use of the capacitive touch panel
#define SDA_FT5206  21
#define SCL_FT5206  22
#define INT_FT5206  18
 
#include <Touch_FT5x06.h>

Touch_FT5x06 touch = Touch_FT5x06();

void setup() {
  Serial.begin(115200);
  touch.begin(SDA_FT5206, SCL_FT5206, INT_FT5206);

  touch.maxPointCount(1);

  // Position change must exceed this margin before a single continuous touch is reported again.
  touch.jitterMargin(5);
}

void loop() {
  if(touch.pointDetected()) {
    uint8_t touches = touch.getPointCount();
    for (uint8_t i = 1; i <= touches; i++) {
      Serial.print("Point "); Serial.print(i);
      Serial.print(": x = "); Serial.print(touch.getPointX[i]);
      Serial.print(", y = "); Serial.println(touch.getPointY[i]);
    }
  }
}
