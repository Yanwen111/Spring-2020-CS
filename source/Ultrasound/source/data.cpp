#include <iostream>
#include <thread>
#include <chrono>
#include <exception>

#include "data.h"
#include "rotation.h" /* for test of fake data */

/* For calculating the scale */
#define Velocity 153800  // speed in phantom, cm/s
#define Frequency 15.6  // MHz

/* Data Processing */
const unsigned char marker[10] = {0x00,0x01,0x00,0x01,0x00,0x01,0x00,0x01,0x00,0x01};
int marker_index, marker_index_next, buffer_length, signal_draw_index;
int16_t adc_max = 0;
int16_t adc_min = 0;
bool marker_flag;
unsigned char time_stamp_char[4];
unsigned long time_stamp = 0;
unsigned char quaternion_char[16];
float quaternion[4];
unsigned char probe_type_char;
unsigned char encoder_char[2];
unsigned short encoder;
unsigned char lx16_char[2]; /* data of the lx16. sth similar to encoder */
unsigned short lx16;
unsigned char qua_wxyz_char[16];
unsigned char adc_char[2*2500];
unsigned char adc_temp[2];
unsigned char crc_char[4];
uint32_t crc_result;
unsigned char crc_result_char[4];
unsigned char crc_input[4+1+2+2+0+2*2500]; /* 0 stands for no IMU data. This length is for v0.7 early */
unsigned char message_buff[10+4+1+2+16+2*2500+4];
int16_t adc;
short buffer[2500];
double intensity;
unsigned char information_byte = 0xE1;

#ifdef __APPLE__
#define MSG_NOSIGNAL 0
#endif

int DEPTH = 2500;
float GAIN = 1.0;
float ROTATION = 0.0;
glm::vec4 ROTATION_QUA = {0.0, 0.0, 0.0, 1.0};
float* GAIN_PTR = &GAIN;
int * DEPTH_PTR = &DEPTH;
float* ROTATION_PTR = &ROTATION;
glm::vec4* ROTATION_QUA_PTR = &ROTATION_QUA;
int samples = -1; /* -1 stands for no data */

bool isFilter = false;
std::vector<double> filter_list;

/* These are the crc input length for different data format */
int V06_TOTAL_LENGTH = 4+1+2+16+2*2500;
int V07_TOTAL_LENGTH = 4+1+2+2+0+2*2500;
int V08_TOTAL_LENGTH = 1+4+1+2+2+16+2*2500;

std::mutex time_mutex; /* For the timer of live rendering */
std::mutex rotate_mutex; /* For the rotation parameters of the probe model */

std::vector<line_data_struct> file_to_pixel_V07(std::vector<unsigned char> _file_bytes, std::vector<int> _marker_locations)
{
    std::vector<scan_data_struct> scan_data;
    std::vector<line_data_struct> line_data;
    unsigned char crc_input_V07[4+1+2+2+0+2*2500]; /* 0 stands for no IMU data */

    for (int i = 0; i < (int)_marker_locations.size()-1; ++i){
        marker_index = _marker_locations.at(i);
        marker_index_next = _marker_locations.at(i+1);
        /* time stamp */
        for (int j = 0; j < (int)sizeof(time_stamp_char); ++j){
            time_stamp_char[j] = _file_bytes.at(marker_index + sizeof(marker) + j);
        }
        std::memcpy(&time_stamp, time_stamp_char, sizeof(time_stamp));
        time_stamp = changed_endian_4Bytes(time_stamp);
        /* probe type char */
        probe_type_char = _file_bytes.at(marker_index + sizeof(marker) + sizeof(time_stamp_char));
        /* encoder */
        for (int j = 0; j < (int) sizeof(encoder_char); ++j){
            encoder_char[j] = _file_bytes.at(marker_index + sizeof(marker) + sizeof(time_stamp_char) + sizeof(probe_type_char) + j);
        }
        std::memcpy(&encoder, encoder_char, sizeof(encoder));
        encoder = changed_endian_2Bytes(encoder);

        /* lx16 */
        for (int j = 0; j < (int) sizeof(lx16_char); ++j){
            lx16_char[j] = _file_bytes.at(marker_index + sizeof(marker) + sizeof(time_stamp_char) + sizeof(probe_type_char) + sizeof(encoder_char) + j);
        }
        std::memcpy(&lx16, lx16_char, sizeof(lx16));
        lx16 = changed_endian_2Bytes(lx16);

        /* IMU */
//        for (int j = 0; j < (int) sizeof(quaternion_char); ++j){
//            quaternion_char[j] = _file_bytes.at(marker_index + sizeof(marker) + sizeof(time_stamp_char) + sizeof(probe_type_char) +
//                                                        sizeof(encoder_char) + j);
//        }
//        for (int j = 0; j < 16; j += 4)
//        {
//            unsigned char temp1[4];
//            for (int k = 0; k < 4; ++k) temp1[k] = quaternion_char[j+3-k];
//            quaternion[j/4] = *(float*)temp1;
//        }
        quaternion[0] = 1;
        quaternion[1] = 0;
        quaternion[2] = 0;
        quaternion[3] = 0;
        /* adc */
        /* determine the length of buffer */
        buffer_length = (int)(_marker_locations.at(i+1) - _marker_locations.at(i) - sizeof(marker) - sizeof(time_stamp_char) -
                              sizeof(probe_type_char) - sizeof(encoder_char) - sizeof(lx16_char) - sizeof(crc_char))/2;
        for (int j = 0; j < buffer_length; ++j){
            for (int k = 0; k < (int)sizeof(adc_temp); ++k){
                adc_temp[k] = _file_bytes.at(marker_index + sizeof(marker) + sizeof(time_stamp_char) + sizeof(probe_type_char) + sizeof(encoder_char) + sizeof(lx16_char) + j * 2 + k);
                adc_char[2*j+k] = adc_temp[k];
            }
            std::memcpy(&adc, adc_temp, sizeof(adc));
            adc = changed_endian_2Bytes(adc);
            buffer[j] = adc;
        }
        /* checksum */
        for (int j = 0; j < (int)sizeof(crc_char); ++j){
            crc_char[j] = _file_bytes.at(marker_index_next-(int)sizeof(crc_char)+j);
        }
        /* calculate crc locally */
        memcpy(crc_input_V07, time_stamp_char, sizeof(time_stamp_char));
        memcpy(crc_input_V07+sizeof(time_stamp_char), &probe_type_char, sizeof(probe_type_char));
        memcpy(crc_input_V07+sizeof(time_stamp_char)+sizeof(probe_type_char), encoder_char, sizeof(encoder_char));
        memcpy(crc_input_V07+sizeof(time_stamp_char)+sizeof(probe_type_char)+sizeof(encoder_char), lx16_char, sizeof(lx16_char));
        memcpy(crc_input_V07+sizeof(time_stamp_char)+sizeof(probe_type_char)+sizeof(encoder_char)+ sizeof(lx16_char), adc_char, sizeof(adc_char));
        crc_result = crc32c(0, crc_input_V07, sizeof(crc_input_V07));
        crc_result = changed_endian_4Bytes(crc_result);
        memcpy(crc_result_char, (unsigned char *)&crc_result, sizeof (crc_result));

        /* if two crc matches */
        if (compare_crc(crc_char, crc_result_char, sizeof(crc_char))){
            //if(1){  // uncommented this for store all the data
            scan_data_struct temp_struct;
            temp_struct.time_stamp = time_stamp;
            for (int j = 0; j < 4; ++j) temp_struct.quaternion[j] = quaternion[j];
            temp_struct.encoder = encoder;
            temp_struct.lx16 = lx16;
            /* normalize on the go */
            for (int j = 0; j < buffer_length; ++j) {
                temp_struct.buffer[j] = buffer[j];
                //printf("Intensity:%f\n", temp_struct.buffer[j]);
            }
            scan_data.push_back(temp_struct);
        }
    }

    //write probe data to test
    //printf("start write imu...\n");
    std::ofstream fileout("data/real_imu.txt", std::ios::trunc|std::ios::out);
    for (auto s: scan_data){
        fileout << s.quaternion[0] << ' ' << s.quaternion[1] << ' ' << s.quaternion[2] << ' ' << s.quaternion[3] << std::endl;
    }
    fileout.close();
    //printf("Real IMU file generate!\n");

    for (int i = 0; i < (int)scan_data.size(); ++i)
    {
        double angle = scan_data.at(i).encoder * 360.0 / 4096.0;
        float piezo = angle + 175;
        /* angle of the lx16 */
        float angle_16 = scan_data.at(i).lx16 * 360.0 / 4096.0;
        /* find min and max */
        for (int j = 0; j < buffer_length; ++j){
            adc_max = std::max(adc_max, scan_data.at(i).buffer[j]);
            adc_min = std::min(adc_min, scan_data.at(i).buffer[j]);
        }
        int piezoProbe = 101; /*assume v = 1000 m/s */

        line_data_struct dataline;
        dataline.p1 = {piezoProbe*Cos(piezo),piezoProbe*Sin(piezo), 0};
        /* normalize on the go */
        for (int j = 0; j < buffer_length; ++j){
            intensity = ((double)scan_data.at(i).buffer[j] - adc_min)/(adc_max-adc_min);
            dataline.vals.push_back(static_cast<unsigned char>(intensity*255));
        }
        dataline.p2 = {(buffer_length+piezoProbe)*Cos(piezo), (buffer_length+piezoProbe)*Sin(piezo), 0};
        glm::mat4 rot = glm::mat4(1.0f);
        rot = glm::rotate(rot, glm::radians(angle_16) , glm::vec3(0, 1, 0)); /* inverse later to compare */
        dataline.p1 = rot * glm::vec4(dataline.p1,1);
        dataline.p2 = rot * glm::vec4(dataline.p2,1);
        line_data.push_back(dataline);
        adc_max = 0; adc_min = 0;
    }
    return line_data;
}

