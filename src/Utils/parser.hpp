#pragma once
#ifndef PARSER_HPP
#define PARSER_HPP

#include <unistd.h>
#include <iostream>
#include <utility>

bool parser(std::string inputString) {
    bool result = inputString == "true" ? true : false;
    return result;
}

#endif
