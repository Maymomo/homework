#pragma once

#include <vector>

enum class ParseStatus {
    HEAD,
    BODY,
    OVER,
};

#define HEADERLEN  4;


class Parser {
public:
    Parser();

    bool Parse(std::vector<char> &buffer, int len);

    void Reset();

    int HeaderLen() const;

    int BodyLen() const;
private:
    ParseStatus status;
    int bodyLen;
};


