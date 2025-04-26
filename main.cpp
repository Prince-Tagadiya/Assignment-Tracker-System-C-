#include <iostream>
#include <vector>
#include <fstream>
#include <iomanip>
#include <algorithm>
#include <ctime>
#include <cstdlib>

#ifdef _WIN32
#include <windows.h>
#define CLEAR "cls"
#define OPEN_CMD "start "
#elif __APPLE__
#define CLEAR "clear"
#define OPEN_CMD "open "
#else
#define CLEAR "clear"
#define OPEN_CMD "xdg-open "
#endif

using namespace std;

#define RESET "\033[0m"
#define BOLD "\033[1m"
#define GREEN "\033[32m"
#define BLUE "\033[34m"
#define CYAN "\033[36m"
#define RED "\033[31m"
#define YELLOW "\033[33m"
#define WHITE "\033[37m"

struct Assignment {
    int id;
    string title;
    string subject;
    string dueDate;
    int daysLeft;
    string pdfPath;
};

vector<Assignment> assignments;
const string FILE_NAME = "assignments_data.txt";
int uniqueID = 1; // Ensures each assignment has a unique ID

// Calculate days left
int calculateDaysLeft(const string &dueDate) {
    int day, month, year;
    sscanf(dueDate.c_str(), "%d/%d/%d", &day, &month, &year);

    time_t now = time(0);
    tm *ltm = localtime(&now);
    
    struct tm due = {};
    due.tm_mday = day;
    due.tm_mon = month - 1;
    due.tm_year = year - 1900;

    time_t dueTime = mktime(&due);
    time_t currentTime = time(0);

    return (dueTime - currentTime) / (60 * 60 * 24);
}

// Save assignments
void saveAssignments() {
    ofstream file(FILE_NAME);
    for (auto &a : assignments) {
        file << a.id << "|" << a.title << "|" << a.subject << "|" << a.dueDate << "|" << a.pdfPath << "\n";
    }
    file.close();
}

// Load assignments
void loadAssignments() {
    ifstream file(FILE_NAME);
    if (!file) return;

    assignments.clear();
    int id;
    string title, subject, dueDate, pdfPath;

    while (file >> id) {
        file.ignore(); // Ignore separator
        getline(file, title, '|');
        getline(file, subject, '|');
        getline(file, dueDate, '|');
        getline(file, pdfPath);
        
        int daysLeft = calculateDaysLeft(dueDate);
        assignments.push_back({id, title, subject, dueDate, daysLeft, pdfPath});
        uniqueID = max(uniqueID, id + 1); // Ensures IDs remain unique
    }
    file.close();
}

// Print header
void printHeader() {
    system(CLEAR);
    cout << BOLD << CYAN << "============================================\n";
    cout << "        📚 Assignment Tracker 📚           \n";
    cout << "============================================" << RESET << endl;
}

// Show menu
void showMenu() {
    cout << BOLD << BLUE << "\n📌 Main Menu\n" << RESET;
    cout << GREEN << "1. Add Assignment\n";
    cout << "2. View Assignments (By Urgency)\n";
    cout << "3. Mark Assignment as Complete\n";
    cout << "4. Exit\n" << RESET;
    cout << BOLD << "👉 Enter your choice: " << RESET;
}

// Add assignment
void addAssignment() {
    Assignment a;
    a.id = uniqueID++; // Assigns a unique ID
    cout << BOLD << YELLOW << "\n➕ Adding a New Assignment\n" << RESET;
    cin.ignore();
    cout << "Enter title: ";
    getline(cin, a.title);
    cout << "Enter subject: ";
    getline(cin, a.subject);
    cout << "Enter due date (DD/MM/YYYY): ";
    getline(cin, a.dueDate);
    cout << "Enter PDF path (or press Enter to skip): ";
    getline(cin, a.pdfPath);

    a.daysLeft = calculateDaysLeft(a.dueDate);
    assignments.push_back(a);
    saveAssignments();
    cout << GREEN << "✅ Assignment added successfully!\n" << RESET;
}

