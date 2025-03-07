#include"configMgr.hpp"
#include<boost/filesystem.hpp>
#include<boost/property_tree/ptree.hpp>
#include<boost/property_tree/ini_parser.hpp>
#include <iostream>

ConfigMgr::ConfigMgr() {    
    //构建config.ini的文件路径
    boost::filesystem::path current_path = boost::filesystem::current_path();
    boost::filesystem::path config_path = current_path / "config.ii";
    std::cout<< "config path : " << config_path << std::endl;

    
    //读取config.ini文件
    boost::property_tree::ptree pt;
    boost::property_tree::read_ini(config_path.string(), pt);
    
    //便利INI文件里的所有section
    for(const auto& section_pair : pt){
        const std::string &section_name = section_pair.first;
        const boost::property_tree::ptree &section_tree = section_pair.second;
        
        //对于每个section,便利其所有的key-value
        std::map<std::string, std::string> section_config;
        for(const auto& key_value_pair : section_tree){
            const std::string &key = key_value_pair.first;
            const std::string &value = key_value_pair.second.get_value<std::string>();
            
            section_config[key] = value;
        }
        SectionInfo section_info;
        section_info.section_datas_ = section_config;
        //将section和key-value的map存储到configMap中
        configMap_[section_name] = section_info;
    }
    
    //输出所有的section和key-value对
    for(const auto& section_pair : configMap_){
        const std::string &section_name = section_pair.first;
        SectionInfo section_info = section_pair.second;
        std::cout << "[" << section_name << "]" << std::endl;
        
        for(const auto& kv_pair : section_info.section_datas_){
            std::cout<<kv_pair.first<<"="<<kv_pair.second<<std::endl;
        }
    }
}
