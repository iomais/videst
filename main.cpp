#include <iostream>
#include <cstdlib>
#include <string>
#include <fstream>
#include <thread>
#include <cstdio>
#include <memory>
#include <array>

using namespace std;

/* ========= ألوان ========= */
#define RESET   "\033[0m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define CYAN    "\033[36m"
#define BOLD    "\033[1m"

class Videoder {
private:
    string homePath;
    string downloadPath;
    string logFile;

public:
    Videoder() {
        const char* home = getenv("HOME");
        if (!home) {
            cout << RED << "Error detecting HOME\n" << RESET;
            exit(1);
        }
        homePath = string(home);
        downloadPath = homePath + "/Videos/YouTube_Downloads/";
        logFile = downloadPath + "log.txt";
        createFolder();
    }

    void createFolder() {
        string cmd = "mkdir -p \"" + downloadPath + "\"";
        system(cmd.c_str());
    }

    void logDownload(const string& url) {
        ofstream log(logFile, ios::app);
        log << "Downloaded: " << url << endl;
    }

    void showProgressBar(float percent) {
        int width = 50;
        cout << "[";
        int pos = width * percent / 100.0;
        for (int i = 0; i < width; ++i) {
            if (i < pos) cout << "█";
            else cout << " ";
        }
        cout << "] " << percent << "%\r";
        cout.flush();
    }

    void executeDownload(const string& command, const string& url) {

        array<char, 256> buffer;
        FILE* pipe = popen(command.c_str(), "r");

        if (!pipe) {
            cout << RED << "Failed to start download\n" << RESET;
            return;
        }

        while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
            string line = buffer.data();

            // تحليل نسبة التحميل
            size_t percentPos = line.find('%');
            if (percentPos != string::npos) {
                size_t start = line.rfind(' ', percentPos);
                if (start != string::npos) {
                    string percentStr = line.substr(start, percentPos - start);
                    try {
                        float percent = stof(percentStr);
                        showProgressBar(percent);
                    } catch (...) {}
                }
            }
        }

        pclose(pipe);

        cout << endl << GREEN << "Download completed!\n" << RESET;
        logDownload(url);
    }

    void downloadBest(const string& url) {
        string cmd = "yt-dlp -f best --newline -o \"" +
                     downloadPath + "%(title)s.%(ext)s\" \"" + url + "\" 2>&1";
        thread t(&Videoder::executeDownload, this, cmd, url);
        t.join();
    }

    void downloadMP3(const string& url) {
        string cmd = "yt-dlp -x --audio-format mp3 --newline -o \"" +
                     downloadPath + "%(title)s.%(ext)s\" \"" + url + "\" 2>&1";
        thread t(&Videoder::executeDownload, this, cmd, url);
        t.join();
    }

    void downloadPlaylist(const string& url) {
        string cmd = "yt-dlp --yes-playlist --newline -o \"" +
                     downloadPath + "%(playlist_title)s/%(title)s.%(ext)s\" \"" + url + "\" 2>&1";
        thread t(&Videoder::executeDownload, this, cmd, url);
        t.join();
    }

    void downloadCustom(const string& url, const string& quality) {
        string cmd = "yt-dlp -f \"bestvideo[height<=" + quality +
                     "]+bestaudio/best\" --newline -o \"" +
                     downloadPath + "%(title)s.%(ext)s\" \"" + url + "\" 2>&1";
        thread t(&Videoder::executeDownload, this, cmd, url);
        t.join();
    }

    void interactiveMenu() {

        while (true) {

            cout << BOLD CYAN
                 << "\n========= 🎬 Videoder =========\n"
                 << RESET;

            cout << YELLOW
                 << "1) Best Quality\n"
                 << "2) MP3\n"
                 << "3) Playlist\n"
                 << "4) Custom Quality\n"
                 << "5) Exit\n"
                 << RESET;

            cout << GREEN << "Choose: " << RESET;

            int choice;
            cin >> choice;

            if (cin.fail()) {
                cin.clear();
                cin.ignore(1000, '\n');
                cout << RED << "Invalid input!\n" << RESET;
                continue;
            }

            if (choice == 5)
                break;

            string url;
            cout << CYAN << "Enter URL: " << RESET;
            cin >> url;

            if (url.find("http") != 0) {
                cout << RED << "Invalid URL!\n" << RESET;
                continue;
            }

            if (choice == 1)
                downloadBest(url);
            else if (choice == 2)
                downloadMP3(url);
            else if (choice == 3)
                downloadPlaylist(url);
            else if (choice == 4) {
                string quality;
                cout << YELLOW << "Enter quality (e.g. 720): " << RESET;
                cin >> quality;
                downloadCustom(url, quality);
            }
        }
    }
};

/* ========= MAIN ========= */

int main(int argc, char* argv[]) {

    Videoder app;

    if (argc == 1) {
        app.interactiveMenu();
        return 0;
    }

    string option = argv[1];

    if (argc < 3) {
        cout << RED << "Missing URL!\n" << RESET;
        return 1;
    }

    string url = argv[2];

    if (option == "-best")
        app.downloadBest(url);
    else if (option == "-mp3")
        app.downloadMP3(url);
    else if (option == "-playlist")
        app.downloadPlaylist(url);
    else if (option[0] == '-' && option.length() > 1 && isdigit(option[1]))
        app.downloadCustom(url, option.substr(1));
    else {
        cout << RED << "Unknown option!\n" << RESET;
        return 1;
    }

    return 0;
}