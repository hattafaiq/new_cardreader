#ifndef CONFIG_PARSER_H
#define CONFIG_PARSER_H
#include "rapidjson/document.h"
#include <fstream>
#include <sstream>
#include <string>

class ConfigJsonReader{
public:
    explicit ConfigJsonReader(){}
    ~ConfigJsonReader(){}
    std::string readfile(std::string_view name){
        std::fstream file(name.data());
        if(file.is_open()){
            std::stringstream log;
            log << file.rdbuf();
            return log.str();
        }
        return "";
    }

    bool LoadFile(std::string_view name){
        auto buf = readfile(name);
        rapidjson::Document doc;
        doc.Parse(buf.c_str());
        if(doc.IsObject()){
            if(doc.HasMember("host") &&
                doc["host"].IsString()){
                host= doc["host"].GetString();
            }
            if(doc.HasMember("port") &&
                doc["port"].IsInt()){
                port = doc["port"].GetInt();
            }
            if(doc.HasMember("bautrate") &&
                doc["bautrate"].IsInt()){
                bautrate = doc["bautrate"].GetInt();
            }
            if(doc.HasMember("com")){
                com = doc["com"].GetString();
            }
            if(doc.HasMember("host_pinpad")){
                host_pinpad = doc["host_pinpad"].GetString();
            }
            if(doc.HasMember("port_pinpad")){
                port_pinpad = doc["port_pinpad"].GetInt();
            }
            return true;
        }
        return false;
    }

    std::string host;
    int port;
    std::string com;
    int bautrate;

    std::string host_pinpad;
    int port_pinpad;
private:

};

#endif