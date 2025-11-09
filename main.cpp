#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#include <fstream>
#include <cstring>
#include <algorithm>
#include <chrono>
#include <cstdio>
#include <ctime>
#include <limits>
#include <sys/stat.h>
#include <unistd.h>

namespace fs = std::filesystem;

class FileExplorer {
private:
    fs::path currentPath;

    void clearScreen() {
        std::cout << "\033[2J\033[1;1H";
    }

    void displayHeader() {
        std::cout << "\n╔════════════════════════════════════════════════════════════════╗\n";
        std::cout << "║           CONSOLE-BASED FILE EXPLORER APPLICATION              ║\n";
        std::cout << "╚════════════════════════════════════════════════════════════════╝\n";
        std::cout << "Current Directory: " << currentPath << "\n";
        std::cout << "================================================================\n\n";
    }

    void displayMenu() {
        std::cout << "\n┌─────────────────── MAIN MENU ───────────────────┐\n";
        std::cout << "│  1.  List Files and Directories                 │\n";
        std::cout << "│  2.  Change Directory                            │\n";
        std::cout << "│  3.  Create New File                             │\n";
        std::cout << "│  4.  Create New Directory                        │\n";
        std::cout << "│  5.  Copy File                                   │\n";
        std::cout << "│  6.  Move File                                   │\n";
        std::cout << "│  7.  Delete File                                 │\n";
        std::cout << "│  8.  Delete Directory                            │\n";
        std::cout << "│  9.  Search Files                                │\n";
        std::cout << "│  10. View File Permissions                       │\n";
        std::cout << "│  11. Change File Permissions                     │\n";
        std::cout << "│  12. View File Details                           │\n";
        std::cout << "│  0.  Exit                                        │\n";
        std::cout << "└─────────────────────────────────────────────────┘\n";
        std::cout << "\nEnter your choice: ";
    }

    std::string getPermissionString(fs::perms p) {
        std::string result;
        result += (fs::perms::none != (p & fs::perms::owner_read)) ? "r" : "-";
        result += (fs::perms::none != (p & fs::perms::owner_write)) ? "w" : "-";
        result += (fs::perms::none != (p & fs::perms::owner_exec)) ? "x" : "-";
        result += (fs::perms::none != (p & fs::perms::group_read)) ? "r" : "-";
        result += (fs::perms::none != (p & fs::perms::group_write)) ? "w" : "-";
        result += (fs::perms::none != (p & fs::perms::group_exec)) ? "x" : "-";
        result += (fs::perms::none != (p & fs::perms::others_read)) ? "r" : "-";
        result += (fs::perms::none != (p & fs::perms::others_write)) ? "w" : "-";
        result += (fs::perms::none != (p & fs::perms::others_exec)) ? "x" : "-";
        return result;
    }

    std::string formatFileSize(uintmax_t size) {
        const char* units[] = {"B", "KB", "MB", "GB", "TB"};
        int unitIndex = 0;
        double fileSize = static_cast<double>(size);
        
        while (fileSize >= 1024.0 && unitIndex < 4) {
            fileSize /= 1024.0;
            unitIndex++;
        }
        
        char buffer[50];
        snprintf(buffer, sizeof(buffer), "%.2f %s", fileSize, units[unitIndex]);
        return std::string(buffer);
    }

public:
    FileExplorer() {
        currentPath = fs::current_path();
    }