void readDataSubmarine(DensityMap& grid, const char* fileName, float Gain, int len, bool& dataUpdate)
{
    std::vector<unsigned char> file_bytes;
    std::vector<int> marker_locations;
    std::vector<line_data_struct> line_data;

    //file_bytes = readFile(fileName);
    try {
        file_bytes = readFile(fileName);
    } catch(const char* msg){
        std::cerr << msg << std::endl;
    }
    /* find all marker locations */
    marker_locations = find_marker(file_bytes);
    /* convert file bytes to data struct */
    line_data = file_to_pixel_V06(file_bytes, marker_locations);
    printf("find the screen_data\n");

    for  (auto l: line_data)
    {
        glm::vec3 ps = {l.p1.x/len + 0.5, l.p1.y/len + 1, l.p1.z/len + 0.5};
        glm::vec3 pe = {l.p2.x/len - l.p1.x/len  + 0.5, l.p2.y/len - l.p1.y/len + 1, l.p2.z/len - l.p1.z/len +0.5};
        for (int i = 0; i < l.vals.size(); ++i)
        {
            l.vals[i] = static_cast<unsigned char>(std::min(static_cast<int>((l.vals[i])*exp(Gain*(i/len))), 255));
        }
        *ROTATION_QUA_PTR = l.quat;
        grid.writeLine(ps, pe, l.vals);
    }

    dataUpdate = true;
}

