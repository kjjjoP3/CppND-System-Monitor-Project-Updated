#include <unistd.h>
#include <cctype>
#include <sstream>
#include <string>
#include <vector>

#include "process.h"
#include "linux_parser.h"

using std::string;
using std::to_string;
using std::vector;

// Return this process's ID
int Process::Pid() { return pid_; }

// Return this process's CPU utilization
float Process::CpuUtilization() const {
    // Tính toán thời gian làm việc và tổng thời gian CPU bằng cách sử dụng jiffies
    vector<string> jiffies_data = LinuxParser::CpuUtilization();
    float total_jiffies = 0;
    float work_jiffies = 0;

    // Tổng hợp thời gian sử dụng jiffies cho toàn bộ CPU và cho các tác vụ chính
    for (size_t i = 0; i < jiffies_data.size(); i++) {
        float jiffies = stof(jiffies_data[i]);
        total_jiffies += jiffies;
        if (i < 3) {  // User, Nice, System jiffies
            work_jiffies += jiffies;
        }
    }

    // Tính toán phần trăm sử dụng CPU dựa trên thời gian làm việc và tổng thời gian
    float cpu_percentage = (work_jiffies / total_jiffies) * 100;
    float seconds = work_jiffies / sysconf(_SC_CLK_TCK);
    return seconds;
}

// Return the command that generated this process
string Process::Command() { 
    return LinuxParser::Command(pid_);
}

// Return this process's memory utilization
string Process::Ram() { return LinuxParser::Ram(pid_); }

// Return the user (name) that generated this process
string Process::User() {
    string user_id = LinuxParser::Uid(pid_);
    return LinuxParser::User(user_id); 
}

// Return the age of this process (in seconds)
long int Process::UpTime() {
    std::ifstream stream("/proc/uptime");
    long int uptime = 0;

    if (stream.is_open()) {
        stream >> uptime;
    }

    return uptime; // Trả về tổng thời gian hệ thống đã chạy
}

// Overload the "less than" comparison operator for Process objects
bool Process::operator<(Process const& a[[maybe_unused]]) const { return true; }
