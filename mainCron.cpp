/*Instruction: 
* This file is a demon file (is working in background) 
* You dont need to execute the script if server goes down 
* Compile this file inside docker without problem
* You MUST restart container to save 
*/ 

/*Build process: 
* 1. Compile: g++ -O2 -o mainCron mainCron.cpp 
* 2. Rebuild: docker-compose build app   
* 3. Restart: docker-compose up -d 
* 4. Check: use TOP like monitor or ps aux | grep mainCron
*/

#include <iostream> 
#include <ctime> 
#include <chrono> 
#include <thread> 
#include <map> 
#include <vector> 
#include <cstdlib>
#include <string>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <mutex>
#include <filesystem>

enum class CronFrequency {
    DAILY,          // Every day
    WEEKLY,         // Every week (specify day)
    MONTHLY,        // Every month (specify day of month)
    YEARLY,         // Every year (specify month and day)
    WEEKDAY,        // Weekdays only (Mon-Fri)
    WEEKEND         // Weekends only (Sat-Sun)
};

enum class LogLevel {
    DEBUG,
    INFO,
    WARNING,
    ERROR,
    SUCCESS
};

struct CronJob {
    int hour;
    int minute;
    CronFrequency frequency;
    int day_param;      
    int month_param;    
    std::string command;
    std::string description;
};

class Logger {
private:
    std::string log_file;
    std::ofstream log_stream;
    std::mutex log_mutex;  // Aggiunto per thread safety
    
    std::string get_timestamp() {
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()) % 1000;
        
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
        ss << "." << std::setfill('0') << std::setw(3) << ms.count();
        return ss.str();
    }
    
    std::string get_level_string(LogLevel level) {
        switch(level) {
            case LogLevel::DEBUG:   return "DEBUG";
            case LogLevel::INFO:    return "INFO";
            case LogLevel::WARNING: return "WARN";
            case LogLevel::ERROR:   return "ERROR";
            case LogLevel::SUCCESS: return "SUCCESS";
            default: return "UNKNOWN";
        }
    }
    
public:
    Logger(const std::string& filename = "logs/cron.log") : log_file(filename) {
        // Crea directory logs usando filesystem (C++17)
        try {
            std::filesystem::create_directories("logs");
        } catch (const std::exception& e) {
            std::cerr << "WARNING: Cannot create logs directory: " << e.what() << std::endl;
        }
        
        log_stream.open(log_file, std::ios::app);
        
        if (!log_stream.is_open()) {
            std::cerr << "FATAL: Cannot open log file: " << log_file << std::endl;
        }
    }
    
    ~Logger() {
        std::lock_guard<std::mutex> lock(log_mutex);
        if (log_stream.is_open()) {
            log_stream.close();
        }
    }
    
    void log(LogLevel level, const std::string& message, const std::string& job_name = "") {
        std::lock_guard<std::mutex> lock(log_mutex);  // Thread safety
        
        std::string timestamp = get_timestamp();
        std::string level_str = get_level_string(level);
        
        std::string log_entry = "[" + timestamp + "] [" + level_str + "]";
        if (!job_name.empty()) {
            log_entry += " [" + job_name + "]";
        }
        log_entry += " " + message;
        
        // Scrivi su file
        if (log_stream.is_open()) {
            log_stream << log_entry << std::endl;
            log_stream.flush();
        }
        
        // Scrivi anche su console
        std::cout << log_entry << std::endl;
    }
    
    void debug(const std::string& message, const std::string& job_name = "") {
        log(LogLevel::DEBUG, message, job_name);
    }
    
    void info(const std::string& message, const std::string& job_name = "") {
        log(LogLevel::INFO, message, job_name);
    }
    
    void warning(const std::string& message, const std::string& job_name = "") {
        log(LogLevel::WARNING, message, job_name);
    }
    
    void error(const std::string& message, const std::string& job_name = "") {
        log(LogLevel::ERROR, message, job_name);
    }
    
    void success(const std::string& message, const std::string& job_name = "") {
        log(LogLevel::SUCCESS, message, job_name);
    }
    
    // Rotazione log migliorata
    void rotate_logs() {
        std::lock_guard<std::mutex> lock(log_mutex);
        
        if (log_stream.is_open()) {
            log_stream.close();
        }
        
        // Usa un timestamp più preciso per evitare collisioni
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        auto local = std::localtime(&time_t);
        
        std::stringstream archive_name;
        archive_name << "logs/cron_" 
                    << (local->tm_year + 1900) << "-"
                    << std::setfill('0') << std::setw(2) << (local->tm_mon + 1) << "-"
                    << std::setfill('0') << std::setw(2) << local->tm_mday << ".log";
        
        // Usa filesystem invece di system()
        try {
            std::filesystem::rename(log_file, archive_name.str());
        } catch (const std::exception& e) {
            std::cerr << "Error rotating log: " << e.what() << std::endl;
        }
        
        // Riapri il file principale
        log_stream.open(log_file, std::ios::app);
        if (log_stream.is_open()) {
            info("Log rotated. Archive: " + archive_name.str());
        }
    }
};

