#pragma once
#include <vector>

class Cube {
private:
    float A, B, C;

    float cubeWidth;
    int width, height;
    float speed;

    std::vector<float> zBuffer;
    std::vector<char> buffer;
    char background_character;

    int horizontal_offset;

    float calculateX(int i, int j, int k);
    float calculateY(int i, int j, int k);
    float calculateZ(int i, int j, int k);

    void calculateSurface(float cube_x, float cube_y, float cube_z, char symbol);

public:
    Cube(float cube_size, int window_width, int window_height, float speed)
        : A(0), B(0), C(0), cubeWidth(cube_size), width(window_width), height(window_height), speed(speed), horizontal_offset(-1.5 * cubeWidth) {
        zBuffer.resize(width * height);
        buffer.resize(width * height);
    }

    void drawToConsole(const std::vector<char>& buffer, int width, int height);

    int get_window_width() const;
    int get_window_height() const;

    bool set_speed(float speed);
    bool set_background(char c);
    bool set_window_width(int width);
    bool set_window_height(int height);

    void printCube();
};
