#include <iostream>  
#include <map>       
#include <vector>
#include <stack>
#include <algorithm>
#include <string>
#include <ctime>   // for current time
#include <fstream> // for loading to file

using namespace std;

// Template for question
struct Question {
    int id;
    string title;
    string topic;
    string difficulty;
    string notes;
    string platform;
    bool isSolved;
    string solvedDate;
};

// GLOBALS
map<int, Question> questionBank;  // key -> id, stores -> question
map<string, vector<int> > topicMap; // search question by topic
map<string, int> solvedCountByTopic; // solved per topic
map<string, int> solvedCountByDate;  // solved on date
map<int, Question> deletedQuestions; // stores deleted question to undo
stack<pair<string, int> > undoStack; // to undo actions
int questionID = 1;

// Get current date in dd/mm/yyyy
string getCurrentDate() {
    time_t now = time(0);
    tm* ltm = localtime(&now);
    char buf[11];
    sprintf(buf, "%02d/%02d/%04d", ltm->tm_mday, ltm->tm_mon + 1, 1900 + ltm->tm_year);
    return string(buf);
}

// Save to file in readable CSV format
void saveDataToFile() {
    ofstream fout("tracker_data.csv");
    fout << "ID,Title,Topic,Difficulty,Platform,Notes,Solved,SolvedDate\n";
    for (map<int, Question>::iterator it = questionBank.begin(); it != questionBank.end(); ++it) {
        Question& q = it->second;
        fout << q.id << ","
             << '"' << q.title << '"' << ","
             << q.topic << ","
             << q.difficulty << ","
             << q.platform << ","
             << '"' << q.notes << '"' << ","
             << (q.isSolved ? "Yes" : "No") << ","
             << q.solvedDate << "\n";
    }
    fout.close();
}

// Menu
void showMenu() {
    cout << "\n -INTERVIEW PREP TRACKER- \n";
    cout << "1. Add a new question\n";
    cout << "2. Mark question as solved\n";
    cout << "3. Search questions by topic\n";
    cout << "4. View solved count by topic\n";
    cout << "5. View solved count by date\n";
    cout << "6. Undo last action\n";
    cout << "7. View all questions solved on a particular date\n";
    cout << "8. Delete a question by ID\n";
    cout << "9. Save data to file manually\n"; // NEW OPTION
    cout << "10. Exit\n";
    cout << "Enter your choice: ";
}

// Add question
void addQuestion() {
    Question q;
    q.id = questionID++;

    cin.ignore();
    cout << "Enter title: ";
    getline(cin, q.title);

    cout << "Enter topic: ";
    getline(cin, q.topic);

    cout << "Enter difficulty (Easy/Medium/Hard): ";
    getline(cin, q.difficulty);

    cout << "Enter notes: ";
    getline(cin, q.notes);

    cout << "Enter platform (LeetCode, GFG, etc.): ";
    getline(cin, q.platform);

    q.isSolved = false;
    q.solvedDate = "";

    questionBank[q.id] = q;
    topicMap[q.topic].push_back(q.id);
    undoStack.push(make_pair("add", q.id));

    saveDataToFile();

    cout << "Question added with ID: " << q.id << "\n";
}

// Mark as solved
void markSolved() {
    int id;
    cout << "Enter question ID to mark as solved: ";
    cin >> id;

    if (questionBank.find(id) != questionBank.end()) {
        if (!questionBank[id].isSolved) {
            cin.ignore();
            questionBank[id].isSolved = true;

            cout << "Enter date of solving (dd/mm/yyyy) or leave blank for today: ";
            string inputDate;
            getline(cin, inputDate);
            if (inputDate.empty()) inputDate = getCurrentDate();

            questionBank[id].solvedDate = inputDate;
            solvedCountByTopic[questionBank[id].topic]++;
            solvedCountByDate[inputDate]++;
            undoStack.push(make_pair("solve", id));

            saveDataToFile();

            cout << "Marked as solved.\n";
        } else {
            cout << "Already marked as solved.\n";
        }
    } else {
        cout << "Invalid Question ID.\n";
    }
}

// Search by topic
void searchByTopic() {
    string topic;
    cin.ignore();
    cout << "Enter topic to search: ";
    getline(cin, topic);

    bool found = false;
    for (map<int, Question>::iterator it = questionBank.begin(); it != questionBank.end(); ++it) {
        int id = it->first;
        Question& q = it->second;
        if (q.topic == topic) {
            if (!found) {
                cout << "\n Questions under topic [" << topic << "]:\n";
                found = true;
            }
            cout << "ID: " << q.id
                 << " | Title: " << q.title
                 << " | Difficulty: " << q.difficulty
                 << " | Platform: " << q.platform
                 << " | Solved: " << (q.isSolved ? "Yes" : "No") << "\n";
        }
    }

    if (!found) {
        cout << "No questions found for this topic.\n";
    }
}