std::vector<line_data_struct> file_to_pixel_V06(std::vector<unsigned char> _file_bytes, std::vector<int> _marker_locations)
{
    std::vector<scan_data_struct> scan_data;
    std::vector<line_data_struct> line_data;
    unsigned char crc_input_V06[4+1+2+16+2*2500]; /* 16 stands for IMU data */

    for (int i = 0; i < (int)_marker_locations.size()-1; ++i){
        marker_index = _marker_locations.at(i);
        marker_index_next = _marker_locations.at(i+1);
        /* time stamp */
        for (int j = 0; j < (int)sizeof(time_stamp_char); ++j){
            time_stamp_char[j] = _file_bytes.at(marker_index + sizeof(marker) + j);
        }
        std::memcpy(&time_stamp, time_stamp_char, sizeof(time_stamp));
        time_stamp = changed_endian_4Bytes(time_stamp);
        /* probe type char */
        probe_type_char = _file_bytes.at(marker_index + sizeof(marker) + sizeof(time_stamp_char));
        /* encoder */
        for (int j = 0; j < (int) sizeof(encoder_char); ++j){
            encoder_char[j] = _file_bytes.at(marker_index + sizeof(marker) + sizeof(time_stamp_char) + sizeof(probe_type_char) + j);
        }
        std::memcpy(&encoder, encoder_char, sizeof(encoder));
        encoder = changed_endian_2Bytes(encoder);

        /* lx16 */
        lx16 = 0;

        /* IMU */
        for (int j = 0; j < (int) sizeof(quaternion_char); ++j){
            quaternion_char[j] = _file_bytes.at(marker_index + sizeof(marker) + sizeof(time_stamp_char) + sizeof(probe_type_char) +
                                                sizeof(encoder_char) + j);
        }
        for (int j = 0; j < 16; j += 4)
        {
            unsigned char temp1[4];
            for (int k = 0; k < 4; ++k) temp1[k] = quaternion_char[j+3-k];
            quaternion[j/4] = *(float*)temp1;
        }
        /* adc */
        /* determine the length of buffer */
        buffer_length = (int)(_marker_locations.at(i+1) - _marker_locations.at(i) - sizeof(marker) - sizeof(time_stamp_char) -
                              sizeof(probe_type_char) - sizeof(encoder_char) - sizeof(quaternion_char) - sizeof(crc_char))/2;
        for (int j = 0; j < buffer_length; ++j){
            for (int k = 0; k < (int)sizeof(adc_temp); ++k){
                adc_temp[k] = _file_bytes.at(marker_index + sizeof(marker) + sizeof(time_stamp_char) + sizeof(probe_type_char) + sizeof(encoder_char) + sizeof(quaternion_char) + j * 2 + k);
                adc_char[2*j+k] = adc_temp[k];
            }
            std::memcpy(&adc, adc_temp, sizeof(adc));
            adc = changed_endian_2Bytes(adc);
            buffer[j] = adc;
        }
        /* checksum */
        for (int j = 0; j < (int)sizeof(crc_char); ++j){
            crc_char[j] = _file_bytes.at(marker_index_next-(int)sizeof(crc_char)+j);
        }
        /* calculate crc locally */
        memcpy(crc_input_V06, time_stamp_char, sizeof(time_stamp_char));
        memcpy(crc_input_V06+sizeof(time_stamp_char), &probe_type_char, sizeof(probe_type_char));
        memcpy(crc_input_V06+sizeof(time_stamp_char)+sizeof(probe_type_char), encoder_char, sizeof(encoder_char));
        memcpy(crc_input_V06+sizeof(time_stamp_char)+sizeof(probe_type_char)+sizeof(encoder_char), quaternion_char, sizeof(quaternion_char));
        memcpy(crc_input_V06+sizeof(time_stamp_char)+sizeof(probe_type_char)+sizeof(encoder_char)+ sizeof(quaternion_char), adc_char, sizeof(adc_char));
        crc_result = crc32c(0, crc_input_V06, sizeof(crc_input_V06));
        crc_result = changed_endian_4Bytes(crc_result);
        memcpy(crc_result_char, (unsigned char *)&crc_result, sizeof (crc_result));

        /* if two crc matches */
        if (compare_crc(crc_char, crc_result_char, sizeof(crc_char))){
            //if(1){  // uncommented this for store all the data
            scan_data_struct temp_struct;
            temp_struct.time_stamp = time_stamp;
            for (int j = 0; j < 4; ++j) temp_struct.quaternion[j] = quaternion[j];
            temp_struct.encoder = encoder;
            temp_struct.lx16 = lx16;
            /* normalize on the go */
            for (int j = 0; j < buffer_length; ++j) {
                temp_struct.buffer[j] = buffer[j];
                //printf("Intensity:%f\n", temp_struct.buffer[j]);
            }
            scan_data.push_back(temp_struct);
        }
    }

    //write probe data to test
    //printf("start write imu...\n");
    std::ofstream fileout("data/real_imu.txt", std::ios::trunc|std::ios::out);
    for (auto s: scan_data){
        fileout << s.quaternion[0] << ' ' << s.quaternion[1] << ' ' << s.quaternion[2] << ' ' << s.quaternion[3] << std::endl;
    }
    fileout.close();
    //printf("Real IMU file generate!\n");

    for (int i = 0; i < (int)scan_data.size(); ++i)
    {
        double angle = scan_data.at(i).encoder * 360.0 / 4096.0;
        double ax = 9*Cos(angle - 222 );
        double ay = 9*Sin(angle - 222 );
        double piezo = atan2(ay+21, ax) * 180.0 / M_PI - 180.0;
        /* find min and max */
        for (int j = 0; j < buffer_length; ++j){
            adc_max = std::max(adc_max, scan_data.at(i).buffer[j]);
            adc_min = std::min(adc_min, scan_data.at(i).buffer[j]);
        }

        int piezoProbe = 101; /*assume v = 1000 m/s */

        line_data_struct dataline;
        dataline.p1 = {piezoProbe*Cos(piezo),piezoProbe*Sin(piezo), 0};
        /* normalize on the go */
        for (int j = 0; j < buffer_length; ++j){
            intensity = ((double)scan_data.at(i).buffer[j] - adc_min)/(adc_max-adc_min);
            dataline.vals.push_back(static_cast<unsigned char>(intensity*255));
        }
        dataline.p2 = {(buffer_length+piezoProbe)*Cos(piezo), (buffer_length+piezoProbe)*Sin(piezo), 0};
        glm::mat4 rot = Rotation::convertRotationMatrix(scan_data.at(i).quaternion[0], scan_data.at(i).quaternion[1],
                                                        scan_data.at(i).quaternion[2], scan_data.at(i).quaternion[3]);
        dataline.p1 = rot * glm::vec4(dataline.p1,1);
        dataline.p2 = rot * glm::vec4(dataline.p2,1);
        line_data.push_back(dataline);
        adc_max = 0; adc_min = 0;
    }
    return line_data;
}

void readDataTest(DensityMap& grid, const char* fileName, float Gain, int len, bool& dataUpdate)
{
    std::vector<unsigned char> file_bytes;
    std::vector<int> marker_locations;
    std::vector<line_data_struct> line_data;

    file_bytes = readFile(fileName);
    /* find all marker locations */
    marker_locations = find_marker(file_bytes);
    /* convert file bytes to data struct */
    line_data = file_to_pixel_V08(file_bytes, marker_locations);
    printf("find the screen_data\n");

    int cnt = 0;
    for  (auto l: line_data)
    {
        cnt++;
        glm::vec3 ps = {l.p1.x/len + 0.5, l.p1.y/len + 1, l.p1.z/len + 0.5};
        glm::vec3 pe = {l.p2.x/len - l.p1.x/len  + 0.5, l.p2.y/len - l.p1.y/len + 1, l.p2.z/len - l.p1.z/len +0.5};
        for (int i = 0; i < l.vals.size(); ++i)
        {
            l.vals[i] = static_cast<unsigned char>(std::min((static_cast<double>(l.vals[i])*exp(Gain*(i/2500.0))), 255.0));
        }
        grid.writeLine(ps, pe, l.vals);
    }

    dataUpdate = true;
}

