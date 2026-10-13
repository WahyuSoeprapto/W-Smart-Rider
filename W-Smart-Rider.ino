#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ChronosESP32.h>
#include <Preferences.h>
#include <WiFi.h>
#include <DNSServer.h>
#include <WebServer.h>
#include <WiFiManager.h>      
#include <Update.h>           
#include "daichi_intro.h"

// -------------------------------------------------------------------
// Konfigurasi Layar OLED 0.96" I2C
// -------------------------------------------------------------------
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// -------------------------------------------------------------------
// Inisialisasi Pustaka Chronos & Preferences (Nama Bluetooth: W-Smart Rider)
// -------------------------------------------------------------------
ChronosESP32 chronos("W-Smart Rider");
Preferences preferences;

// -------------------------------------------------------------------
// Pemetaan Pin (ESP32-C3)
// -------------------------------------------------------------------
const int PIN_SDA = 20;   
const int PIN_SCL = 21;   
const int TOUCH_PIN = 1;  

// ===================================================================
// VARIABEL KONTROL OTA & WIFI MANAGER
// ===================================================================
WebServer server(80);
bool modeWiFiOTA = false;
unsigned long otaStartTime = 0;
const unsigned long OTA_TIMEOUT = 120000; 

// Tampilan Halaman Web Update Firmware
const char* updateIndex = 
  "<!DOCTYPE html><html><head><meta name='viewport' content='width=device-width, initial-scale=1'>"
  "<style>"
  "body { font-family: Arial, sans-serif; text-align: center; background-color: #f4f4f9; color: #333; margin-top: 20px; }"
  ".container { max-width: 400px; margin: auto; background: white; padding: 20px; border-radius: 8px; box-shadow: 0 4px 8px rgba(0,0,0,0.1); }"
  "h2 { color: #007BFF; margin-bottom: 5px; }"
  ".wifi-name { font-weight: bold; color: #555; margin-bottom: 15px; font-size: 15px; }"
  ".status { background: #e9ecef; padding: 10px; border-radius: 4px; margin-bottom: 15px; font-size: 14px; }"
  "input[type='file'] { margin: 15px 0; padding: 5px; width: 100%; box-sizing: border-box; }"
  "input[type='submit'] { background: #28a745; color: white; border: none; padding: 10px 20px; font-size: 16px; border-radius: 4px; cursor: pointer; width: 100%; }"
  "input[type='submit']:hover { background: #218838; }"
  "#prg { margin-top: 15px; font-weight: bold; color: #555; }"
  ".info { font-size: 12px; color: #775; margin-top: 20px; border-top: 1px solid #ddd; padding-top: 10px; }"
  "</style>"
  "<script src='https://ajax.googleapis.com/ajax/libs/jquery/3.2.1/jquery.min.js'></script>"
  "</head><body>"
  "<div class='container'>"
  "<h2>W-Smart Rider</h2>"
  "<div class='wifi-name'>Wi-Fi: W-Smart Rider</div>"
  "<div class='status'><b>Hubungkan Wi-Fi Aktif</b><br>Mode OTA Firmware Update</div>"
  "<form method='POST' enctype='multipart/form-data' id='upload_form'>"
  "<input type='file' name='update' required><br>"
  "<input type='submit' value='Update File'>"
  "</form>"
  "<div id='prg'>Progress: 0%</div>"
  "<div class='info'>Info: Pastikan file .bin sesuai dengan arsitektur ESP32-C3.<br>Perangkat akan restart otomatis setelah update selesai.</div>"
  "</div>"
  "<script>"
  "$('form').submit(function(e){"
  "e.preventDefault();"
  "var data = new FormData($('#upload_form')[0]);"
  "$.ajax({"
  "url: '/update', type: 'POST', data: data, contentType: false, processData: false,"
  "xhr: function() {"
  "var xhr = new window.XMLHttpRequest();"
  "xhr.upload.addEventListener('progress', function(evt) {"
  "if (evt.lengthComputable) {"
  "var percentComplete = Math.round((evt.loaded / evt.total) * 100);"
  "$('#prg').html('Progress: ' + percentComplete + '%');"
  "}"
  "}, false);"
  "return xhr;"
  "},"
  "success: function(d, status) { alert('Update Berhasil! Perangkat akan merestart.'); location.reload(); },"
  "error: function() { alert('Update Gagal! Periksa kembali file .bin.'); }"
  "});"
  "});"
  "</script>"
  "</body></html>";

