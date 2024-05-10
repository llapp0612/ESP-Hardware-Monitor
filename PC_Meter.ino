// Uncomment to draw meter digits and label text
#define DRAW_DIGITS

#include "lcd_3inch5.h"
#include <SPI.h>
#include <TFT_eSPI.h> // Hardware-specific library
#include <ArduinoJson.h>
#include <SD.h>
#include "sdcard.h"

#ifdef DRAW_DIGITS
#include "VeniteAdoremus_rgRBA.h"
#include <OpenFontRender.h>
#define TTF_FONT VeniteAdoremus_rgRBA
#endif

// Define the maximum size of the received JSON string
const int JSON_BUFFER_SIZE = 1024;
// Buffer for the received data stream
char receivedData[JSON_BUFFER_SIZE];

// Timeout for serial reception in milliseconds (10 seconds)
const int SERIAL_TIMEOUT = 10000;
// Variable to store the last milliseconds
unsigned long previousMillis = 0;
// Interval in milliseconds for updating statistics
const long interval = 1000;

// Invoke custom library with default width and height
TFT_eSPI tft = TFT_eSPI();
// Declare Sprite object "spr" with pointer to "tft" object
TFT_eSprite spr = TFT_eSprite(&tft);

#ifdef DRAW_DIGITS
OpenFontRender ofr;
#endif

#define GREY 0x6b6d
#define DARKER_GREY 0x18E3
#define LITTLE_DARKER_GREY 0x1082
#define TFT_RED_BLACK 0x528a

void Init_Display()
{
  // Must be used for the Seengreat Display
  Lcd_Gpio_Init();
  SPI.setDataMode(SPI_MODE0);
  SPI.setBitOrder(MSBFIRST);
  SPI.setClockDivider(SPI_CLOCK_DIV2);
  SPI.begin();
  Lcd_Init();
  LCD_Clear(TFT_BLACK);
  BlackLight_SetValue(0);
}

struct RingData
{
  int xpos;
  int ypos;
  int radius;
  int value;
  const char *unit;
  const char *subunit;
};

RingData rings[] = {
    {0, 0, 99, 0, "CPU", "Usage"},
    {-160, 0, 60, 0, "CPU", "Temp"},
    {160, 0, 60, 0, "CPU", "Fan"},
    {-110, 88, 40, 0, "GPU", "Usage"},
    {-187, 115, 40, 0, "GPU", "Temp"},
    {110, 88, 40, 0, "GPU", "Fan"},
    {187, 115, 40, 0, "GPU", "Power"},
    {-112, -101, 50, 0, "Ram", "Usage"},
    {112, -101, 50, 0, "Case", "Fan"}
};

int xpos = tft.width() / 2;
int ypos = tft.height() / 2;

void InitRings()
{
  for (size_t i = 0; i < sizeof(rings) / sizeof(rings[0]); i++)
  {
    RingData v = rings[i];
    ringMeter(xpos + v.xpos, ypos + v.ypos, v.radius, v.value, v.unit);
#ifdef DRAW_DIGITS
    drawTitle(xpos + v.xpos, ypos + v.ypos, v.radius, v.unit, v.subunit);
    drawFont(xpos + v.xpos, ypos + v.ypos, v.radius, v.value, i);
#endif
  }
}

void setup(void)
{
  Serial.begin(115200);
  Init_Display();
  SD_Init();
  InitRings();
}

void loop()
{
  drawOnDisplay();
}

