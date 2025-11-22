#include "Log.hpp"

#include <iomanip>
#include <iostream>
#include <string>


void REX_LOG(const std::string& message) {
        std::clog << Color::GREEN << std::left << std::setw(10)
                  << "LOG:" << message << Color::RESET << std::endl
                  << std::right;
}

void REX_ERROR(const std::string& message) {
        std::cerr << Color::RED << std::left << std::setw(10)
                  << "ERROR:" << message << Color::RESET << std::endl
                  << std::right;
}

void REX_PRINT(const std::string& message) {
        std::cerr << std::left << std::setw(10) << "INFO:" << message
                  << std::endl
                  << std::right;
}