// ===================================================================
// VARIABEL TEKS & DISPLAY
// ===================================================================
String runningText = "Selamat Datang di W-Smart Rider!"; 
String popupText   = "INFO PENTING"; 
bool isNegativeDisplay = false; 

const unsigned long POPUP_SHOW_TIME = 3000; 
const unsigned long POPUP_HIDE_TIME = 2000; 

int screenMode = 0; 
const int TOTAL_MODES = 8; 

int homeSubMode = 0; 
unsigned long lastHomeToggleTimer = 0;
bool homeToggleState = false; 

int textSubMode = 0; 

unsigned long touchStartTime = 0;
unsigned long lastTapTime = 0;
bool isTouching = false;
int tapCount = 0;

int progressWidth = 0;
unsigned long lastProgressUpdate = 0;
String lastTitle = "";

int xPosMusicTitle = 128;
unsigned long lastMusicTitleUpdate = 0;

bool isVolumePopupActive = false;
int currentVolume = 50; 
unsigned long lastVolumeActionTime = 0;

String navDistance = ""; 
String navStreet = "";   
bool isNavActive = false;
bool hasNewNavIcon = false;
uint8_t navIconBuffer[288]; 

String waSender = "";
String waMessage = "";
int waPage = 0;
unsigned long lastWaPageToggle = 0;

int xPosRunning = 128;
unsigned long lastRunningUpdate = 0;
unsigned long lastPopupTimer = 0;
bool showPopupState = true;

int currentGifFrame = 0;
unsigned long lastGifFrameTime = 0;

// Forward Declaration
void handleTouchInput();
void updateDisplay();
void showStartupSplash();
void jalankanModeWiFiOTA();
void setupOTAServer();
String getIndonesianDay(String engDay);
String getIndonesianMonth(String engMonth);
void playCurrentGIFFrame(const AnimatedGIF* gif, int frameIndex);

String getIndonesianDay(String engDay) {
  engDay.trim();
  if (engDay == "Sunday") return "Minggu";
  if (engDay == "Monday") return "Senin";
  if (engDay == "Tuesday") return "Selasa";
  if (engDay == "Wednesday") return "Rabu";
  if (engDay == "Thursday") return "Kamis";
  if (engDay == "Friday") return "Jumat";
  if (engDay == "Saturday") return "Sabtu";
  return engDay;
}

String getIndonesianMonth(String engMonth) {
  engMonth.trim();
  if (engMonth == "Jan") return "Jan";
  if (engMonth == "Feb") return "Feb";
  if (engMonth == "Mar") return "Mar";
  if (engMonth == "Apr") return "Apr";
  if (engMonth == "May") return "Mei";
  if (engMonth == "Jun") return "Jun";
  if (engMonth == "Jul") return "Jul";
  if (engMonth == "Aug") return "Ags";
  if (engMonth == "Sep") return "Sep";
  if (engMonth == "Oct") return "Okt";
  if (engMonth == "Nov") return "Nov";
  if (engMonth == "Dec") return "Des";
  return engMonth;
}

void showStartupSplash() {
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(22, 14);
  display.print("W-SMART");
  display.setCursor(34, 38);
  display.print("RIDER");
  display.display();
  delay(3000); 

  for (int step = 0; step < 6; step++) {
    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(22, 14);
    display.print("W-SMART");
    display.setCursor(34, 38);
    display.print("RIDER");

    for (int i = 0; i < (128 * 64); i += (12 - (step * 2))) {
      int x = random(0, 128);
      int y = random(0, 64);
      display.drawPixel(x, y, SSD1306_BLACK);
    }
    display.display();
    delay(100);
  }
  display.clearDisplay();
  display.display();
}

