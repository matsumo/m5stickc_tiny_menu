#include <M5StickC.h>
#include <M5StackUpdater.h>
#include <SPIFFS.h>

#define tft M5.Lcd
#define M5_FS SPIFFS

#include "partition_manager.h"

#define MAX_FILES 8
char filelist[MAX_FILES][33];
int files = 0, cursor = 0;

void setup() {
  Serial.begin(115200);
  Serial.print("M5StickC initializing...");
  M5.begin();
  Wire.begin();
  SPIFFS.begin();
  pinMode(M5_BUTTON_HOME, INPUT);
  pinMode(M5_BUTTON_RST, INPUT);

  M5.Axp.ScreenBreath(8);   // screen brightness (7-15)
  M5.Lcd.setRotation(3);
  M5.Lcd.setTextFont(1);    // Adafruit 8pixel ASCII font
  M5.Lcd.fillScreen(TFT_BLACK);
  M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);

  tft.print("  [[M5StickC Tiny Menu]]");
  checkMenuStickyPartition();

  // get list
  memset(filelist, MAX_FILES * 33, 0);
  File dir = SPIFFS.open( "/" );
  dir.seek(0);

  while( true ) {
    File entry = dir.openNextFile();
    if( !entry ) {
      entry.close();
      break;
    }
    String f = entry.name();
    if(f.compareTo(MENU_BIN) == 0) continue;
    if(!f.endsWith(".bin")) continue;
    f.replace(".bin", "");
    f = f.substring(1);
    tft.setCursor(6, 8 * (files + 1));
    tft.print(f.c_str());
    strcpy(filelist[files], entry.name());
    entry.close();
    files++;
    if(files >= MAX_FILES) break;
  }
  dir.close();

  tft.setCursor(0, 8);
  tft.print(">");

  xTaskCreatePinnedToCore(bridge, "bridge", 4096, NULL, 1, NULL, 1);
}

void loop() {

  // launch!
  if(digitalRead(M5_BUTTON_RST) == 0) {
    Serial.println("Will Load binary");
    updateFromFS(SPIFFS, filelist[cursor]);
    ESP.restart();
  }

  // move next
  if(digitalRead(M5_BUTTON_HOME) == 0) {
    tft.setCursor(0, 8 * (cursor + 1));
    tft.print(" ");
    cursor++;
    if(cursor >= files) cursor = 0;
    tft.setCursor(0, 8 * (cursor + 1));
    tft.print(">");
    while(digitalRead(M5_BUTTON_HOME) == LOW);
  }

  delay(10);
}