// Istanza globale del logger
Logger logger;

// Function to get weekday name
std::string get_weekday_name(int wday) {
    const std::string days[] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
    if (wday < 0 || wday > 6) return "Unknown";  // Safety check
    return days[wday];
}

// Function to check if a job should run
bool should_run_job(const CronJob& job, const std::tm& local, 
                    const std::map<std::string, std::pair<int, int>>& last_exec) {
    
    // Check time
    if (job.hour != local.tm_hour || job.minute != local.tm_min) {
        return false;
    }
    
    // Check if already executed in this minute
    auto it = last_exec.find(job.command);
    if (it != last_exec.end() && 
        it->second.first == local.tm_hour && 
        it->second.second == local.tm_min) {
        logger.debug("Job already executed this minute", job.description);
        return false;
    }
    
    // Check frequency
    switch (job.frequency) {
        case CronFrequency::DAILY:
            return true;
            
        case CronFrequency::WEEKLY:
            return local.tm_wday == job.day_param;
            
        case CronFrequency::MONTHLY:
            return local.tm_mday == job.day_param;
            
        case CronFrequency::YEARLY:
            return local.tm_mday == job.day_param && 
                   (local.tm_mon + 1) == job.month_param;
            
        case CronFrequency::WEEKDAY:
            return local.tm_wday >= 1 && local.tm_wday <= 5; // Mon-Fri
            
        case CronFrequency::WEEKEND:
            return local.tm_wday == 0 || local.tm_wday == 6; // Sun-Sat
            
        default:
            return false;
    }
}

// Function to print job schedule
void print_job_schedule(const CronJob& job) {
    std::stringstream ss;
    ss << "Job: " << job.command << " (" << job.description << ")\n";
    ss << "  Time: " << job.hour << ":" << (job.minute < 10 ? "0" : "") << job.minute << "\n";
    
    switch (job.frequency) {
        case CronFrequency::DAILY:
            ss << "  Frequency: Every day";
            break;
        case CronFrequency::WEEKLY:
            ss << "  Frequency: Every " << get_weekday_name(job.day_param);
            break;
        case CronFrequency::MONTHLY:
            ss << "  Frequency: Day " << job.day_param << " of every month";
            break;
        case CronFrequency::YEARLY:
            ss << "  Frequency: " << job.day_param << "/" << job.month_param << " every year";
            break;
        case CronFrequency::WEEKDAY:
            ss << "  Frequency: Weekdays only (Mon-Fri)";
            break;
        case CronFrequency::WEEKEND:
            ss << "  Frequency: Weekends only (Sat-Sun)";
            break;
    }
    
    logger.info(ss.str());
}

