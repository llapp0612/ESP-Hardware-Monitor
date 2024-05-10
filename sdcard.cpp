#include <SD.h>
#include <SPI.h>
#include <FS.h>
#include <TFT_eSPI.h>
#include "sdcard.h"
#include <SPIFFS.h>
#include "lcd_3inch5.h"


#define  SD_CS 27
#define  SD_CS_1  digitalWrite(SD_CS, HIGH)
#define  SD_CS_0  digitalWrite(SD_CS, LOW)

#define LCD_CS_1 digitalWrite(LCD_CS, HIGH)
#define LCD_CS_0 digitalWrite(LCD_CS, LOW)

void setupPinMode() {
  pinMode(LCD_CS, OUTPUT);
  pinMode(SD_CS, OUTPUT);
}

void SD_Init()
{
  setupPinMode();
  SD_CS_1;
  //LCD_CS_1;

  if (!SD.begin(SD_CS)){
    Serial.println("SD init failed!");
    return;
  }
  else
  {
    Serial.println("SD init OK!");
    initSDSerialConnect();
  }
}

void initSDSerialConnect() {
  SD_CS_0;
  delay(10);
  LCD_CS_1;
  delay(10);
  LCD_DC_1;
  uint8_t cardType = SD.cardType();

  if(cardType == CARD_NONE){
    Serial.println("No SD card attached");
    return;
  }

  Serial.print("SD Card Type: ");
  if(cardType == CARD_MMC){
    Serial.println("MMC");
  } else if(cardType == CARD_SD){
    Serial.println("SDSC");
  } else if(cardType == CARD_SDHC){
    Serial.println("SDHC");
  } else {
    Serial.println("UNKNOWN");
  }

  uint64_t cardSize = SD.cardSize() / (1024 * 1024);
  Serial.printf("SD Card Size: %lluMB\n", cardSize);

  writeFile(SD, "/test.txt", "Hello");
  delay(100);
  listDir(SD, "/", 0);
}

void listDir(fs::FS &fs, const char * dirname, uint8_t levels){
  Serial.printf("Listing directory: %s\n", dirname);

  File root = fs.open(dirname);
  if(!root){
    Serial.println("Failed to open directory");
    return;
  }
  if(!root.isDirectory()){
    Serial.println("Not a directory");
    return;
  }

  File file = root.openNextFile();
  while(file){
    if(file.isDirectory()){
      Serial.print("DIR: ");
      Serial.println(file.name());
      if(levels){
        listDir(fs, file.name(), levels -1);
      }
    } else {
      Serial.print("FILE: ");
      Serial.println(file.name());
      Serial.print("SIZE: ");
      Serial.println(file.size());
    }
    file = root.openNextFile();
  }
}

void createDir(fs::FS &fs, const char * path){
  Serial.printf("Creating Dir: %s\n", path);
  if(fs.mkdir(path)){
    Serial.println("Dir created");
  } else {
    Serial.println("mkdir failed");
  }
}

void removeDir(fs::FS &fs, const char * path){
  Serial.printf("Removing Dir: %s\n", path);
  if(fs.rmdir(path)){
    Serial.println("Dir removed");
  } else {
    Serial.println("rmdir failed");
  }
}

void readFile(fs::FS &fs, const char * path){
  Serial.printf("Reading file: %s\n", path);

  File file = fs.open(path);
  if(!file){
    Serial.println("Failed to open file for reading");
    return;
  }

  Serial.print("Read from file: ");
  while(file.available()){
    Serial.write(file.read());
  }
  file.close();
}

void writeFile(fs::FS &fs, const char * path, const char * message){
  Serial.printf("Writing file: %s\n", path);

  File file = fs.open(path, FILE_WRITE);
  if(!file){
    Serial.println("Failed to open file for writing");
    return;
  }
  if(file.print(message)){
    Serial.println("File written");
  } else {
    Serial.println("Write failed");
  }
  file.close();
}

void appendFile(fs::FS &fs, const char * path, const char * message){
  Serial.printf("Appending to file: %s\n", path);

  File file = fs.open(path, FILE_APPEND);
  if(!file){
    Serial.println("Failed to open file for appending");
    return;
  }
  if(file.print(message)){
      Serial.println("Message appended");
  } else {
    Serial.println("Append failed");
  }
  file.close();
}

void renameFile(fs::FS &fs, const char * path1, const char * path2){
  Serial.printf("Renaming file %s to %s\n", path1, path2);
  if (fs.rename(path1, path2)) {
    Serial.println("File renamed");
  } else {
    Serial.println("Rename failed");
  }
}

void deleteFile(fs::FS &fs, const char * path){
  Serial.printf("Deleting file: %s\n", path);
  if(fs.remove(path)){
    Serial.println("File deleted");
  } else {
    Serial.println("Delete failed");
  }
}

/*
  listDir(SD, "/", 0);
  createDir(SD, "/mydir");
  listDir(SD, "/", 0);
  removeDir(SD, "/mydir");
  listDir(SD, "/", 2);
  writeFile(SD, "/hello.txt", "Hello ");
  appendFile(SD, "/hello.txt", "World!\n");
  readFile(SD, "/hello.txt");
  deleteFile(SD, "/foo.txt");
  renameFile(SD, "/hello.txt", "/foo.txt");
  readFile(SD, "/foo.txt");
  testFileIO(SD, "/test.txt");
  Serial.printf("Total space: %lluMB\n", SD.totalBytes() / (1024 * 1024));
  Serial.printf("Used space: %lluMB\n", SD.usedBytes() / (1024 * 1024));
*/

void switchSPIFromTFTToSD(bool toSD) {
  if(toSD){
    SD_CS_0; // Chip-Select-Pin für SD-Karte auf LOW setzen
    delay(10);
    LCD_CS_1; // Chip-Select-Pin für TFT-Display auf HIGH setzen
    delay(10);
    LCD_DC_1; // Modus-Pin für TFT-Display auf HIGH setzen (je nach Anforderungen des Displays)
  } else {
    SD_CS_1; // Chip-Select-Pin für SD-Karte auf HIGH setzen
    delay(10);
    LCD_CS_0; // Chip-Select-Pin für TFT-Display auf LOW setzen
    delay(10);
    LCD_DC_0; // Modus-Pin für TFT-Display auf LOW setzen (je nach Anforderungen des Displays)
  }
}


