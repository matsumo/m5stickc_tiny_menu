#include <M5StickC.h>
#include <M5StackUpdater.h>
#include <SPIFFS.h>
#include <SD.h>

#define tft M5.Lcd

// PIN配置
enum { spi_sck = 0, spi_miso = 36, spi_mosi = 26, spi_ss = -1 };

#define M5_FS SD
#define TFCARD_CS_PIN spi_ss
SPIClass SPI_EXT;

#include "partition_manager.h"

/*#define PWR_BTN_STABLEL     0 // 押していない or 1秒以上押し続ける
#define PWR_BTN_LONG_PRESS  1 // 1秒の長押し発生
#define PWR_BTN_SHORT_PRESS 2 // 1秒以下のボタンクリック

boolean is_sleep = false;*/

#define MAX_FILES 8
char filelist[MAX_FILES][33];
int files = 0, cursor = 0, mode = 0;

void setup() {
  Serial.begin(115200);
  Serial.print("M5StickC initializing...");
  M5.begin();
  Wire.begin();
  SPIFFS.begin();

  // SPI初期化
  SPI_EXT.begin(spi_sck, spi_miso, spi_mosi, spi_ss);

  pinMode(M5_BUTTON_HOME, INPUT);
  pinMode(M5_BUTTON_RST, INPUT);

  M5.Axp.ScreenBreath(8);   // screen brightness (7-15)
  tft.setRotation(3);
  tft.setTextFont(1);    // Adafruit 8pixel ASCII font
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);

  checkMenuStickyPartition();
  mode = M5_FS.begin( TFCARD_CS_PIN, /*SPI*/SPI_EXT, 40000000 );
//  if(mode == 1) SD.begin(spi_ss, SPI_EXT);
  getList();
}

void getList(){
  tft.fillScreen(TFT_BLACK);
  tft.setCursor(0, 0);
  tft.print("  [[M5StickC Tiny Menu]]");
  tft.print(mode == 0 ? "FS" : "SD");

  // get list
  memset(filelist, MAX_FILES * 33, 0);
  File dir = (mode == 0 ? SPIFFS.open( "/" ) : SD.open( "/" ));
  dir.seek(0);

  while( true ) {
    File entry = dir.openNextFile();
    if( !entry ) {
      entry.close();
      break;
    }
    String f = entry.name();
    Serial.println(f);
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
  Serial.println("-----");

  tft.setCursor(0, 8);
  tft.print(">");
}

void loop() {
  M5.update();

  // switch SD <-> SPIFFS
/*  if(M5.BtnB.wasReleasefor(1000)) {
    if(mode == 0){
      if (!SD.begin(spi_ss)) {
        return;
      }
    }
    mode = 1 - mode;
    getList();
  }

  // launch!
  else */if(M5.BtnB.wasPressed()) {
    Serial.println("Will Load binary");
    if(mode == 1){
      if(SD.open(filelist[cursor])){
        Serial.println("OK!!");
      }else{
        Serial.println("failed");
      }
    }
    updateFromFS((mode == 0 ? (fs::FS &)SPIFFS : (fs::FS &)SD), filelist[cursor]);
    ESP.restart();
  }

  // move prev
  else if(M5.BtnA.wasReleasefor(1000)) {
/*    tft.setCursor(0, 8 * (cursor + 1));
    tft.print(" ");
    cursor--;
    if(cursor < 0) cursor = files - 1;
    tft.setCursor(0, 8 * (cursor + 1));
    tft.print(">");*/
    if(mode == 0){
      if (!SD.begin(spi_ss)) {
        return;
      }
    }
    mode = 1 - mode;
    files = 0;
    cursor = 0;
    getList();
  }

  // move next
  else if(M5.BtnA.wasReleased()) {
    tft.setCursor(0, 8 * (cursor + 1));
    tft.print(" ");
    cursor++;
    if(cursor >= files) cursor = 0;
    tft.setCursor(0, 8 * (cursor + 1));
    tft.print(">");
  }

/*  if(M5.Axp.GetBtnPress() == PWR_BTN_SHORT_PRESS) {
    // 電源ボタンの短押し
    if(is_sleep){
      ; // 復帰するだけで、何もしない
    }else{
      M5.Axp.SetSleep(); // 画面が消えるだけのスリープに入る
    }
    is_sleep = !is_sleep;
  }*/

  delay(100);
}
