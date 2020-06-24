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
unsigned char crc_input[4+1+2+2+0+2*2500]; /* 0 stands for no IMU data */
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
int samples = -1; /* -1 stands for no data */

std::mutex time_mutex;

void fakeDemo(DensityMap& grid, bool& dataUpdate)
{
    // directly draw the cube in the space
    int data_length = 2500; /* eqaul to the start variables */
    float length = 5; /* length of the vector in coordinate. the cube is 2x2x2 with origin at (0, 2, 0) */
    int cnt = 0;

    /* open file to save the data*/
    FILE * fp;
    fp = fopen("data/fakeCube_2.txt", "w");
    time_stamp = changed_endian_4Bytes(time_stamp);
    memcpy(time_stamp_char, (unsigned char *)&time_stamp, sizeof(time_stamp_char));
//    printf("start write encoder & piezo...\n");
//    std::ofstream fileout("data/encoder_piezo.txt", std::ios::trunc|std::ios::out);
    for (unsigned short i = 1200; i < 1800; i += 10)
    {
        encoder = i;
        double angle = encoder * 360.0 / 4096.0;
        double ax = 9*Cos(angle - 222 );
        double ay = 9*Sin(angle - 222 );
        double piezo = atan2(ay+21, ax) * 180.0 / M_PI - 90.0;
        glm::vec3 ray0 = glm::vec3{length*Sin(piezo), length*Cos(piezo), 0};
//        fileout << encoder << ' ' << piezo << std::endl;
//        continue;
        /* package the encoder */
        encoder = changed_endian_2Bytes(encoder);
        memcpy(encoder_char, &encoder, sizeof encoder_char);
        for (float w = -1; w < 1; w += 0.035) /*quaternion*/
        {
            float y = pow(1 - w*w, 0.5);
            //if (y > 0.2) y -= 0.1;
            glm::vec3 euler = Rotation::convertToEulerAngle(w, 0, y, 0);
            glm::mat4 rotationMatrix = glm::mat4(1.0f);
            rotationMatrix = glm::rotate(rotationMatrix, float(euler.y), glm::vec3(0, 1, 0));
            glm::vec3 ray = rotationMatrix * glm::vec4{ray0, 1};
            //printf("This ray is (%f, %f, %f)\n", ray.x, ray.y, ray.z);
            /* package the quaternion */
            float nw = ReverseFloat(w);
            float nx = ReverseFloat(0);
            float ny = ReverseFloat((y));
            float nz = ReverseFloat(0);
            memcpy(qua_wxyz_char, &nw, sizeof(nw));
            memcpy(qua_wxyz_char+sizeof(nw), &nx, sizeof(nx));
            memcpy(qua_wxyz_char+sizeof(nw)+sizeof(nx), &ny, sizeof(ny));
            memcpy(qua_wxyz_char+sizeof(nw)+sizeof(nx)+sizeof(ny), &nz, sizeof(nz));

            if (cnt%10000 == 0)
                printf("It is drawing line No.%d\n", cnt);

            /* Draw the line and find the points inside the cube*/
            float dx = ray.x / data_length, dy = ray.y / data_length, dz = ray.z / data_length;
            std::vector<unsigned char> vals;
            float ix = 0, iy = 0, iz = 0;
            for (int i = 0 ; i < data_length; ++i)
            {
                unsigned short tmp = 30;
                if (ix >= -1 && ix <= 1 && iy >= 2.5 && iy <= 4.5 && iz >= -1 && iz <= 1)
                {
                    vals.push_back(250);
                    tmp = 250;
                    //printf("This point is (%f, %f, %f)\n", ix, iy, iz);
                }
                else
                    vals.push_back(30);
                tmp = changed_endian_2Bytes(tmp);
                memcpy(adc_char+i*sizeof(tmp), &tmp, sizeof tmp);
                ix += dx;
                iy += dy;
                iz += dz;
            }
            /* generate crc and the whole data*/
            memcpy(crc_input, time_stamp_char, sizeof(time_stamp_char));
            memcpy(crc_input+sizeof(time_stamp_char), &information_byte, sizeof(information_byte));
            memcpy(crc_input+sizeof(time_stamp_char)+sizeof(information_byte), encoder_char, sizeof(encoder_char));
            memcpy(crc_input+sizeof(time_stamp_char)+sizeof(information_byte)+sizeof(encoder_char), qua_wxyz_char, sizeof(qua_wxyz_char));
            memcpy(crc_input+sizeof(time_stamp_char)+sizeof(information_byte)+sizeof(encoder_char)+sizeof(qua_wxyz_char), adc_char, sizeof(adc_char));
            unsigned short crc_result = 36;
            crc_result = changed_endian_4Bytes(crc_result);
            memcpy(crc_char, (unsigned char *)&crc_result, sizeof crc_char);
            memcpy(message_buff, marker, sizeof(marker));
            memcpy(message_buff+sizeof(marker), crc_input, sizeof(crc_input));
            memcpy(message_buff+sizeof(marker)+sizeof(crc_input), crc_char, sizeof(crc_char));
            fwrite(&message_buff, sizeof(message_buff), 1, fp);

            /* draw the fake data */
            glm::vec3 ps = glm::vec3{0.5, 0.5, 0.5};
            glm::vec3 pe = glm::vec3{0.5 *ray.x/length + 0.5, 0.5 *ray.y/length + 0.5, 0.5 * ray.z/length + 0.5};
            grid.writeLine(ps, pe, vals);
            cnt++;
        }
    }
    fclose(fp);
    printf("fake byte file is generated!\n");
//    fileout.close();
//    printf("encoder & piezo file generate!\n");
    dataUpdate = true;
}

