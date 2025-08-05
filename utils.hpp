#pragma once
#include <string>
#include <vector>

#define GREEN "\033[1;32m"
#define YELLOW "\033[1;33m"
#define BLACK "\033[0m"

extern std::string marqueeText;
extern std::string actualMarqueeText;
extern std::vector<std::string> art;

extern int refreshRate;
extern int pollInterval;
extern int marqueeRate;
extern int between;

extern int height;
extern int width;
extern int screenWidth;
extern bool firstRun;

std::string getTime();
void updatePosition();
std::string printGPUSummary();
void printSlider(int offset);
void printMarquee();
std::string commandMsg(std::string msg);
std::vector<std::string> split(std::string str, char delimiter);
int getConsoleWidth();
int getConsoleHeight();
uint64_t getRandomInstructionCount(uint64_t minInstructions, uint64_t maxInstructions);
bool fiftyFiftyChance();
uint8_t getRandomUint8();
uint16_t getRandomUint16();
int getRandomFromRange(int, int);
uint16_t getRandomEvenFrom16Range(uint16_t, uint16_t);
uint16_t getRandomFrom16Range(uint16_t min, uint16_t max);
unsigned int hexToInt(const std::string& hexStr);
std::string intToHex(unsigned int value, int width);
uint16_t intoToBytes(uint8_t first, uint8_t second);
std::pair<uint8_t, uint8_t> splitToBytes(uint16_t value);
uint16_t getRandomMemory(uint16_t min, uint16_t max);
std::vector<std::string> splitString(const std::string& str, char delimiter);