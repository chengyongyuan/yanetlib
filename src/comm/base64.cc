#include "common.h"
#include "base64.h"

namespace yanetlib {
namespace comm {

namespace {
    static Mutex mutex;
}

const unsigned char Base64Encoder::OUT_ENCODING[64] = {
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
    'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
    'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
    'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
    'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
    'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
    'w', 'x', 'y', 'z', '0', '1', '2', '3', 
    '4', '5', '6', '7', '8', '9', '+', '/'
};

Base64Encoder::Base64Encoder(int line_length) : 
    group_length_(0), pos_(0), line_length_(line_length) { }

Base64Encoder::~Base64Encoder() { }

void Base64Encoder::SetLineLength(int line_length) {
    line_length_ = line_length;
}

int Base64Encoder::Encode(const void* input, int ilen,
                          char* output, int* outlen) {
    const unsigned char* pinput = (unsigned char*) input;
    char *poutput = output;

    if (input == NULL || ilen <= 0 ||
        output == NULL || outlen == NULL)
        return E_BADPARAM;

    int max_outlen = 4*(ilen+2)/3;
    int newline_cnt = line_length_ ? max_outlen/line_length_ : 0;
    newline_cnt *= 2;

    //output may contains ('\r\n'), so this is a rough calculation.
    if (*outlen < max_outlen+newline_cnt)
        return E_BUFSHORT;

    for (int i = 0; i < ilen; ++i) {
        group_[group_length_++] = pinput[i];
        if (group_length_ == 3) {
            unsigned char idx;
            idx = group_[0] >> 2;
            *poutput++ = OUT_ENCODING[idx];
            idx = ((group_[0] & 0x03) << 4) | (group_[1] >> 4);
            *poutput++ = OUT_ENCODING[idx];
            idx = ((group_[1] & 0x0F) << 2) | (group_[2] >> 6);
            *poutput++ = OUT_ENCODING[idx];
            idx = group_[2] & 0x3F;
            *poutput++ = OUT_ENCODING[idx];
            pos_ += 4;
            if (pos_ > 0 && pos_ >= line_length_) {
                *poutput++ = '\r';
                *poutput++ = '\n';
                pos_ = 0;
            }
            group_length_ = 0;
        }
    }
    if (group_length_ == 1) {
        group_[1] = 0;
        unsigned char idx = group_[0] >> 2;
        *poutput++ = OUT_ENCODING[idx];
        idx = ((group_[0] & 0x03) << 4) | (group_[1] >> 4);
        *poutput++ = OUT_ENCODING[idx];
        *poutput++ = '=';
        *poutput++ = '=';
    } else if (group_length_ == 2) {
        group_[2] = 0;
        unsigned char idx = group_[0] >> 2;
        *poutput++ = OUT_ENCODING[idx];
        idx = ((group_[0] & 0x03) << 4) | (group_[1] >> 4);
        *poutput++ = OUT_ENCODING[idx];
        idx = ((group_[1] & 0x0F) << 2) | (group_[2] >> 6);
        *poutput++ = OUT_ENCODING[idx];
        *poutput++ = '=';
    }
    group_length_ = 0;
    pos_ = 0;
    *outlen = static_cast<int>(poutput-output);

    return 0;
}

Base64Decoder::Base64Decoder() { 
    ScopedLock<Mutex> lock(&mutex);
    if (!IS_INIT) {
        for(unsigned i = 0; i < sizeof(IN_ENCODING); ++i) {
            IN_ENCODING[i] = 0xFF;
        }
        for(unsigned i = 0; i < sizeof(Base64Encoder::OUT_ENCODING); ++i) {
            IN_ENCODING[Base64Encoder::OUT_ENCODING[i]] = i;
        }
        IN_ENCODING[static_cast<unsigned char>('=')] = '\0';
        IS_INIT = true;
    }
}

Base64Decoder::~Base64Decoder() { }

unsigned char Base64Decoder::IN_ENCODING[256];
bool Base64Decoder::IS_INIT = false;

int Base64Decoder::ReadOneChar(const char* input, int ilen, int* pi) {

    const char* ptr = input;
    int i = *pi;

    for ( ; i < ilen; ++i) {
        if (ptr[i] == ' ' || ptr[i] == '\t' ||
            ptr[i] == '\r' || ptr[i] == '\n')
            continue;
        break;
    }
    if (i == ilen) return -1;
    //skip to next
    *pi = i+1;
    return ptr[i];
    
}

int Base64Decoder::Decode(const char* input, int ilen,
                          char* output, int* outlen) {
    char *pout = output;
    unsigned char buffer[4];
    int buf_cnt = 0;
    int write_len = 0;

    if(input == NULL || ilen <= 0 ||
       output == NULL || outlen == NULL)
        return E_BADPARAM;
    //input may contains ('\r\n'), so this is a rough calculation.
    if (*outlen < 3*(ilen+3)/4)
        return E_BUFSHORT;

    for (int i = 0; i < ilen; ) {
        int c;
        if ((c = ReadOneChar(input, ilen, &i)) == -1) break;
        buffer[buf_cnt] = (unsigned char)c;
        if (IN_ENCODING[buffer[buf_cnt++]] == 0xFF) return E_FORMAT;
        if ((c = ReadOneChar(input, ilen, &i)) == -1) break;
        buffer[buf_cnt] = (unsigned char)c;
        if (IN_ENCODING[buffer[buf_cnt++]] == 0xFF) return E_FORMAT;
        if ((c = ReadOneChar(input, ilen, &i)) == -1) break;
        buffer[buf_cnt] = (unsigned char)c;
        if (IN_ENCODING[buffer[buf_cnt++]] == 0xFF) return E_FORMAT;
        if ((c = ReadOneChar(input, ilen, &i)) == -1) break;
        buffer[buf_cnt] = (unsigned char)c;
        if (IN_ENCODING[buffer[buf_cnt++]] == 0xFF) return E_FORMAT;
        buf_cnt = 0;

        group_[0] = (IN_ENCODING[buffer[0]] << 2) | (IN_ENCODING[buffer[1]] >> 4);
        group_[1] = ((IN_ENCODING[buffer[1]] & 0x0F) << 4) | (IN_ENCODING[buffer[2]] >> 2);
        group_[2] = (IN_ENCODING[buffer[2]] << 6) | IN_ENCODING[buffer[3]];

        if (buffer[2] == '=')
            write_len = 1;
        else if (buffer[3] == '=')
            write_len = 2;
        else 
            write_len = 3;
        for (int j = 0; j < write_len; ++j)
            *pout++ = group_[j];
        write_len = 0;
    }
    if (buf_cnt != 0) return E_FORMAT;
    *outlen = static_cast<int>(pout-output);

    return 0;
}

} //namespace comm
} //namespace yanetlib