void configCallback(Config config, uint32_t a, uint32_t b) {
  switch (config) {
    case CF_NAV_DATA:
      if (a) {
        Navigation nav = chronos.getNavigation();
        isNavActive = true;
        navDistance = nav.title;
        navStreet = nav.directions;
        screenMode = 2; 
      } else {
        isNavActive = false;
      }
      break;

    case CF_NAV_ICON:
      if (a == 2) {
        Navigation nav = chronos.getNavigation();
        memcpy(navIconBuffer, nav.icon, 288);
        hasNewNavIcon = true;
        isNavActive = true;
        screenMode = 2; 
      }
      break;
  }
}

void notificationCallback(Notification notification) {
  String title = notification.title;
  String message = notification.message;
  int icon = notification.icon;
  String appName = notification.app;
  appName.toLowerCase();

  if (title.length() > 0 || message.length() > 0) {
    String combinedText = title + " " + message;
    String lowerCombined = combinedText;
    lowerCombined.toLowerCase();

    if (lowerCombined.indexOf("jam tenang") >= 0 || lowerCombined.indexOf("do not disturb") >= 0) return; 

    bool isWA = (icon == 10 || appName.indexOf("whatsapp") >= 0 || lowerCombined.indexOf("whatsapp") >= 0);

    if (isWA) {
      bool isSummary = (lowerCombined.indexOf("pesan dari") >= 0 || 
                        lowerCombined.indexOf("chats") >= 0 ||
                        lowerCombined.indexOf("new messages") >= 0 ||
                        lowerCombined.indexOf("pesan baru") >= 0 || 
                        lowerCombined.indexOf("new message") >= 0 ||
                        (lowerCombined.indexOf("pesan") >= 0 && lowerCombined.indexOf("baru") >= 0));

      if (!isSummary) {
        screenMode = 4; 
        if (title.length() > 0) waSender = title;
        if (message.length() > 0) {
          waMessage = message;
          waPage = 0; 
          lastWaPageToggle = millis();
        } 
      }
      return; 
    }

    if (icon == 7) {
      if (message.length() > 0) runningText = message;
      else if (title.length() > 0) runningText = title;
      
      preferences.begin("storage", false);
      preferences.putString("savedText", runningText);
      preferences.end();

      xPosRunning = 128;
      screenMode = 5; 
      textSubMode = 0;
      return;
    }
  }
}

void setup() {
  Serial.begin(115200);
  delay(500);

  pinMode(TOUCH_PIN, INPUT);

  preferences.begin("storage", true);
  runningText = preferences.getString("savedText", "Selamat Datang di W-Smart Rider!");
  isNegativeDisplay = preferences.getBool("isNegative", false); 
  preferences.end();

  Wire.begin(PIN_SDA, PIN_SCL);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    for (;;);
  }

  display.setRotation(0);
  if (isNegativeDisplay) display.invertDisplay(true);

  showStartupSplash();
  
  chronos.setNotificationCallback(notificationCallback);
  chronos.setConfigurationCallback(configCallback);
  chronos.begin();

  lastPopupTimer = millis();
  lastHomeToggleTimer = millis();
}

void loop() {
  unsigned long now = millis();

  if (modeWiFiOTA) {
    handleTouchInput(); 
    server.handleClient();

    if (now - otaStartTime > OTA_TIMEOUT) {
      ESP.restart();
    }
    return; 
  }

  chronos.loop();       
  handleTouchInput();   
  updateDisplay();      
  delay(15);
}