// Stats by topic
void showSolvedStatsByTopic() {
    cout << "\n Solved Questions by Topic:\n";
    if (solvedCountByTopic.empty()) {
        cout << "No solved questions yet.\n";
    }
    for (map<string, int>::iterator it = solvedCountByTopic.begin(); it != solvedCountByTopic.end(); ++it) {
        cout << it->first << ": " << it->second << " solved\n";
    }
}

// Stats by date
void showSolvedStatsByDate() {
    cout << "\n Solved Questions by Date:\n";
    if (solvedCountByDate.empty()) {
        cout << "No solved questions yet.\n";
    }
    for (map<string, int>::iterator it = solvedCountByDate.begin(); it != solvedCountByDate.end(); ++it) {
        cout << it->first << ": " << it->second << " solved\n";
    }
}

// View solved questions on a date
void viewQuestionsByDate() {
    string date;
    cin.ignore();
    cout << "Enter date (dd/mm/yyyy): ";
    getline(cin, date);

    vector<Question> solvedOnDate;

    for (map<int, Question>::iterator it = questionBank.begin(); it != questionBank.end(); ++it) {
        Question& q = it->second;
        if (q.isSolved && q.solvedDate == date) {
            solvedOnDate.push_back(q);
        }
    }

    if (solvedOnDate.empty()) {
        cout << "No questions were marked as solved on this date.\n";
        return;
    }

    sort(solvedOnDate.begin(), solvedOnDate.end(), [](const Question& a, const Question& b) {
        return a.title < b.title;
    });

    cout << "\n Questions solved on " << date << ":\n";
    for (size_t i = 0; i < solvedOnDate.size(); ++i) {
        Question& q = solvedOnDate[i];
        cout << "ID: " << q.id
             << " | Title: " << q.title
             << " | Topic: " << q.topic
             << " | Difficulty: " << q.difficulty
             << " | Platform: " << q.platform
             << " | Notes: " << q.notes << "\n";
    }

    cout << "\n Total solved on this date: " << solvedOnDate.size() << "\n";
}

// Delete by ID
void deleteQuestionByID() {
    int id;
    cout << "Enter the Question ID to delete: ";
    cin >> id;

    if (questionBank.find(id) == questionBank.end()) {
        cout << " No such question found.\n";
        return;
    }

    Question q = questionBank[id];
    string topic = q.topic;

    if (q.isSolved) {
        solvedCountByTopic[topic]--;
        solvedCountByDate[q.solvedDate]--;
    }

    topicMap[topic].erase(
        remove(topicMap[topic].begin(), topicMap[topic].end(), id),
        topicMap[topic].end());

    deletedQuestions[id] = q;
    questionBank.erase(id);

    undoStack.push(make_pair("delete", id));
    saveDataToFile();

    cout << "🗑 Question deleted successfully.\n";
}

// Undo last action
void undoLastAction() {
    if (undoStack.empty()) {
        cout << "Nothing to undo.\n";
        return;
    }

    pair<string, int> top = undoStack.top();
    string action = top.first;
    int id = top.second;
    undoStack.pop();

    if (action == "add") {
        string topic = questionBank[id].topic;
        topicMap[topic].erase(
            remove(topicMap[topic].begin(), topicMap[topic].end(), id),
            topicMap[topic].end());
        questionBank.erase(id);
        cout << "Undid last added question.\n";
    } else if (action == "solve") {
        string topic = questionBank[id].topic;
        string date = questionBank[id].solvedDate;
        questionBank[id].isSolved = false;
        questionBank[id].solvedDate = "";
        solvedCountByTopic[topic]--;
        solvedCountByDate[date]--;
        cout << "Undid last mark as solved.\n";
    } else if (action == "delete") {
        Question q = deletedQuestions[id];
        questionBank[id] = q;
        topicMap[q.topic].push_back(id);
        if (q.isSolved) {
            solvedCountByTopic[q.topic]++;
            solvedCountByDate[q.solvedDate]++;
        }
        deletedQuestions.erase(id);
        cout << "Undid question deletion.\n";
    }

    saveDataToFile();
}

// Main
int main() {
    int choice;
    while (true) {
        showMenu();
        cin >> choice;

        switch (choice) {
        case 1: addQuestion(); break;
        case 2: markSolved(); break;
        case 3: searchByTopic(); break;
        case 4: showSolvedStatsByTopic(); break;
        case 5: showSolvedStatsByDate(); break;
        case 6: undoLastAction(); break;
        case 7: viewQuestionsByDate(); break;
        case 8: deleteQuestionByID(); break;
        case 9: saveDataToFile(); cout << "✔ Data saved to tracker_data.csv successfully!\n"; break;
        case 10: cout << "Exiting... Good luck with your prep!\n"; return 0;
        default: cout << "Invalid choice. Try again.\n";
        }
    }
}
