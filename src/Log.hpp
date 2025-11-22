#pragma once

#include <iomanip>
#include <iostream>
#include <string>
#include <format>
#include <unordered_map>

namespace Color {
constexpr const char* RED    = "\033[31m";
constexpr const char* GREEN  = "\033[32m";
constexpr const char* YELLOW = "\033[33m";
constexpr const char* BLUE   = "\033[34m";
constexpr const char* RESET  = "\033[0m";
}

const std::unordered_map<size_t, const char*> statementTypeString = {
    {StatementType::ERROR, "ERROR"},
    {StatementType::WARNING, "WARNING"},
    {StatementType::INFO, "INFO"},
    {StatementType::LOG, "LOG"},
};

enum StatementType {
        ERROR,
        WARNING,
        INFO,
        LOG
};

template <typename... Args>
void my_print(const StatementType type, std::format_string<Args...> fmt,
              Args&&... args) {
        // Determine color of this message
        const char* color = nullptr;
        switch (type) {
        case ERROR:
                color = Color::RED;
                break;
        
        case WARNING:
                color = Color::YELLOW;
                break;
        
        case INFO:
                color = Color::BLUE;
                break;
        
        case LOG:
                color = Color::RESET;
                break;
        
        default:
                color = Color::RESET;
                break;
        }

        std::clog << color << std::left << std::setw(10)
                  << statementTypeString[type]
                  << std::format(fmt, std::forward<Args>(args)...)
                  << Color::RESET << std::endl
                  << std::right;
}

#define REX_LOG_V();

void REX_LOG(const std::string& message);
void REX_ERROR(const std::string& message);
void REX_PRINT(const std::string& message);
