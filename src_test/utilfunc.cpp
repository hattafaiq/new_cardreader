#include "utilfunc.h"
#include <chrono>
#include <sstream>
#include <iomanip>

std::vector<std::string> splitByNewlineNoRanges(const std::string& str) {
    std::vector<std::string> lines;
    std::string current_line;
    for (size_t i = 0; i < str.size(); ++i) {
        if (str[i] == '\n') {
            lines.push_back(current_line);
            current_line.clear();
        } else if (str[i] == '\r') {
            lines.push_back(current_line);
            current_line.clear();
            // Check for the following '\n' in case of Windows-style newline
            if (i + 1 < str.length() && str[i + 1] == '\n') {
                i++; // Skip the '\n'
            }
        } else {
            current_line += str[i];
        }
    }
    // Add the last line if it's not empty
    if (!current_line.empty()) {
        lines.push_back(current_line);
    }
    return lines;
}

std::string  current_time(){
    auto now = std::chrono::system_clock::now();
    auto time_t_now = std::chrono::system_clock::to_time_t(now);

    std::stringstream ss;
    ss << std::put_time(std::localtime(&time_t_now), "%Y%m%d%H%M%S");
    return ss.str();
}