std::vector<line_data_struct> file_to_pixel_V08(std::vector<unsigned char> _file_bytes, std::vector<int> _marker_locations)
{
    std::vector<scan_data_struct> scan_data;
    std::vector<line_data_struct> line_data;
    unsigned char version_number_char;
    unsigned char crc_input_V08[V08_TOTAL_LENGTH]; /* 16 stands for IMU data, just place holder */

    for (int i = 0; i < (int)_marker_locations.size()-1; ++i){
        marker_index = _marker_locations.at(i);
        marker_index_next = _marker_locations.at(i+1);
        /* version number */
        version_number_char = _file_bytes.at(marker_index + sizeof(marker));
        /* time stamp */
        for (int j = 0; j < (int)sizeof(time_stamp_char); ++j){
            time_stamp_char[j] = _file_bytes.at(marker_index + sizeof(marker) + sizeof(version_number_char) + j);
        }
        std::memcpy(&time_stamp, time_stamp_char, sizeof(time_stamp));
        time_stamp = changed_endian_4Bytes(time_stamp);
        /* probe type char */
        probe_type_char = _file_bytes.at(marker_index + sizeof(marker) + sizeof(version_number_char) + sizeof(time_stamp_char));
        /* encoder (sweeping angle) */
        for (int j = 0; j < (int) sizeof(encoder_char); ++j){
            encoder_char[j] = _file_bytes.at(marker_index + sizeof(marker) + sizeof(version_number_char) + sizeof(time_stamp_char) + sizeof(probe_type_char) + j);
        }
        std::memcpy(&encoder, encoder_char, sizeof(encoder));
        encoder = changed_endian_2Bytes(encoder);
        if (encoder > 4096) continue;

        /* lx16 (rotation angle) */
        for (int j = 0; j < (int) sizeof(lx16_char); ++j){
            lx16_char[j] = _file_bytes.at(marker_index + sizeof(marker) + sizeof(version_number_char) + sizeof(time_stamp_char) + sizeof(probe_type_char) + sizeof(encoder_char) + j);
        }
        std::memcpy(&lx16, lx16_char, sizeof(lx16));
        lx16 = changed_endian_2Bytes(lx16);

        /* IMU (still just a place holder) */
        for (int j = 0; j < (int) sizeof(quaternion_char); ++j){
            quaternion_char[j] = _file_bytes.at(marker_index + sizeof(marker) + sizeof(version_number_char) + sizeof(time_stamp_char) + sizeof(probe_type_char) +
                                                sizeof(encoder_char) + sizeof(lx16_char)+ j);
        }
//        for (int j = 0; j < 16; j += 4)
//        {
//            unsigned char temp1[4];
//            for (int k = 0; k < 4; ++k) temp1[k] = quaternion_char[j+3-k];
//            quaternion[j/4] = *(float*)temp1;
//        }
        quaternion[0] = 1;
        quaternion[1] = 0;
        quaternion[2] = 0;
        quaternion[3] = 0;
        /* adc */
        /* determine the length of buffer */
        buffer_length = (int)(_marker_locations.at(i+1) - _marker_locations.at(i) - sizeof(marker) - sizeof(version_number_char) - sizeof(time_stamp_char) -
                              sizeof(probe_type_char) - sizeof(encoder_char) - sizeof(lx16_char) - sizeof(quaternion_char) - sizeof(crc_char))/2;
        for (int j = 0; j < buffer_length; ++j){
            for (int k = 0; k < (int)sizeof(adc_temp); ++k){
                adc_temp[k] = _file_bytes.at(marker_index + sizeof(marker) + sizeof(version_number_char) + sizeof(time_stamp_char)
                                             + sizeof(probe_type_char) + sizeof(encoder_char) + sizeof(lx16_char) + sizeof(quaternion_char) + j * 2 + k);
                adc_char[2*j+k] = adc_temp[k];
            }
            std::memcpy(&adc, adc_temp, sizeof(adc));
            adc = changed_endian_2Bytes(adc);
            buffer[j] = adc;
        }
        /* checksum */
        for (int j = 0; j < (int)sizeof(crc_char); ++j){
            crc_char[j] = _file_bytes.at(marker_index_next-(int)sizeof(crc_char)+j);
        }
        /* calculate crc locally */
        memcpy(crc_input_V08, &version_number_char, sizeof(version_number_char));
        memcpy(crc_input_V08+sizeof(version_number_char), time_stamp_char, sizeof(time_stamp_char));
        memcpy(crc_input_V08+sizeof(version_number_char)+sizeof(time_stamp_char), &probe_type_char, sizeof(probe_type_char));
        memcpy(crc_input_V08+sizeof(version_number_char)+sizeof(time_stamp_char)+sizeof(probe_type_char), encoder_char, sizeof(encoder_char));
        memcpy(crc_input_V08+sizeof(version_number_char)+sizeof(time_stamp_char)+sizeof(probe_type_char)+sizeof(encoder_char), lx16_char, sizeof(lx16_char));
        memcpy(crc_input_V08+sizeof(version_number_char)+sizeof(time_stamp_char)+sizeof(probe_type_char)+sizeof(encoder_char)
               +sizeof(lx16_char), quaternion_char, sizeof(quaternion_char));
        memcpy(crc_input_V08+sizeof(version_number_char)+sizeof(time_stamp_char)+sizeof(probe_type_char)+sizeof(encoder_char)
               +sizeof(lx16_char)+sizeof(quaternion_char), adc_char, sizeof(adc_char));
        crc_result = crc32c(0, crc_input_V08, sizeof(crc_input_V08));
        crc_result = changed_endian_4Bytes(crc_result);
        memcpy(crc_result_char, (unsigned char *)&crc_result, sizeof (crc_result));

        /* if two crc matches */
        if (compare_crc(crc_char, crc_result_char, sizeof(crc_char))){
            //if(1){  // uncommented this for store all the data
            scan_data_struct temp_struct;
            temp_struct.version = version_number_char;
            temp_struct.time_stamp = time_stamp;
            for (int j = 0; j < 4; ++j) temp_struct.quaternion[j] = quaternion[j];
            temp_struct.encoder = encoder;
            temp_struct.lx16 = lx16;
            /* normalize on the go */
            for (int j = 0; j < buffer_length; ++j) {
                temp_struct.buffer[j] = buffer[j];
            }
            scan_data.push_back(temp_struct);
        }
    }

    //write probe data to test
    //printf("start write imu...\n");
    std::ofstream fileout("data/real_imu.txt", std::ios::trunc|std::ios::out);
    for (auto s: scan_data){
        fileout << s.quaternion[0] << ' ' << s.quaternion[1] << ' ' << s.quaternion[2] << ' ' << s.quaternion[3] << std::endl;
    }
    fileout.close();
    //printf("Real IMU file generate!\n");

    //printf("the version of this data is %d \n", scan_data[0].version);

    float encoder_os = encoder_offset(scan_data, 400); /* automatically rotate the sector to vertical direction */

    for (int i = 0; i < (int)scan_data.size(); ++i)
    {
        double angle = scan_data.at(i).encoder * 360.0 / 4096.0;
        //float piezo = 270-angle;
        //float piezo = angle - 352.882812 - 90;
        float piezo = angle - encoder_os;
        /* angle of the lx16 */
        float angle_16 = scan_data.at(i).lx16 * 360.0 / 4096.0;

        /* filter */
        if (!isFilter)
            filter_list.clear();
        else
            some_Filters(scan_data.at(i).buffer);

        /* find min and max */
        for (int j = 0; j < 2500; ++j)
        {
            scan_data.at(i).buffer[j] = abs(scan_data.at(i).buffer[j] + 170); /* an offset from the board */
            if (j < 250)
               scan_data.at(i).buffer[j] = scan_data.at(i).buffer[j] / 30; /* not bother the signals we want */
        }

        adc_max = 0; adc_min = 1000;
        for (int j = 0; j < buffer_length; ++j){
            adc_max = std::max(adc_max, scan_data.at(i).buffer[j]);
            adc_min = std::min(adc_min, scan_data.at(i).buffer[j]);
        }

        int piezoProbe = 101; /* assume v = 1540 m/s , 5mm*/

        line_data_struct dataline;
        dataline.p1 = {piezoProbe*Cos(piezo),piezoProbe*Sin(piezo), 0};
        /* normalize on the go */
        for (int j = 0; j < buffer_length; ++j){
            intensity = ((double)scan_data.at(i).buffer[j] - adc_min)/(adc_max-adc_min);
            dataline.vals.push_back(static_cast<unsigned char>(intensity*255));
        }
        dataline.p2 = {(buffer_length+piezoProbe)*Cos(piezo), (buffer_length+piezoProbe)*Sin(piezo), 0};
//        glm::mat4 rot = Rotation::convertRotationMatrix(scan_data.at(i).quaternion[0], scan_data.at(i).quaternion[1],
//                                                        scan_data.at(i).quaternion[2], scan_data.at(i).quaternion[3]);
        glm::mat4 rot = glm::mat4(1.0f);
        rot = glm::rotate(rot, glm::radians(angle_16) , glm::vec3(0, 1, 0)); /* inverse later to compare */
        dataline.p1 = rot * glm::vec4(dataline.p1,1);
        dataline.p2 = rot * glm::vec4(dataline.p2,1);
        dataline.vertical_angle = 90.0 + piezo; /* The angle with y- */
        dataline.rotation_angle = angle_16;
        line_data.push_back(dataline);
    }
    return line_data;
}

