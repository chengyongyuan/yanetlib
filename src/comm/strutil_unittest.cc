#include <vector>
#include <string>
#include "strutil.h"
#include "gtest/gtest.h"

using namespace std;

using namespace ::yanetlib::comm;

//Unittest for strutil
class StrUtilTest : public testing::Test {
 protected:
     
     StrUtilTest() {
         //upper/lower
         upper_str1 = "this is lower";
         lower_str1 = "THIS IS UPPER";
         upper_str2 =  "####uppe#r";
         lower_str2 = "###LOWER##";
         //prefix/suffix
         prefix_str1 = "COLIN_DEBUG:this is a prefix test";
         prefix_str2 = "  COLIN_DEBUG:this is a prefix test";
         suffix_str1 = "COLIN_DEBUG:this is a suffix END";
         suffix_str2 = "COLIN_DEBUG:this is a suffix END ";

         //strip
         strip_str1 = "2014-06-20:colincheng:male:::IT engineer;WIN-WIN";
         strip_str2 = "this: is : only : test";
         strip_str3 = "      strip left";
         strip_str4 = "strip right";
         strip_str5 = "###strip left";
         strip_str6 = "strip right####";

         //replace
         repl_str1 = "this is no TAG in this TAG, with that TAG";
         repl_str2 = "this is no TAG in this TAG, with that this ";

         //split
         split_str1 = "this is   a test";
         split_str2 = " this :: is:a:test ";

         //file name test
         //  /home/file.log : Basename() == file.log; PathName() == /home
         //  /file.log      : Basename() == file.log; PathName() == "/"
         //  file.log       : Basename() == file.log; PathName() == "."
         //  dir/file.log   : Basename() == file.log; PathName() == "./dir"
         //  /onlydir/      : Basename() == "";       PathName() == "/onlydir"
         file_str1 = "/home/file.log";
         file_str2 = "/file.log";
         file_str3 = "file.log";
         file_str4 = "dir/file.log";
         file_str5 = "/onlydir/";

     }

     //setup
     virtual void SetUp() {
     }

     virtual void TearDown() {
     }

     //upper/lower
     string upper_str1;
     string lower_str1;
     string upper_str2;
     string lower_str2;

     //For Prefix/Suffix functions
     string prefix_str1;
     string prefix_str2;
     string suffix_str1;
     string suffix_str2;

     //For Strip functions
     string strip_str1;
     string strip_str2;
     string strip_str3;
     string strip_str4;
     string strip_str5;
     string strip_str6;

     //For Repalce functions
     string repl_str1;
     string repl_str2;

     //For Join/Split functions
     string split_str1;
     string split_str2;

     string file_str1;
     string file_str2;
     string file_str3;
     string file_str4;
     string file_str5;
};

TEST_F(StrUtilTest, StrPrefixTest) {
    EXPECT_TRUE(HasPrefixString(prefix_str1, "COLIN_DEBUG"));
    EXPECT_FALSE(HasPrefixString(prefix_str2, "COLIN_DEBUG"));
    EXPECT_TRUE(HasPrefixString(prefix_str2, "  COLIN_DEBUG"));
    EXPECT_TRUE(HasSuffixString(suffix_str1, "END"));
    EXPECT_FALSE(HasSuffixString(suffix_str2, "END"));
    EXPECT_TRUE(HasSuffixString(suffix_str2, "END "));
    
    EXPECT_EQ(StripPrefixString(prefix_str1, "COLIN_DEBUG"), ":this is a prefix test");
    EXPECT_EQ(StripPrefixString(prefix_str2, "  "), "COLIN_DEBUG:this is a prefix test");
    EXPECT_EQ(StripSuffixString(suffix_str1, "END"), "COLIN_DEBUG:this is a suffix ");
    EXPECT_EQ(StripSuffixString(suffix_str2, " END "), "COLIN_DEBUG:this is a suffix");
}