void drawOnDisplay()
{
  static unsigned long lastValidDataMillis = 0;
  // Variable to track the state of the backlight
  static bool backlightOn = false;
  unsigned long elapsedTimeSinceLastData = 0;
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval)
  {
    previousMillis = currentMillis;

    elapsedTimeSinceLastData = currentMillis - lastValidDataMillis;

    if (elapsedTimeSinceLastData >= 10000)
    {
      int stats[] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
      for (size_t i = 0; i < sizeof(stats) / sizeof(stats[0]); i++)
      {
        stats[i] = 0;
      }
    }
  }

  if (Serial.available())
  {
    String receivedJson = Serial.readStringUntil('\n');
    Serial.flush();
    DynamicJsonDocument doc(JSON_BUFFER_SIZE);
    deserializeJson(doc, receivedJson);

    if (!doc.isNull())
    {
      lastValidDataMillis = millis();

      int cpuUsage = 0;
      int cpuTemp = 0;
      int cpuFan = 0;
      int gpuUsage = 0;
      int gpuTemp = 0;
      int gpuFan = 0;
      int gpuPower = 0;
      int ramUsage = 0;
      int caseFan = 0;
      int backLight = 0;
      int reset = 0;
      static int currentBackLight = 200;

      // Turn on the backlight if it's not already on
      if (!backlightOn)
      {
        BlackLight_SetValue(200);
        // Mark that the backlight is on
        backlightOn = true;
      }

      for (JsonObject obj : doc.as<JsonArray>())
      {
        if (obj.containsKey("CT"))
        {
          cpuTemp = obj["CT"];
        }
        else if (obj.containsKey("CF"))
        {
          cpuFan = obj["CF"];
        }
        else if (obj.containsKey("CaF"))
        {
          caseFan = obj["CaF"];
        }
        else if (obj.containsKey("CU"))
        {
          cpuUsage = obj["CU"];
        }
        else if (obj.containsKey("RU"))
        {
          ramUsage = obj["RU"];
        }
        else if (obj.containsKey("GT"))
        {
          gpuTemp = obj["GT"];
        }
        else if (obj.containsKey("GU"))
        {
          gpuUsage = obj["GU"];
        }
        else if (obj.containsKey("GF"))
        {
          gpuFan = obj["GF"];
        }
        else if (obj.containsKey("GP"))
        {
          gpuPower = obj["GP"];
        }
        else if (obj.containsKey("BL"))
        {
          backLight = obj["BL"];
          if(backLight != currentBackLight){
            currentBackLight = backLight;
            BlackLight_SetValue(backLight);
          }
        }
        else if (obj.containsKey("RX"))
        {
          reset = obj["RX"];
          if(reset > 0) ESP.restart();
        }
      }

      int stats[] = {cpuUsage, cpuTemp, cpuFan, gpuUsage, gpuTemp, gpuFan, gpuPower, ramUsage, caseFan};
      for (int i = 0; i < sizeof(stats) / sizeof(stats[0]); i++)
      {
        fillRing(xpos + rings[i].xpos, ypos + rings[i].ypos, rings[i].radius - 3, stats[i], i);
      }
    }
  }
  else
  {
    // No data received
    unsigned long elapsedTime = millis() - lastValidDataMillis;
    if (elapsedTime >= SERIAL_TIMEOUT && backlightOn)
    {
      // Turn off backlight, if on
      BlackLight_SetValue(0);
      backlightOn = false;
    }
  }
}

void ringMeter(int x, int y, int r, int val, const char *units)
{
  static uint16_t last_angle = 60;

  tft.fillCircle(x, y, r, DARKER_GREY);
  tft.drawSmoothCircle(x, y, r, TFT_RED_BLACK, DARKER_GREY);

  uint16_t tmp = r - 3;
  tft.drawArc(x, y, tmp, tmp - tmp / 5 / 2, last_angle, 300, LITTLE_DARKER_GREY, DARKER_GREY);
}

void fillRing(int x, int y, int r, int val, int id)
{
    static float last_angle[9] = {60.0, 60.0, 60.0, 60.0, 60.0, 60.0, 60.0, 60.0, 60.0};

    int maxValue = (id == 1 || id == 4) ? 125 : (id == 5) ? 2600 : (id == 2 || id == 8) ? 1400 : (id == 6) ? 340 : 100;
    float val_angle = mapWithRound(val, 0, maxValue, 60, 300);

    // Allocate a value to the arc thickness dependant of radius
    uint8_t thickness = r / 5 / 2;
    if (r < 25)
        thickness = r / 3;

    int barColor = (id <= 2) ? TFT_CYAN : (id > 2 && id <= 6) ? TFT_GREEN : (id == 7) ? TFT_ORANGE : TFT_RED;

    // Smooth animation
    if (val_angle > last_angle[id])
    {
        for (float i = last_angle[id]; i <= val_angle; i++)
        {
            tft.drawArc(x, y, r, r - thickness, last_angle[id], i, barColor, LITTLE_DARKER_GREY);
            // Map angle back to value
            int mapped_val = mapWithRound(i, 60, 300, 0, maxValue);
            if (i < 60)
            {
                mapped_val = mapWithRound(i, 0, 60, maxValue, 0);
            }
            drawFont(x, y, r, mapped_val, id);
        }
    }
    else if (val_angle < last_angle[id])
    {
        for (float i = last_angle[id]; i >= val_angle; i--)
        {
            tft.drawArc(x, y, r, r - thickness, i, last_angle[id], LITTLE_DARKER_GREY, DARKER_GREY);
            // Map angle back to value
            int mapped_val = mapWithRound(i, 60, 300, 0, maxValue);
            if (i < 60)
            {
                mapped_val = mapWithRound(i, 0, 60, maxValue, 0);
            }
            drawFont(x, y, r, mapped_val, id);
        }
    }
    // Update angle for next Update
    last_angle[id] = val_angle;
}

