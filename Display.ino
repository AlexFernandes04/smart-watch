
#include <GxEPD2_BW.h>

#include <Fonts/FreeMonoBold12pt7b.h>
#include <Fonts/FreeMonoBold18pt7b.h>
#include <SPI.h>

#include "bitmaps/Bitmaps200x200.h"  // 1.54" b/w

GxEPD2_BW<GxEPD2_154_GDEY0154D67, GxEPD2_154_GDEY0154D67::HEIGHT> display(GxEPD2_154_GDEY0154D67(/*CS=*/10, /*DC=*/5, /*RST=*/4, /*BUSY=*/3));  // GDEY0154D67 200x200, SSD1681, (FPC-B001 20.05.21)

#include <WiFi.h>
#include "time.h"

const char* ssid       = "";
const char* password   = "";

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = -5 * 3600;
const int   daylightOffset_sec = 3600;

char hourmin[64];
char day[64];
char date[64];

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println("setup");
  delay(100);

  SPI.begin(6, -1, 7, 10);
  display.init(115200);  // default 10ms reset pulse, e.g. for bare panels with DESPI-C02

  display.clearScreen();

  // if (display.epd2.hasPartialUpdate) {
  //   Serial.println("PARTIAL ENABLED");
  //   showPartialUpdate();
  //   delay(1000);
  // }  // else // on GDEW0154Z04 only full update available, doesn't look nice

  Serial.printf("Connecting to %s ", ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
  }
  Serial.println(" CONNECTED");
  
  //init and get the time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  printLocalTime();

  //disconnect WiFi as it's no longer needed
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
}


// Variables to save date and time
String datetime;
String formattedDate;
String formattedTime;

void loop() {
  Serial.println(hourmin);

  // char time[formattedTime.length()-3];
  // strcpy(time, formattedTime.c_str());

  // char date[formattedDate.length()];
  // strcpy(date, formattedDate.c_str());

  printLocalTime();
  helloWorld(hourmin, day, date);
  esp_sleep_enable_timer_wakeup(60 * 1000000);  // microseconds
  esp_light_sleep_start();
}

void printLocalTime()
{
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }

  strftime(hourmin, sizeof(hourmin), "%H:%M", &timeinfo);
  strftime(day, sizeof(day), "%A", &timeinfo);
  strftime(date, sizeof(date), "%B %d", &timeinfo);
  }


const char HelloWorld[] = "Hello World!";

// note for partial update window and setPartialWindow() method:
// partial update window size and position is on byte boundary in physical x direction
// the size is increased in setPartialWindow() if x or w are not multiple of 8 for even rotation, y or h for odd rotation
// see also comment in GxEPD2_BW.h, GxEPD2_3C.h or GxEPD2_GFX.h for method setPartialWindow()

void helloWorld(char time[], char day[], char date[]) {
  //Serial.println("helloWorld");
  display.setRotation(1);
  display.setFont(&FreeMonoBold18pt7b);

  display.setTextColor(GxEPD_BLACK);
  int16_t tbx, tby;
  uint16_t tbw, tbh;
  
  display.getTextBounds(time, 0, 0, &tbx, &tby, &tbw, &tbh);

  // center bounding box by transposition of origin:
  uint16_t x = ((display.width() - tbw) / 2) - tbx;
  uint16_t y = (((display.height() - tbh) / 2) - tby)*0.75;
  display.setFullWindow();
  display.firstPage();
  do {
    display.fillScreen(GxEPD_WHITE);
    display.setCursor(x, y);
    display.print(time);
    display.setFont(&FreeMonoBold12pt7b);
    display.getTextBounds(day, 0, 0, &tbx, &tby, &tbw, &tbh);

    // center bounding box by transposition of origin:
    x = ((display.width() - tbw) / 2) - tbx;
    display.setCursor(x, y+40);
    display.print(day);
    display.getTextBounds(date, 0, 0, &tbx, &tby, &tbw, &tbh);

    // center bounding box by transposition of origin:
    x = ((display.width() - tbw) / 2) - tbx;
    display.setCursor(x, y+65);
    display.print(date);
  } while (display.nextPage());
  //Serial.println("helloWorld done");
}

