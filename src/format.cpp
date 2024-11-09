#include <string>
#include "format.h"
#include <iomanip>
#include <sstream>

using std::string;

// Converts seconds into HH:MM:SS format
string Format::ElapsedTime(long seconds) {
    int hours = seconds / 3600;            // Total hours
    int minutes = (seconds % 3600) / 60;   // Remaining minutes
    int secs = seconds % 60;               // Remaining seconds

    std::ostringstream formattedTime;
    formattedTime << std::setw(2) << std::setfill('0') << hours << ":"
                  << std::setw(2) << std::setfill('0') << minutes << ":"
                  << std::setw(2) << std::setfill('0') << secs;

    return formattedTime.str();  // Returns formatted time as HH:MM:SS
}