void UDP_timer(int& time_milisecond, long& total_time)
{
    int step = 10;
    while(1)
    {
        time_mutex.lock();
        time_milisecond += step;
        total_time += step;
        time_mutex.unlock();
        std::this_thread::sleep_for(std::chrono::milliseconds(step));
    }
}

void render_lines(DensityMap& grid, std::vector<line_data_struct> line_data)
{
    for (auto l: line_data)
    {
        glm::vec3 ps = {l.p1.x/DEPTH + 0.5, l.p1.y/DEPTH + 1, l.p1.z/DEPTH + 0.5};
        glm::vec3 pe = {l.p2.x / DEPTH - l.p1.x / DEPTH + 0.5, l.p2.y / DEPTH - l.p1.y / DEPTH + 1,
                        l.p2.z / DEPTH - l.p1.z / DEPTH + 0.5};
        for (int i = 0; i < l.vals.size(); ++i)
        {
            l.vals[i] = static_cast<unsigned char>(std::min(static_cast<int>((l.vals[i])*exp(GAIN*(i/DEPTH))), 255));
        }
        rotate_mutex.lock();
        *ROTATION_PTR = l.rotation_angle;
        *ROTATION_QUA_PTR = Rotation::convertToQuaterion(0, l.rotation_angle, 0);
        rotate_mutex.unlock();
        grid.writeLine(ps, pe, l.vals);
    }
}

