#ifndef __AT_COMMAND_HPP_INCLUDED__
#define __AT_COMMAND_HPP_INCLUDED__

#include <cstdint>
#include <Arduino.h>
#include <vector>

#define MAX_COMMAND_SIZE (200)
#define FIRST_LEVEL_COMMANDS (128)

typedef bool (*FirstLevelCommand)(Stream *answerTo, const char *szString, int length);

class AtCommandAnalyzer {
  private:
  char data[MAX_COMMAND_SIZE];
  char *rPtr;
  char *wPtr;
  uint32_t maxSize;
  uint32_t currentSize;
  FirstLevelCommand firstLevelCommands[FIRST_LEVEL_COMMANDS];
  Stream *serial;

  public:
  AtCommandAnalyzer(void);
  void init(uint32_t size);
  char read(void);
  void reInit(void);
  void checkAndStrip(void);
  void addChar(char car);
  void analyze(char *szString);
  void addCallback(char c, FirstLevelCommand command);
  void setSerial(Stream *s);
};


#endif // __AT_COMMAND_HPP_INCLUDED__
