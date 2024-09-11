#include "osgb.h"
#include <iostream>
#include <cmath>
#include <filesystem>

#include <future>
#include <memory>
#include <cstring>
#include <nlohmann/json.hpp>
#include <algorithm>

#include "tileset.h"
#include "osgb23dtiles.h"

namespace fs = std::filesystem;

double get_geometric_error(double center_y, int lvl) {
    double x = center_y * M_PI / 180.0;
    double round = std::cos(x) * 2.0 * M_PI * 6378137.0;
    int pow = 1 << (lvl - 2); // 2^(lvl - 2)
    return 4.0 * round / (256 * pow);
}

std::vector<double> box_to_tileset_box(const std::vector<double>& box_v) {
    std::vector<double> box_new;
    
    box_new.push_back((box_v[0] + box_v[3]) / 2.0);
    box_new.push_back((box_v[1] + box_v[4]) / 2.0);
    box_new.push_back((box_v[2] + box_v[5]) / 2.0);

    box_new.push_back((box_v[3] - box_v[0]) / 2.0);
    box_new.push_back(0.0);
    box_new.push_back(0.0);

    box_new.push_back(0.0);
    box_new.push_back((box_v[4] - box_v[1]) / 2.0);
    box_new.push_back(0.0);

    box_new.push_back(0.0);
    box_new.push_back(0.0);
    box_new.push_back((box_v[5] - box_v[2]) / 2.0);

    return box_new;
}

void osgb_batch_convert(const fs::path& input, const fs::path& output, double center_x, double center_y){
    fs::path path = input / "Data";
    if (!fs::exists(path) || !fs::is_directory(path)) {
        throw std::runtime_error("Directory " + path.string() + " does not exist");
    }

    mkdirs(output.c_str());
    std::vector<OsgbInfo> osgb_dir_pair;
    std::vector<TileResult> tiles;
    
    double rad_x = degree2rad(center_x);
    double rad_y = degree2rad(center_y);
    // TODO: asio
    for (const auto& entry : fs::directory_iterator(path)) {
        if (fs::is_directory(entry)) {
            fs::path path_tile = entry.path();
            std::string stem = path_tile.stem().string();
            fs::path osgb = path_tile / (stem + ".osgb");

            if (fs::exists(osgb) && !fs::is_directory(osgb)) {
                fs::path out_dir = output / "Data" / stem;
                fs::create_directories(out_dir);
                std::vector<double> box(6, 0.0);
                int len = 0;
                auto out_ptr = osgb23dtile_path(osgb.string().c_str(), out_dir.string().c_str(), box.data(), &len, rad_x, rad_y, 100, true);
                std::vector<char> json_buf;
                if (!out_ptr)
                {
                    std::cout << "failed: " << osgb << "\n";
                }
                else
                {
                    json_buf.resize(len);
                    std::memcpy(json_buf.data(), out_ptr, len);
                    std::free(out_ptr);
                }
                auto json = std::string(json_buf.begin(), json_buf.end());
                auto t = TileResult{json, out_dir, box};
                tiles.push_back(t);
            } else {
                std::cerr << "Directory error: " << osgb << std::endl;
            }
        }
    }

    std::vector<double> root_box = {
        std::numeric_limits<double>::min(),  std::numeric_limits<double>::min(), std::numeric_limits<double>::min(),
        std::numeric_limits<double>::max(),  std::numeric_limits<double>::max(), std::numeric_limits<double>::max()
    };

    for(const auto& tile : tiles){
        for (size_t i = 0; i < 3; i++)
        {
            if (tile.box_v[i] > root_box[i])
            {
                root_box[i] = tile.box_v[i];
            }
        }

        for (size_t j = 3; j < 6; j++)
        {
            if (tile.box_v[j] < root_box[j])
            {
                root_box[j] = tile.box_v[j];
            }
        }
    }

    double matrix[16];
    transform_c(center_x, center_y, 0.0, matrix);
    nlohmann::json root_json = {
        {"asset", {
            {"version", "1.0"},
            {"gltfUpAxis", "Z"}
        }},
        {"geometricError", 2000},
        {"root", {
            {"transform", matrix},
            {"boundingVolume", {
                {"box", box_to_tileset_box(root_box)}
            }},
            {"geometricError", 2000},
            {"children", nlohmann::json::array()}
        }}
    };

    for (const auto& tile : tiles)
    {
        auto path = tile.path;
        nlohmann::json json_val = nlohmann::json::parse(tile.json);
        auto tile_box = json_val["boundingVolume"]["box"];
        
        std::string uri_path = path;
        size_t pos = uri_path.find(output);
        if (pos != std::string::npos) {
            uri_path.replace(pos, output.string().length(), "./");
        }

        nlohmann::json tile_object = {
            {"boundingVolume", {
                {"box", tile_box}
            }},
            {"geometricError", 1000},
            {"content", {
                {"uri", uri_path + "/tileset.json"}
            }}
        };

        root_json["root"]["children"].push_back(tile_object);

        nlohmann::json sub_tile = {
            {"asset", {
                {"version", "1.0"},
                {"gltfUpAxis","Z"}
            }},
            {"geometricError", 1000},
            {"root", json_val}
        };

        write_file(std::string(fs::path(path) / "tileset.json").c_str(), sub_tile.dump().c_str(), sub_tile.dump().size());
    }
    
    write_file(std::string(output / "tileset.json").c_str(), root_json.dump().c_str(), root_json.dump().size());
}