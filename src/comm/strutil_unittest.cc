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

         //replace
         repl_str1 = "this is no TAG in this TAG, with that TAG";
         repl_str2 = "this is no TAG in this TAG, with that this ";

         //split
         split_str1 = "this is   a test";
         split_str2 = " this :: is:a:test ";
     }

     //setup
     virtual void SetUp() {
     }

     virtual void TearDown() {
     }

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

     //For Repalce functions
     string repl_str1;
     string repl_str2;

     //For Join/Split functions
     string split_str1;
     string split_str2;
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

    string result;
    JoinStrings(vs, ":", &result);
    EXPECT_EQ("this:is:a:test", result);
    result.clear();
    JoinStrings(vs, " ", &result);
    EXPECT_EQ("this is a test", result);
    
}