void helloFullScreenPartialMode() {
  //Serial.println("helloFullScreenPartialMode");
  const char fullscreen[] = "full screen update";
  const char fpm[] = "fast partial mode";
  const char spm[] = "slow partial mode";
  const char npm[] = "no partial mode";
  display.setPartialWindow(0, 0, display.width(), display.height());
  display.setRotation(1);
  display.setFont(&FreeMonoBold12pt7b);
  if (display.epd2.WIDTH < 104) display.setFont(0);
  display.setTextColor(GxEPD_BLACK);
  const char* updatemode;
  if (display.epd2.hasFastPartialUpdate) {
    updatemode = fpm;
  } else if (display.epd2.hasPartialUpdate) {
    updatemode = spm;
  } else {
    updatemode = npm;
  }
  // do this outside of the loop
  int16_t tbx, tby;
  uint16_t tbw, tbh;
  // center update text
  display.getTextBounds(fullscreen, 0, 0, &tbx, &tby, &tbw, &tbh);
  uint16_t utx = ((display.width() - tbw) / 2) - tbx;
  uint16_t uty = ((display.height() / 4) - tbh / 2) - tby;
  // center update mode
  display.getTextBounds(updatemode, 0, 0, &tbx, &tby, &tbw, &tbh);
  uint16_t umx = ((display.width() - tbw) / 2) - tbx;
  uint16_t umy = ((display.height() * 3 / 4) - tbh / 2) - tby;
  // center HelloWorld
  display.getTextBounds(HelloWorld, 0, 0, &tbx, &tby, &tbw, &tbh);
  uint16_t hwx = ((display.width() - tbw) / 2) - tbx;
  uint16_t hwy = ((display.height() - tbh) / 2) - tby;
  display.firstPage();
  do {
    display.fillScreen(GxEPD_WHITE);
    display.setCursor(hwx, hwy);
    display.print(HelloWorld);
    display.setCursor(utx, uty);
    display.print(fullscreen);
    display.setCursor(umx, umy);
    display.print(updatemode);
  } while (display.nextPage());
  //Serial.println("helloFullScreenPartialMode done");
}

#if defined(ESP8266) || defined(ESP32)
#include <StreamString.h>
#define PrintString StreamString
#else
class PrintString : public Print, public String {
public:
  size_t write(uint8_t data) override {
    return concat(char(data));
  };
};
#endif

void deepSleepTest() {
  //Serial.println("deepSleepTest");
  const char hibernating[] = "hibernating ...";
  const char wokeup[] = "woke up";
  const char from[] = "from deep sleep";
  const char again[] = "again";
  display.setRotation(1);
  display.setFont(&FreeMonoBold12pt7b);
  if (display.epd2.WIDTH < 104) display.setFont(0);
  display.setTextColor(GxEPD_BLACK);
  int16_t tbx, tby;
  uint16_t tbw, tbh;
  // center text
  display.getTextBounds(hibernating, 0, 0, &tbx, &tby, &tbw, &tbh);
  uint16_t x = ((display.width() - tbw) / 2) - tbx;
  uint16_t y = ((display.height() - tbh) / 2) - tby;
  display.setFullWindow();
  display.firstPage();
  do {
    display.fillScreen(GxEPD_WHITE);
    display.setCursor(x, y);
    display.print(hibernating);
  } while (display.nextPage());
  display.hibernate();
  delay(5000);
  display.getTextBounds(wokeup, 0, 0, &tbx, &tby, &tbw, &tbh);
  uint16_t wx = (display.width() - tbw) / 2;
  uint16_t wy = ((display.height() / 3) - tbh / 2) - tby;  // y is base line!
  display.getTextBounds(from, 0, 0, &tbx, &tby, &tbw, &tbh);
  uint16_t fx = (display.width() - tbw) / 2;
  uint16_t fy = ((display.height() * 2 / 3) - tbh / 2) - tby;  // y is base line!
  display.firstPage();
  do {
    display.fillScreen(GxEPD_WHITE);
    display.setCursor(wx, wy);
    display.print(wokeup);
    display.setCursor(fx, fy);
    display.print(from);
  } while (display.nextPage());
  delay(5000);
  display.getTextBounds(hibernating, 0, 0, &tbx, &tby, &tbw, &tbh);
  uint16_t hx = (display.width() - tbw) / 2;
  uint16_t hy = ((display.height() / 3) - tbh / 2) - tby;  // y is base line!
  display.getTextBounds(again, 0, 0, &tbx, &tby, &tbw, &tbh);
  uint16_t ax = (display.width() - tbw) / 2;
  uint16_t ay = ((display.height() * 2 / 3) - tbh / 2) - tby;  // y is base line!
  display.firstPage();
  do {
    display.fillScreen(GxEPD_WHITE);
    display.setCursor(hx, hy);
    display.print(hibernating);
    display.setCursor(ax, ay);
    display.print(again);
  } while (display.nextPage());
  display.hibernate();
  //Serial.println("deepSleepTest done");
}

