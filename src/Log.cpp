#include "Log.hpp"

#include <iostream>
#include <iomanip>
#include <string>

namespace Color {
    constexpr const char* RED = "\033[31m";
    constexpr const char* GREEN = "\033[32m";
    constexpr const char* YELLOW = "\033[33m";
    constexpr const char* BLUE = "\033[34m";
    constexpr const char* RESET = "\033[0m";
}

void REX_LOG(const std::string& message) {
    std::clog << Color::GREEN 
        << std::left << std::setw(10) << "LOG:"
        << message 
        << Color::RESET << std::endl << std::right;
}

void REX_ERROR(const std::string& message) {
    std::cerr << Color::RED
        << std::left << std::setw(10) << "ERROR:"
        << message
        << Color::RESET << std::endl << std::right;
}

void REX_PRINT(const std::string& message) {
    std::cerr
        << std::left << std::setw(10) << "INFO:"
        << message
        << std::endl << std::right;
}