TEST_F(StrUtilTest, StrStripTest) {
    string old_strip1 = strip_str1;
    StripString(&strip_str1, "-", '|');
    EXPECT_STREQ("2014|06|20:colincheng:male:::IT engineer;WIN|WIN", strip_str1.c_str());
    strip_str1 = old_strip1;
    StripString(&strip_str1, "-", '/');
    EXPECT_STREQ("2014/06/20:colincheng:male:::IT engineer;WIN/WIN", strip_str1.c_str());
    strip_str1 = old_strip1;
    StripString(&strip_str1, "-:;", '|');
    EXPECT_STREQ("2014|06|20|colincheng|male|||IT engineer|WIN|WIN", strip_str1.c_str());
    EXPECT_STREQ("thisisonlytest", StripString(strip_str2, ": ").c_str());
    EXPECT_STREQ("stripleft", StripString(strip_str3, " ").c_str());
    EXPECT_STREQ("stripright", StripString(strip_str4, " ").c_str());
    EXPECT_STREQ("strip left", StripStringLeft(strip_str3, " ").c_str());
    EXPECT_STREQ("strip right", StripStringRight(strip_str4, " ").c_str());
    EXPECT_STREQ("strip left", StripStringLeft(strip_str5, "#").c_str());
    EXPECT_STREQ("strip right", StripStringRight(strip_str6, "#").c_str());
}

TEST_F(StrUtilTest, StrReplaceTest) {
    EXPECT_EQ("this is no tag in this tag, with that tag", 
              StringReplace(repl_str1, "TAG", "tag", true));
    EXPECT_EQ("this is no tag in this TAG, with that TAG", 
              StringReplace(repl_str1, "TAG", "tag", false));
    EXPECT_EQ("# is no TAG in # TAG, with that # ", 
              StringReplace(repl_str2, "this", "#", true));
    EXPECT_EQ("# is no TAG in this TAG, with that this ", 
              StringReplace(repl_str2, "this", "#", false));

}

TEST_F(StrUtilTest, SplitTest) {
    vector<string> vs;
    SplitString(split_str1, " ", &vs);
    EXPECT_EQ(4U, vs.size());
    EXPECT_EQ("this", vs[0]);
    EXPECT_EQ("is",   vs[1]);
    EXPECT_EQ("a",    vs[2]);
    EXPECT_EQ("test", vs[3]);

    vs.clear();
    SplitString(split_str2, ": ", &vs);
    EXPECT_EQ(4U, vs.size());
    EXPECT_EQ("this", vs[0]);
    EXPECT_EQ("is",   vs[1]);
    EXPECT_EQ("a",    vs[2]); 
    EXPECT_EQ("test", vs[3]);

    vs.clear();
    SplitString(split_str2, ": ", &vs, 4);
    EXPECT_EQ(4U, vs.size());
    EXPECT_EQ("this", vs[0]);
    EXPECT_EQ("is",   vs[1]);
    EXPECT_EQ("a",    vs[2]); 
    EXPECT_EQ("test", vs[3]);

    vs.clear();
    SplitString(split_str2, ": ", &vs, 0);
    EXPECT_EQ(4U, vs.size());
    EXPECT_EQ("this", vs[0]);
    EXPECT_EQ("is",   vs[1]);
    EXPECT_EQ("a",    vs[2]); 
    EXPECT_EQ("test", vs[3]);

    vs.clear();
    SplitString(split_str2, ":", &vs, 2);
    EXPECT_EQ(3U, vs.size());
    EXPECT_EQ(" this ", vs[0]);
    EXPECT_EQ(" is",   vs[1]);
    EXPECT_EQ("a:test ",    vs[2]); 

    vs.clear();
    SplitString(split_str2, ":", &vs, 0);
    EXPECT_EQ(4U, vs.size());
    EXPECT_EQ(" this ", vs[0]);
    EXPECT_EQ(" is",   vs[1]);
    EXPECT_EQ("a",    vs[2]); 
    EXPECT_EQ("test ",    vs[3]); 

    vs.clear();
    SplitString(split_str2, ": ", &vs, 1);
    EXPECT_EQ(2U, vs.size());
    EXPECT_EQ("this", vs[0]);
    EXPECT_EQ("is:a:test ", vs[1]);

    vs.clear();
    SplitString(split_str2, ": ", &vs, 2);
    EXPECT_EQ(3U, vs.size());
    EXPECT_EQ("this", vs[0]);
    EXPECT_EQ("is", vs[1]);
    EXPECT_EQ("a:test ", vs[2]);

    string result;
    JoinStrings(vs, ":", &result);
    EXPECT_EQ("this:is:a:test ", result);
    result.clear();
    JoinStrings(vs, " ", &result);
    EXPECT_EQ("this is a:test ", result);
    
}