    void listFiles() {
        clearScreen();
        displayHeader();
        std::cout << "Listing contents of: " << currentPath << "\n\n";
        
        std::cout << "┌────────────────────────────────────────────────────────────────────────┐\n";
        std::cout << "│ Type │ Name                              │ Size         │ Permissions │\n";
        std::cout << "├────────────────────────────────────────────────────────────────────────┤\n";
        
        try {
            std::vector<fs::directory_entry> entries;
            for (const auto& entry : fs::directory_iterator(currentPath)) {
                entries.push_back(entry);
            }
            
            std::sort(entries.begin(), entries.end(), 
                [](const fs::directory_entry& a, const fs::directory_entry& b) {
                    if (a.is_directory() != b.is_directory())
                        return a.is_directory();
                    return a.path().filename() < b.path().filename();
                });
            
            for (const auto& entry : entries) {
                std::string type = entry.is_directory() ? "[DIR]" : "[FILE]";
                std::string name = entry.path().filename().string();
                std::string size = entry.is_directory() ? "---" : formatFileSize(entry.file_size());
                std::string perms = getPermissionString(entry.status().permissions());
                
                printf("│ %-4s │ %-33s │ %-12s │ %-11s │\n", 
                       type.c_str(), 
                       name.substr(0, 33).c_str(), 
                       size.c_str(), 
                       perms.c_str());
            }
            
            std::cout << "└────────────────────────────────────────────────────────────────────────┘\n";
        } catch (const fs::filesystem_error& e) {
            std::cout << "Error: " << e.what() << "\n";
        }
        
        std::cout << "\nPress Enter to continue...";
        std::cin.get();
    }

    void changeDirectory() {
        clearScreen();
        displayHeader();
        std::cout << "Change Directory\n";
        std::cout << "────────────────\n\n";
        std::cout << "Enter directory path (or '..' for parent, '~' for home): ";
        
        std::string newPath;
        std::getline(std::cin, newPath);
        
        try {
            if (newPath == "~") {
                currentPath = fs::path(getenv("HOME"));
            } else if (newPath == "..") {
                if (currentPath.has_parent_path()) {
                    currentPath = currentPath.parent_path();
                }
            } else {
                fs::path targetPath = newPath;
                if (!targetPath.is_absolute()) {
                    targetPath = currentPath / newPath;
                }
                
                if (fs::exists(targetPath) && fs::is_directory(targetPath)) {
                    currentPath = fs::canonical(targetPath);
                } else {
                    std::cout << "\nError: Directory does not exist!\n";
                }
            }
            std::cout << "\nCurrent directory: " << currentPath << "\n";
        } catch (const fs::filesystem_error& e) {
            std::cout << "\nError: " << e.what() << "\n";
        }
        
        std::cout << "\nPress Enter to continue...";
        std::cin.get();
    }

    void createFile() {
        clearScreen();
        displayHeader();
        std::cout << "Create New File\n";
        std::cout << "───────────────\n\n";
        std::cout << "Enter file name: ";
        
        std::string fileName;
        std::getline(std::cin, fileName);
        
        try {
            fs::path filePath = currentPath / fileName;
            
            if (fs::exists(filePath)) {
                std::cout << "\nError: File already exists!\n";
            } else {
                std::ofstream file(filePath);
                if (file.is_open()) {
                    file.close();
                    std::cout << "\nFile created successfully: " << filePath << "\n";
                } else {
                    std::cout << "\nError: Could not create file!\n";
                }
            }
        } catch (const std::exception& e) {
            std::cout << "\nError: " << e.what() << "\n";
        }
        
        std::cout << "\nPress Enter to continue...";
        std::cin.get();
    }

    void createDirectory() {
        clearScreen();
        displayHeader();
        std::cout << "Create New Directory\n";
        std::cout << "────────────────────\n\n";
        std::cout << "Enter directory name: ";
        
        std::string dirName;
        std::getline(std::cin, dirName);
        
        try {
            fs::path dirPath = currentPath / dirName;
            
            if (fs::exists(dirPath)) {
                std::cout << "\nError: Directory already exists!\n";
            } else {
                if (fs::create_directory(dirPath)) {
                    std::cout << "\nDirectory created successfully: " << dirPath << "\n";
                } else {
                    std::cout << "\nError: Could not create directory!\n";
                }
            }
        } catch (const fs::filesystem_error& e) {
            std::cout << "\nError: " << e.what() << "\n";
        }
        
        std::cout << "\nPress Enter to continue...";
        std::cin.get();
    }

