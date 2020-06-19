#ifndef ULTRASOUND_OPENGL_MAIN_H
#define ULTRASOUND_OPENGL_MAIN_H

#include <iostream>
#include <cstdio>
#include <cstring>
#include <vector>
#include <cstdarg>
#include <fstream>
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
#include <mutex>

#include <cstdlib>
#include <stdlib.h>
#include <stdio.h>
//#pragma comment(lib, "libws2_32.a")
// for linux UDP
#include <netinet/in.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "densityMap.h"
#include "rotation.h"
#include "remote.h"

#include <cstdlib> // for srand() and rand()



struct scan_data_struct{
    unsigned char version;
    unsigned long time_stamp;
    unsigned short encoder;
    unsigned short lx16;
    float quaternion[4];
    short buffer[2500];
};

struct screen_data_struct{
    double X;
    double Y;
    double Z;
    double I;
};

struct line_data_struct{
    glm::vec3 p1; //start point. on default (dim/2, dim, dim/2) (top middle)
    glm::vec3 p2; //end point. need calculation.
    std::vector<unsigned char> vals; //Intensities
};


#define Cos(th) cos(M_PI/180*(th))
#define Sin(th) sin(M_PI/180*(th))



int compare_crc(unsigned char a[], unsigned char b[], size_t len);
int16_t changed_endian_2Bytes(int16_t value);
unsigned long changed_endian_4Bytes(unsigned long num);
std::vector<int> find_marker(std::vector<unsigned char> _file_bytes);
void file_to_data(std::vector<unsigned char> _file_bytes, std::vector<int> _marker_locations, std::vector<scan_data_struct> & _scan_data);
void data_to_pixel(std::vector<scan_data_struct> _scan_data, std::vector<line_data_struct>& _line_data);
uint32_t crc32c(uint32_t crc, const unsigned char *buf, size_t len);
float ReverseFloat( const float inFloat );
void gainControl(DensityMap& grid, float Gain, bool& dataUpdate);
std::vector<unsigned char> readFile(const char* directory);
void realDemo(DensityMap& grid, bool& dataUpdate);
void fakeDemo(DensityMap& grid, bool& dataUpdate);

void readDataSubmarine(DensityMap& grid, const char* fileName, float Gain, int len, bool& dataUpdate);
void readDataWhitefin(DensityMap& grid, const char* fileName, float Gain, int len, bool& dataUpdate, bool& error, std::string& errorMessage);
void readDataTest(DensityMap& grid, const char* fileName, float Gain, int len, bool& dataUpdate);
std::vector<line_data_struct> file_to_pixel_V07(std::vector<unsigned char> _file_bytes, std::vector<int> _marker_locations);
std::vector<line_data_struct> file_to_pixel_V06(std::vector<unsigned char> _file_bytes, std::vector<int> _marker_locations);
std::vector<line_data_struct> file_to_pixel_V08(std::vector<unsigned char> _file_bytes, std::vector<int> _marker_locations);

/* real-time based on TCP */
void realDemo2(DensityMap& grid, bool& dataUpdate);
void readSubfile(std::vector<unsigned char> file_bytes, std::vector<int> marker_locations, int sub_length,
                 bool& newDataline);

/* Gantry (for multiple files) */
void realDemo3(DensityMap& grid, bool& dataUpdate);

/* real-time based on UDP */
void realDemo4(DensityMap& grid, bool& dataUpdate);
void UDP_timer(int& time_milisecond);
void render_lines(DensityMap& grid, std::vector<line_data_struct> line_data);

/* ALL-IN-ONE for GUI */
bool connectToProbe(DensityMap& grid, std::string probeIP, std::string username, std::string password, std::string compIP,
                    bool isSubmarine,
                    int lxRangeMin, int lxRangeMax, int lxRes, int servoRangeMin, int servoRangeMax, int servoRes,
                    std::string customCommand,
                    int connectionType, std::string& output, bool& connected, bool& error, std::string& errorMessage
);
void live_rendering(DensityMap& grid, bool isSubmarine, std::string probeIP, std::string compIP, bool& transmit_end);

int getDepth();
void setDepth(int dep);
void setGain(float g);
float getGain();
int getSamples();

#endif //ULTRASOUND_OPENGL_MAIN_H