#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <arpa/inet.h>
#include <unistd.h>
#include <thread>
#include <bits/stdc++.h>

#include <pqxx/pqxx>

using namespace std;

string startMenu() {
    string response = "";
    response += "\t\t\t\tWelcome to the application\n";
    response += "'Manager for working with the data of hockey teams of the WDHL (World Hockey League)'\n";
    return response;
}

string authorizationMenu() {
    string response = "";
    response += "\t\t\t\tAuthorization";
    response += "Please enter your username";
    return response;
}

string menu() {
    string response = "";
    response += "Select an action\n";
    response += "[1] - Printing table data\n";
    response += "[2] - Printing player statistics\n";
    response += "[3] - Printing the best players\n";
    response += "[4] - Player Search\n";
    response += "[5] - Changing table data (only admin)\n";
    response += "[6] - Add a favorite player (only regular user)\n";
    response += "[7] - Registration (only guest)\n";
    return response;
}

class UserFactory {
    User createUser();
    UserSpecification createSpecification();
};

class adminUserFactory: UserFactory {
public: 
    User createUser() { return }

};

class User {

};

class AdminUser: User {

};

class Configuration {
public:
    string dataBaseName;
    vector<string> tableNames;
    map<string, vector<string>> columnNames;

    Configuration(): dataBaseName("") {}

    void readConfiguration(const string& pathToFile) {
        ifstream confFile(pathToFile);
        if (confFile.is_open()) {
            dataBaseName = readDataBaseName(confFile);
            readTableNames(confFile);
        }
        confFile.close();
    }   

private:
    string removeQuotes(const string& inputStr) {
        string modifiedStr = inputStr;
        if (modifiedStr[0] == '\"') modifiedStr.erase(0, 1);
        if (modifiedStr[modifiedStr.size() - 1] == '\"') modifiedStr.erase(modifiedStr.size() - 1, 1);
        return modifiedStr;
    }

    string removeColon(const string& inputStr) {
        string modifiedStr = inputStr;
        if (modifiedStr[modifiedStr.size() - 1] == ':') modifiedStr.erase(modifiedStr.size() - 1, 1);
        return modifiedStr;
    }

    string removeSquareBrackets(const string& inputStr) {
        string modifiedStr = inputStr;
        if (modifiedStr[0] == '[') modifiedStr.erase(0, 1);
        if (modifiedStr[modifiedStr.size() - 1] == ']') modifiedStr.erase(modifiedStr.size() - 1, 1);
        return modifiedStr;
    }

    string removeComma(const string& inputStr) {
        string modifiedStr = inputStr;
        if (modifiedStr[modifiedStr.size() - 1] == ',') modifiedStr.erase(modifiedStr.size() - 1, 1);
        return modifiedStr;
    }

    void readColumnNames(ifstream& confFile, const string& tableName) {
        string inputStr;
        getline(confFile, inputStr);
        istringstream inputStrStream(inputStr);
        string columnName;
        while (inputStrStream >> columnName) {
            columnName = removeComma(columnName);
            columnName = removeSquareBrackets(columnName);
            columnName = removeQuotes(columnName);
            columnNames[tableName].push_back(columnName);
        }
    }

    void readTableNames(ifstream& confFile) {
        string inputStr;
        confFile >> inputStr;
        if (inputStr != "\"structure\":") throw runtime_error("Incorrect syntax in the JSON file");
        confFile >> inputStr;
        if (inputStr != "{") throw runtime_error("Incorrect syntax in the JSON file");
        while (inputStr != "}") {
            confFile >> inputStr;
            if (inputStr == "}") break;
            inputStr = removeColon(inputStr);
            inputStr = removeQuotes(inputStr);
            tableNames.push_back(inputStr);
            readColumnNames(confFile, inputStr);
        }
    }

    string readDataBaseName(ifstream& confFile) {
        string inputStr;
        confFile >> inputStr;
        if (inputStr != "{") throw runtime_error("Incorrect syntax in the JSON file");
        confFile >> inputStr;
        if (inputStr == "\"name\":") confFile >> inputStr;
        inputStr = removeComma(inputStr);
        inputStr = removeQuotes(inputStr);
        return inputStr;
    }
public:
    void print() {
        cout << dataBaseName << endl;
        for (string str: tableNames) {
            cout << str << " ";
        }
        cout << endl;
        for (string str: tableNames) {
            for (string strMp: columnNames[str]) {
                cout << strMp << " ";
            }
        }
        cout << endl;
    }
};

void createDataBase(pqxx::work& db, Configuration& configuration) {
    pqxx::result r{db.exec("SELECT table_name FROM information_schema.tables WHERE table_schema = '" + configuration.dataBaseName + "' AND table_name = '" + configuration.tableNames[0] + "';")};
    if (!r.empty()) cout << "таблица уже создана" << endl;
    else {
        db.exec("INSERT INTO forwards (name, age, team, goals, assists, pim, awards) VALUES ('Vlad', 19, 'NSTU', 0, 0, 10, 0)");
        r = db.exec("SELECT * FROM forwards");
        db.commit();
        for (auto const &row: r) {
            for (auto const &field: row) std::cout << field.c_str() << '\t';
            std::cout << '\n';
        }
    }
}

void processingRequests() {

}

int64_t hashFunction(const string& password) {
    hash<string> hash;
    string salt = "salt";
    return hash(password + salt);
}

void userAuthorization(int32_t clientSocket) {
    string response = startMenu();
    send(clientSocket, response.c_str(), response.length(), 0);
    int32_t amountOfAttempts = 3;
    while (amountOfAttempts != 0) {
        amountOfAttempts--;
    }
    processingRequests();
}

void startingServer() {
    int32_t serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == 0) throw runtime_error("Error create socket");

    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(7432);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    if (bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) throw runtime_error("Address binding error");
    if (listen(serverSocket, 3) < 0) throw runtime_error("Listening error");

    cout << "The server is listening" << endl;
    while (true) {
        int32_t clientSocket = accept(serverSocket, nullptr, nullptr);
        if (clientSocket < 0) throw runtime_error("failed to accept the data");

        cout << "A new client has connected: " << clientSocket << endl;

        thread t(userAuthorization, clientSocket);
		t.detach();
    }
	close(serverSocket);
}

int main() {
    try {
	    setlocale(LC_ALL, "RUSSIAN");
        pqxx::connection c("user=tester password=testPassword1 host=172.16.1.4 port=5432 dbname=tester target_session_attrs=read-write");
        pqxx::work db(c);
        Configuration configuration;
        configuration.readConfiguration("/configuration/configuration.json");
        createDataBase(db, configuration);
        startingServer();
    }
    catch (exception &e) {
		    cout << e.what();
	    }
}
