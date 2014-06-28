#ifndef YANETLIB_COMM_BASE64_H
#define YANETLIB_COMM_BASE64_H

//This file implement basic base64 encoding.
//base64 encoding: encode 3 binary bytes -> 4 ascii bytes.
//when we processing, we consider 3 bytes as one group.
//one or two '=' may append to the end if nessary.
//Thread Safe

namespace yanetlib {
namespace comm {

class Base64Encoder {
 public:
     enum {
         E_BADPARAM = -1,
         E_BUFSHORT = -2,
     };

     //Constructor
     Base64Encoder(int line_length = 72);
     
     ~Base64Encoder();

     //Encode input buffer by base64 encoding. result is put in
     //outputbuf. olength is [in|out] parameter. input: *olength
     //specify length of output buffer. base64's encoded result
     //is 4ceil(ilength/3). roughly 135% longer than input.
     //if length of outputbuff can not hold encoded result, 
     //Encode fail. when return, olength store the length of 
     //encoded result.
     //RETURN: 0: OK. <0: FAIL 
     int Encode(const void* input, int ilength, char* output, int* olength);

     void SetLineLength(int line_length);
 private:
     //disable copy
     Base64Encoder(const Base64Encoder&);
     void operator=(const Base64Encoder&);

     friend class Base64Decoder;
     //keep track of currently processing bytes by group
     unsigned char group_[3];
     //currently poroceesing group length.
     int group_length_;
     //keep track how many character per line
     int pos_;
     //max line length
     int line_length_;
     //binary->ascii mapping
     static const unsigned char OUT_ENCODING[64];
};

class Base64Decoder {
 public:
     enum {
         E_BADPARAM = -1,
         E_BUFSHORT = -2,
         E_FORMAT   = -3,
     };

     Base64Decoder();

     ~Base64Decoder();

     //Decode ascii encoed base64 to binary string.
     //RETURN: 0 OK, < 0 fail
     int Decode(const char* input, int ilen, char* output, int* olen);

 private:
     int ReadOneChar(const char* input, int ilen, int* pi);

     //diable copy
     Base64Decoder(const Base64Decoder&);
     void operator=(const Base64Decoder&);

     unsigned char group_[3];

     //ascii->binary mapping
     static unsigned char IN_ENCODING[256];
     static bool IS_INIT;
};

} //namespace comm
} //namespace yanetlib
#endif //base64.h