TEST_F(StrUtilTest, StrConvertTest) {
    string str1 = "2014";
    string str2 = "2015this";
    string str3 = "xxx2015";
    string str4 = "abcde";
    string str5 = "0xFF";
    string str6 = " 3.14";
    string str7 = " 3.1415126xxxx";
    string str8 = " xxxx3.1";
    long val = 0;
    EXPECT_TRUE(String2Long(str1, val));
    EXPECT_EQ(2014, val);
    EXPECT_TRUE(String2Long(str2, val));
    EXPECT_EQ(2015, val);
    EXPECT_FALSE(String2Long(str3, val));
    EXPECT_FALSE(String2Long(str4, val));
    EXPECT_TRUE(String2Long(str5, val, 16));
    EXPECT_EQ(255, val);
    str5 = "  0xffff";
    EXPECT_TRUE(String2Long(str5, val, 16));
    EXPECT_EQ(65535, val);

    double dval = 0.0;
    EXPECT_TRUE(String2Double(str6, dval));
    EXPECT_TRUE((dval-3.14 < 0.0001)); 
    EXPECT_TRUE(String2Double(str7, dval));
    EXPECT_TRUE((dval-3.1415126 < 0.0001)); 
    EXPECT_FALSE(String2Double(str8, dval));
}

TEST_F(StrUtilTest, StrUpperLower) {
    UpperString(&upper_str1);
    UpperString(&upper_str1);
    LowerString(&lower_str1);
    UpperString(&upper_str2);
    LowerString(&lower_str2);

    EXPECT_EQ("THIS IS LOWER", upper_str1);
    EXPECT_EQ("THIS IS LOWER", upper_str1);
    EXPECT_EQ("this is upper", lower_str1);
    EXPECT_EQ("####UPPE#R", upper_str2);
    EXPECT_EQ("###lower##", lower_str2);
}

//file name test
//  /home/file.log : Basename() == file.log; PathName() == /home
//  /file.log      : Basename() == file.log; PathName() == "/"
//  file.log       : Basename() == file.log; PathName() == "."
//  dir/file.log   : Basename() == file.log; PathName() == "dir"
//  /onlydir/      : Basename() == "";       PathName() == "/onlydir"
TEST_F(StrUtilTest, FileNameTest) {
    EXPECT_EQ("file.log", BaseName(file_str1));
    EXPECT_EQ("/home", PathName(file_str1));
    EXPECT_EQ("file.log", BaseName(file_str2));
    EXPECT_EQ("/", PathName(file_str2));
    EXPECT_EQ("file.log", BaseName(file_str3));
    EXPECT_EQ(".", PathName(file_str3));
    EXPECT_EQ("file.log", BaseName(file_str4));
    EXPECT_EQ("./dir", PathName(file_str4));
    EXPECT_EQ("", BaseName(file_str5));
    EXPECT_EQ("/onlydir", PathName(file_str5));

    EXPECT_EQ(BaseName("/home/file.log"), "file.log");
    EXPECT_EQ(BaseName("/file.log"), "file.log");
    EXPECT_EQ(BaseName("file.log"), "file.log");
    EXPECT_EQ(BaseName("dir/file.log"), "file.log");

    EXPECT_EQ(PathName("/home/file.log"), "/home");
    EXPECT_EQ(PathName("/file.log"), "/");
    EXPECT_EQ(PathName("file.log"), ".");
    EXPECT_EQ(PathName("dir/file.log"), "./dir");

    string filename = "test.cc";
    string filename2 = "test";
    string filename3 = "/home/oicq/icq_finger.c";
    EXPECT_EQ("cc", FileExt(filename));
    EXPECT_EQ("", FileExt(filename2));
    EXPECT_EQ("c", FileExt(filename3));
    EXPECT_EQ(FileExt("/data/test.log"), "log");
    EXPECT_EQ(FileExt("test.a.b.log"), "log");
    EXPECT_EQ(FileExt("log"), "");
    EXPECT_EQ(FileExt(".log"), "log");
    EXPECT_EQ(FileExt("."), "");
    EXPECT_EQ(FileExt(""), "");
}
