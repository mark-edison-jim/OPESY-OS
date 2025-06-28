#include "utils.hpp"
#include <iostream>
#include <sstream>
#include <thread>
#include <chrono>
#include <string>
#include <vector>
#include "Figlet.hh"
#define NOMINMAX
#include <Windows.h>
#undef max
#include <ctime>
#include <sstream>
#include <chrono>
#include <iomanip>
#include <random>
#include <cstdint>
#include <limits>

#define RESET   "\033[0m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define CYAN    "\033[36m"

int colorIndex = 0;
std::vector<std::string> bounceColors = { RED, GREEN, YELLOW, CYAN };

std::string marqueeText = "CSOPESY";
std::vector<std::string> art = Figlet::standard.print(marqueeText.c_str());


//std::string actualMarqueeText = "Testing Hello World";
int refreshRate = 17;
int pollInterval = 4;
int marqueeRate = refreshRate / 4;
int between = 5;
bool hittingWall = false;


int dx = 0, dy = 0;
int dx_dir = 1, dy_dir = 1;

int height = art.size();
int width = art[0].size();

int screenWidth = width * 1.1;
bool firstRun = true;

int maxProcessNameLength = 32;

int getConsoleWidth() {
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    return csbi.srWindow.Right - csbi.srWindow.Left + 1;
}

int getConsoleHeight() {
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    return csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
}

bool fiftyFiftyChance() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::bernoulli_distribution dist(0.5); // 50% true, 50% false
    return dist(gen);
}

int getRandomFromRange(int min, int max) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distr(min, max);

    return distr(gen);
}

uint16_t getRandomUint16() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<uint16_t> distr(0, std::numeric_limits<uint16_t>::max());

    return distr(gen);
}

uint8_t getRandomUint8() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> distr(0, std::numeric_limits<uint8_t>::max());

    return static_cast<uint8_t>(distr(gen));
}

uint64_t getRandomInstructionCount(uint64_t minInstructions, uint64_t maxInstructions) {
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<uint64_t> distr(minInstructions, maxInstructions);
    return distr(gen);
}

int spaceWidth = getConsoleWidth();
int spaceHeight = getConsoleHeight()/2;

std::string getTime() {
    std::time_t t = std::time(nullptr);
    std::tm localTime;
    localtime_s(&localTime, &t);
    std::ostringstream oss;
    oss << std::put_time(&localTime, "%m/%d/%Y, %I:%M:%S %p");
	return oss.str();
}

std::string generateRandomString(size_t length) {
    const std::string charset = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(0, charset.size() - 1);

    std::string result;
    for (size_t i = 0; i < length; ++i) {
        result += charset[dist(gen)];
    }
    return result;
}

std::string printGPUSummary() {
    std::srand(static_cast<unsigned int>(std::time(nullptr)));
    std::vector<std::vector<std::string>> processInfo;

    std::vector<std::string> processNames = {
       "C:\\Users\\Alice\\Anaconda3\\envs\\torch\\python.exe",
       "C:\\Program Files\\Google\\Chrome\\Application\\chrome.exe",
       "C:\\Users\\Bob\\Projects\\DeepLearning\\train_model.py",
       "C:\\Program Files\\Blender Foundation\\Blender 3.6\\blender.exe",
       "C:\\Games\\Cyberpunk2077\\bin\\x64\\Cyberpunk2077.exe"
    };

	for (int i = 0; i < 5; ++i) {
        int memUsage = std::rand() % (99999 + 1);
        std::string gpuUsage = std::to_string(memUsage) + " MiB";

        int randNamelength = 10 + std::rand() % (50);

		//std::string processName = generateRandomString(randNamelength) + ".exe";
		std::string processName = processNames[i];
		int pnLength = processName.length();
		if (processName.length() > maxProcessNameLength) {
			processName = "..." + processName.substr(pnLength - maxProcessNameLength, pnLength); // Truncate if too long
		}

		processInfo.push_back({ "0", "N/A", "N/A", std::to_string(1000 + i), 
            (std::rand() % 2 == 0) ? "C" : "C+G", processName, gpuUsage });
	}
    std::ostringstream out;
    std::string topSummary = R"(
+-----------------------------------------------------------------------------------------+
| NVIDIA-SMI 572.42                 Driver Version: 572.42         CUDA Version: 12.8     |
|-----------------------------------------+------------------------+----------------------+
| GPU  Name                  Driver-Model | Bus-Id          Disp.A | Volatile Uncorr. ECC |
| Fan  Temp   Perf          Pwr:Usage/Cap |           Memory-Usage | GPU-Util  Compute M. |
|                                         |                        |               MIG M. |
|=========================================+========================+======================|
|   0  NVIDIA GeForce RTX 4060 ...  WDDM  |   00000000:01:00.0 Off |                  N/A |
| N/A   47C    P3             11W /   55W |       0MiB /   8188MiB |      0%      Default |
|                                         |                        |                  N/A |
+-----------------------------------------+------------------------+----------------------+
)";

    std::string bottomSummary = R"(