    void copyFile() {
        clearScreen();
        displayHeader();
        std::cout << "Copy File\n";
        std::cout << "─────────\n\n";
        
        std::string source, destination;
        
        std::cout << "Enter source file path: ";
        std::getline(std::cin, source);
        
        std::cout << "Enter destination file path: ";
        std::getline(std::cin, destination);
        
        try {
            fs::path sourcePath = currentPath / source;
            fs::path destPath = currentPath / destination;
            
            if (!fs::exists(sourcePath)) {
                std::cout << "\nError: Source file does not exist!\n";
            } else if (fs::is_directory(sourcePath)) {
                std::cout << "\nError: Source is a directory. Use file path only.\n";
            } else {
                fs::copy_file(sourcePath, destPath, fs::copy_options::overwrite_existing);
                std::cout << "\nFile copied successfully!\n";
                std::cout << "From: " << sourcePath << "\n";
                std::cout << "To:   " << destPath << "\n";
            }
        } catch (const fs::filesystem_error& e) {
            std::cout << "\nError: " << e.what() << "\n";
        }
        
        std::cout << "\nPress Enter to continue...";
        std::cin.get();
    }

    void moveFile() {
        clearScreen();
        displayHeader();
        std::cout << "Move File\n";
        std::cout << "─────────\n\n";
        
        std::string source, destination;
        
        std::cout << "Enter source file path: ";
        std::getline(std::cin, source);
        
        std::cout << "Enter destination file path: ";
        std::getline(std::cin, destination);
        
        try {
            fs::path sourcePath = currentPath / source;
            fs::path destPath = currentPath / destination;
            
            if (!fs::exists(sourcePath)) {
                std::cout << "\nError: Source file does not exist!\n";
            } else {
                fs::rename(sourcePath, destPath);
                std::cout << "\nFile moved successfully!\n";
                std::cout << "From: " << sourcePath << "\n";
                std::cout << "To:   " << destPath << "\n";
            }
        } catch (const fs::filesystem_error& e) {
            std::cout << "\nError: " << e.what() << "\n";
        }
        
        std::cout << "\nPress Enter to continue...";
        std::cin.get();
    }

    void deleteFile() {
        clearScreen();
        displayHeader();
        std::cout << "Delete File\n";
        std::cout << "───────────\n\n";
        
        std::string fileName;
        
        std::cout << "Enter file name to delete: ";
        std::getline(std::cin, fileName);
        
        try {
            fs::path filePath = currentPath / fileName;
            
            if (!fs::exists(filePath)) {
                std::cout << "\nError: File does not exist!\n";
            } else if (fs::is_directory(filePath)) {
                std::cout << "\nError: This is a directory. Use 'Delete Directory' option.\n";
            } else {
                std::cout << "\nAre you sure you want to delete '" << fileName << "'? (y/n): ";
                char confirm;
                std::cin >> confirm;
                
                if (confirm == 'y' || confirm == 'Y') {
                    fs::remove(filePath);
                    std::cout << "\nFile deleted successfully!\n";
                } else {
                    std::cout << "\nDeletion cancelled.\n";
                }
            }
        } catch (const fs::filesystem_error& e) {
            std::cout << "\nError: " << e.what() << "\n";
        }
        
        std::cout << "\nPress Enter to continue...";
        std::cin.ignore();
        std::cin.get();
    }

    void deleteDirectory() {
        clearScreen();
        displayHeader();
        std::cout << "Delete Directory\n";
        std::cout << "────────────────\n\n";
        
        std::string dirName;
        
        std::cout << "Enter directory name to delete: ";
        std::getline(std::cin, dirName);
        
        try {
            fs::path dirPath = currentPath / dirName;
            
            if (!fs::exists(dirPath)) {
                std::cout << "\nError: Directory does not exist!\n";
            } else if (!fs::is_directory(dirPath)) {
                std::cout << "\nError: This is a file. Use 'Delete File' option.\n";
            } else {
                std::cout << "\nAre you sure you want to delete '" << dirName << "' and all its contents? (y/n): ";
                char confirm;
                std::cin >> confirm;
                
                if (confirm == 'y' || confirm == 'Y') {
                    fs::remove_all(dirPath);
                    std::cout << "\nDirectory deleted successfully!\n";
                } else {
                    std::cout << "\nDeletion cancelled.\n";
                }
            }
        } catch (const fs::filesystem_error& e) {
            std::cout << "\nError: " << e.what() << "\n";
        }
        
        std::cout << "\nPress Enter to continue...";
        std::cin.ignore();
        std::cin.get();
    }

