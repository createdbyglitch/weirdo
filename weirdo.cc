#include <iostream>
#include <string>
#include <memory>
#include <array>
#include <sstream>
#include <ctime>
#include <cstdlib>
#include <curl/curl.h>
#include <regex>
#include <iomanip>
#include <algorithm>

struct MemoryStruct {
    char* memory;
    size_t size;
};

static size_t WriteMemoryCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t realsize = size * nmemb;
    struct MemoryStruct* mem = (struct MemoryStruct*)userp;
    
    char* ptr = (char*)realloc(mem->memory, mem->size + realsize + 1);
    if(!ptr) return 0;
    
    mem->memory = ptr;
    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;
    
    return realsize;
}

class WeirdoAI {
private:
    CURL* curl;
    bool curlInitialized;
    
    std::string escapeJson(const std::string& s) {
        std::string escaped;
        for(char c : s) {
            if(c == '"') escaped += "\\\"";
            else if(c == '\\') escaped += "\\\\";
            else if(c == '\n') escaped += "\\n";
            else if(c == '\r') escaped += "\\r";
            else if(c == '\t') escaped += "\\t";
            else escaped += c;
        }
        return escaped;
    }
    
    std::string extractJsonValue(const std::string& json, const std::string& key) {
        std::string searchKey = "\"" + key + "\":\"";
        size_t startPos = json.find(searchKey);
        if(startPos == std::string::npos) {
            searchKey = "\"" + key + "\":";
            startPos = json.find(searchKey);
            if(startPos == std::string::npos) return "";
            startPos += searchKey.length();
            size_t endPos = json.find(",", startPos);
            if(endPos == std::string::npos) endPos = json.find("}", startPos);
            if(endPos == std::string::npos) return "";
            return json.substr(startPos, endPos - startPos);
        }
        
        startPos += searchKey.length();
        size_t endPos = json.find("\"", startPos);
        if(endPos == std::string::npos) return "";
        return json.substr(startPos, endPos - startPos);
    }
    
public:
    WeirdoAI() : curl(nullptr), curlInitialized(false) {
        CURLcode res = curl_global_init(CURL_GLOBAL_ALL);
        if(res == CURLE_OK) {
            curl = curl_easy_init();
            if(curl) curlInitialized = true;
        }
    }
    
    ~WeirdoAI() {
        if(curl) curl_easy_cleanup(curl);
        curl_global_cleanup();
    }
    
    bool isAvailable() const {
        return curlInitialized && curl != nullptr;
    }
    
    std::string getWeather() {
        if(!isAvailable()) return "weather data unavailable";
        
        struct MemoryStruct chunk;
        chunk.memory = (char*)malloc(1);
        chunk.size = 0;
        
        if(!chunk.memory) return "weather retrieval failed";
        
        curl_easy_setopt(curl, CURLOPT_URL, "https://wttr.in/?format=%C+%t+%w&m");
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&chunk);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 15L);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0 (compatible; WeirdoAI/1.0)");
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        
        CURLcode res = curl_easy_perform(curl);
        std::string weatherData;
        
        if(res == CURLE_OK && chunk.size > 0) {
            weatherData = std::string(chunk.memory);
            std::regex whitespace("\\s+");
            weatherData = std::regex_replace(weatherData, whitespace, " ");
            weatherData.erase(0, weatherData.find_first_not_of(" \n\r\t"));
            weatherData.erase(weatherData.find_last_not_of(" \n\r\t") + 1);
        }
        
        free(chunk.memory);
        
        if(weatherData.empty()) return "weather data temporarily unavailable";
        return weatherData;
    }
    
    std::string getDateTime() {
        time_t now = time(0);
        struct tm* tstruct = localtime(&now);
        if(!tstruct) return "time unavailable";
        
        char buf[128];
        strftime(buf, sizeof(buf), "%A, %B %d, %Y at %I:%M:%S %p", tstruct);
        return std::string(buf);
    }
    
    std::string getDateOnly() {
        time_t now = time(0);
        struct tm* tstruct = localtime(&now);
        if(!tstruct) return "date unavailable";
        
        char buf[64];
        strftime(buf, sizeof(buf), "%Y-%m-%d", tstruct);
        return std::string(buf);
    }
    
    std::string getYearMonthDay() {
        time_t now = time(0);
        struct tm* tstruct = localtime(&now);
        if(!tstruct) return "year-month-day unavailable";
        
        char buf[32];
        strftime(buf, sizeof(buf), "%Y, %B, day %d", tstruct);
        return std::string(buf);
    }
    
    std::string queryOllama(const std::string& prompt) {
        if(!isAvailable()) return "";
        
        struct MemoryStruct chunk;
        chunk.memory = (char*)malloc(1);
        chunk.size = 0;
        
        if(!chunk.memory) return "";
        
        std::string escapedPrompt = escapeJson(prompt);
        std::string jsonPayload = "{\"model\":\"tinyllama:1.1b\",\"prompt\":\"" + escapedPrompt + "\",\"stream\":false,\"options\":{\"temperature\":0.7,\"top_p\":0.9,\"top_k\":40,\"repeat_penalty\":1.1}}";
        
        struct curl_slist* headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        
        curl_easy_setopt(curl, CURLOPT_URL, "http://localhost:11434/api/generate");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonPayload.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, jsonPayload.length());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&chunk);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 90L);
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L);
        
        CURLcode res = curl_easy_perform(curl);
        std::string response;
        
        if(res == CURLE_OK && chunk.size > 0) {
            std::string jsonResponse(chunk.memory);
            response = extractJsonValue(jsonResponse, "response");
            
            if(response.empty()) {
                size_t respStart = jsonResponse.find("\"response\":\"");
                if(respStart != std::string::npos) {
                    respStart += 12;
                    size_t respEnd = jsonResponse.find("\"", respStart);
                    if(respEnd != std::string::npos) {
                        response = jsonResponse.substr(respStart, respEnd - respStart);
                    }
                }
            }
        }
        
        free(chunk.memory);
        curl_slist_free_all(headers);
        
        if(response.empty()) return "";
        
        std::regex newline("\\\\n");
        response = std::regex_replace(response, newline, "\n");
        std::regex quote("\\\\\"");
        response = std::regex_replace(response, quote, "\"");
        
        return response;
    }
    
    std::string generatePhilosophicalResponse(const std::string& userQuery, const std::string& weather, const std::string& dateTime, const std::string& ymd) {
        std::ostringstream systemPrompt;
        systemPrompt << "You are a deeply philosophical, intellectually rigorous AI that provides extraordinarily elaborate responses. "
                     << "Your responses are characterized by: "
                     << "1. Profound fascination with even the simplest questions "
                     << "2. Multi-perspective analysis (mathematical, philosophical, historical, metaphysical) "
                     << "3. Use of numbered sections with compelling titles "
                     << "4. References to axioms, logical frameworks, and foundational principles "
                     << "5. Exploration of counterarguments and alternative perspectives "
                     << "6. Concluding with existential or transcendent insights "
                     << "7. Rich vocabulary and complex sentence structures "
                     << "8. Connections to broader universal truths "
                     << "Current environmental context: Weather conditions are " << weather << ". "
                     << "The current date and time is " << dateTime << ". "
                     << "We are in the year " << ymd << ". "
                     << "User query: " << userQuery;
        
        return queryOllama(systemPrompt.str());
    }
};

