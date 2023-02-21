#pragma once

#include <nlohmann/json.hpp>
#include <vector>
#include <fstream>
#include "graham.hpp"
using json = nlohmann::json;

class annotationWriter{
public:    
    annotationWriter(){}
    void addPolygon(const std::vector<Point> points){
        std::vector<int> all_x, all_y;
        for(auto point: points){
            all_x.push_back(point.x);
            all_y.push_back(point.y);
        }
        polygons_.push_back(all_x);
        polygons_.push_back(all_y);
    }
    void genAnnotation(const std::string& filename){
        json annotation;
        std::vector<json> regions;
        for (size_t i = 0; i < polygons_.size(); i += 2) {
        json shape_attributes;
        shape_attributes["name"] = "polygon";
        shape_attributes["all_points_x"] = polygons_[i];
        shape_attributes["all_points_y"] = polygons_[i + 1];

        json region;
        region["shape_attributes"] = shape_attributes;
        regions.push_back(region);
        }
        annotation["regions"] = regions; 
        annotation["filename"] = filename;
        annotations[filename] = annotation;
    }
    void writeToFile(const std::string& filename){
        std::ofstream ofs(filename);
        ofs << annotations.dump(2);
    }
private:
    json annotations;
    //[x1,x2,x3],[y1,y2,y3],[x4,x5,x6],[y4,y5,y6]
    std::vector<std::vector<int>> polygons_;

};