+-----------------------------------------------------------------------------------------+
| Processes:                                                                              |
|  GPU   GI   CI              PID   Type   Process name                        GPU Memory |
|        ID   ID                                                               Usage      |
|=========================================================================================|
)";

    
	out << getTime() << std::endl;
    out << topSummary << std::endl;
    out << bottomSummary;

	for (int i = 0; i < processInfo.size(); ++i) {
		out << "|" << std::setw(5) << processInfo[i][0]
            << std::setw(5) << processInfo[i][1]
            << std::setw(5) << processInfo[i][2]
            << std::setw(17) << processInfo[i][3]
            << std::setw(7) << processInfo[i][4]
            << std::setw(2) << " "
            << std::left << std::setw(37) << processInfo[i][5]
            << std::right << std::setw(10) << processInfo[i][6] << " |" << "\n";
	}

	out << "+-----------------------------------------------------------------------------------------+\n";

    return out.str();
}

std::string commandMsg(std::string msg) {
    return YELLOW + msg + BLACK + '\n';
}

static void setCursorPosition(int x, int y) {
    COORD pos = { (SHORT)x, (SHORT)y };
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
}
static COORD getCursorPosition() {
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    COORD pos = { 0, 0 };
    if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi)) {
        pos = csbi.dwCursorPosition;
    }
    return pos;
}

void updatePosition() {
    int artHeight = art.size();
    int artWidth = art[0].size();

    int maxX = spaceWidth - artWidth;
    int maxY = spaceHeight - artHeight;

    dx += dx_dir;
    dy += dy_dir;

    bool hitWall = false;

    if (dx <= 0 || dx >= maxX) {
        dx_dir *= -1;
        dx += dx_dir;
        hitWall = true;
    }

    if (dy <= 0 || dy >= maxY) {
        dy_dir *= -1;
        dy += dy_dir;
        hitWall = true;
    }

    if (hitWall) {
        colorIndex = (colorIndex + 1) % bounceColors.size();
    }
}

void printMarquee() {
    static int prevDx = -1, prevDy = -1;
    static int frameCount = 0;
    static const int bounceSpeed = 4;

    int artHeight = art.size();
    int artWidth = art[0].size();

    // Clear previous
    if (prevDx >= 0 && prevDy >= 0) {
        for (int i = 0; i < artHeight; ++i) {
            if (prevDy + i < spaceHeight) {
                setCursorPosition(prevDx, prevDy + i);
                std::cout << std::string(artWidth, ' ');
            }
        }
    }

    std::string color = bounceColors[colorIndex];
    for (int i = 0; i < artHeight; ++i) {
        if (dy + i < spaceHeight) {
            setCursorPosition(dx, dy + i);
            std::cout << color << art[i] << RESET;
        }
    }

    prevDx = dx;
    prevDy = dy;

    frameCount++;
    if (frameCount >= bounceSpeed) {
        updatePosition();
        frameCount = 0;
    }

    setCursorPosition(0, spaceHeight);
}


void printSlider(int offset) {
    int modulo = offset;

    if (firstRun && offset == screenWidth)
        firstRun = false;

    if (!firstRun) {
        int scale = (between + width);
        modulo = (offset - screenWidth) % scale;
        if (modulo < 0) modulo += scale; 
    }

    for (int i = 0; i < height; ++i) {
        std::string line = art[i];
        std::string wordSpace(between, ' ');

        std::string scrolled = (firstRun ? std::string(screenWidth, ' ') : "")
            + line + wordSpace + line + wordSpace;

        if (modulo + screenWidth > scrolled.size()) {
            scrolled += std::string((modulo + screenWidth) - scrolled.size(), ' ');
        }

        std::cout << scrolled.substr(modulo, screenWidth) << std::endl;
    }

    std::cout << GREEN << "Hello, Welcome to CSOPESY commandline!" << BLACK << std::endl;
    std::cout << YELLOW << "Type 'exit' to quit, 'clear' to clear the screen" << BLACK << std::endl;
}


std::vector<std::string> split(std::string str, char delimiter) {
    if (str.length() == 0) return std::vector<std::string>(1, str);
    std::stringstream ss(str);
    std::vector<std::string> res;
    std::string token;
    while (getline(ss, token, delimiter))
        res.push_back(token);

    return res;
}