// Mapping function without rounding
float mapNoRound(float x, float in_min, float in_max, float out_min, float out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

// Mapping function with rounding
int mapWithRound(int x, int in_min, int in_max, int out_min, int out_max) {
  float mapped_value = (float)(x - in_min) * (float)(out_max - out_min) / (float)(in_max - in_min) + out_min;
  return round(mapped_value);
}

int inverseMap(int val, int minVal, int maxVal, int minAngle, int maxAngle) {
    float t = (float)(val - minVal) / (maxVal - minVal);
    int angle = minAngle + t * (maxAngle - minAngle);
    return angle;
}

void drawFont(int x, int y, int r, int val, int id)
{
#ifdef DRAW_DIGITS
  if (ofr.loadFont(TTF_FONT, sizeof(TTF_FONT)))
  {
    Serial.println("Render initialize error");
  }

  ofr.setDrawer(spr);

  if (r >= 25)
  {
    ofr.setFontSize((5 * r) / 12);
    ofr.setFontColor(TFT_WHITE, DARKER_GREY);

    char str_buf[8];
    itoa(val, str_buf, 10);

    // Calculate the width and height of the text
    uint8_t textWidth = ofr.getTextWidth(str_buf);
    uint8_t textHeight = ofr.getTextHeight("4") + 4;

    // Create the sprite with the correct size
    spr.createSprite((5 * r) / 4, (5 * r) / 10);

    // Clear the sprite content
    spr.fillSprite(DARKER_GREY);

    // Calculate the x offset to center the text
    int dx = spr.width() / 2 - (textWidth / 2);
    // Calculate the y offset to center the text
    int dy = spr.height() / 2 - (textHeight / 2);
    // Set the cursor position for drawing the text
    ofr.setCursor(dx, dy);

    ofr.setLineSpaceRatio(0.7);
    // Draw the text
    ofr.printf(str_buf);

    //  Position the sprite on the screen using the original coordinates (x, y)
    spr.pushSprite(x - (spr.width() / 2), y - (spr.height() / 2));

    // Delete the sprite to recover used memory
    spr.deleteSprite();
  }
  // Recover space used by font metrics etc
  ofr.unloadFont();
#endif
  spr.pushSprite(0, 0);
}

void drawTitle(int x, int y, int r, const char *unit, const char *subunit)
{
#ifdef DRAW_DIGITS
  uint8_t w = ofr.getTextWidth(unit);
  uint8_t h = ofr.getTextHeight("4") + 4;

  int posy[2] =
  {
    y + (-h / 4.5) + (r / 3.8) - (r / 1.4),
    y + (-h / 4.5) + (r / 3.8) + (r / 9)
  };

  int color = TFT_GOLD;
  for (int i = 0; i < sizeof(posy) / sizeof(posy[0]); i++)
  {
    if(i == 1)
    {
      unit = subunit;
      color = GREY;
    }

    printFont(x, posy[i], r, unit, color);
  }
#endif
}

void printFont(int x, int y, int r, const char *unit, int color)
{
#ifdef DRAW_DIGITS
  // Make the TFT the print destination, print the units label direct to the TFT
  ofr.setDrawer(tft);
  ofr.setFontSize(r / 4);
  ofr.setFontColor(color, DARKER_GREY);

  if (ofr.loadFont(TTF_FONT, sizeof(TTF_FONT)))
  {
    Serial.println("Render initialize error");
  }

  ofr.setCursor(x, y);
  ofr.cprintf(unit);
  ofr.unloadFont();
#endif
}