void showBox(uint16_t x, uint16_t y, uint16_t w, uint16_t h, bool partial) {
  //Serial.println("showBox");
  display.setRotation(1);
  if (partial) {
    display.setPartialWindow(x, y, w, h);
  } else {
    display.setFullWindow();
  }
  display.firstPage();
  do {
    display.fillScreen(GxEPD_WHITE);
    display.fillRect(x, y, w, h, GxEPD_BLACK);
  } while (display.nextPage());
  //Serial.println("showBox done");
}

void drawCornerTest() {
  display.setFullWindow();
  display.setFont(&FreeMonoBold12pt7b);
  display.setTextColor(GxEPD_BLACK);
  for (uint16_t r = 0; r <= 4; r++) {
    display.setRotation(r);
    display.firstPage();
    do {
      display.fillScreen(GxEPD_WHITE);
      display.fillRect(0, 0, 8, 8, GxEPD_BLACK);
      display.fillRect(display.width() - 18, 0, 16, 16, GxEPD_BLACK);
      display.fillRect(display.width() - 25, display.height() - 25, 24, 24, GxEPD_BLACK);
      display.fillRect(0, display.height() - 33, 32, 32, GxEPD_BLACK);
      display.setCursor(display.width() / 2, display.height() / 2);
      display.print(display.getRotation());
    } while (display.nextPage());
    delay(2000);
  }
}


// note for partial update window and setPartialWindow() method:
// partial update window size and position is on byte boundary in physical x direction
// the size is increased in setPartialWindow() if x or w are not multiple of 8 for even rotation, y or h for odd rotation
// see also comment in GxEPD2_BW.h, GxEPD2_3C.h or GxEPD2_GFX.h for method setPartialWindow()
// showPartialUpdate() purposely uses values that are not multiples of 8 to test this

void showPartialUpdate(char text[]) {
  // some useful background

  helloWorld(text, text, text);
  // use asymmetric values for test
  uint16_t box_x = 10;
  uint16_t box_y = 15;
  uint16_t box_w = 70;
  uint16_t box_h = 20;
  uint16_t cursor_y = box_y + box_h - 6;
  if (display.epd2.WIDTH < 104) cursor_y = box_y + 6;
  float value = 13.95;
  uint16_t incr = display.epd2.hasFastPartialUpdate ? 1 : 3;
  display.setFont(&FreeMonoBold12pt7b);
  if (display.epd2.WIDTH < 104) display.setFont(0);
  display.setTextColor(GxEPD_BLACK);
  // show where the update box is
  for (uint16_t r = 0; r < 4; r++) {
    display.setRotation(r);
    display.setPartialWindow(box_x, box_y, box_w, box_h);
    display.firstPage();
    do {
      display.fillRect(box_x, box_y, box_w, box_h, GxEPD_BLACK);
      //display.fillScreen(GxEPD_BLACK);
    } while (display.nextPage());
    delay(2000);
    display.firstPage();
    do {
      display.fillRect(box_x, box_y, box_w, box_h, GxEPD_WHITE);
    } while (display.nextPage());
    delay(1000);
  }
  //return;
  // show updates in the update box
  for (uint16_t r = 0; r < 4; r++) {
    display.setRotation(r);
    display.setPartialWindow(box_x, box_y, box_w, box_h);
    for (uint16_t i = 1; i <= 10; i += incr) {
      display.firstPage();
      do {
        display.fillRect(box_x, box_y, box_w, box_h, GxEPD_WHITE);
        display.setCursor(box_x, cursor_y);
        display.print(value * i, 2);
      } while (display.nextPage());
      delay(500);
    }
    delay(1000);
    display.firstPage();
    do {
      display.fillRect(box_x, box_y, box_w, box_h, GxEPD_WHITE);
    } while (display.nextPage());
    delay(1000);
  }
}

void drawBitmaps() {

  drawBitmaps200x200();

}

