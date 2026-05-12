#include <iostream>
#include <vector>
#include <thread>
#include <string>
#include <chrono>
#include <mutex>
#include <algorithm>
#include <iomanip>
#include <atomic>
#include <numeric>
#include <fstream>
#include <map>
#include <windows.h>
#include <wininet.h>

#pragma comment(lib, "wininet.lib")


enum class Lang { TR, EN };

struct UI_Strings {
    std::string title;
    std::string prompt_url;
    std::string prompt_total;
    std::string prompt_concurrency;
    std::string progress;
    std::string report_header;
    std::string target_url;
    std::string total_req;
    std::string duration;
    std::string rps;
    std::string latency_min;
    std::string latency_avg;
    std::string latency_max;
    std::string exit_msg;
};

struct RequestResult {
    long responseTime;
    DWORD statusCode;
    bool success;
};

class StressTester {
private:
    std::wstring url;
    int totalRequests;
    int initialConcurrency;
    std::vector<RequestResult> results;
    std::mutex resultMutex;
    std::atomic<int> completedCount{ 0 };
    Lang selectedLang;
    UI_Strings ui;

    void setLanguage(Lang l) {
        if (l == Lang::TR) {
            ui = { "--- PROFESYONEL SISTEM ANALIZ ARACI ---", "URL: ", "Toplam Istek Sayisi: ", "Baslangic Eszamanlilik: ",
                   "Ilerleme", "ANALIZ RAPORU", "Hedef URL", "Toplam Istek", "Sure", "Istek/Saniye",
                   "Min Gecikme", "Ort. Gecikme", "Max Gecikme", "Cikmak icin bir tusa basin..." };
        }
        else {
            ui = { "--- PROFESSIONAL SYSTEM ANALYSIS TOOL ---", "URL: ", "Total Request Count: ", "Initial Concurrency: ",
                   "Progress", "ANALYSIS REPORT", "Target URL", "Total Request", "Duration", "Requests/Sec",
                   "Min Latency", "Avg Latency", "Max Latency", "Press any key to exit..." };
        }
    }

