#include <iostream>
#include <thread>
#include <chrono>

#include "data.h"
#include "rotation.h" /* for test of fake data */

/* For calculating the scale */
#define Velocity 110200  // speed in phantom, cm/s
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

void realDemo(DensityMap& grid, bool& dataUpdate)
{
    //read the data from current red pitaya 2d data.
    std::vector<unsigned char> file_bytes;
    std::vector<int> marker_locations;
    std::vector<scan_data_struct> scan_data;
    std::vector<line_data_struct> line_data;

    char fileName[255];
    std::cout << "Please type the file you want to open (<data/xxxx.txt>): " << std::endl;
    std::cin >> fileName;
    file_bytes = readFile(fileName);
    //file_bytes = readFile("data/tapioca_1.txt");
    /* find all marker locations */
    marker_locations = find_marker(file_bytes);
    /* convert file bytes to data struct */
    file_to_data(file_bytes, marker_locations, scan_data);
//    printf("the size of scan_data is %d\n", scan_data.size());
    /* convert data to vertex on screen */
    data_to_pixel(scan_data, line_data);
    printf("find the screen_data\n");

    int ddim = grid.getDim();
    int len = line_data[0].vals.size(); // 2500, equl to buffer size
//    printf("=====\nPlease choose the maximum depth you want to show ( from 1 to %d):", len);
//    std::cin >> len;
    len = 1500; // change range
    int cnt = 0;
    for  (auto l: line_data)
    {
        glm::vec3 ps = {0.5, 1, 0.5};
        glm::vec3 pe = {l.p2.x/len - l.p1.x/len  + 0.5, l.p2.y/len - l.p1.y/len + 1, l.p2.z/len - l.p1.z/len +0.5};
        grid.writeLine(ps, pe, l.vals);
        /* for rendering line by line */
//        cnt ++;
//        if (cnt%200 == 0)
//        {
//            printf("draw line %d\n", cnt);
//            std::this_thread::sleep_for(std::chrono::seconds(2));
//        }
    }

    /* add some scale. Each line refer to 1 centimeter */
    int d1c = (ddim * 2 * Frequency * 1e6)/(Velocity * len) ;
    int d1m = d1c / 10;
    int d1c0 = d1c, d1m0 = d1m;
    if (d1m > 2 ) /* zoom big, so we can add milimeter scales */
    {
        while (d1m < ddim)
        {
            for (int i = 0; i < ddim; ++i)
                //grid.cells[0][ddim - d1m - 1][i] = 130;
                grid.writeCell(ddim/2, ddim - d1m - 1, i, 130);
            d1m += d1m0;
        }
    }
    while (d1c < ddim)
    {
        for (int i = 0; i < ddim; ++i)
            //grid.cells[0][ddim - d1c -1][i] = 254;
            grid.writeCell(ddim/2, ddim - d1c - 1, i, 254);
        d1c += d1c0;
    }
    dataUpdate = true;
}


void gainControl(DensityMap& grid, float Gain, bool& dataUpstate)
{
    printf("6000s later it will do some gain control\n");
    std::this_thread::sleep_for(std::chrono::seconds(6000));

    int deep = grid.getDim();

    //std::vector<std::vector<std::vector<float> > > gMtx(deep, std::vector<std::vector<float> >(deep, std::vector<float>(deep, 0.0)));
    std::vector<std::vector<std::vector<float>>> gMtx;
    for (int i = 0; i < deep; ++i) {
        gMtx.push_back(std::vector<std::vector<float>>{});
        for (int j = 0; j < deep; ++j) {
            gMtx.back().push_back(std::vector<float>{});
            for (int k = 0; k < deep; ++k) {
                gMtx.back().back().push_back(0);
            }
        }
    }

    //int forTest = deep/2;
    for (int x = 0; x < deep; ++x)
    {
        for (int y = 0; y < deep; ++y) //y is 0 at bottom and deep at top
        {
            for (int z = 0; z < deep; ++z)
            {
                float r = sqrt((float)((x-deep/2)*(x-deep/2) + (deep-y)*(deep-y) + (z-deep/2)*(z-deep/2))) / deep; //the distance between the point and piezo origin
                gMtx[x][y][z] = exp(Gain *r);
                //grid.cells[x][y][z]*= gMtx[x][y][z];
            }
        }
    }
    dataUpstate = true;
}

std::vector<unsigned char> readFile(const char* directory)
{
    std::ifstream inFile(directory, std::ios::in | std::ios::binary);
    if (!inFile){
        printf("Failed to open file.\n");
        //return -1;
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
    printf("%d\n", (int)_scan_data.size());
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

        line_data_struct dataline;
        dataline.p1 = {Cos(piezo),Sin(piezo), 0};
        /* normalize on the go */
        for (int j = 0; j < buffer_length; ++j){
            intensity = ((double)_scan_data.at(i).buffer[j] - adc_min)/(adc_max-adc_min);
            dataline.vals.push_back(static_cast<unsigned char>(intensity*255));
        }
        dataline.p2 = {buffer_length*Cos(piezo), buffer_length*Sin(piezo), 0};
        //glm::mat4 rot = Rotation::convertRotationMatrix(_scan_data.at(i).quaternion[0], _scan_data.at(i).quaternion[1], _scan_data.at(i).quaternion[2], _scan_data.at(i).quaternion[3]);
        glm::mat4 rot = glm::mat4(1.0f);
        rot = glm::rotate(rot, glm::radians(angle_16) , glm::vec3(0, 1, 0)); /* inverse later to compare */
        dataline.p1 = rot * glm::vec4(dataline.p1,1);
        dataline.p2 = rot * glm::vec4(dataline.p2,1);
        _line_data.push_back(dataline);
        adc_max = 0; adc_min = 0;
    }
    printf("start write euler...\n");
    std::ofstream fileout("data/real_euler.txt", std::ios::trunc|std::ios::out);
    for (auto s: _scan_data){
        glm::vec3 euler = Rotation::convertToEulerAngle(s.quaternion[0], s.quaternion[1], s.quaternion[2], s.quaternion[3]);
        fileout << euler.x << ' ' << euler.y << ' ' << euler.z << ' ' << std::endl;
    }
    fileout.close();
    printf("euler file generate!\n");
    /*
    Rotation rot = Rotation(glm::vec4(-0.43914795f, -0.11865234f, -0.05950928f,-PI));
    int len = _screen_data.size();
    printf("%d\n", _screen_data.size());
    for (int i = 0; i < len; ++i)
    {
        glm::vec4 origin = {_screen_data[i].X, _screen_data[i].Y, _screen_data[i].Z, 1};
        glm::vec4 trans = rot.getRotationMatrix()*origin;
        screen_data_struct tmp = {trans.x, trans.y, trans.z, _screen_data[i].I};
        _screen_data.push_back(tmp);
    }
    printf("%d\n", _screen_data.size());
     */
}

void file_to_data(std::vector<unsigned char> _file_bytes, std::vector<int> _marker_locations, std::vector<scan_data_struct> & _scan_data){
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
    printf("start write imu...\n");
    std::ofstream fileout("data/real_imu.txt", std::ios::trunc|std::ios::out);
    for (auto s: _scan_data){
        fileout << s.quaternion[0] << ' ' << s.quaternion[1] << ' ' << s.quaternion[2] << ' ' << s.quaternion[3] << std::endl;
    }
    fileout.close();
    printf("Real IMU file generate!\n");
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