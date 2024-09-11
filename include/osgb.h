#pragma once
#include <string>
#include <vector>
#include <functional>
#include <filesystem>

namespace fs = std::filesystem;

struct TileResult {
    std::string json;
    std::string path;
    std::vector<double> box_v;
};

struct OsgbInfo {
    std::string in_dir;
    std::string out_dir;
    std::function<void(const TileResult&)> sender;
};

double get_geometric_error(double center_y, int lvl);

std::vector<double> box_to_tileset_box(const std::vector<double>& box_v);

void osgb_batch_convert(const fs::path& input, const fs::path& output, double center_x, double center_y);