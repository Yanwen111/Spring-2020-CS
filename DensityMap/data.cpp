#include <iostream>

#include "data.h"

/* Data Processing */
const unsigned char marker[10] = {0x00,0x01,0x00,0x01,0x00,0x01,0x00,0x01,0x00,0x01};
int marker_index, marker_index_next, buffer_length, signal_draw_index;
int16_t adc_max = 0;
int16_t adc_min = 0;
bool marker_flag;
unsigned char time_stamp_char[4];
unsigned long time_stamp;
unsigned char probe_type_char;
unsigned char encoder_char[2];
unsigned short encoder;
unsigned char adc_char[2*2500];
unsigned char adc_temp[2];
unsigned char crc_char[4];
uint32_t crc_result;
unsigned char crc_result_char[4];
unsigned char crc_input[4+1+2+2*2500];
int16_t adc;
short buffer[2500];
double intensity;

void gainControl(DensityMap& grid, float Gain)
{
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

    int forTest = deep/2;
    for (int x = 0; x < deep; ++x)
    {
        for (int y = 0; y < deep; ++y) //y is 0 at bottom and deep at top
        {
            //for (int z = 0; z < deep; ++z)
            float r = sqrt((float)((x-deep/2)*(x-deep/2) + (deep-y)*(deep-y))) / deep; //the distance between the point and piezo origin
            gMtx[x][y][forTest] = exp(Gain *r);
            grid.cells[x][y][forTest]*= gMtx[x][y][forTest];
        }
    }
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
    }
    printf("\n");
    return file_bytes;
}

void data_to_pixel(std::vector<scan_data_struct> _scan_data, std::vector<screen_data_struct> & _screen_data, std::vector<line_data_struct>& _line_data){
    //printf("%d\n", (int)_scan_data.size());
    for (int i = 0; i < (int)_scan_data.size(); ++i){
        double angle = _scan_data.at(i).encoder * 360.0 / 4096.0;
        //angle = convert_angle_2d_probe(angle);
        double ax = 9*Cos(angle - 85 );
        double ay = 9*Sin(angle - 85 );
        double piezo = atan2(ay+21, ax) * 180.0 / M_PI - 180.0;
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
            screen_data_struct temp_data = {(j+1) * Cos(piezo), (j+1) * Sin(piezo), 0, intensity};
            _screen_data.push_back(temp_data);
        }
        dataline.p2 = {buffer_length*Cos(piezo), buffer_length*Sin(piezo), 0};
        _line_data.push_back(dataline);
        adc_max = 0; adc_min = 0;
    }
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
        /* adc */
        /* determine the length of buffer */
        buffer_length = (int)(_marker_locations.at(i+1) - _marker_locations.at(i) - sizeof(marker) - sizeof(time_stamp_char) -
                              sizeof(probe_type_char) - sizeof(encoder_char) - sizeof(crc_char))/2;
        for (int j = 0; j < buffer_length; ++j){
            for (int k = 0; k < (int)sizeof(adc_temp); ++k){
                adc_temp[k] = _file_bytes.at(marker_index + sizeof(marker) + sizeof(time_stamp_char) + sizeof(probe_type_char) + sizeof(encoder_char) + j * 2 + k);
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
        memcpy(crc_input+sizeof(time_stamp_char)+sizeof(probe_type_char)+sizeof(encoder_char), adc_char, sizeof(adc_char));
        crc_result = crc32c(0, crc_input, sizeof(crc_input));
        crc_result = changed_endian_4Bytes(crc_result);
        memcpy(crc_result_char, (unsigned char *)&crc_result, sizeof (crc_result));
        /* if two crc matches */
        if (compare_crc(crc_char, crc_result_char, sizeof(crc_char))){
            scan_data_struct temp_struct;
            temp_struct.time_stamp = time_stamp;
            temp_struct.encoder = encoder;
            /* normalize on the go */
            for (int j = 0; j < buffer_length; ++j) {
                temp_struct.buffer[j] = buffer[j];
                //printf("Intensity:%f\n", temp_struct.buffer[j]);
            }
            _scan_data.push_back(temp_struct);
        }
    }
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