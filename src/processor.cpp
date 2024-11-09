#include "processor.h"
#include "linux_parser.h"
#include <vector>
#include <string>

using std::vector;
using std::string;

// Return the aggregate CPU utilization
float Processor::Utilization() {
    // Lấy dữ liệu jiffies cho CPU từ LinuxParser
    vector<string> jiffies = LinuxParser::CpuUtilization();
    float total = 0.0; // Tổng số thời gian CPU tính bằng jiffies
    float workers = 0.0; // Thời gian dành cho các tác vụ chính (User, Nice, System)

    // Tính tổng thời gian CPU và thời gian làm việc cho các tác vụ chính
    for (size_t i = 0; i < jiffies.size(); i++) {
        float jiffy_value = stof(jiffies[i]);
        total += jiffy_value;  // Cộng dồn toàn bộ jiffies vào tổng thời gian
        if (i < 3) {           // Các jiffies dành cho User, Nice, System
            workers += jiffy_value;
        }
    }

    // Tính toán tỷ lệ phần trăm sử dụng CPU dựa trên thời gian làm việc và tổng thời gian
    float percentage = workers / total;

    return percentage;
}