void handleTouchInput() {
  bool currentTouch = digitalRead(TOUCH_PIN);

  if (currentTouch && !isTouching) {
    touchStartTime = millis();
    isTouching = true;
  } 
  else if (!currentTouch && isTouching) {
    unsigned long duration = millis() - touchStartTime;
    isTouching = false;

    if (duration > 800) { 
      if (screenMode == 7 && !modeWiFiOTA) {
        ESP.restart();
      }
      else if (screenMode == 0 && !modeWiFiOTA) {
        homeSubMode = (homeSubMode + 1) % 3; 
        lastHomeToggleTimer = millis();
        homeToggleState = false; 
      }
      else if (screenMode == 1 && !modeWiFiOTA) {
        if (isVolumePopupActive) {
          currentVolume -= 10;
          if (currentVolume < 0) currentVolume = 0;
          chronos.setVolume(currentVolume);
          lastVolumeActionTime = millis();
        } else {
          bool isPlaying = (chronos.getMusicInfo().state == 1);
          if (isPlaying) chronos.musicControl(MUSIC_PAUSE);
          else chronos.musicControl(MUSIC_PLAY);
        }
      }
      else if (screenMode == 5 && !modeWiFiOTA) {
        textSubMode = (textSubMode + 1) % 2; 
        if (textSubMode == 0) xPosRunning = 128;
        else {
          lastPopupTimer = millis();
          showPopupState = true;
        }
      }
      tapCount = 0;
    } 
    else {
      tapCount++;
      lastTapTime = millis();
    }
  }

  if (tapCount > 0 && (millis() - lastTapTime > 400)) {
    if (tapCount == 1) {
      if (!modeWiFiOTA) {
        if (screenMode == 1 && isVolumePopupActive) {
          currentVolume += 10;
          if (currentVolume > 100) currentVolume = 100;
          chronos.setVolume(currentVolume);
          lastVolumeActionTime = millis();
        } else {
          screenMode = (screenMode + 1) % TOTAL_MODES;
          if (screenMode == 5) xPosRunning = 128;
          if (screenMode == 6) {
            currentGifFrame = 0;
            lastGifFrameTime = millis();
          }
        }
      }
    } 
    else if (tapCount == 2) {
      if (modeWiFiOTA) {
        ESP.restart(); 
      } else {
        if (screenMode == 7) {
          ESP.restart();
        }
        else if (screenMode == 0) {
          isNegativeDisplay = !isNegativeDisplay;
          display.invertDisplay(isNegativeDisplay);
          preferences.begin("storage", false);
          preferences.putBool("isNegative", isNegativeDisplay);
          preferences.end();
        }
        else if (screenMode == 1 && !isVolumePopupActive) {
          chronos.musicControl(MUSIC_NEXT);
          progressWidth = 0;
        }
      }
    }
    else if (tapCount == 3) {
      if (!modeWiFiOTA) {
        if (screenMode == 1 && !isVolumePopupActive) {
          chronos.musicControl(MUSIC_PREVIOUS);
          progressWidth = 0;
        } 
        else if (screenMode == 4) {
          chronos.musicControl(MUSIC_PAUSE); 
          waSender = "";
          waMessage = "";
        }
        else if (screenMode == 7) {
          jalankanModeWiFiOTA();
        }
      }
    }
    else if (tapCount >= 4) {
      if (screenMode == 1 && !modeWiFiOTA) {
        isVolumePopupActive = !isVolumePopupActive;
        lastVolumeActionTime = millis();
      }
    }
    tapCount = 0;
  }
}

void jalankanModeWiFiOTA() {
  modeWiFiOTA = true;
  otaStartTime = millis();
  
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(10, 16);
  display.println("MEMBUKA WI-FI...");
  display.setCursor(0, 36);
  display.println("W-Smart Rider");
  display.display();

  WiFiManager wm;
  wm.setConfigPortalTimeout(60); 

  if(!wm.autoConnect("W-Smart Rider")) {
    modeWiFiOTA = false;
    screenMode = 0;
    return;
  }
  
  setupOTAServer();
  modeWiFiOTA = true;
  otaStartTime = millis();
}