bool connectToProbe(DensityMap& grid, std::string probeIP, std::string username, std::string password, std::string compIP,
                    bool isSubmarine,
                    int lxRangeMin, int lxRangeMax, int lxRes, int servoRangeMin, int servoRangeMax, int servoRes,
                    std::string customCommand,
                    int connectionType, std::string& output, bool& connected, bool& error, std::string& errorMessage
) {
    try{
        /* connect to Red Pitaya */
        Socket soc("Linux");

        soc.setRPIP(const_cast<char*>(probeIP.c_str()));
        soc.setRPName(const_cast<char*>(username.c_str()));
        soc.setRPPassword(const_cast<char*>(password.c_str()));
        soc.saveConfig();
        soc.linkStart();

        if (connectionType == 3) /* custom command */
        {
            soc.customCommand(const_cast<char*>(customCommand.c_str()), 1000, output);
            //soc.interactiveShell();
            connected = true;
            return connected;
        }

        /* start the live rendering server on the computer */
        bool transmit_end = false;
        std::thread live_thread;
        live_thread = std::thread(live_rendering, std::ref(grid), isSubmarine, probeIP, compIP, std::ref(transmit_end));
        live_thread.detach();

        connected = true;

        /* pass some parameters */
        soc.customCommand("sh ./whitefin/tx.sh", 1000, output);
        soc.customCommand("cat /opt/redpitaya/fpga/fpga_0.94.bit > /dev/xdevcfg", 2000, output);
        soc.customCommand("cd whitefin", 500, output);
        soc.customCommand("make clean", 500, output);
        soc.customCommand("make all && LD_LIBRARY_PATH=/opt/redpitaya/lib ./adc", 1000000, output);
//        std::string command0 = "./test";
//        if (lxRangeMin) command0 += " " + std::to_string(lxRangeMin);
//        if (lxRangeMax) command0 += + " " + std::to_string(lxRangeMax);
//        if (lxRes) command0 += " " + std::to_string(lxRes);
//        if (servoRangeMin) command0 += " " + std::to_string(servoRangeMin);
//        if (servoRangeMax) command0 += " " + std::to_string(servoRangeMax);
//        if (servoRes) command0 += " " + std::to_string(servoRes);
//        soc.customCommand(const_cast<char*>(command0.c_str()));

        printf(">>> Live rendering starts! <<<\n");
        while(!transmit_end) /* waiting for live rendering */
            usleep(1000);
        printf("===== now decide which mode to choose =====\n");
        if (connectionType == 0) /* sending live scan */
        {
            if (soc.remove_cachefile() == -1) printf("Remove cachefile failed!\n");
        }
        else if (connectionType == 1) /* scan to file */
        {
            std::string newname;
            if (isSubmarine)
                newname += "submarine ";
            else
                newname += "whitefin ";

            /* generate the date-time pair for renaming the temperoray files */
            time_t now = time(0);
            tm *ltm = localtime(&now);
            std::string monthname = std::to_string(ltm->tm_mon + 1);
            std::string dayname = std::to_string(ltm->tm_mday);
            std::string hourname = std::to_string(ltm->tm_hour);
            std::string minutename = std::to_string(ltm->tm_min);
            std::string secondname = std::to_string(ltm->tm_sec);
            if (ltm->tm_mon < 10) monthname = "0" + monthname;
            if (ltm->tm_mday < 10) dayname = "0" + dayname;
            if (ltm->tm_hour < 10) hourname = "0" + hourname;
            if (ltm->tm_min < 10) minutename = "0" + minutename;
            if (ltm->tm_sec < 10) secondname = "0" + secondname;
            newname = newname + std::to_string(ltm->tm_year - 100) + monthname+ dayname
                      + "_" + hourname + minutename + secondname;
            if (soc.save_datafile(const_cast<char*>(newname.c_str())) == -1) printf("save and rename the data file failed!\n");
        }
    } catch(std::exception& e){
        errorMessage = e.what();
        error = true;
        return false;
    }

    return true;
}

void live_rendering(DensityMap& grid, bool isSubmarine, std::string probeIP, std::string compIP, bool& transmit_end)
{
    // read the data from current red pitaya 2d data.
    // only for Linux right now

    /* for real time trial */
    int sub_length = 1;
    int buffer_size = 1000; /* maximum size of scan lines each time rendered */
    bool newDataline = true;
    bool in_transmit = true;
    int recv_buffer_size;

    if (isSubmarine)
        recv_buffer_size = (10+V06_TOTAL_LENGTH+4) * sub_length;
    else
        recv_buffer_size = (10+V08_TOTAL_LENGTH+4) * sub_length;
    char recvBuf[recv_buffer_size];
    char sendBuf[100] = "I received";

    int sockSrv = socket(AF_INET, SOCK_DGRAM, 0);

    sockaddr_in addrSrv;
    addrSrv.sin_family = AF_INET;
    addrSrv.sin_port = htons(8888);
    addrSrv.sin_addr.s_addr = INADDR_ANY;

    sockaddr_in addrClt;
    addrClt.sin_family = AF_INET;
    addrClt.sin_port = htons(8000);
    addrClt.sin_addr.s_addr = inet_addr(probeIP.c_str());

    bind(sockSrv, (sockaddr*)&addrSrv, sizeof(sockaddr));

    int length = sizeof(sockaddr);

    int buffer_cnt = 0, loop_cnt = 0;
    std::vector<unsigned char> sub_file_bytes;


    long total_time = 0;
    long time_bar_step = 5 * 60 * 1000;
    long time_bar = time_bar_step; /* ms */
    int total_line_cnt = 0;
    long line_bar_step = 5000;
    long line_bar = line_bar_step;

    int time_milisecond = 0;
    std::thread timer_thread;
    timer_thread = std::thread(UDP_timer, std::ref(time_milisecond), std::ref(total_time));
    timer_thread.detach();
    int update_rate = 200; /* ms */

    //write bytes data to a file
    int file_cnt = 0;
    std::ofstream fileout("data/tempr" + std::to_string(file_cnt) + ".dat", std::ios::trunc|std::ios::out); /* WARNING: remember to delete this large file before git commit */
    setDepth(1500);
    setGain(1.0);

    // zlib compression
    Byte compr[6000], uncompr[6000];
    uLong comprLen = 6000, uncomprLen = 6000;

    while(in_transmit) {
        if (time_milisecond < update_rate || sub_file_bytes.empty())
        {
            memset(recvBuf, 0, sizeof(recvBuf));

            //recvfrom(sockSrv, recvBuf, sizeof(recvBuf), MSG_NOSIGNAL, (sockaddr *) &addrSrv, (socklen_t *) &length);
            recvfrom(sockSrv, compr, sizeof(compr), MSG_NOSIGNAL, (sockaddr *) &addrSrv, (socklen_t *) &length);
            buffer_cnt++;
            total_line_cnt++;
//            uncompress(uncompr, &uncomprLen, compr, comprLen);
            for (int k = 0; k < recv_buffer_size; ++k)
                recvBuf[k] = static_cast<char>(uncompr[k]);

            /* seperate the file according to number of lines or the time */
            if (total_line_cnt >= line_bar || total_time >= time_bar)
            {
                line_bar += line_bar_step;
                time_bar += time_bar_step;
                fileout.close();
                file_cnt++;
                fileout.open("data/tempr" + std::to_string(file_cnt) + ".dat", std::ios::trunc|std::ios::out);
            }

            if (recvBuf[0] == 'O')
            {
                if (recvBuf[1] == 'v' && recvBuf[2] == 'e'&& recvBuf[3] == 'r')
                {
                    printf(">>>>> Transmitting over! <<<<<\n");
                    buffer_cnt = buffer_size;
                    in_transmit = false;
                    transmit_end = true;
                }
            } else{
                for (auto r: recvBuf) {
                    sub_file_bytes.push_back(static_cast<unsigned char>(r));
                    fileout << r;
                }
            }
        }

        if (buffer_cnt == buffer_size || (time_milisecond >= update_rate && buffer_cnt >= 1))
        {
            printf("==> timer is %d, buffer count is %d <==\n", time_milisecond, buffer_cnt);
            std::vector<scan_data_struct> scan_data;
            std::vector<line_data_struct> line_data;
            for (auto m: marker)
                sub_file_bytes.push_back(m);
            /* convert file bytes to data struct */
            std::vector<int> sub_marker_locations = find_marker(sub_file_bytes);
            if (isSubmarine)
                line_data = file_to_pixel_V06(sub_file_bytes, sub_marker_locations);
            else
                line_data = file_to_pixel_V08(sub_file_bytes, sub_marker_locations);
            samples = line_data.size(); /* The last line is abandoned due to some reasons*/
            printf("the number of scan_data samples is %d\n", samples);

            render_lines(grid, line_data);
            buffer_cnt = 0;
            sub_file_bytes.clear();
            printf("new buffer No. %d has been drawned!\n", loop_cnt++);

            time_mutex.lock();
            printf("Time milisecond value is %d \n", time_milisecond);
            time_milisecond = 0;
            time_mutex.unlock();
        }
        sendto(sockSrv, sendBuf, sizeof(sendBuf), MSG_NOSIGNAL, (sockaddr*)&addrClt, length);
    }
    fileout.close();
    printf("Transmitting end!\n");
    close(sockSrv);
    transmit_end = true;
}

