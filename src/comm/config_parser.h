#ifndef YANETLIB_COMMON_CONFIG_PARSER_H
#define YANETLIB_COMMON_CONFig_PARSER_H

#include <vector>
#include <string>
#include <map>

namespace yanetlib {
namespace comm {

//A simple text line based config parser.
//Just list a minimum apache's config format.
//Config file is processed in a lined based fasion.
//lines begin with '#' are consided as comments, ingored.
//every line has the format ' KEY [SPACED]+ VALUES
//every line can have one value, or many values, those
//many values in one line CAN SEPARATED BY ',' ':', ';'
//spaces are igored.
//if two key happends to be same, using last one appear 
//in file.
//eg.
//KEY1  val1
//#this is a comment
//KEY2 val1, val2
//KEY3      val3;val3
class SimpleConf {
 public:
     //Constructor, set the config filename.
     //not parse it yet.
     SimpleConf(const std::string& fname = "");
     
     ~SimpleConf();

     //Set the config file and parse it.
     bool Init(const std::string& fname = "");

     //Get One Value for 'key'. all value are taken as
     //one long large string. spaces ahead of value 
     //are striped. if value is a empty string.or any
     //errors occured during parsing, use the default
     //string 'def'  eg.
     //Key     this is a long value
     //GetVal("Key") == "this is a lone value"
     //RETURN: true if parse ok. false: not ok, using def.
     bool GetVal(const std::string& key, std::string& val,
                 const std::string& def = "");

     //Just As GetVal except value are transformed to int val.
     //if any failure occured, use the default value instead.
     bool GetIntVal(const std::string& key, int& val, int def = 0);

     //Just As GetVal except value are transformed to double val.
     //if any failure occured, use the default value instead.
     bool GetFloatVal(const std::string& key, double& val, double def = 0.0);

     //Get the value by key, value are split using delimiters 
     //' ', ';', ':', '|'
     bool GetArray(const std::string& key, std::vector<std::string>& ar,
                   const std::vector<std::string>& def = std::vector<std::string>());

     //Just like GetArray except all value are converted to int.
     bool GetIntArray(const std::string& key,  std::vector<int>& ar,
                      const std::vector<int>& def = std::vector<int>());

     //Just like GetArray except all value are converted to double.
     bool GetFloatArray(const std::string& key, std::vector<double>& ar,
                        const std::vector<double>& def = std::vector<double>());
 private:
     void PrintKeyVal() const;

 private:
     std::string filename_;
     std::map<std::string, std::string> kmap_;
};

} //namespace comm
} //namespace yanetlib

#endif //config_parser.h
