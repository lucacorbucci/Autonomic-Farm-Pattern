/*
    author: Luca Corbucci
    student number: 516450
*/

#pragma once
#ifndef PARSER_HPP
#define PARSER_HPP

#include <unistd.h>
#include <iostream>
#include <utility>

struct NonValidValue : public std::exception {
    const char* what() const throw() {
        return "NonValidValue";
    }
};

bool parser(std::string inputString) {
    bool result;

    if (inputString == "true") {
        result = true;
    } else if (inputString == "false") {
        result = false;
    } else {
        throw NonValidValue();
    }
    return result;
}

#endif