bool remove_tempr_files(bool& error, std::string& errorMessage)
{
    try {
        if (system("rm data/tempr*.dat") == -1)
            throw "Fail to remove temperate files!\n";
    }
    catch (char* str) {
        errorMessage = str;
        error = true;
        return false;
    }

    printf("temp files have been successfully removed!\n");
    return true;
    //system("del data/tempr*.dat");
}

bool rename_tempr_files(bool isSubmarine, bool& error, std::string& errorMessage)
{
    std::string newname;
    if (isSubmarine)
        newname += "submarine ";
    else
        newname += "whitefin ";

    time_t now = time(0);
    tm *ltm = localtime(&now);
    std::string monthname = std::to_string(ltm->tm_mon + 1);
    std::string dayname = std::to_string(ltm->tm_mday);
    std::string hourname = std::to_string(ltm->tm_hour);
    std::string minutename = std::to_string(ltm->tm_min);
    std::string secondname = std::to_string(ltm->tm_sec);
    if (ltm->tm_mon < 10) monthname = "0" + monthname;
    if (ltm->tm_mday < 10) dayname = "0" + dayname;
    if (ltm->tm_hour < 10) hourname = "0" + hourname;
    if (ltm->tm_min < 10) minutename = "0" + minutename;
    if (ltm->tm_sec < 10) secondname = "0" + secondname;
    newname = newname + std::to_string(ltm->tm_year - 100) + monthname+ dayname
              + "_" + hourname + minutename + secondname;

    std::string src_name = "file_list_23.txt";
    std::string command = "ls data > data/" + src_name; /* save the file names in current folders into a txt file */
//        command = "dir \\data > \\data\\" + src_name;

    try {
        if (system(command.c_str()) == -1)
            throw "Create file names list file failed!\n";
    } catch (char* str) {
        errorMessage = str;
        error = true;
        return false;
    }

    std::vector<std::string> file_list;
    std::string line, word;
    std::ifstream filein((std::string("data/") + src_name).c_str());
    int cnt = 0;
    while (std::getline(filein, line))
    {
        if(line.find("tempr") != std::string::npos)
        {
            std::istringstream stream(line);
            while (stream >> word)
            {
                if (word.find("tempr") != std::string::npos)
                {
                    std::string newName = std::string("data/") + newname + std::string("_") + std::to_string(cnt);
                    std::string oldName = std::string("data/") + word;
                    try {
                        if (rename(oldName.c_str(), newName.c_str()) == -1) {
                            printf("rename file %s failed!\n", oldName.c_str());
                            throw "Error occured when renaming the temp files.\n";
                        }
                    } catch (char* str) {
                        errorMessage = str;
                        error = true;
                        return false;
                    }
                    cnt++;
                }
            }
        }
    }

    printf("Already rename all the temp files!\n");
    command = "rm data/" + src_name; /* delete the txt file with all file names */
    try {
        if (system(command.c_str()) == -1)
            throw "Remove file names list file failed!\n";
    } catch (char* str) {
        errorMessage = str;
        error = true;
        return false;
    }
    return true;
}

std::vector<unsigned char> readFile(const char* directory)
{
    printf("===> %s <===\n", directory);
    std::ifstream inFile(directory, std::ios::in | std::ios::binary);
    if (!inFile){
        printf("Failed to open file.\n");
//        throw "Failed to open file!\n";
        throw(std::runtime_error("Failed to open file!"));
        std::vector<unsigned char> fail_out;
        return fail_out;
    }
    /* convert file to bytes vector */
    /* DO NOT USE ISTREAM_ITERATOR*/
    std::vector<unsigned char> file_bytes(
            (std::istreambuf_iterator<char>(inFile)),
            (std::istreambuf_iterator<char>()));
    for(int i = 0; i < 20; ++i){
        printf("%02X ", file_bytes.at(i));
    }
    printf("\n");
    return file_bytes;
}

int compare_crc(unsigned char a[], unsigned char b[], size_t len){
    int ii;
    for (ii = 0; ii < (int)len; ii++){
        if (a[ii] != b[ii]){
            return 0;
        }
    }
    return 1;
}


std::vector<int> find_marker(std::vector<unsigned char> _file_bytes){
    std::vector<int> _marker_locations;
    for (int i = 0; i < (int)_file_bytes.size(); i++){
        if (_file_bytes.at(i) == marker[0]){
            for (int j = 0; j < (int)sizeof(marker); j++) {
                if ((i + j) < _file_bytes.size()){
                    if (_file_bytes.at(i + j) != marker[j]) {
                        marker_flag = false;
                        break;
                    } else {
                        marker_flag = true;
                    }
                }
            }
            if (marker_flag) {
                _marker_locations.push_back(i);
                i += (int)sizeof(marker);
            }
        }
        marker_flag = false;
    }
    return _marker_locations;
}