// Funzione migliorata per eseguire job con timeout
int execute_job_with_timeout(const std::string& command, int timeout_seconds = 300) {
    auto start_time = std::chrono::steady_clock::now();
    
    // Costruisci comando con path assoluto se necessario
    std::string full_command = command;
    if (command.find("./") == 0) {
        try {
            auto current_path = std::filesystem::current_path();
            full_command = current_path.string() + "/" + command.substr(2);
        } catch (const std::exception& e) {
            logger.warning("Could not resolve absolute path for: " + command);
        }
    }
    
    // Prova prima con timeout, poi fallback a comando diretto
    std::string timed_command = "timeout " + std::to_string(timeout_seconds) + " " + full_command;
    int result = std::system(timed_command.c_str());
    
    // Se timeout non è disponibile (exit code 127), prova comando diretto
    if (result == 127 * 256) {  // system() returns exit_code * 256
        logger.warning("timeout command not available, executing without timeout");
        result = std::system(full_command.c_str());
    }
    
    auto end_time = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::seconds>(end_time - start_time);
    
    // Gestisci diversi codici di uscita
    if (result == 124 * 256) { // timeout exit code
        logger.error("Job timed out after " + std::to_string(timeout_seconds) + " seconds", full_command);
    } else if (result == -1) {
        logger.error("Failed to execute command", full_command);
    } else {
        logger.debug("Job execution time: " + std::to_string(duration.count()) + " seconds", full_command);
    }
    
    return result;
}

int main() {
    logger.info("=== ADVANCED MINI CRON STARTED ===");
    
    std::vector<CronJob> jobs = {
        //INSERT HERE A NEW JOB!!
        /*HH:MM   -   Frequency   - DAY:MONTH        - Binary URL       - TEXT DESCRIPTION  */
        {23, 0, CronFrequency::DAILY, 0, 0, "./Jobs/closeSessionJob", "Daily session cleanup"},
        {5, 0, CronFrequency::MONTHLY, 1, 0, "./Jobs/makeAttendanceJob", "Monthly xml generation"},
        {1, 0, CronFrequency::MONTHLY, 1, 0, "./Jobs/makeReportJob", "Monthly pdf generation"},
    };
    
    std::map<std::string, std::pair<int, int>> last_execution;
    
    // Print job configuration
    logger.info("Configured jobs:");
    for (const auto& job : jobs) {
        print_job_schedule(job);
    }
    logger.info("===================================");
    
    // Variabili per controlli periodici
    int last_rotation_day = -1;
    int last_debug_hour = -1;
    
    while (true) {
        std::time_t now = std::time(nullptr);
        
        // Usa localtime_r se disponibile per thread safety, altrimenti copia il risultato
        std::tm local_time;
        #ifdef _WIN32
            localtime_s(&local_time, &now);
        #else
            localtime_r(&now, &local_time);
        #endif
        
        // Rotazione log giornaliera (controllo più flessibile)
        if (local_time.tm_mday != last_rotation_day && 
            local_time.tm_hour == 0 && local_time.tm_min == 0) {
            logger.rotate_logs();
            last_rotation_day = local_time.tm_mday;
        }
        
        // Debug info ogni 4 ore invece che ogni ora (meno verboso)
        if (local_time.tm_hour != last_debug_hour && local_time.tm_hour % 4 == 0) {
            std::stringstream ss;
            ss << "Current time: " << local_time.tm_hour << ":" 
               << (local_time.tm_min < 10 ? "0" : "") << local_time.tm_min
               << " - " << get_weekday_name(local_time.tm_wday) 
               << " " << local_time.tm_mday << "/" << (local_time.tm_mon + 1) 
               << "/" << (local_time.tm_year + 1900) << " - System running normally";
            logger.debug(ss.str());
            last_debug_hour = local_time.tm_hour;
        }
        
        // Controlla ed esegui job
        for (const auto& job : jobs) {
            if (should_run_job(job, local_time, last_execution)) {
                logger.info("Starting job: " + job.command, job.description);
                
                auto start_time = std::chrono::steady_clock::now();
                int result = execute_job_with_timeout(job.command, 300); // 5 minuti timeout
                auto end_time = std::chrono::steady_clock::now();
                
                auto duration = std::chrono::duration_cast<std::chrono::seconds>(end_time - start_time);
                
                if (result == 0) {
                    logger.success("Job completed successfully in " + std::to_string(duration.count()) + " seconds", job.description);
                } else if (result == 124 * 256) {
                    logger.error("Job timed out after 300 seconds", job.description);
                } else {
                    logger.error("Job failed with exit code " + std::to_string(result / 256) + 
                               " after " + std::to_string(duration.count()) + " seconds", job.description);
                }
                
                last_execution[job.command] = {local_time.tm_hour, local_time.tm_min};
            }
        }
        
        std::this_thread::sleep_for(std::chrono::seconds(20));
    }
    
    return 0;
}