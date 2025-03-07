#pragma once
#include <map>
#include <string>

struct SectionInfo {
    SectionInfo() {}
    ~SectionInfo() {}
    SectionInfo(const SectionInfo &src) { section_datas_ = src.section_datas_; }
    SectionInfo &operator=(const SectionInfo &src) {
        if (this == &src) {
            return *this;
        }
        this->section_datas_ = src.section_datas_;
        return *this;
    }
    std::map<std::string, std::string> section_datas_;
    std::string operator[](const std::string &key) {
        if (section_datas_.find(key) == section_datas_.end()) {
            return "";
        }
        return section_datas_[key];
    }
};

class ConfigMgr  {
  public:
    ~ConfigMgr() { 
        configMap_.clear();
    }
    SectionInfo operator[](const std::string &section) {
        if(configMap_.find(section) == configMap_.end()){
            return SectionInfo();
        }
        return configMap_[section];
    }
    ConfigMgr(const ConfigMgr& src){
        configMap_ = src.configMap_;
    }
    ConfigMgr& operator=(const ConfigMgr& src){
        if(this == &src){
            return *this;
        }
        configMap_ = src.configMap_;
        return *this;
    }
    static ConfigMgr& GetInstance(){
        static ConfigMgr instance;
        return instance;
    }

  private:
    //单例私有化构造函数
    ConfigMgr();
    //存储section和keyvalue的map
    std::map<std::string, SectionInfo> configMap_;
};