void setupOTAServer() {
  server.on("/", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", updateIndex);
  });
  
  server.on("/update", HTTP_POST, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/plain", (Update.hasError()) ? "UPDATE GAGAL!" : "UPDATE BERHASIL!");
    delay(1000);
    ESP.restart();
  }, []() {
    HTTPUpload& upload = server.upload();
    if (upload.status == UPLOAD_FILE_START) {
      if (!Update.begin(UPDATE_SIZE_UNKNOWN)) Update.printError(Serial);
    } else if (upload.status == UPLOAD_FILE_WRITE) {
      if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) Update.printError(Serial);
    } else if (upload.status == UPLOAD_FILE_END) {
      if (!Update.end(true)) Update.printError(Serial);
    }
  });
  server.begin();
}

void playCurrentGIFFrame(const AnimatedGIF* gif, int frameIndex) {
    for (uint16_t y = 0; y < gif->height; y++) {
        for (uint16_t x = 0; x < gif->width; x++) {
            uint16_t byteIndex = y * (((gif->width + 7) / 8)) + (x / 8);
            uint8_t bitIndex = 7 - (x % 8);
            if (pgm_read_byte(&(gif->frames[frameIndex][byteIndex])) & (1 << bitIndex)) {
                display.drawPixel(x, y, SSD1306_WHITE);
            }
        }
    }
}

