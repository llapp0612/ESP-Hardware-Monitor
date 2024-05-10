#ifndef SDCARD_H
#define SDCARD_H

void SD_Init(void);

void listDir(fs::FS &fs, const char * dirname, uint8_t levels);
void createDir(fs::FS &fs, const char * path);
void removeDir(fs::FS &fs, const char * path);
void writeFile(fs::FS &fs, const char * path, const char * message);
void appendFile(fs::FS &fs, const char * path, const char * message);
void readFile(fs::FS &fs, const char * path);
void deleteFile(fs::FS &fs, const char * path);
void renameFile(fs::FS &fs, const char * path1, const char * path2);
void readFile(fs::FS &fs, const char * path);

void switchSPIFromTFTToSD(bool toSD);
void setupPinMode(void);
void initSDSerialConnect(void);

#endif