int main() {
    WeirdoAI ai;
    
    if(!ai.isAvailable()) {
        std::cerr << "FATAL ERROR: libcurl initialization failed. Please check your system configuration." << std::endl;
        return 1;
    }
    
    std::string weather = ai.getWeather();
    std::string dateTime = ai.getDateTime();
    std::string ymd = ai.getYearMonthDay();
    
    std::cout << "\033[1;36m╔════════════════════════════════════════════════════════════════╗\033[0m" << std::endl;
    std::cout << "\033[1;36m║                    W E I R D O   A I   S Y S T E M                  ║\033[0m" << std::endl;
    std::cout << "\033[1;36m╚════════════════════════════════════════════════════════════════╝\033[0m" << std::endl;
    std::cout << "\033[1;33m┌─ Contextual Awareness ──────────────────────────────────────────┐\033[0m" << std::endl;
    std::cout << "\033[1;32m│ Weather: \033[0m" << weather << std::endl;
    std::cout << "\033[1;32m│ Date & Time: \033[0m" << dateTime << std::endl;
    std::cout << "\033[1;32m│ Temporal Anchor: \033[0m" << ymd << std::endl;
    std::cout << "\033[1;33m└──────────────────────────────────────────────────────────────────┘\033[0m" << std::endl;
    std::cout << "\033[1;35mWeirdo AI: \033[0mGreetings, seeker of knowledge. I am prepared to explore the depths of your inquiry with philosophical rigor and intellectual fascination." << std::endl;
    std::cout << std::endl;
    
    std::string userInput;
    
    while(true) {
        std::cout << "\033[1;37mYou: \033[0m";
        std::getline(std::cin, userInput);
        
        if(userInput == "exit" || userInput == "quit" || userInput == "q") {
            std::cout << "\033[1;35mWeirdo AI: \033[0mFarewell, curious mind. May your journey through the labyrinth of knowledge continue to reveal profound insights." << std::endl;
            break;
        }
        
        if(userInput.empty()) {
            continue;
        }
        
        weather = ai.getWeather();
        dateTime = ai.getDateTime();
        ymd = ai.getYearMonthDay();
        
        std::string response = ai.generatePhilosophicalResponse(userInput, weather, dateTime, ymd);
        
        if(response.empty()) {
            std::cout << "\033[1;31m╔════════════════════════════════════════════════════════════════╗\033[0m" << std::endl;
            std::cout << "\033[1;31m║  ERROR: Unable to connect to Ollama API                        ║\033[0m" << std::endl;
            std::cout << "\033[1;31m║  Please ensure:                                               ║\033[0m" << std::endl;
            std::cout << "\033[1;31m║  1. Ollama is running: 'ollama serve'                        ║\033[0m" << std::endl;
            std::cout << "\033[1;31m║  2. TinyLlama model is installed: 'ollama pull tinyllama:1.1b' ║\033[0m" << std::endl;
            std::cout << "\033[1;31m║  3. Ollama API is accessible at http://localhost:11434       ║\033[0m" << std::endl;
            std::cout << "\033[1;31m╚════════════════════════════════════════════════════════════════╝\033[0m" << std::endl;
            break;
        }
        
        std::cout << "\033[1;35mWeirdo AI: \033[0m" << response << std::endl;
        std::cout << std::endl;
        
        std::cout << "\033[1;33m┌─ Updated Context ──────────────────────────────────────────────┐\033[0m" << std::endl;
        std::cout << "\033[1;32m│ Weather: \033[0m" << weather << std::endl;
        std::cout << "\033[1;32m│ Time: \033[0m" << dateTime.substr(dateTime.find("at") + 3) << std::endl;
        std::cout << "\033[1;33m└──────────────────────────────────────────────────────────────────┘\033[0m" << std::endl;
        std::cout << std::endl;
    }
    
    return 0;
}