#ifdef _GxBitmaps200x200_H_
void drawBitmaps200x200() {
#if defined(ARDUINO_AVR_PRO)
  const unsigned char* bitmaps[] = {
    logo200x200
  };
#elif defined(__AVR)
  const unsigned char* bitmaps[] = {
    logo200x200,  //first200x200
  };
#elif defined(_BOARD_GENERIC_STM32F103C_H_) || defined(STM32F1xx)
  const unsigned char* bitmaps[] = {
    logo200x200, first200x200, second200x200, third200x200,  //fourth200x200, fifth200x200, sixth200x200, senventh200x200, eighth200x200
  };
#else
  const unsigned char* bitmaps[] = {
    logo200x200, first200x200, second200x200, third200x200, fourth200x200, fifth200x200, sixth200x200, senventh200x200, eighth200x200
    //logo200x200, first200x200, second200x200, fourth200x200, third200x200, fifth200x200, sixth200x200, senventh200x200, eighth200x200 // ED037TC1 test
  };
#endif
  if (display.epd2.hasColor) return;  // to avoid many long refreshes
  if ((display.epd2.WIDTH == 200) && (display.epd2.HEIGHT == 200) && !display.epd2.hasColor) {
    bool m = display.mirror(true);
    for (uint16_t i = 0; i < sizeof(bitmaps) / sizeof(char*); i++) {
      display.firstPage();
      do {
        display.fillScreen(GxEPD_WHITE);
        display.drawInvertedBitmap(0, 0, bitmaps[i], 200, 200, GxEPD_BLACK);
      } while (display.nextPage());
      delay(2000);
    }
    display.mirror(m);
  }
  //else
  {
    bool mirror_y = (display.epd2.panel != GxEPD2::GDE0213B1);
    display.clearScreen();  // use default for white
    int16_t x = (int16_t(display.epd2.WIDTH) - 200) / 2;
    int16_t y = (int16_t(display.epd2.HEIGHT) - 200) / 2;
    for (uint16_t i = 0; i < sizeof(bitmaps) / sizeof(char*); i++) {
      display.drawImage(bitmaps[i], x, y, 200, 200, false, mirror_y, true);
      delay(2000);
    }
  }
  bool mirror_y = (display.epd2.panel != GxEPD2::GDE0213B1);
  for (uint16_t i = 0; i < sizeof(bitmaps) / sizeof(char*); i++) {
    int16_t x = -60;
    int16_t y = -60;
    for (uint16_t j = 0; j < 10; j++) {
      display.writeScreenBuffer();  // use default for white
      display.writeImage(bitmaps[i], x, y, 200, 200, false, mirror_y, true);
      display.refresh(true);
      if (display.epd2.hasFastPartialUpdate) {
        // for differential update: set previous buffer equal to current buffer in controller
        display.epd2.writeScreenBufferAgain();  // use default for white
        display.epd2.writeImageAgain(bitmaps[i], x, y, 200, 200, false, mirror_y, true);
      }
      delay(2000);
      x += display.epd2.WIDTH / 4;
      y += display.epd2.HEIGHT / 4;
      if ((x >= int16_t(display.epd2.WIDTH)) || (y >= int16_t(display.epd2.HEIGHT))) break;
    }
    if (!display.epd2.hasFastPartialUpdate) break;  // comment out for full show
    break;                                          // comment out for full show
  }
  display.writeScreenBuffer();  // use default for white
  display.writeImage(bitmaps[0], int16_t(0), 0, 200, 200, false, mirror_y, true);
  display.writeImage(bitmaps[0], int16_t(int16_t(display.epd2.WIDTH) - 200), int16_t(display.epd2.HEIGHT) - 200, 200, 200, false, mirror_y, true);
  display.refresh(true);
  // for differential update: set previous buffer equal to current buffer in controller
  display.epd2.writeScreenBufferAgain();  // use default for white
  display.epd2.writeImageAgain(bitmaps[0], int16_t(0), 0, 200, 200, false, mirror_y, true);
  display.epd2.writeImageAgain(bitmaps[0], int16_t(int16_t(display.epd2.WIDTH) - 200), int16_t(display.epd2.HEIGHT) - 200, 200, 200, false, mirror_y, true);
  delay(2000);
}
#endif

void drawGraphics() {
  display.setRotation(0);
  display.firstPage();
  do {
    display.drawRect(display.width() / 8, display.height() / 8, display.width() * 3 / 4, display.height() * 3 / 4, GxEPD_BLACK);
    display.drawLine(display.width() / 8, display.height() / 8, display.width() * 7 / 8, display.height() * 7 / 8, GxEPD_BLACK);
    display.drawLine(display.width() / 8, display.height() * 7 / 8, display.width() * 7 / 8, display.height() / 8, GxEPD_BLACK);
    display.drawCircle(display.width() / 2, display.height() / 2, display.height() / 4, GxEPD_BLACK);
    display.drawPixel(display.width() / 4, display.height() / 2, GxEPD_BLACK);
    display.drawPixel(display.width() * 3 / 4, display.height() / 2, GxEPD_BLACK);
  } while (display.nextPage());
  delay(1000);
}
