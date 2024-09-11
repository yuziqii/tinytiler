#pragma once
#include <string>
struct MeshInfo;

void* osgb23dtile_path(const char* in_path, const char* out_path,
                    double *box, int* len, double x, double y,
                    int max_lvl, bool pbr_texture);

bool osgb2glb_buf(std::string path, std::string& glb_buff, MeshInfo& mesh_info);