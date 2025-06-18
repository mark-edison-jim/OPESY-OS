#include <sstream>
#include <iostream>
#include <string.h>
#include <stdio.h>
#include <vector>
#include <math.h>
#include <chrono>
#include <thread>
#include <Windows.h>
#include "Cube.hpp"


float Cube::calculateX(int i, int j, int k) {
    return j * sin(A) * sin(B) * cos(C) - k * cos(A) * sin(B) * cos(C) +
        j * cos(A) * sin(C) + k * sin(A) * sin(C) +
        i * cos(B) * cos(C);
}

float Cube::calculateY(int i, int j, int k) {
    return j * cos(A) * cos(C) + k * sin(A) * cos(C) -
        j * sin(A) * sin(B) * sin(C) + k * cos(A) * sin(B) * sin(C) -
        i * cos(B) * sin(C);
}

float Cube::calculateZ(int i, int j, int k) {
    return k * cos(A) * cos(B) - j * sin(A) * cos(B) + i * sin(B);
}

void Cube::calculateSurface(float cubeX, float cubeY, float cubeZ, char symbol) {
    float x = calculateX(cubeX, cubeY, cubeZ);
    float y = calculateY(cubeX, cubeY, cubeZ);
    float z = calculateZ(cubeX, cubeY, cubeZ) + 100;

    float z_reciprocal = 1 / z;


    int x_plane = (width / 2 + cubeWidth * z_reciprocal * x * 2);
    int y_plane = (height / 2 + cubeWidth * z_reciprocal * y);

    int idx = x_plane + y_plane * width;

    if (idx >= 0 && idx < width * height) {
        if (z_reciprocal > zBuffer[idx]) {
            zBuffer[idx] = z_reciprocal;
            buffer[idx] = symbol;
        }
    }       
}

void Cube::printCube() {
    std::fill(buffer.begin(), buffer.end(), background_character);
    std::fill(zBuffer.begin(), zBuffer.end(), 0.0f);

    for (float cube_x = -cubeWidth; cube_x < cubeWidth; cube_x += speed) {
        for (float cube_y = -cubeWidth; cube_y < cubeWidth; cube_y += speed) {
            calculateSurface(cube_x, cube_y, -cubeWidth, '.'); // side 1 of the cube.
            calculateSurface(cubeWidth, cube_y, cube_x, '-'); // side 2 of the cube.
            calculateSurface(-cubeWidth, cube_y, -cube_x, ':'); // side 3 of the cube.
            calculateSurface(-cube_x, cube_y, cubeWidth, '='); // side 4 of the cube.
            calculateSurface(cube_x, -cubeWidth, -cube_y, '+'); // side 5 of the cube.
            calculateSurface(cube_x, cubeWidth, cube_y, '~'); // side 6 of the cube.
        }
    }

    //system("cls");
    //for (int i = 0; i < width * height; ++i) {
    //    std::cout << ((i % width) ? buffer[i] : '\n');
    //}
	drawToConsole(buffer, width, height);

    A += 0.05;
    B += 0.05;
    C += 0.05;
}

void Cube::drawToConsole(const std::vector<char>& buffer, int width, int height) {
    std::cout << "\x1b[H";  // Move cursor to top-left

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            std::cout << buffer[y * width + x];
        }
        std::cout << '\n';
    }

    // Move cursor BELOW the cube (e.g., line 21)
    std::cout << "\x1b[" << (height + 1) << ";1H";
}


bool Cube::set_speed(float speed) {
    if (speed < 0) return false;
    speed = speed;
    return true;
}

bool Cube::set_background(char c) {
    background_character = c;
    return true;
}