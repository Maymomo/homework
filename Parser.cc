#include "Parser.h"
#include <iostream>

Parser::Parser() {
    Reset();
}

void Parser::Reset() {
    status = ParseStatus::HEAD;
    bodyLen = 0;
}

int Parser::HeaderLen() const {
    return HEADERLEN;
}

int Parser::BodyLen() const {
    return bodyLen;
}

bool Parser::Parse(std::vector<char> &buffer, int len) {
    if (status == ParseStatus::HEAD && len >= HeaderLen()) {
        bodyLen = *(int*)(buffer.data());
        status = ParseStatus::BODY;
        return false;
    }
    if (status == ParseStatus::BODY && len == (BodyLen() + HeaderLen())) {
        status = ParseStatus::OVER;
        return true;
    }
    return false;
}