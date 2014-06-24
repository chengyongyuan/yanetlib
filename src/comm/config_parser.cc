#include <fstream>
#include "strutil.h"
#include "config_parser.h"

namespace yanetlib {
namespace comm {

using std::string;
using std::vector;
using std::map;

SimpleConf::SimpleConf(const string& fname)
                   : filename_(fname) {
}

SimpleConf::~SimpleConf() { }

bool SimpleConf::Init(const string& fname) {
    string line;
    vector<string> vkval;

    filename_ = fname;
    //filename not init
    if (filename_.empty()) return false;
    std::ifstream infile(filename_.c_str());
    if (!infile.good()) return false;
    while(getline(infile, line)) {
        vkval.clear();
        //trim uncessary space first    
        line = StripStringLeft(line, " ");
        //empty line or comment line are ingored
        if (line.empty() || line[0] == '#') continue;
        //split key  value
        SplitString(line, " ", &vkval, 1);
        if (vkval.size() != 2) return false;
        //strip head spaces in value, put it in map
        kmap_[vkval[0]] = StripStringLeft(vkval[1], " ");
    }
    //TODO: for unittest
    //PrintKeyVal();
    infile.close();
    return true;
}

void SimpleConf::PrintKeyVal() const {
    for (map<string, string>::const_iterator it = kmap_.begin();
         it != kmap_.end(); ++it) {
        fprintf(stderr, "%s -> %s\n", it->first.c_str(), it->second.c_str());
    }
}

bool SimpleConf::GetVal(const string& key, string& val, 
                        const string& def) {
    val = def;
    map<string, string>::const_iterator it = kmap_.find(key);
    if (it == kmap_.end()) {//not found the key
        return false;
    }
    val = it->second;
    return true;
}

bool SimpleConf::GetIntVal(const string& key, int& val,
                           int def) {
    long longval = 0;

    val = def;
    map<string, string>::const_iterator it = kmap_.find(key);
    if (it == kmap_.end()) {//not found the key
        return false;
    }
    if (!String2Long(it->second, longval)) { //convert to long fail
        return false;
    }
    val = longval;
    return true;
}

bool SimpleConf::GetFloatVal(const string& key, double& val,
                           double def) {
    double dval = 0;

    val = def;
    map<string, string>::const_iterator it = kmap_.find(key);
    if (it == kmap_.end()) {//not found the key
        return false;
    }
    if (!String2Double(it->second, dval)) { //convert to long fail
        return false;
    }
    val = dval;
    return true;
}

bool SimpleConf::GetArray(const string& key, vector<string>& ar,
              const vector<string>& def) {
    map<string, string>::const_iterator it = kmap_.find(key);
    if (it == kmap_.end()) {//not found the key
        ar.assign(def.begin(), def.end());
        return false;
    }
    SplitString(it->second, " ;:|,", &ar);
    return true; 
}

bool SimpleConf::GetIntArray(const string& key, vector<int>& ar,
              const vector<int>& def) {
    map<string, string>::const_iterator it = kmap_.find(key);
    if (it == kmap_.end()) {//not found the key
        ar.assign(def.begin(), def.end());
        return false;
    }
    vector<string> vtmp;
    SplitString(it->second, " ;:|,", &vtmp);
    for (vector<string>::const_iterator it = vtmp.begin();
         it != vtmp.end(); ++it) {
        //strict condition:
        //if one value is invalid, then dump all others, using default.
        long longval;
        if (!String2Long(*it, longval)) {
            ar.clear();
            ar.assign(def.begin(), def.end());
            return false;
        }
        ar.push_back(longval);
    }
    return true; 
}

bool SimpleConf::GetFloatArray(const string& key, vector<double>& ar,
              const vector<double>& def) {
    map<string, string>::const_iterator it = kmap_.find(key);
    if (it == kmap_.end()) {//not found the key
        ar.assign(def.begin(), def.end());
        return false;
    }
    vector<string> vtmp;
    SplitString(it->second, " ;:|,", &vtmp);
    for (vector<string>::const_iterator it = vtmp.begin();
         it != vtmp.end(); ++it) {
        //strict condition:
        //if one value is invalid, then dump all others, using default.
        double dval;
        if (!String2Double(*it, dval)) {
            ar.clear();
            ar.assign(def.begin(), def.end());
            return false;
        }
        ar.push_back(dval);
    }
    return true; 
}

} //namespace comm 
} //namespace yanetlib
