#ifndef ULTRASOUND_OPENGL_MAIN_H
#define ULTRASOUND_OPENGL_MAIN_H

#include <iostream>
#include <cstdio>
#include <cstring>
#include <vector>
#include <cstdarg>
#include <fstream>
#include "omp.h"
#include <algorithm>
#include <cmath>
#include <cmath>
#include <numeric>
#include <random>
#include <queue>
#include <ctime>
#include <chrono>
#include <string.h>
#include <cinttypes>
#include <cstdint>

#include "densityMap.h"



struct scan_data_struct{
    unsigned long time_stamp;
    unsigned short encoder;
    short buffer[2500];
};

struct screen_data_struct{
    double X;
    double Y;
    double Z;
    double I;
};


#define Cos(th) cos(M_PI/180*(th))
#define Sin(th) sin(M_PI/180*(th))



int compare_crc(unsigned char a[], unsigned char b[], size_t len);
int16_t changed_endian_2Bytes(int16_t value);
unsigned long changed_endian_4Bytes(unsigned long num);
std::vector<int> find_marker(std::vector<unsigned char> _file_bytes);
void file_to_data(std::vector<unsigned char> _file_bytes, std::vector<int> _marker_locations, std::vector<scan_data_struct> & _scan_data);
void data_to_pixel(std::vector<scan_data_struct> _scan_data, std::vector<screen_data_struct> & _screen_data);
uint32_t crc32c(uint32_t crc, const unsigned char *buf, size_t len);
void gainControl(DensityMap& grid, float Gain);
std::vector<unsigned char> readFile(const char* directory);

#endif //ULTRASOUND_OPENGL_MAIN_H