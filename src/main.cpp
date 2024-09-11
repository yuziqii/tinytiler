#include <iostream>
#include <cxxopts.hpp>
#include <fstream>
#include <tinyxml2.h>
#include <string>
#include <vector>
#include "osgb.h"
#include "tileset.h"

struct srs {
    std::string authority;
    int code;

    srs() = default;
    srs(const std::string& auth, int c) : authority(auth), code(c) {}

    std::string to_string() const {
        return authority + ":" + std::to_string(code);
    }

    static srs from_string(const std::string& srs_str) {
        srs result;
        auto pos = srs_str.find(':');
        if (pos != std::string::npos) {
            result.authority = srs_str.substr(0, pos);
            result.code = std::stoi(srs_str.substr(pos + 1));
        }
        return result;
    }
};

struct srs_origin {
    double x;
    double y;
    double z;

    srs_origin() = default;
    srs_origin(double x, double y, double z) : x(x), y(y), z(z) {}

    std::string to_string() const {
        std::ostringstream oss;
        oss << x << "," << y << "," << z;
        return oss.str();
    }

    static srs_origin from_string(const std::string& origin_str) {
        srs_origin result;
        std::istringstream iss(origin_str);
        std::string token;
        if (std::getline(iss, token, ',')) result.x = std::stod(token);
        if (std::getline(iss, token, ',')) result.y = std::stod(token);
        if (std::getline(iss, token, ',')) result.z = std::stod(token);
        return result;
    }
};

class model_metadata {
public:
    std::string version_;
    srs srs_;
    srs_origin srs_origin_;

    model_metadata() = default;
    model_metadata(const std::string& version, const struct srs& srs_val, const struct srs_origin& srs_origin_val)
        : version_(version), srs_(srs_val), srs_origin_(srs_origin_val) {}

    static model_metadata from_xml(const tinyxml2::XMLElement* root) {
        model_metadata metadata;

        if (root) {
            if (const char* version_attr = root->Attribute("version")) {
                metadata.version_ = version_attr;
            }

            const tinyxml2::XMLElement* srs_element = root->FirstChildElement("SRS");
            if (srs_element && srs_element->GetText()) {
                metadata.srs_ = srs::from_string(srs_element->GetText());
            }

            const tinyxml2::XMLElement* srs_origin_element = root->FirstChildElement("SRSOrigin");
            if (srs_origin_element && srs_origin_element->GetText()) {
                metadata.srs_origin_ = srs_origin::from_string(srs_origin_element->GetText());
            }
        }

        return metadata;
    }

    tinyxml2::XMLElement* to_xml(tinyxml2::XMLDocument& doc) const {
        tinyxml2::XMLElement* root = doc.NewElement("ModelMetadata");
        root->SetAttribute("version", version_.c_str());

        tinyxml2::XMLElement* srs_element = doc.NewElement("SRS");
        srs_element->SetText(srs_.to_string().c_str());
        root->InsertEndChild(srs_element);

        tinyxml2::XMLElement* srs_origin_element = doc.NewElement("SRSOrigin");
        srs_origin_element->SetText(srs_origin_.to_string().c_str());
        root->InsertEndChild(srs_origin_element);

        return root;
    }

    operator std::string() const {
        tinyxml2::XMLDocument doc;
        tinyxml2::XMLElement* root = to_xml(doc);
        doc.InsertFirstChild(root);

        tinyxml2::XMLPrinter printer;
        doc.Print(&printer);
        return std::string(printer.CStr());
    }

    static model_metadata from_string(const std::string& xml_content) {
        tinyxml2::XMLDocument doc;
        doc.Parse(xml_content.c_str());
        return from_xml(doc.FirstChildElement("ModelMetadata"));
    }
};

int main(int argc, char* argv[])
{
    cxxopts::Options options("osgb2tiles", "A simple program which can convert osgb files to 3dtiles.");
    options.add_options()
        ("i,input", "Input directory", cxxopts::value<std::string>())
        ("o,output", "Output directory", cxxopts::value<std::string>())
        //("f,format", "Output format (e.g., 3dtiles)", cxxopts::value<std::string>())
        ("h,help", "Print usage");
    auto result = options.parse(argc, argv);

    if (result.count("help")) {
        std::cout << options.help() << std::endl;
        return 0;
    }

    if (!result.count("input") || !result.count("output")) {
            std::cerr << "Error: Input and Output are required.\n";
            std::cerr << options.help() << std::endl;
            return 1;
    }

    std::string input = result["input"].as<std::string>();
    std::string output = result["output"].as<std::string>();

    // 836974.635391304,815456.572217391
    // 114.18373090671055,22.277972645442148
    tinyxml2::XMLDocument doc;
    std::string metadata_path = input + "metadata.xml";
    if (doc.LoadFile(metadata_path.c_str()) != tinyxml2::XML_SUCCESS) {
        std::cerr << "Failed to load XML file: " << metadata_path << std::endl;
        return 1;
    }

    tinyxml2::XMLElement* root = doc.RootElement();
    if (!root) {
        std::cerr << "Failed to find root element." << std::endl;
        return 1;
    }

    auto metadata = model_metadata::from_xml(root);
    if (metadata.srs_.authority == "EPSG"){
        if (metadata.srs_.code == 4326){
            osgb_batch_convert(input, output, metadata.srs_origin_.x, metadata.srs_origin_.y);
        }else{
            // TODO: to be implemented
            std::cerr << "Error: The spatial reference system '" 
                      << metadata.srs_.to_string() 
                      << "' is not supported. Please manually convert it to 'EPSG:4326'." 
                      << std::endl;
            return 1;
        }
    }
    else if (metadata.srs_.authority == "ENU"){
        // TODO: to be implemented
        std::cerr << "Error: The spatial reference system '" 
                      << metadata.srs_.to_string() 
                      << "' is not supported. Please manually convert it to 'EPSG:4326'." 
                      << std::endl;
        return 1;
    }else{
        // invalid srs
        std::cerr << "Error: The spatial reference system '" 
                      << metadata.srs_.to_string() 
                      << "' is not supported. Please manually convert it to 'EPSG:4326'." 
                      << std::endl;

        return 1;
    }

    return 0;
}