    void performRequest() {
        auto start = std::chrono::high_resolution_clock::now();
        RequestResult res = { 0, 0, false };

        HINTERNET hInternet = InternetOpenW(L"ProAnalysis/3.0", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
        if (hInternet) {
            DWORD timeout = 5000;
            InternetSetOptionW(hInternet, INTERNET_OPTION_CONNECT_TIMEOUT, &timeout, sizeof(timeout));

            HINTERNET hConnect = InternetOpenUrlW(hInternet, url.c_str(), NULL, 0, INTERNET_FLAG_RELOAD | INTERNET_FLAG_NO_CACHE_WRITE, 0);
            if (hConnect) {
                DWORD statusCode = 0, length = sizeof(statusCode);
                if (HttpQueryInfoW(hConnect, HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER, &statusCode, &length, NULL)) {
                    res.statusCode = statusCode;
                    res.success = (statusCode >= 200 && statusCode < 400);
                }
                InternetCloseHandle(hConnect);
            }
            else {
                res.statusCode = GetLastError();
            }
            InternetCloseHandle(hInternet);
        }

        auto end = std::chrono::high_resolution_clock::now();
        res.responseTime = static_cast<long>(std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count());

        std::lock_guard<std::mutex> lock(resultMutex);
        results.push_back(res);
        completedCount++;
    }

public:
    StressTester(std::string targetUrl, int total, int concurrency, Lang l)
        : totalRequests(total), initialConcurrency(concurrency), selectedLang(l) {
        url = std::wstring(targetUrl.begin(), targetUrl.end());
        setLanguage(l);
    }

    void run() {
        auto startTime = std::chrono::high_resolution_clock::now();
        int currentConcurrency = initialConcurrency;

        for (int i = 0; i < totalRequests; i += currentConcurrency) {
            std::vector<std::thread> workers;
            int batchSize = (i + currentConcurrency > totalRequests) ? (totalRequests - i) : currentConcurrency;

            for (int j = 0; j < batchSize; ++j) {
                workers.emplace_back(&StressTester::performRequest, this);
            }

            for (auto& t : workers) {
                if (t.joinable()) t.join();
            }

            currentConcurrency = static_cast<int>(currentConcurrency * 1.1) + 1;
            printProgress();
        }

        auto endTime = std::chrono::high_resolution_clock::now();
        displayReport(std::chrono::duration_cast<std::chrono::duration<double>>(endTime - startTime).count());
    }

    void printProgress() {
        float progress = (completedCount.load() * 100.0f) / totalRequests;
        std::cout << "\r[" << ui.progress << "] %" << std::fixed << std::setprecision(1) << progress << " [" << std::string(progress / 5, '#') << std::string(20 - (progress / 5), '.') << "] " << std::flush;
    }

    void displayReport(double totalDuration) {
        std::map<DWORD, int> stats;
        std::vector<long> times;

        for (const auto& r : results) {
            stats[r.statusCode]++;
            if (r.success) times.push_back(r.responseTime);
        }

        std::sort(times.begin(), times.end());
        double avg = times.empty() ? 0 : std::accumulate(times.begin(), times.end(), 0.0) / times.size();

        std::cout << "\n\n" << std::string(45, '=') << "\n";
        std::cout << "  " << ui.report_header << "\n";
        std::cout << std::string(45, '=') << "\n";
        std::cout << std::left << std::setw(20) << ui.target_url << ": " << std::string(url.begin(), url.end()) << "\n";
        std::cout << std::left << std::setw(20) << ui.total_req << ": " << totalRequests << "\n";
        std::cout << std::left << std::setw(20) << ui.duration << ": " << std::fixed << std::setprecision(2) << totalDuration << " s\n";
        std::cout << std::left << std::setw(20) << ui.rps << ": " << totalRequests / totalDuration << "\n";
        std::cout << std::string(45, '-') << "\n";

        for (auto const& [code, count] : stats) {
            std::cout << "Status " << code << " : " << count << "\n";
        }

        if (!times.empty()) {
            std::cout << std::string(45, '-') << "\n";
            std::cout << std::left << std::setw(20) << ui.latency_min << ": " << times.front() << " ms\n";
            std::cout << std::left << std::setw(20) << ui.latency_avg << ": " << avg << " ms\n";
            std::cout << std::left << std::setw(20) << ui.latency_max << ": " << times.back() << " ms\n";
        }
        std::cout << std::string(45, '=') << "\n";

        saveToFile(avg, totalRequests / totalDuration);
    }

    void saveToFile(double avg, double rps) {
        std::ofstream report("final_report.txt");
        report << ui.report_header << "\nAverage MS: " << avg << "\nRPS: " << rps;
        report.close();
    }
};

int main() {
    int langChoice;
    std::cout << "Select Language / Dil Secin (1: EN, 2: TR): ";
    std::cin >> langChoice;
    Lang selected = (langChoice == 2) ? Lang::TR : Lang::EN;

    UI_Strings tempUI;
    if (selected == Lang::TR) tempUI.title = "--- PROFESYONEL SISTEM ANALIZ ARACI ---";
    else tempUI.title = "--- PROFESSIONAL SYSTEM ANALYSIS TOOL ---";

    std::string url;
    int total, concurrency;

    std::cout << "\n" << tempUI.title << "\n";
    std::cout << (selected == Lang::TR ? "URL: " : "URL: "); std::cin >> url;
    std::cout << (selected == Lang::TR ? "Toplam Istek: " : "Total Requests: "); std::cin >> total;
    std::cout << (selected == Lang::TR ? "Baslangic Eszamanlilik: " : "Initial Concurrency: "); std::cin >> concurrency;

    StressTester tester(url, total, concurrency, selected);
    tester.run();

    std::cout << "\n" << (selected == Lang::TR ? "Cikmak icin bir tusa basin..." : "Press any key to exit...");
    std::cin.ignore(); std::cin.get();
    return 0;
}