unsigned long changed_endian_4Bytes(unsigned long num){
    int byte0, byte1, byte2, byte3;
    byte0 = (num & 0x000000FF) >> 0 ;
    byte1 = (num & 0x0000FF00) >> 8 ;
    byte2 = (num & 0x00FF0000) >> 16 ;
    byte3 = (num & 0xFF000000) >> 24 ;
    return((byte0 << 24) | (byte1 << 16) | (byte2 << 8) | (byte3 << 0));
}

int16_t changed_endian_2Bytes(int16_t value){
    return ((value >> 8) & 0x00ff) | ((value & 0x00ff) << 8);
}

uint32_t crc32c(uint32_t crc, const unsigned char *buf, size_t len)
{
    int k;

    crc = ~crc;
    while (len--) {
        crc ^= *buf++;
        for (k = 0; k < 8; k++)
            crc = crc & 1 ? (crc >> 1) ^ 0xedb88320 : crc >> 1;
    }
    return ~crc;
}

float ReverseFloat( const float inFloat ){
    float retVal;
    char *floatToConvert = ( char* ) & inFloat;
    char *returnFloat = ( char* ) & retVal;

    // swap the bytes into a temporary buffer
    returnFloat[0] = floatToConvert[3];
    returnFloat[1] = floatToConvert[2];
    returnFloat[2] = floatToConvert[1];
    returnFloat[3] = floatToConvert[0];

    return retVal;
}

void some_Filters(short* origin_buffer)
{
    int length = 2500;
    const int order = 4;
    double sampling_rate = 15600000;

    int count = filter_list.size() / 3;
    if (count*3 != filter_list.size() or count <= 0)
    {
        printf("Wrong length of input filter list! \n");
        return;
    }
    /* filter list: {filter type, cutoff frequency, frequency width} */

    /* filter type 2: 1-lowpass 2-highpass 3-bandpass 4-bandstop 5-moving average */
    for (int i = 0; i < count; ++i)
    {
        int filter_type = static_cast<int>(filter_list[i*3]);

        if (filter_type == 5) /* moving average */
        {
            moving_average(origin_buffer, static_cast<int>(filter_list[i*3 + 1]));
            continue;
        }

        if (filter_type == 1) /* lowpass */
        {
            Iir::Butterworth::LowPass<order> f;
            f.setup(sampling_rate, filter_list[i*3 + 1] * 1e6);
            for (int j = 0; j < length; ++j)
                origin_buffer[j] = (short)f.filter(origin_buffer[j]);
        }
        else if (filter_type == 2) /* highpass */
        {
            Iir::Butterworth::HighPass<order> f;
            f.setup(sampling_rate, filter_list[i*3 + 1] * 1e6);
            for (int j = 0; j < length; ++j)
                origin_buffer[j] = (short)f.filter(origin_buffer[j]);
        }
        else if (filter_type == 3) /* bandpass */
        {
            Iir::Butterworth::BandPass<order/2> f;
            f.setup(sampling_rate, filter_list[i*3 + 1] * 1e6, filter_list[i*3 + 2]);
            for (int j = 0; j < length; ++j)
                origin_buffer[j] = (short)f.filter(origin_buffer[j]);
        }
        else if (filter_type == 4) /* bandstop */
        {
            Iir::Butterworth::BandStop<order/2> f;
            f.setup(sampling_rate, filter_list[i*3 + 1] * 1e6, filter_list[i*3 + 2]);
            for (int j = 0; j < length; ++j)
                origin_buffer[j] = (short)f.filter(origin_buffer[j]);
        } else{
            printf("NO such filter type! \n");
            return;
        }
    }

}

void Apply_filters(DensityMap& grid, const char* fileName, float Gain, int len, bool& dataUpdate, std::vector<double> filter_list_in)
{
    isFilter = true;
    filter_list = filter_list_in;
    readDataTest(grid, fileName, Gain, len, dataUpdate);
    isFilter = false;
}

void moving_average(short* origin_buffer, int size)
{
    int left = size / 2; int right = size - left;
    int length = 2500;
    short result[2500];
    for (int i = 0; i < length; ++i)
    {
        if (i < left || i > length - right)
            result[i] = origin_buffer[i];
        else{
            short tmp = 0;
            for (int j = 0; j < size; ++j)
                tmp += origin_buffer[i - left + j];
            result[i] = tmp / size;
        }
    }
    for (int i = 0 ; i < length; ++i)
        origin_buffer[i] = result[i];

    printf("Finish the moving average with window size of %d\n", size);
}

float encoder_offset(std::vector<scan_data_struct> scan_data, int count)
{
    float a0, amax, amin;
    int offset = 200;
    std::vector<float> atemps;
    float gap1, gap2;
    for (int i = 0; i < count; ++i)
    {
        float angle = scan_data.at(i + offset).encoder * 360.0 / 4096.0;
        //printf("%d\n", scan_data.at(i + offset).encoder);
        if (i == 0) a0 = angle;
        float atemp = angle - a0;
        if (atemp > 180) atemp -= 360;
        else if (atemp < -180) atemp += 360;
        if (atemp > 180 || atemp < -180)
            continue;
//        amax = amax>atemp? amax:atemp;
//        amin = amin<atemp? amin:atemp;
        atemps.push_back(atemp);
        //if (i > 0) printf("%f\n", atemps.at(atemps.size()-1) - atemps.at(atemps.size()-2));
    }
    amax = *std::max_element(atemps.begin(), atemps.end());
    amin = *std::min_element(atemps.begin(), atemps.end());

    printf(">>>>%f === %f, %f <<<< \n",a0, amin, amax);
    return (amax + amin + 1) / 2.0 + a0 + 90;
}

int getDepth()
{
    return DEPTH;
}

void setDepth(int dep)
{
    DEPTH = dep;
}

float getGain()
{
    return GAIN;
}

void setGain(float g)
{
    GAIN = g;
}

int getSamples()
{
    return samples;
}

float* getRotationPtr() {
    return ROTATION_PTR;
}