    void searchFiles() {
        clearScreen();
        displayHeader();
        std::cout << "Search Files\n";
        std::cout << "────────────\n\n";
        
        std::string searchTerm;
        
        std::cout << "Enter file name to search: ";
        std::getline(std::cin, searchTerm);
        
        std::cout << "\nSearching in: " << currentPath << "\n";
        std::cout << "────────────────────────────────────────────────────────────────\n\n";
        
        int count = 0;
        try {
            for (const auto& entry : fs::recursive_directory_iterator(currentPath)) {
                std::string fileName = entry.path().filename().string();
                if (fileName.find(searchTerm) != std::string::npos) {
                    std::string type = entry.is_directory() ? "[DIR]" : "[FILE]";
                    std::cout << type << " " << entry.path() << "\n";
                    count++;
                }
            }
            
            if (count == 0) {
                std::cout << "No files found matching '" << searchTerm << "'\n";
            } else {
                std::cout << "\n────────────────────────────────────────────────────────────────\n";
                std::cout << "Found " << count << " item(s) matching '" << searchTerm << "'\n";
            }
        } catch (const fs::filesystem_error& e) {
            std::cout << "\nError during search: " << e.what() << "\n";
        }
        
        std::cout << "\nPress Enter to continue...";
        std::cin.get();
    }

    void viewPermissions() {
        clearScreen();
        displayHeader();
        std::cout << "View File Permissions\n";
        std::cout << "─────────────────────\n\n";
        
        std::string fileName;
        
        std::cout << "Enter file/directory name: ";
        std::getline(std::cin, fileName);
        
        try {
            fs::path filePath = currentPath / fileName;
            
            if (!fs::exists(filePath)) {
                std::cout << "\nError: File/Directory does not exist!\n";
            } else {
                auto perms = fs::status(filePath).permissions();
                struct stat fileStat;
                stat(filePath.c_str(), &fileStat);
                
                std::cout << "\nFile: " << filePath << "\n";
                std::cout << "────────────────────────────────────────────────────────────────\n";
                std::cout << "Permissions: " << getPermissionString(perms) << "\n";
                std::cout << "Octal:       " << std::oct << (fileStat.st_mode & 0777) << std::dec << "\n";
                std::cout << "\nOwner:  Read=" << ((perms & fs::perms::owner_read) != fs::perms::none ? "Yes" : "No");
                std::cout << " Write=" << ((perms & fs::perms::owner_write) != fs::perms::none ? "Yes" : "No");
                std::cout << " Execute=" << ((perms & fs::perms::owner_exec) != fs::perms::none ? "Yes" : "No") << "\n";
                std::cout << "Group:  Read=" << ((perms & fs::perms::group_read) != fs::perms::none ? "Yes" : "No");
                std::cout << " Write=" << ((perms & fs::perms::group_write) != fs::perms::none ? "Yes" : "No");
                std::cout << " Execute=" << ((perms & fs::perms::group_exec) != fs::perms::none ? "Yes" : "No") << "\n";
                std::cout << "Others: Read=" << ((perms & fs::perms::others_read) != fs::perms::none ? "Yes" : "No");
                std::cout << " Write=" << ((perms & fs::perms::others_write) != fs::perms::none ? "Yes" : "No");
                std::cout << " Execute=" << ((perms & fs::perms::others_exec) != fs::perms::none ? "Yes" : "No") << "\n";
            }
        } catch (const fs::filesystem_error& e) {
            std::cout << "\nError: " << e.what() << "\n";
        }
        
        std::cout << "\nPress Enter to continue...";
        std::cin.get();
    }

