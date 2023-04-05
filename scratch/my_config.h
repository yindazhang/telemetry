#ifndef MY_CONFIG_H
#define MY_CONFIG_H

#define DEFAULT_PORT 80

uint32_t collectorMbps = 10000;
double start_time = 2;
double duration = 0.5;

uint32_t utilGap = 10000;
uint16_t intSize = 0;
uint32_t OrbWeaver = 0;

uint32_t taskId = 2;

uint32_t topology = 0;

uint32_t ecmpConfig = 0;
uint32_t failConfig = 0;
uint32_t recordConfig = 0;

int fct_record = 0;

std::string file_name;

#endif