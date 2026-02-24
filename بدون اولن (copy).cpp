#include <iostream>
#include <string>
#include <fstream>
#include <filesystem>
#include <limits>
#include <cstdlib>

using namespace std;
namespace fs = std::filesystem;

#define RESET "\033[0m"
#define RED "\033[31m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define CYAN "\033[36m"

const string VERSION = "2.0.0";
const string CONFIG_FILE = "videoder.conf";

struct Config {
    string downloadPath = ".";
    bool warnings = true;
};

Config loadConfig() {
    Config config;
    ifstream file(CONFIG_FILE);
    if (file.is_open()) {
        getline(file, config.downloadPath);
        file >> config.warnings;
        file.close();
    }
    return config;
}

void saveConfig(const Config& config) {
    ofstream file(CONFIG_FILE);
    file << config.downloadPath << endl;
    file << config.warnings << endl;
    file.close();
}

void downloadVideo(const string& format, const string& url, const Config& config) {
    string command = "yt-dlp -P \"" + config.downloadPath + "\" ";

    if (format == "best")
        command += "-f best ";
    else if (format == "mp3")
        command += "-x --audio-format mp3 ";
    else if (format == "720")
        command += "-f \"bestvideo[height<=720]+bestaudio\" ";

    command += "\"" + url + "\"";

    system(command.c_str());
}

void settingsMenu(Config& config) {
    while (true) {
        cout << CYAN << "\n=== Settings ===\n" << RESET;
        cout << "1. Change Download Path\n";
        cout << "2. Toggle Warnings\n";
        cout << "3. Back\n";
        cout << "Choice: ";

        int choice;
        cin >> choice;

        if (cin.fail()) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << RED << "Invalid input! Numbers only.\n" << RESET;
            continue;
        }

        if (choice < 1 || choice > 3) {
            cout << RED << "Invalid choice! Select 1-3 only.\n" << RESET;
            continue;
        }

        if (choice == 1) {
            cin.ignore(numeric_limits<streamsize>::max(), '\n');

            while (true) {
                cout << YELLOW << "Enter new download path: " << RESET;
                string newPath;
                getline(cin, newPath);

                if (newPath.empty()) {
                    cout << RED << "Path cannot be empty!\n" << RESET;
                    continue;
                }

                if (!fs::exists(newPath) || !fs::is_directory(newPath)) {
                    cout << RED << "Invalid folder path!\n" << RESET;
                    continue;
                }

                config.downloadPath = newPath;
                saveConfig(config);
                cout << GREEN << "Download path updated successfully!\n" << RESET;
                break;
            }
        }

        else if (choice == 2) {
            config.warnings = !config.warnings;
            saveConfig(config);
            cout << GREEN << "Warnings "
                 << (config.warnings ? "Enabled" : "Disabled")
                 << "!\n" << RESET;
        }

        else if (choice == 3) {
            break;
        }
    }
}

void helpMenu() {
    cout << CYAN << "\n=== Videoder CLI Help ===\n" << RESET;
    cout << "videoder -best URL\n";
    cout << "videoder -mp3 URL\n";
    cout << "videoder -720 URL\n";
    cout << "videoder -playlist URL\n";
    cout << "videoder --update\n";
    cout << "videoder --help\n\n";

    cout << "1. Update yt-dlp\n";
    cout << "2. Self Update\n";
    cout << "3. Back\n";
    cout << "Choice: ";

    int choice;
    cin >> choice;

    if (choice == 1) {
        system("yt-dlp -U");
    }
    else if (choice == 2) {
        cout << YELLOW << "Self updating...\n" << RESET;
        system("git pull");
    }
}

void menuMode(Config& config) {
    while (true) {
        cout << CYAN << "\n=== Videoder CLI v" << VERSION << " ===\n" << RESET;
        cout << "1. Download Best\n";
        cout << "2. Download MP3\n";
        cout << "3. Download 720p\n";
        cout << "4. Playlist\n";
        cout << "5. Settings\n";
        cout << "6. Help\n";
        cout << "7. Exit\n";
        cout << "Choice: ";

        int choice;
        cin >> choice;

        if (cin.fail()) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << RED << "Invalid input! Numbers only.\n" << RESET;
            continue;
        }

        if (choice < 1 || choice > 7) {
            cout << RED << "Invalid choice! Select 1-7 only.\n" << RESET;
            continue;
        }

        if (choice == 7)
            break;

        if (choice == 5) {
            settingsMenu(config);
            continue;
        }

        if (choice == 6) {
            helpMenu();
            continue;
        }

        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Enter URL: ";
        string url;
        getline(cin, url);

        if (url.empty()) {
            cout << RED << "URL cannot be empty!\n" << RESET;
            continue;
        }

        if (choice == 1)
            downloadVideo("best", url, config);
        else if (choice == 2)
            downloadVideo("mp3", url, config);
        else if (choice == 3)
            downloadVideo("720", url, config);
        else if (choice == 4)
            system(("yt-dlp \"" + url + "\"").c_str());
    }
}

int main(int argc, char* argv[]) {

    Config config = loadConfig();

    if (argc > 1) {
        string arg = argv[1];

        if (arg == "--help") {
            helpMenu();
        }
        else if (arg == "--update") {
            system("git pull");
        }
        else if (arg == "-best" && argc > 2) {
            downloadVideo("best", argv[2], config);
        }
        else if (arg == "-mp3" && argc > 2) {
            downloadVideo("mp3", argv[2], config);
        }
        else if (arg == "-720" && argc > 2) {
            downloadVideo("720", argv[2], config);
        }
        else {
            cout << RED << "Invalid CLI usage.\n" << RESET;
        }
    }
    else {
        menuMode(config);
    }

    return 0;
}