    void changePermissions() {
        clearScreen();
        displayHeader();
        std::cout << "Change File Permissions\n";
        std::cout << "───────────────────────\n\n";
        
        std::string fileName;
        
        std::cout << "Enter file/directory name: ";
        std::getline(std::cin, fileName);
        
        try {
            fs::path filePath = currentPath / fileName;
            
            if (!fs::exists(filePath)) {
                std::cout << "\nError: File/Directory does not exist!\n";
            } else {
                std::cout << "\nCurrent permissions: " << getPermissionString(fs::status(filePath).permissions()) << "\n";
                std::cout << "\nEnter new permissions in octal format (e.g., 644, 755): ";
                std::string octalPerms;
                std::getline(std::cin, octalPerms);
                
                try {
                    int perms = std::stoi(octalPerms, nullptr, 8);
                    fs::permissions(filePath, static_cast<fs::perms>(perms), fs::perm_options::replace);
                    std::cout << "\nPermissions changed successfully!\n";
                    std::cout << "New permissions: " << getPermissionString(fs::status(filePath).permissions()) << "\n";
                } catch (const std::exception& e) {
                    std::cout << "\nError: Invalid permission format!\n";
                }
            }
        } catch (const fs::filesystem_error& e) {
            std::cout << "\nError: " << e.what() << "\n";
        }
        
        std::cout << "\nPress Enter to continue...";
        std::cin.get();
    }

    void viewFileDetails() {
        clearScreen();
        displayHeader();
        std::cout << "View File Details\n";
        std::cout << "─────────────────\n\n";
        
        std::string fileName;
        
        std::cout << "Enter file/directory name: ";
        std::getline(std::cin, fileName);
        
        try {
            fs::path filePath = currentPath / fileName;
            
            if (!fs::exists(filePath)) {
                std::cout << "\nError: File/Directory does not exist!\n";
            } else {
                auto fileStatus = fs::status(filePath);
                struct stat fileStat;
                stat(filePath.c_str(), &fileStat);
                
                std::cout << "\n╔═══════════════════ FILE DETAILS ═══════════════════╗\n";
                std::cout << "║ Name:        " << filePath.filename() << "\n";
                std::cout << "║ Path:        " << fs::absolute(filePath) << "\n";
                std::cout << "║ Type:        " << (fs::is_directory(filePath) ? "Directory" : "File") << "\n";
                
                if (fs::is_regular_file(filePath)) {
                    std::cout << "║ Size:        " << formatFileSize(fs::file_size(filePath)) << "\n";
                }
                
                std::cout << "║ Permissions: " << getPermissionString(fileStatus.permissions()) << "\n";
                
                auto ftime = fs::last_write_time(filePath);
                auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
                    ftime - fs::file_time_type::clock::now() + std::chrono::system_clock::now());
                std::time_t cftime = std::chrono::system_clock::to_time_t(sctp);
                
                std::cout << "║ Modified:    " << std::ctime(&cftime);
                std::cout << "╚════════════════════════════════════════════════════╝\n";
            }
        } catch (const fs::filesystem_error& e) {
            std::cout << "\nError: " << e.what() << "\n";
        }
        
        std::cout << "\nPress Enter to continue...";
        std::cin.get();
    }

    int getValidMenuChoice() {
        int choice;
        while (true) {
            if (std::cin >> choice) {
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                return choice;
            } else {
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                std::cout << "\nInvalid input! Please enter a number: ";
            }
        }
    }

    void run() {
        int choice;
        
        while (true) {
            clearScreen();
            displayHeader();
            displayMenu();
            
            choice = getValidMenuChoice();
            
            switch (choice) {
                case 1:
                    listFiles();
                    break;
                case 2:
                    changeDirectory();
                    break;
                case 3:
                    createFile();
                    break;
                case 4:
                    createDirectory();
                    break;
                case 5:
                    copyFile();
                    break;
                case 6:
                    moveFile();
                    break;
                case 7:
                    deleteFile();
                    break;
                case 8:
                    deleteDirectory();
                    break;
                case 9:
                    searchFiles();
                    break;
                case 10:
                    viewPermissions();
                    break;
                case 11:
                    changePermissions();
                    break;
                case 12:
                    viewFileDetails();
                    break;
                case 0:
                    clearScreen();
                    std::cout << "\n╔═══════════════════════════════════════════════╗\n";
                    std::cout << "║  Thank you for using File Explorer!           ║\n";
                    std::cout << "║  Goodbye!                                      ║\n";
                    std::cout << "╚═══════════════════════════════════════════════╝\n\n";
                    return;
                default:
                    std::cout << "\nInvalid choice! Please try again.\n";
                    std::cout << "Press Enter to continue...";
                    std::cin.get();
            }
        }
    }
};

int main() {
    FileExplorer explorer;
    explorer.run();
    return 0;
}
