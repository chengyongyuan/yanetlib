#ifndef YANETLIB_COMM_STRUTIL_H
#define YANETLIB_COMM_STRUTIL_H

//This Files Contains Some String Utilility Functions.
//C++ string class is powerful, but not as python's. Many strings
//functions in this files are implemented like python's string or 
//list interface. eg. SplitString/ JoinString / StripString ...
//Many of its implementation are learned from google protocol bufffer
//src.
//
//
//Author: colincheng 334089103@qq.com
//

#include <limits.h>
#include <string>
#include <vector>

namespace yanetlib {
namespace comm {

// ----------------------------------------------------------------------
// HasPrefixString()
//	Check if a string has a prefix
// StripPrefixString()
//  If a string has a prefix, remove the prefix string, otherwise
//  return the original string.
// ---------------------------------------------------------------------
inline bool HasPrefixString(const std::string& str, 
                            const std::string& prefix) {
    return str.size() >= prefix.size() &&
           str.compare(0, prefix.size(), prefix) == 0;
}

inline std::string StripPrefixString(const std::string& str, 
                                      const std::string& prefix) {
	if (HasPrefixString(str, prefix)) {
		return str.substr(prefix.size());
	}
	return str;
}

// ----------------------------------------------------------------------
// HasSuffixString()
//	Check if a string has a suffix 
// StripSuffixString()
//  If a string has a suffix, remove the suffix string, otherwise
//  return the original string.
// ---------------------------------------------------------------------
inline bool HasSuffixString(const std::string& str, const std::string& suffix)
{
	return str.size() >= suffix.size() &&
		   str.compare(str.size()-suffix.size(), suffix.size(), suffix) == 0;
}

inline std::string StripSuffixString(const std::string& str, 
                                     const std::string& suffix) {
	if (HasSuffixString(str, suffix)) {
		return str.substr(0, str.size()-suffix.size());
	}
	return str;
}

// ----------------------------------------------------------------------
// StripString()
// StripStringLeft()
// StripStringRight()
//  Replace any character in 'remove' with 'replacewith'
//  Two verions. one replace the 'delim' chars. the other
//  one just delete the 'delim' chars(consective delim are deleted).
//
// ----------------------------------------------------------------------
void StripString(std::string* s, const char *remove,
                 char replacewith);

std::string StripStringLeft(const std::string& s,
                            const char* remove);

std::string StripStringRight(const std::string& s,
                            const char* remove);
std::string 
StripString(const std::string& s, const char* remove);

// ----------------------------------------------------------------------
//  LowerString()
//  UpperString()
//   Convert String To Lower Or Upper String. ASCII only. unlike
//   glibc. these functions not handle different locale
// ----------------------------------------------------------------------
inline void LowerString(std::string* s) {
    std::string::iterator end = s->end();
    for (std::string::iterator it = s->begin(); it != end; ++it) {
        if (*it >= 'A' && *it <= 'Z') *it += 'a' - 'A';
    }
}

inline void UpperString(std::string* s) {
    std::string::iterator end = s->end();
    for (std::string::iterator it = s->begin(); it != end; ++it) {
        if (*it >= 'a' && *it <= 'z') *it += 'A' - 'a';
    }
}

// ----------------------------------------------------------------------
//  StringReplace()
//  UpperString()
//   Give a string and two string pattern "old" and "new". Repalce first
//   occure of "old" with "new" if EXIST. if "replace_all" set then 
//   replace every occurence of.
//   Always RETURN a new string.
// ----------------------------------------------------------------------
std::string StringReplace(const std::string& s, const std::string& oldsub,
                          const std::string& newsub, bool replace_all);

// ----------------------------------------------------------------------
//  SplitString()
//   Split a string using a character delimiter. Append each string in
//   result. If there are conseutive delimiters, this function skips
//   them. split at most 'count' times. count=0(default) means split all
// ----------------------------------------------------------------------
void SplitString(const std::string& full, const char* delim,
                 std::vector<std::string>* result, int count = 0);

// ----------------------------------------------------------------------
//  JoinString()
//   Join string in vector into a single string use delimiter.We define
//   two version of this function. One return the string. one take a 
//   pointer to target string. this function will clear 'result' first.
//   delim can be empty string "".
// ----------------------------------------------------------------------
void JoinStrings(const std::vector<std::string>& components,
                const char* delim, std::string* result);

inline std::string JoinStrings(const std::vector<std::string>& components,
                             const char* delim) {
    std::string result;
    JoinStrings(components, delim, &result);
    return result;
}


// ----------------------------------------------------------------------
//  String2Long()
//  String2Double()
//  Convert String to Long , double val. a think wrapper of
//  strtol, strtod, more robust than simple atoi, atof, since
//  it can handle more error conditions
//  RETURN true if convert ok. when return false, caller should
//  not use value.
bool String2Long(const std::string& str, long& val, int base = 10);

bool String2Double(const std::string& str, double& val);
}//namespace comm 
}//namespace yanetlib

#endif //strutil.h
