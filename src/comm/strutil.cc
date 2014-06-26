#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include <assert.h>
#include <iterator>
#include <comm/strutil.h>

namespace yanetlib {
namespace comm {

using std::string;
using std::vector;
using std::back_insert_iterator;

void StripString(string* s, const char *remove,
                 char replacewith) {
    const char *str_start = s->c_str();
    const char *str = str_start;
    for (str = strpbrk(str, remove);
         str != NULL;
         str = strpbrk(str + 1, remove)) {
        (*s)[str-str_start] = replacewith;
    }
}

string StripString(const string& s, const char* remove) {
    string result;
    result.reserve(s.size());
    string::size_type begin_index = s.find_first_not_of(remove);
    string::size_type end_index;
    while (begin_index != string::npos) {
        end_index = s.find_first_of(remove, begin_index);
        if (end_index == string::npos) {
            result.append(s.substr(begin_index));
            break;
        }
        result.append(s.substr(begin_index, end_index - begin_index));
        begin_index = s.find_first_not_of(remove, end_index);
    }
    return result;
}

string StripStringLeft(const string& s, const char* remove) {
    string::size_type begin_index = s.find_first_not_of(remove);
    if (begin_index == string::npos) {//remove all
        return "";
    }
    return s.substr(begin_index);
}

string StripStringRight(const string& s, const char* remove) {
    string::size_type begin_index = s.find_last_not_of(remove);
    if (begin_index == string::npos) {//remove all
        return "";
    }
    return s.substr(0, begin_index+1);
}

// ----------------------------------------------------------------------
//  StringReplace()
//  UpperString()
//   Give a string and two string pattern "old" and "new". Repalce first
//   occure of "old" with "new" if EXIST. if "replace_all" set then 
//   replace every occurence of.
//   put result string in 'res'
// ----------------------------------------------------------------------
void StringReplace(const string& s, const string& oldsub,
                   const string& newsub, bool replace_all,
                   string* res) {
    if (oldsub.empty()) {
        res->append(s); // if empty, append the string
        return;
    }
    string::size_type start_pos = 0;
    string::size_type pos;
    do {
        pos = s.find(oldsub, start_pos);
        if (pos == string::npos) {
            break;
        }
        res->append(s, start_pos, pos - start_pos);
        res->append(newsub);
        start_pos = pos + oldsub.size();
    } while (replace_all);
    res->append(s, start_pos, s.length() - start_pos);
}

string StringReplace(const string& s, const string& oldsub,
                     const string& newsub, bool repalce_all) {
    string result;
    StringReplace(s, oldsub, newsub, repalce_all, &result);
    return result;
}

// ----------------------------------------------------------------------
//  SplitString()
//   Split a string using a character delimiter. Append the components
//   to 'result'.
//   If delimiter is multi-character. then split on *ANY* of them, not
//   the entire stirng as a single string. you can split at most count
//   times, if count==0, we mean split all :-
// ----------------------------------------------------------------------
template <typename ITER>
static inline void SplitStringByIterator(const string& full,
                                    const char* delim,
                                    ITER& result, 
                                    int count = 0) {
    int split_cnt = 0;
    //Optimize for single delim characters.
    if (delim[0] != '\0' && delim[1] == '\0') {
        char c = delim[0];
        const char* p = full.data();
        const char* end = full.data() + full.size();
        while(p != end) {
            if (*p == c) {
                ++p;
            } else {
                const char* start = p;
                while(++p != end && *p != c) ;
                *result++ = string(start, p -start);
                //split only once
                if (++split_cnt == count) {
                    while(p++ != end && *p == c); //spip consecutive delim
                    if (p != end) *result++ = string(p, end-p);
                    return ;
                }
            }
        }
        return;
    }
    string::size_type begin_index = full.find_first_not_of(delim);
    string::size_type end_index;
    while (begin_index != string::npos) {
        end_index = full.find_first_of(delim, begin_index);
        if (end_index == string::npos) {
            *result++ = full.substr(begin_index);
            return;
        }
        *result++ = full.substr(begin_index, end_index - begin_index);
        begin_index = full.find_first_not_of(delim, end_index);
        if (++split_cnt == count) {
            if (begin_index != string::npos) 
                *result++ = full.substr(begin_index);
            return;
        }
    }
}

void SplitString(const string& full, const char* delim,
                 vector<string>* result, int count) {
    back_insert_iterator< vector<string> > it(*result);
    SplitStringByIterator(full, delim, it, count);
}

// ----------------------------------------------------------------------
//  JoinStrings()
//   Join string in vector into a single string use delimiter.We define
//   two version of this function. One return the string. one take a 
//   pointer to target string
//  JoinStringsIterator is the innter interface.
// ----------------------------------------------------------------------
template <typename ITER>
void JoinStringsIterator(const ITER& begin,
                         const ITER& end,
                         const char* delim,
                         string* result) {
    assert(result != NULL);
    result->clear();
    int delim_length = strlen(delim);

    //precomputer finall length. so that we can reserved memory once.
    int length = 0;
    for (ITER it = begin; it != end; ++it) {
        if (it != begin) {
            length += delim_length;
        }
        length += it->size();
    }
    result->reserve(length);

    for (ITER it = begin; it != end; ++it) {
        if (it != begin) {
            result->append(delim, delim_length);
        }
        result->append(it->data(), it->size());
    }
}

void JoinStrings(const vector<string>& components,
                 const char* delim,
                 string* result) {
    JoinStringsIterator(components.begin(), components.end(), delim, result);
}

//see man strtol
bool String2Long(const std::string& str, long& val, int base) {
    char *endptr;
    errno = 0;

    long longval = strtol(str.c_str(), &endptr, base);

    if (str.empty()) return false;
    //overflow or underflow
    if ((errno == ERANGE || (longval == LONG_MAX || longval == LONG_MIN))
        || (errno != 0 && longval == 0))
        return false;
    //no digit
    if (endptr == str.c_str()) return false;
    val = longval;

    return true;
}

bool String2Double(const std::string& str, double& val) {
    char *endptr;
    errno = 0;

    double dval = strtod(str.c_str(), &endptr);

    if (str.empty()) return false;
    //overflow or underflow
    if (errno == ERANGE || (errno != 0 && dval == 0))
        return false;
    //no digit
    if (endptr == str.c_str()) return false;
    val = dval;

    return true;
}

} //namespace comm
} //namspace yanetlib