void updateDisplay() {
  bool isGifActive = false;
  if (screenMode == 0) {
    if (homeSubMode == 0) {
      if (millis() - lastHomeToggleTimer >= 6000) { 
        lastHomeToggleTimer = millis();
        homeToggleState = !homeToggleState; 
        currentGifFrame = 0; 
      }
      isGifActive = homeToggleState; 
    } else if (homeSubMode == 1) {
      isGifActive = false; 
    } else if (homeSubMode == 2) {
      isGifActive = true;  
    }
  }

  if (modeWiFiOTA) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(14, 10);
    display.println("WI-FI OTA AKTIF");
    display.setCursor(4, 28);
    display.println("SSID: W-Smart Rider");
    display.setCursor(14, 46);
    display.println("Ketuk 2x: Matikan");
    display.display();
    return;
  }

  if (!isGifActive && screenMode != 6) display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  if (screenMode == 0) {
    if (isGifActive) {
      AnimatedGIF myGif = {DAICHI_INTRO_FRAME_COUNT, DAICHI_INTRO_WIDTH, DAICHI_INTRO_HEIGHT, daichi_intro_delays, daichi_intro_frames};
      if (millis() - lastGifFrameTime >= pgm_read_word(&myGif.delays[currentGifFrame])) {
        lastGifFrameTime = millis();
        display.clearDisplay();
        playCurrentGIFFrame(&myGif, currentGifFrame);
        display.display();
        currentGifFrame = (currentGifFrame + 1) % myGif.frame_count;
      }
    } else {
      display.setTextSize(1);
      display.setCursor(0, 0);
      if (chronos.isConnected()) {
        display.print("BLE: OK");
        display.setCursor(80, 0);
        display.print("Bat:");
        display.print(chronos.getPhoneBattery());
        display.print("%");
      } else {
        display.print("BLE: Disconnected");
      }
      display.drawFastHLine(0, 10, 128, SSD1306_WHITE);
      display.setTextSize(3);
      display.setCursor(14, 16);
      if (chronos.isConnected()) display.print(chronos.getTime("%H:%M"));
      else display.print("--:--");

      display.setTextSize(1);
      display.setCursor(12, 48);
      if (chronos.isConnected()) {
        display.print(getIndonesianDay(chronos.getTime("%A")) + ", " + chronos.getTime("%d") + " " + getIndonesianMonth(chronos.getTime("%b")) + " " + chronos.getTime("%Y"));
      } else {
        display.print("Sabtu, -- --- ----");
      }
    }
  }

  // MODE 3: PERKIRAAN CUACA (HEADER JAM DIHILANGKAN, TAMPILAN BERSIH & BESAR)
  else if (screenMode == 3) {
    display.setTextSize(1);
    display.setCursor(0, 16);
    display.println("PERKIRAAN CUACA");
    display.setCursor(0, 32);
    display.print("Suhu: ");
    display.print(chronos.getWeatherAt(0).temp);
    display.println(" C");

    display.setCursor(0, 46);
    display.print("Kota: ");
    display.println(chronos.getWeatherCity());
  }

  else if (screenMode == 1) {
    String title = chronos.getMusicInfo().title;
    String artist = chronos.getMusicInfo().artist;
    bool isPlaying = (chronos.getMusicInfo().state == 1);

    if (title != lastTitle) {
      xPosMusicTitle = 128; 
      progressWidth = 0;
      lastTitle = title;
    }

    if (isVolumePopupActive) {
      display.fillRect(14, 12, 100, 40, SSD1306_BLACK); 
      display.drawRect(14, 12, 100, 40, SSD1306_WHITE); 
      display.setTextSize(1);
      display.setCursor(32, 18);
      display.print("ATUR VOLUME");
      display.setTextSize(2);
      display.setCursor(42, 32);
      display.print(currentVolume);
      display.print("%");

      if (millis() - lastVolumeActionTime > 5000) isVolumePopupActive = false;
    } 
    else {
      if (title.length() > 0) {
        display.setTextWrap(false);
        display.setTextSize(2);
        display.setCursor(xPosMusicTitle, 4);
        display.println(title);

        if (millis() - lastMusicTitleUpdate > 100) {
          lastMusicTitleUpdate = millis();
          xPosMusicTitle -= 3;
          int titleWidth = title.length() * 12; 
          if (xPosMusicTitle < -titleWidth) {
            xPosMusicTitle = 128;
          }
        }

        display.setTextSize(1);
        int artistLen = 18;
        if (artist.length() < 18) artistLen = artist.length();
        int artistX = (128 - (artistLen * 6)) / 2;
        if (artistX < 0) artistX = 0;
        display.setCursor(artistX, 24);
        display.println(artist.substring(0, 18));

        if (isPlaying && millis() - lastProgressUpdate > 1900) {
          lastProgressUpdate = millis();
          if (progressWidth < 120) progressWidth++;
        }

        display.drawRect(4, 38, 120, 5, SSD1306_WHITE);
        display.fillRect(4, 38, progressWidth, 5, SSD1306_WHITE);

        display.setTextSize(1);
        display.setCursor(15, 50); display.print("|<");
        display.setCursor(60, 50); display.print(isPlaying ? "||" : ">");
        display.setCursor(100, 50); display.print(">|");
      } else {
        display.setTextSize(1);
        display.setCursor(22, 20);
        display.println("Tidak ada lagu");
        display.setCursor(35, 36);
        display.println("Chronos");
      }
    }
  }

  else if (screenMode == 2) {
    if (isNavActive) {
      if (hasNewNavIcon) display.drawBitmap(0, 8, navIconBuffer, 48, 48, SSD1306_WHITE);
      display.drawLine(50, 0, 50, SCREEN_HEIGHT - 1, SSD1306_WHITE);
      display.setTextSize(1);
      display.setCursor(54, 2);
      display.print(chronos.getTime("%H:%M"));
      display.setCursor(54, 18);
      display.print(navDistance.substring(0, 11));
      display.setCursor(54, 34);
      display.print(navStreet.substring(0, 11));
      if (navStreet.length() > 11) {
        display.setCursor(54, 46);
        display.print(navStreet.substring(11, 22));
      }
    } else {
      display.setTextSize(1);
      display.setCursor(10, 22);
      display.println("NAVIGASI MAPS");
      display.setCursor(0, 38);
      display.println("Menunggu Rute...");
    }
  }

  else if (screenMode == 4) {
    if (waSender.length() > 0 || waMessage.length() > 0) {
      int totalLen = waMessage.length();
      int charsPerPage = 20;
      int totalPages = (totalLen + charsPerPage - 1) / charsPerPage;
      if (totalPages < 1) totalPages = 1;

      if (totalPages > 1 && (millis() - lastWaPageToggle >= 3500)) {
        lastWaPageToggle = millis();
        waPage = (waPage + 1) % totalPages;
      }

      display.setTextSize(2);
      display.setCursor(0, 2);
      display.println(waSender.substring(0, 10));
      display.drawFastHLine(0, 22, 128, SSD1306_WHITE);

      display.setTextSize(2);
      int startIndex = waPage * charsPerPage;
      
      String part1 = "";
      if (startIndex < totalLen) {
        int len1 = (startIndex + 10 <= totalLen) ? 10 : (totalLen - startIndex);
        part1 = waMessage.substring(startIndex, startIndex + len1);
      }
      display.setCursor(0, 28);
      display.println(part1);

      String part2 = "";
      if (startIndex + 10 < totalLen) {
        int len2 = (startIndex + 20 <= totalLen) ? 10 : (totalLen - (startIndex + 10));
        part2 = waMessage.substring(startIndex + 10, startIndex + 10 + len2);
        display.setCursor(0, 46);
        display.println(part2);
      }
    } else {
      display.setTextSize(1);
      display.setCursor(10, 25);
      display.println("PESAN WHATSAPP");
      display.setCursor(0, 42);
      display.println("Menunggu pesan...");
    }
  }

  else if (screenMode == 5) {
    if (textSubMode == 0) {
      display.setTextWrap(false); 
      display.setTextSize(3);     
      display.setCursor(xPosRunning, 20); 
      display.print(runningText);

      if (millis() - lastRunningUpdate > 30) {
        lastRunningUpdate = millis();
        xPosRunning -= 3; 
        int textWidth = runningText.length() * 18; 
        if (xPosRunning < -textWidth) xPosRunning = 128; 
      }
    } 
    else if (textSubMode == 1) {
      unsigned long currentMillis = millis();
      if (showPopupState) {
        if (currentMillis - lastPopupTimer >= POPUP_SHOW_TIME) {
          showPopupState = false;
          lastPopupTimer = currentMillis;
        }
      } else {
        if (currentMillis - lastPopupTimer >= POPUP_HIDE_TIME) {
          showPopupState = true;
          lastPopupTimer = currentMillis;
        }
      }

      if (showPopupState) {
        display.setTextSize(2);
        display.setCursor(0, 22);
        display.println(popupText.substring(0, 10));
      }
    }
  }

  else if (screenMode == 6) {
    AnimatedGIF myGif = {DAICHI_INTRO_FRAME_COUNT, DAICHI_INTRO_WIDTH, DAICHI_INTRO_HEIGHT, daichi_intro_delays, daichi_intro_frames};
    if (millis() - lastGifFrameTime >= pgm_read_word(&myGif.delays[currentGifFrame])) {
      lastGifFrameTime = millis();
      display.clearDisplay();
      playCurrentGIFFrame(&myGif, currentGifFrame);
      display.display();
      currentGifFrame = (currentGifFrame + 1) % myGif.frame_count;
    }
  }

  // MODE 7: MENU OTA UPDATE WI-FI
  else if (screenMode == 7) {
    display.setTextSize(1);
    display.setCursor(16, 8);
    display.println("MENU OTA UPDATE");
    display.drawFastHLine(0, 18, 128, SSD1306_WHITE);
    
    display.setCursor(0, 26);
    display.println("Ketuk 3x: Buka Wi-Fi");
    display.setCursor(0, 42);
    display.println("Ketuk 2x / Tahan: Restart");
  }

  if (!isGifActive && screenMode != 6) display.display();
}