// Categorize and display assignments
void viewAssignments() {
    if (assignments.empty()) {
        cout << RED << "\n🚫 No assignments available!\n" << RESET;
        return;
    }

    sort(assignments.begin(), assignments.end(), [](const Assignment &a, const Assignment &b) {
        return a.daysLeft < b.daysLeft;
    });

    vector<Assignment> urgent, upcoming, later, overdue;
    
    for (auto &a : assignments) {
        if (a.daysLeft < 0)
            overdue.push_back(a);
        else if (a.daysLeft <= 2)
            urgent.push_back(a);
        else if (a.daysLeft <= 7)
            upcoming.push_back(a);
        else
            later.push_back(a);
    }

    auto printCategory = [](const string &title, const vector<Assignment> &list) {
        if (!list.empty()) {
            cout << BOLD << CYAN << "\n" << title << ":\n" << RESET;
            cout << BOLD << setw(5) << "ID" << setw(25) << "Title" << setw(20) << "Subject" << setw(15) << "Due Date" << setw(15) << "Days Left" << setw(15) << "Action" << RESET << endl;
            cout << "---------------------------------------------------------------------------------------------\n";
            for (const auto &a : list) {
                cout << setw(5) << a.id
                     << setw(25) << a.title
                     << setw(20) << a.subject
                     << setw(15) << a.dueDate
                     << setw(15) << (a.daysLeft < 0 ? "🚨 OVERDUE!" : to_string(a.daysLeft) + " days")
                     << setw(15) << (a.pdfPath.empty() ? "N/A" : "📂 Open PDF") << endl;
            }
        }
    };

    printCategory("🔥 URGENT", urgent);
    printCategory("⏳ UPCOMING", upcoming);
    printCategory("📅 LATER", later);
    printCategory("🚨 OVERDUE", overdue);

    cout << "\n📂 Enter assignment ID to open PDF (or 0 to go back): ";
    int id;
    cin >> id;

    auto it = find_if(assignments.begin(), assignments.end(), [id](const Assignment &a) {
        return a.id == id;
    });

    if (it != assignments.end() && !it->pdfPath.empty()) {
        string command = OPEN_CMD + it->pdfPath;
        system(command.c_str());
    } else if (id != 0) {
        cout << RED << "⚠ Invalid ID or no PDF available!\n" << RESET;
    }
}

// Mark assignment as complete (No PDF Opening)
void markAsComplete() {
    if (assignments.empty()) {
        cout << RED << "\n🚫 No assignments to mark as complete!\n" << RESET;
        return;
    }

    cout << BOLD << YELLOW << "\n📌 Current Assignments:\n" << RESET;
    cout << BOLD << setw(5) << "ID" << setw(25) << "Title" << setw(20) << "Subject" << setw(15) << "Due Date" << RESET << endl;
    cout << "--------------------------------------------------------------\n";
    
    for (const auto &a : assignments) {
        cout << setw(5) << a.id
             << setw(25) << a.title
             << setw(20) << a.subject
             << setw(15) << a.dueDate << endl;
    }

    cout << BOLD << YELLOW << "\n✅ Enter the assignment ID to mark as complete: " << RESET;
    int id;
    cin >> id;

    auto it = remove_if(assignments.begin(), assignments.end(), [id](const Assignment &a) {
        return a.id == id;
    });

    if (it != assignments.end()) {
        assignments.erase(it, assignments.end());
        saveAssignments();  // Save updated list
        cout << GREEN << "🎉 Assignment marked as complete!\n" << RESET;
    } else {
        cout << RED << "⚠ Invalid ID! Try again.\n" << RESET;
    }
}


// Main function
int main() {
    loadAssignments();
    int choice;

    do {
        printHeader();
        showMenu();
        cin >> choice;

        switch (choice) {
            case 1:
                addAssignment();
                break;
            case 2:
                viewAssignments();
                break;
            case 3:
                markAsComplete();
                break;
            case 4:
                cout << BOLD << GREEN << "\n👋 Exiting. Stay productive!\n" << RESET;
                break;
            default:
                cout << RED << "⚠ Invalid choice! Try again.\n" << RESET;
        }

        cout << BOLD << "\nPress Enter to continue..." << RESET;
        cin.ignore();
        cin.get();
    } while (choice != 4);

    return 0;
}