void readDataWhitefin(DensityMap& grid, const char* fileName, float Gain, int len, bool& dataUpdate, bool& error, std::string& errorMessage)
{
    std::vector<unsigned char> file_bytes;
    std::vector<int> marker_locations;
    std::vector<line_data_struct> line_data;

    //file_bytes = readFile(fileName);
    try {
        file_bytes = readFile(fileName);

        /* find all marker locations */
        marker_locations = find_marker(file_bytes);
        /* convert file bytes to data struct */
        line_data = file_to_pixel_V07(file_bytes, marker_locations);
        printf("find the screen_data\n");

        for  (auto l: line_data)
        {
            glm::vec3 ps = {l.p1.x/len + 0.5, l.p1.y/len + 1, l.p1.z/len + 0.5};
            glm::vec3 pe = {l.p2.x/len - l.p1.x/len  + 0.5, l.p2.y/len - l.p1.y/len + 1, l.p2.z/len - l.p1.z/len +0.5};
            for (int i = 0; i < l.vals.size(); ++i)
            {
                l.vals[i] = static_cast<unsigned char>(std::min(static_cast<int>((l.vals[i])*exp(Gain*(i/len))), 255));
            }
            grid.writeLine(ps, pe, l.vals);
        }

        dataUpdate = true;
    } catch(std::exception& e){
//        errorMessage = e.what();
        fprintf(stderr, e.what());
        error = true;
        errorMessage = e.what();
//        return false;fdas
    }
}

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

    for  (auto l: line_data)
    {
        glm::vec3 ps = {l.p1.x/len + 0.5, l.p1.y/len + 1, l.p1.z/len + 0.5};
        glm::vec3 pe = {l.p2.x/len - l.p1.x/len  + 0.5, l.p2.y/len - l.p1.y/len + 1, l.p2.z/len - l.p1.z/len +0.5};
        for (int i = 0; i < l.vals.size(); ++i)
        {
            l.vals[i] = static_cast<unsigned char>(std::min(static_cast<int>((l.vals[i])*exp(Gain*(i/len))), 255));
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
    //unsigned char version_number;
    unsigned char crc_input_V08[1+4+1+2+2+16+2*2500]; /* 16 stands for IMU data */

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

    //printf("the version of this data is %d \n", scan_data[0].version);

    for (int i = 0; i < (int)scan_data.size(); ++i)
    {
//        double angle = scan_data.at(i).encoder * 360.0 / 4096.0;
//        double ax = 9*Cos(angle - 222 );
//        double ay = 9*Sin(angle - 222 );
//        double piezo = atan2(ay+21, ax) * 180.0 / M_PI - 180.0;

        double angle = scan_data.at(i).encoder * 360.0 / 4096.0;
        float piezo = 270-angle;
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
//        glm::mat4 rot = Rotation::convertRotationMatrix(scan_data.at(i).quaternion[0], scan_data.at(i).quaternion[1],
//                                                        scan_data.at(i).quaternion[2], scan_data.at(i).quaternion[3]);
        glm::mat4 rot = glm::mat4(1.0f);
        rot = glm::rotate(rot, glm::radians(angle_16) , glm::vec3(0, 1, 0)); /* inverse later to compare */
        dataline.p1 = rot * glm::vec4(dataline.p1,1);
        dataline.p2 = rot * glm::vec4(dataline.p2,1);
        line_data.push_back(dataline);
        adc_max = 0; adc_min = 0;
    }
    return line_data;
}


void realDemo(DensityMap& grid, bool& dataUpdate)
{
    char fileName[] = "../ALLDATA/0316_RedPitaya_WhiteFin/beansouplarge_startionary_3d_1.txt";
//    switch (MODE)
//    {
//        case 1:
//            readDataSubmarine(fileName, Gain, len);
//            break;
//        case 2:
//            readDataWhitefin(fileName, Gain, len);
//            break;
//        default:
//            printf("<%d> is an invalid mode!\n");
//    }
    std::vector<unsigned char> file_bytes;
    std::vector<int> marker_locations;
    std::vector<scan_data_struct> scan_data;
    std::vector<line_data_struct> line_data;

//    char fileName[255];
//    std::cout << "Please type the file you want to open (<data/xxxx.txt>): " << std::endl;
//    std::cin >> fileName;
    file_bytes = readFile(fileName);
    /* find all marker locations */
    marker_locations = find_marker(file_bytes);
    /* convert file bytes to data struct */
    file_to_data(file_bytes, marker_locations, scan_data);
    samples = scan_data.size();
    printf("the number of scan_data samples is %d\n", samples);
    /* convert data to vertex on screen */
    data_to_pixel(scan_data, line_data);
    printf("find the screen_data\n");

    int ddim = grid.getDim();
    int len = line_data[0].vals.size(); // 2500, equl to buffer size
    setDepth(2500);
    int cnt = 0;
    for  (auto l: line_data)
    {
        glm::vec3 ps = {l.p1.x/len + 0.5, l.p1.y/len + 1, l.p1.z/len + 0.5};
        glm::vec3 pe = {l.p2.x/len - l.p1.x/len  + 0.5, l.p2.y/len - l.p1.y/len + 1, l.p2.z/len - l.p1.z/len +0.5};
        grid.writeLine(ps, pe, l.vals);
    }
    dataUpdate = true;
}

//
//void realDemo2(DensityMap& grid, bool& dataUpdate)
//{
//    //read the data from current red pitaya 2d data.
//    std::vector<unsigned char> file_bytes;
//    std::vector<int> marker_locations;
//    std::vector<scan_data_struct> scan_data;
//    std::vector<line_data_struct> line_data;
//
//    /* for real time trial */
//    int sub_length = 1;
//    int buffer_size = 1000;
//    bool newDataline = true;
//    //std::mutex readtex;
//
////    char fileName[255];
////    std::cout << "Please type the file you want to open (<data/xxxx.txt>): " << std::endl;
////    std::cin >> fileName;
////    file_bytes = readFile(fileName);
////    /* find all marker locations */
////    marker_locations = find_marker(file_bytes);
////
////    //sub_length = marker_locations.size() / 35;
////
////    std::thread fileThread;
////    fileThread = std::thread(readSubfile, file_bytes, marker_locations, sub_length,
////            std::ref(newDataline));
////    fileThread.detach();
//
//    char recvBuf[(10+4+1+2+2+2*2500+4)*sub_length];
//    char sendBuf[100] = "I received!";
//    SOCKET sockConn;
//    WSADATA wsaData;
//    int port = 8888;
//    if (WSAStartup(MAKEWORD(2,2), &wsaData) != 0)
//    {
//        printf("Initialize failed!\n");
//        return;
//    }
//    SOCKET sockSrv = socket(AF_INET, SOCK_STREAM, 0);
//    SOCKADDR_IN addrSrv;
//    addrSrv.sin_family = AF_INET;
//    addrSrv.sin_port = htons(port);
//    addrSrv.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
//    int retVal = bind(sockSrv, (LPSOCKADDR)&addrSrv, sizeof(SOCKADDR_IN));
//    if (retVal == SOCKET_ERROR)
//    {
//        printf("Connect failed: %d\n", WSAGetLastError());
//        return;
//    }
//    if (listen(sockSrv, 10) == SOCKET_ERROR)
//    {
//        printf("listen failed: %d\n", WSAGetLastError());
//        return;
//    }
//    SOCKADDR_IN addrClient;
//    int length = sizeof(SOCKADDR);
//    sockConn = accept(sockSrv, (SOCKADDR *)&addrClient, &length);
//    if (sockConn == SOCKET_ERROR)
//    {
//        printf("wait for request fail: %d", WSAGetLastError());
//        return;
//    }
//    printf("The IP of client is :[%s]\n", inet_ntoa(addrClient.sin_addr));
//
//    int buffer_cnt = 0, loop_cnt = 0;
//    std::vector<unsigned char> sub_file_bytes;
//    while(1)
//    {
//        //if (buffer_cnt % 100 == 0) printf("time try no. %d\n", buffer_cnt);
//        memset(recvBuf, 0, sizeof(recvBuf));
//        recv(sockConn, recvBuf, sizeof(recvBuf), 0);
//        buffer_cnt++;
//
//        for (auto r: recvBuf)
//        {
//            sub_file_bytes.push_back(static_cast<unsigned char>(r));
//        }
//
//        if (buffer_cnt == buffer_size)
//        {
//            /* convert file bytes to data struct */
//            std::vector<int> sub_marker_locations = find_marker(sub_file_bytes);
//            file_to_data(sub_file_bytes, sub_marker_locations, scan_data);
//            samples = scan_data.size();
//            printf("the number of scan_data samples is %d\n", samples);
//            /* convert data to vertex on screen */
//            data_to_pixel(scan_data, line_data);
//            //printf("find the screen_data\n");
//
//            int ddim = grid.getDim();
//            len = line_data[0].vals.size(); // 2500, equl to buffer size
//            //            printf("=====\nPlease choose the maximum depth you want to show ( from 1 to %d): ", len);
//            //            std::cin >> len;
//            //len = 1500; // change range
//            setDepth(1500);
//            int cnt = 0;
//            for (auto l: line_data)
//            {
//                glm::vec3 ps = {0.5, 1, 0.5};
//                glm::vec3 pe = {l.p2.x / len - l.p1.x / len + 0.5, l.p2.y / len - l.p1.y / len + 1,
//                                l.p2.z / len - l.p1.z / len + 0.5};
//                grid.writeLine(ps, pe, l.vals);
//            }
//            buffer_cnt = 0;
//            sub_file_bytes.clear();
//            printf("new buffer No. %d has been drawned!\n", loop_cnt++);
//            dataUpdate = true;
//        }
//    }
//    closesocket(sockConn);
//    closesocket(sockSrv);
//    WSACleanup();
//}
//
//void readSubfile(std::vector<unsigned char> file_bytes, std::vector<int> marker_locations, int sub_length,
//                 bool& newDataline)
//{
//    /* TCP client*/
//    WSADATA wsaData;
//    char buff[(10+4+1+2+2+2*2500+4)*sub_length];
//    memset(buff, 0, sizeof(buff));
//    if (WSAStartup(MAKEWORD(2,2), &wsaData) != 0)
//    {
//        printf("Winsock initialization failed!\n");
//        return;
//    }
//
//    SOCKADDR_IN addrSrv;
//    addrSrv.sin_family = AF_INET;
//    addrSrv.sin_port = htons(8888);
//    addrSrv.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
//
//    SOCKET socketClient = socket(AF_INET, SOCK_STREAM, 0);
//    if (SOCKET_ERROR == socketClient)
//    {
//        printf("Socket() errr: %d", WSAGetLastError());
//        return;
//    }
//    if (connect(socketClient, (struct sockaddr*)&addrSrv, sizeof(addrSrv)) == INVALID_SOCKET)
//    {
//        printf("Connect fail: %d", WSAGetLastError());
//        return;
//    }
//
//    // sub_length: the length of each part of the file_bytes
//    int pt = 0; /* current position in marker_locations */
//    while (pt < marker_locations.size()-1)
//    {
//        if (newDataline)
//        {
//            continue;
//        }
//        //sub_file_bytes.clear();
//        std::vector<unsigned char> sub_file_bytes;
//        for (int i = 0; i < sub_length; ++i)
//        {
//            if (pt >= marker_locations.size()-1)
//            {
//                printf("========END========\n");
//                closesocket(socketClient);
//                WSACleanup();
//                break;
//            }
//            for (int j = marker_locations[pt]; j < marker_locations[pt+1]; j++)
//                sub_file_bytes.push_back(file_bytes[j]);
//            pt++;
//        }
//
//        int csf = 0;
//        for (int i = 0; i < sub_file_bytes.size(); ++i)
//        {
//            memset(buff+csf, sub_file_bytes[i], sizeof(char));
//            csf++;
//        }
//        send(socketClient, buff, sizeof(buff), 0);
//        //write(socketClient, buff, sizeof(buff));
//
//        //printf("Read some new data lines!\n");
//        newDataline = true;
//        //std::this_thread::sleep_for(std::chrono::milliseconds (500));
//    }
//    closesocket(socketClient);
//    WSACleanup();
//}
//
//void realDemo3(DensityMap& grid, bool& dataUpdate)
//{
//    printf("Try gantry ~ \n");
//    float stepLength = 0.5; /* (cm) step of the gantry*/
//    int step = stepLength * 1e6 / Velocity * 15.6;
//    printf("step is %d\n", step);
//    glm::vec3 starts = {0.2, 1, 0.5};
//
//    std::vector<unsigned char> file_bytes;
//    std::vector<int> marker_locations;
//    std::vector<scan_data_struct> scan_data;
//    std::vector<line_data_struct> line_data;
//
//    for (int i = 0; i < 3; ++ i)
//    {
//        file_bytes.clear();
//        marker_locations.clear();
//        scan_data.clear();
//        line_data.clear();
//
//        std::string fileNameS = "";
//        std::string fileNameBase = "../ALLDATA/0316_RedPitaya_WhiteFin/beansouplarge_startionary_3d_";
//        std::string fileNameTail = ".txt";
//        fileNameS = fileNameBase += std::to_string(1+i) += fileNameTail;
//        char *fileName = new char[strlen(fileNameS.c_str()) + 1];
//        strcpy(fileName, fileNameS.c_str());
//        file_bytes = readFile(fileName);
//        /* find all marker locations */
//        marker_locations = find_marker(file_bytes);
//        /* convert file bytes to data struct */
//        file_to_data(file_bytes, marker_locations, scan_data);
//        samples = scan_data.size();
//        printf("the number of scan_data samples is %d\n", samples);
//        /* convert data to vertex on screen */
//        data_to_pixel(scan_data, line_data);
//        printf("find the screen_data\n");
//
//        int ddim = grid.getDim();
//        len = line_data[0].vals.size(); // 2500, equl to buffer size
//        glm::vec3 actualStarts = {0.2+float(step)*5.0 * i/len, 1, 0.5};
//        //len = 1500; // change range
//        setDepth(2500);
//        int cnt = 0;
//        for (auto l: line_data)
//        {
//            glm::vec3 ps = {l.p1.x / len + actualStarts.x, l.p1.y / len + actualStarts.y, l.p1.z / len + actualStarts.z};
//            glm::vec3 pe = {l.p2.x / len - l.p1.x / len + actualStarts.x,
//                            l.p2.y / len - l.p1.y / len + actualStarts.y,
//                            l.p2.z / len - l.p1.z / len + actualStarts.z};
//            grid.writeLine(ps, pe, l.vals);
//        }
//    }
//}
//

void realDemo4(DensityMap& grid, bool& dataUpdate)
{
    //read the data from current red pitaya 2d data.

    /* for real time trial */
    int sub_length = 1;
    int buffer_size = 1000;
    //bool newDataline = false;
    bool newDataline = true;
    bool in_transmit = true;
    //std::mutex readtex;

    char recvBuf[(10+4+1+2+2+2*2500+4)*sub_length];
    char sendBuf[100] = "I received";
//    SOCKET sockConn;
//    WSADATA wsaData;
    int port = 8888;
//    if (WSAStartup(MAKEWORD(2,2), &wsaData) != 0)
//    {
//        printf("Initialize failed!\n");
//        return;
//    }
    //SOCKET sockSrv = socket(AF_INET, SOCK_DGRAM, 0);
    int sockSrv = socket(AF_INET, SOCK_DGRAM, 0);
    //SOCKADDR_IN addrSrv;
    sockaddr_in addrSrv;
    addrSrv.sin_family = AF_INET;
    addrSrv.sin_port = htons(port);
    //addrSrv.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
    //addrSrv.sin_addr.s_addr = htonl(INADDR_ANY);
    addrSrv.sin_addr.s_addr = inet_addr("192.168.1.39");

    sockaddr_in addrClt;
    addrClt.sin_family = AF_INET;
    addrClt.sin_port = htons(8000);
    //addrSrv.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
    //addrSrv.sin_addr.s_addr = htonl(INADDR_ANY);
    addrClt.sin_addr.s_addr = inet_addr("192.168.1.42");

    //bind(sockSrv, (SOCKADDR*)&addrSrv, sizeof(SOCKADDR));
    bind(sockSrv, (sockaddr*)&addrSrv, sizeof(sockaddr));

    int length = sizeof(sockaddr);

    int buffer_cnt = 0, loop_cnt = 0;
    std::vector<unsigned char> sub_file_bytes;

    int time_milisecond = 0;
    long total_time = 0;
    std::thread timer_thread;
    timer_thread = std::thread(UDP_timer, std::ref(time_milisecond), std::ref(total_time));
    timer_thread.detach();
    int update_rate = 200; /* ms */

    //write bytes data to a file
    //printf("start write imu...\n");
    std::ofstream fileout("data/tempr.dat", std::ios::trunc|std::ios::out); /* WARNING: remember to delete this large file before git commit */
    //printf("Real IMU file generate!\n");
    setDepth(1500);
    setGain(1.0);

    while(in_transmit) {
        //if (newDataline)
        if (time_milisecond < update_rate || sub_file_bytes.empty())
        {
            //if (buffer_cnt % 100 == 0) printf("time try no. %d\n", buffer_cnt);
            memset(recvBuf, 0, sizeof(recvBuf));
            //printf("Waiting...\n");
//            recv(sockConn, recvBuf, sizeof(recvBuf), 0);
            //recvfrom(sockSrv, recvBuf, sizeof(recvBuf), 0, (SOCKADDR*)&addrClient, &length);
            recvfrom(sockSrv, recvBuf, sizeof(recvBuf), MSG_NOSIGNAL, (sockaddr *) &addrSrv, (socklen_t *) &length);
            buffer_cnt++;

            if (recvBuf[0] == 'O')
            {
                if (recvBuf[1] == 'v' && recvBuf[2] == 'e'&& recvBuf[3] == 'r')
                {
                    printf(">>>>> Transmitting over! <<<<<\n");
                    buffer_cnt = buffer_size;
                    in_transmit = false;
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
//            file_to_data(sub_file_bytes, sub_marker_locations, scan_data);
//            samples = scan_data.size(); /* The last line is abandoned due to some reasons*/
//            printf("the number of scan_data samples is %d\n", samples);
//            /* convert data to vertex on screen */
//            data_to_pixel(scan_data, line_data);
            line_data = file_to_pixel_V07(sub_file_bytes, sub_marker_locations);
            samples = line_data.size(); /* The last line is abandoned due to some reasons*/
            printf("the number of scan_data samples is %d\n", samples);
            //printf("find the screen_data\n");

            //len = line_data[0].vals.size(); // 2500, equl to buffer size
            //            printf("=====\nPlease choose the maximum depth you want to show ( from 1 to %d): ", len);
            //            std::cin >> len;
            //len = 1500; // change range
//            setDepth(1500);
//            for (auto l: line_data)
//            {
//                glm::vec3 ps = {l.p1.x/len + 0.5, l.p1.y/len + 1, l.p1.z/len + 0.5};
//                glm::vec3 pe = {l.p2.x / len - l.p1.x / len + 0.5, l.p2.y / len - l.p1.y / len + 1,
//                                l.p2.z / len - l.p1.z / len + 0.5};
//                grid.writeLine(ps, pe, l.vals);
//            }
            render_lines(grid, line_data);
            buffer_cnt = 0;
            sub_file_bytes.clear();
            printf("new buffer No. %d has been drawned!\n", loop_cnt++);
            dataUpdate = true;

            time_mutex.lock();
            printf("Time milisecond value is %d \n", time_milisecond);
            time_milisecond = 0;
            time_mutex.unlock();
        }
        //newDataline = false;
        //sendto(sockSrv, sendBuf, sizeof(sendBuf), 0, (SOCKADDR*)&addrClient, length);
        //sendto(sockSrv, sendBuf, sizeof(sendBuf), MSG_NOSIGNAL, (sockaddr*)&addrSrv, length);
        sendto(sockSrv, sendBuf, sizeof(sendBuf), MSG_NOSIGNAL, (sockaddr*)&addrClt, length);
    }
    //closesocket(sockSrv);
    fileout.close();
    printf("Transmitting end!\n");
    close(sockSrv);
//    WSACleanup();
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

            time_t now = time(0);
            tm *ltm = localtime(&now);
            newname = newname + std::to_string(ltm->tm_year) + std::to_string(ltm->tm_mon) + std::to_string(ltm->tm_mday)
                      + "_" + std::to_string(ltm->tm_hour) + std::to_string(ltm->tm_min) + std::to_string(ltm->tm_sec);
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
    int buffer_size = 1000;
    bool newDataline = true;
    bool in_transmit = true;
    int recv_buffer_size;

    if (isSubmarine)
        recv_buffer_size = (10+4+1+2+16+2*2500+4) * sub_length;
    else
        recv_buffer_size = (10+4+1+2+2+2*2500+4) * sub_length;
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

    while(in_transmit) {
        if (time_milisecond < update_rate || sub_file_bytes.empty())
        {
            memset(recvBuf, 0, sizeof(recvBuf));

            recvfrom(sockSrv, recvBuf, sizeof(recvBuf), MSG_NOSIGNAL, (sockaddr *) &addrSrv, (socklen_t *) &length);
            buffer_cnt++;
            total_line_cnt++;

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
                line_data = file_to_pixel_V07(sub_file_bytes, sub_marker_locations);
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
    newname = newname + std::to_string(ltm->tm_year) + std::to_string(ltm->tm_mon) + std::to_string(ltm->tm_mday)
              + "_" + std::to_string(ltm->tm_hour) + std::to_string(ltm->tm_min) + std::to_string(ltm->tm_sec);

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


//
//void gainControl(DensityMap& grid, float Gain, bool& dataUpstate)
//{
//    printf("6000s later it will do some gain control\n");
//    std::this_thread::sleep_for(std::chrono::seconds(6000));
//
//    int deep = grid.getDim();
//
//    //std::vector<std::vector<std::vector<float> > > gMtx(deep, std::vector<std::vector<float> >(deep, std::vector<float>(deep, 0.0)));
//    std::vector<std::vector<std::vector<float>>> gMtx;
//    for (int i = 0; i < deep; ++i) {
//        gMtx.push_back(std::vector<std::vector<float>>{});
//        for (int j = 0; j < deep; ++j) {
//            gMtx.back().push_back(std::vector<float>{});
//            for (int k = 0; k < deep; ++k) {
//                gMtx.back().back().push_back(0);
//            }
//        }
//    }
//
//    //int forTest = deep/2;
//    for (int x = 0; x < deep; ++x)
//    {
//        for (int y = 0; y < deep; ++y) //y is 0 at bottom and deep at top
//        {
//            for (int z = 0; z < deep; ++z)
//            {
//                float r = sqrt((float)((x-deep/2)*(x-deep/2) + (deep-y)*(deep-y) + (z-deep/2)*(z-deep/2))) / deep; //the distance between the point and piezo origin
//                gMtx[x][y][z] = exp(Gain *r);
//                //grid.cells[x][y][z]*= gMtx[x][y][z];
//            }
//        }
//    }
//    dataUpstate = true;
//}

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
        if (i % 2500 == 0) printf("\n");
    }
    printf("\n");
    return file_bytes;
}

void data_to_pixel(std::vector<scan_data_struct> _scan_data, std::vector<line_data_struct>& _line_data){
    for (int i = 0; i < (int)_scan_data.size(); ++i){
        double angle = _scan_data.at(i).encoder * 360.0 / 4096.0;
        //angle = convert_angle_2d_probe(angle);
//        double ax = 9*Cos(angle - 222 );
//        double ay = 9*Sin(angle - 222 );
//        double piezo = atan2(ay+21, ax) * 180.0 / M_PI - 180.0;
        float piezo = angle + 175;

        /* angle of the lx16 */
        float angle_16 = _scan_data.at(i).lx16 * 360.0 / 4096.0;
        /* find min and max */
        for (int j = 0; j < buffer_length; ++j){
            adc_max = std::max(adc_max, _scan_data.at(i).buffer[j]);
            adc_min = std::min(adc_min, _scan_data.at(i).buffer[j]);
        }

        int piezoProbe = 101; /*assume v = 1000 m/s */

        line_data_struct dataline;
        dataline.p1 = {piezoProbe*Cos(piezo),piezoProbe*Sin(piezo), 0};
        /* normalize on the go */
        for (int j = 0; j < buffer_length; ++j){
            intensity = ((double)_scan_data.at(i).buffer[j] - adc_min)/(adc_max-adc_min);
            dataline.vals.push_back(static_cast<unsigned char>(intensity*255));
        }
        dataline.p2 = {(buffer_length+piezoProbe)*Cos(piezo), (buffer_length+piezoProbe)*Sin(piezo), 0};
        //glm::mat4 rot = Rotation::convertRotationMatrix(_scan_data.at(i).quaternion[0], _scan_data.at(i).quaternion[1], _scan_data.at(i).quaternion[2], _scan_data.at(i).quaternion[3]);
        glm::mat4 rot = glm::mat4(1.0f);
        rot = glm::rotate(rot, glm::radians(angle_16) , glm::vec3(0, 1, 0)); /* inverse later to compare */
        dataline.p1 = rot * glm::vec4(dataline.p1,1);
        dataline.p2 = rot * glm::vec4(dataline.p2,1);
        _line_data.push_back(dataline);
        adc_max = 0; adc_min = 0;
    }
//    printf("start write euler...\n");
//    std::ofstream fileout("data/real_euler.txt", std::ios::trunc|std::ios::out);
//    for (auto s: _scan_data){
//        glm::vec3 euler = Rotation::convertToEulerAngle(s.quaternion[0], s.quaternion[1], s.quaternion[2], s.quaternion[3]);
//        fileout << euler.x << ' ' << euler.y << ' ' << euler.z << ' ' << std::endl;
//    }
//    fileout.close();
//    printf("euler file generate!\n");
}

void file_to_data(std::vector<unsigned char> _file_bytes, std::vector<int> _marker_locations, std::vector<scan_data_struct> & _scan_data){
    _scan_data.clear();
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
        memcpy(crc_input, time_stamp_char, sizeof(time_stamp_char));
        memcpy(crc_input+sizeof(time_stamp_char), &probe_type_char, sizeof(probe_type_char));
        memcpy(crc_input+sizeof(time_stamp_char)+sizeof(probe_type_char), encoder_char, sizeof(encoder_char));
        memcpy(crc_input+sizeof(time_stamp_char)+sizeof(probe_type_char)+sizeof(encoder_char), lx16_char, sizeof(lx16_char));
        memcpy(crc_input+sizeof(time_stamp_char)+sizeof(probe_type_char)+sizeof(encoder_char)+ sizeof(lx16_char), adc_char, sizeof(adc_char));
        crc_result = crc32c(0, crc_input, sizeof(crc_input));
        crc_result = changed_endian_4Bytes(crc_result);
        memcpy(crc_result_char, (unsigned char *)&crc_result, sizeof (crc_result));

        // add a judgement based on the quaternion values
//        float epsilon = 0.01;
//        float sumQ = 0;
//        for (auto q: quaternion) sumQ += q*q;
//        if (pow(sumQ, 0.5) > 1 - epsilon && pow(sumQ, 0.5) < 1 + epsilon){  // uncomment this for only using data with (w2+x2+y2+z2) == 1
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
            _scan_data.push_back(temp_struct);
        }
    }

    //write probe data to test
    //printf("start write imu...\n");
    std::ofstream fileout("data/real_imu.txt", std::ios::trunc|std::ios::out);
    for (auto s: _scan_data){
        fileout << s.quaternion[0] << ' ' << s.quaternion[1] << ' ' << s.quaternion[2] << ' ' << s.quaternion[3] << std::endl;
    }
    fileout.close();
    //printf("Real IMU file generate!\n");
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