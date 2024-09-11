#pragma once
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

void log_error(const char* msg);
bool mkdirs(const char* path);
bool write_file(const char* filename, const char* buf, unsigned long buf_len);

#ifdef _WIN32
#define LOG_E(fmt,...) \
			char buf[512];\
			sprintf(buf,fmt,__VA_ARGS__);\
			log_error(buf);
#else
#define LOG_E(fmt,...) \
			char buf[512];\
			sprintf(buf,fmt,##__VA_ARGS__);\
			log_error(buf);
#endif

//// -- others 
struct Transform
{
	double radian_x;
	double radian_y;
	double min_height;
};

struct Box
{
	double matrix[12];
};

struct Region
{
	double min_x;
	double min_y;
	double max_x;
	double max_y;
	double min_height;
	double max_height;
};

bool write_tileset_region(
	Transform* trans, 
	Region& region,
	double geometricError,
	const char* b3dm_file,
	const char* json_file);

bool write_tileset_box( 
	Transform* trans, Box& box,  	
	double geometricError,
	const char* b3dm_file,
	const char* json_file);

bool write_tileset(
	double longti, double lati, 
	double tile_w, double tile_h, 
	double height_min, double height_max,
	double geometricError,
	const char* filename, const char* full_path
	) ;

double degree2rad(double val);
double lati_to_meter(double diff);
double longti_to_meter(double diff, double lati);
double meter_to_lati(double m);
double meter_to_longti(double m, double lati);

std::vector<double> transfrom_xyz(double radian_x, double radian_y, double height_min);
void transform_c(double center_x, double center_y, double height_min, double* ptr);