#pragma once
#include "Configuration.h"
#include <iostream>
#include <arpa/inet.h>
#include <unistd.h>
#include <set>
#include <fstream>
#include <string>
#include <chrono>
#include <ctime>

using namespace std;

int64_t hashFunction(const string& password) {
    hash<string> hash;
    string salt = "salt";
    return hash(password + salt);
}

void createLog(string message) {
    auto tm = chrono::system_clock::now();
    time_t end_time = chrono::system_clock::to_time_t(tm);
    //cout << ctime(&end_time) << " LOG: " << message << endl;
    cout << " LOG: " << message << endl;
}

void disconnectClient(int32_t clientSocket) {
    string message = "You are disconnected from the server";
    send(clientSocket, message.c_str(), message.length(), 0);
    close(clientSocket);
}

bool checkForSpecialCharacters(string& inputStr) {
    if (inputStr == "") return true;
    for (char character: inputStr) {
        if (character < '0' or (character > '9' and character < 'A') or (character > 'Z' and character < 'a') or character > 'z') return true;
    }
    return false;
}

string readClientsRequest(int32_t clientSocket) {
    char buffer[1024] = {0};
    int32_t bytes_received = read(clientSocket, buffer, sizeof(buffer));
    if (bytes_received > 0) {
        string request = buffer;
        return request;
    }
    else {
        createLog("The client's request is missing: client socket(" + to_string(clientSocket) + ")" );
        return "";
    }
}

bool checkForSpecialCharactersNumber(string& inputStr) {
    if (inputStr == "") return true;
    for (char character: inputStr) if (character < '0' or character > '9') return true;
    return false;
}

int32_t readClientsRequestNumber(int32_t clientSocket) {
    string inputData = readClientsRequest(clientSocket);
    if (checkForSpecialCharactersNumber(inputData)) return -1;
    return stoi(inputData);
}

class UserInterface {
public:
    virtual ~UserInterface() {}
    virtual string printingTableData(Configuration& configuration, string& columnsForOutput) const = 0;
    virtual string printingTheBestPlayers(Configuration& configuration, string& columnsForOutput) const = 0;
    virtual string playerSearch(Configuration& configuration, string& columnsForOutput) const = 0;
    virtual string printStatOfTeamsPlayers(Configuration& configuration, string& columnsForOutput) const = 0;
    virtual string changeTableData(Configuration& configuration) const = 0;
    virtual string registration(Configuration& configuration) const = 0;
private:
    virtual string getAllColumns(Configuration& configuration, const string& tableName) const = 0;
    virtual string getColumnForSearch(Configuration& configuration, const string& tableName) const = 0;
    virtual void printMessageForLastNameInput() const = 0;
    virtual void printMessageForFirstNameInput() const = 0;
    virtual void printPlayerSearchMenu(Configuration& configuration) const = 0;
    virtual void printListOfPlayers(Configuration& configuration) const = 0;
    virtual string getColumnNameToDetermineBest(const string& tableName) const = 0;
    virtual void printListOfTables(Configuration& configuration) const = 0;
    virtual int32_t printListOfColumnes(Configuration& configuration, const string& tableName) const = 0;
    virtual vector<string> getColumnNames(Configuration& configuration, const string& tableName, string& columnNumbers, int32_t Allnumber,  string& columnsForOutput) const = 0;
    virtual string receiveDataFromDBToPrint(const vector<string>& columnNamesForPrint, const string& tableName) const = 0;
};

class GuestUser: public UserInterface {
public:
    string userName;
    int32_t clientSocket;

    GuestUser(string name, int32_t socket): userName(name), clientSocket(socket) {}
    GuestUser(): userName("nobody"), clientSocket(0) {}

    string printingTableData(Configuration& configuration, string& columnsForOutput) const override {
        printListOfTables(configuration);
        int32_t inputData = readClientsRequestNumber(clientSocket);
        if (inputData == -1) return "";
        string tableName = configuration.tableNames[inputData - 1];
        int32_t Allnumber = printListOfColumnes(configuration, tableName);
        string columnNumbers = readClientsRequest(clientSocket);
        vector<string> columnNamesForPrint = getColumnNames(configuration, tableName, columnNumbers, Allnumber, columnsForOutput);
        string result = receiveDataFromDBToPrint(columnNamesForPrint, tableName); 
        return result;
    }

    string printingTheBestPlayers(Configuration& configuration, string& columnsForOutput) const override {
        printListOfPlayers(configuration);
        int32_t inputData = readClientsRequestNumber(clientSocket);
        if (inputData == -1) return "";
        string tableName = configuration.tableNames[inputData - 1];
        string columnName = getColumnNameToDetermineBest(tableName);
        columnsForOutput = getAllColumns(configuration, tableName);
        string result = "SELECT * FROM " + tableName + " ORDER BY " + columnName + " DESC LIMIT 5";
        return result;
    }

    string playerSearch(Configuration& configuration, string& columnsForOutput) const override {
        printPlayerSearchMenu(configuration);
        int32_t inputData = readClientsRequestNumber(clientSocket);
        if (inputData == -1) return "";
        printMessageForFirstNameInput();
        string playerName = readClientsRequest(clientSocket);
        if (checkForSpecialCharacters(playerName)) return "";
        printMessageForLastNameInput();
        string playerLastName = readClientsRequest(clientSocket);
        if (checkForSpecialCharacters(playerLastName)) return "";
        string tableName = configuration.tableNames[inputData - 1];
        string columnName = getColumnForSearch(configuration, tableName);
        columnsForOutput = getAllColumns(configuration, tableName);
        string result = "SELECT * FROM " + tableName + " WHERE " + tableName + "." + columnName + " = '" +  playerName + " " + playerLastName + "'";
        return result;
    }

    string printStatOfTeamsPlayers(Configuration& configuration, string& columnsForOutput) const override {
        return "#menu\nYou are not a regular user";
    }

    string changeTableData(Configuration& configuration) const override {
        return "#menu\nYou are not an Administrator";
    }

    string registration(Configuration& configuration) const override {
        string message = "menu\nPlease enter your username";
        send(clientSocket, message.c_str(), message.length(), 0);
        string newUsername = readClientsRequest(clientSocket);
        if (checkForSpecialCharacters(newUsername)) return "";
        message = "menu\nPlease enter your password\nThe password must consist only of Latin letters and numbers";
        send(clientSocket, message.c_str(), message.length(), 0);
        string newPassword = readClientsRequest(clientSocket);
        if (checkForSpecialCharacters(newPassword)) return "";
        message = "menu\nPlease enter your password again";
        send(clientSocket, message.c_str(), message.length(), 0);
        string newPasswordSecond = readClientsRequest(clientSocket);
        if (checkForSpecialCharacters(newPasswordSecond)) return "";
        if (newPassword != newPasswordSecond) return "#Passwords dont match";
        string result = "INSERT INTO users (username, right_user, hash_pswrd) VALUES ('" + newUsername + "', 'regular', '" + to_string(hashFunction(newPassword)) + "')";
        return result;
    }

private:
    string getAllColumns(Configuration& configuration, const string& tableName) const override {
        string columnsForOutput;
        for (string columnName: configuration.columnNames[tableName]) {
            columnsForOutput += columnName + "\t\t|\t\t";
        }
        columnsForOutput += "\n";
        return columnsForOutput;
    }

    string getColumnForSearch(Configuration& configuration, const string& tableName) const override {
        string columnName = configuration.columnNames[tableName][0];
        return columnName;
    }

     void printMessageForFirstNameInput() const override {
        string message = "menu\nEnter the player's name";
        send(clientSocket, message.c_str(), message.length(), 0);
    }

    void printMessageForLastNameInput() const override {
        string message = "menu\nEnter the player's last name";
        send(clientSocket, message.c_str(), message.length(), 0);
    }

    void printPlayerSearchMenu(Configuration& configuration) const override {
        string message = "menu\nChoose which position the player is playing in\n";
        int32_t i = 1;
        for (string tableName: configuration.tableNames) {
            if (tableName != "coaches" and tableName != "users" and tableName != "hashes" and tableName != "teams") 
                message += "[" + to_string(i++) + "] - " + tableName + "\n";
        }
        send(clientSocket, message.c_str(), message.length(), 0);
    }

    void printListOfPlayers(Configuration& configuration) const override {
        string message = "menu\n";
        int32_t i = 1;
        message += "Choose which top players to print\n";
        for (string tableName: configuration.tableNames) {
            if (tableName != "coaches" and tableName != "users" and tableName != "hashes" and tableName != "teams") 
                message += "[" + to_string(i++) + "] - " + tableName + "\n";
            else i++;
        }
        send(clientSocket, message.c_str(), message.length(), 0);
    } 

    string getColumnNameToDetermineBest(const string& tableName) const override {
        string result;
        if (tableName == "forwards" or tableName == "defenders") result = "goals";
        if (tableName == "goalkeepers") result = "SV";
        return result;
    }

    void printListOfTables(Configuration& configuration) const override {
        string message = "menu\n";
        int32_t i = 1;
        message += "Select the table to print\n";
        for (string tableName: configuration.tableNames) {
            message += "[" + to_string(i++) + "] - " + tableName + "\n";
        }
        send(clientSocket, message.c_str(), message.length(), 0);
    }

    int32_t printListOfColumnes(Configuration& configuration, const string& tableName) const override {
        string message = "menu\n";
        message += "Select the column to print\n";
        int32_t i = 1;
        for (string columnName: configuration.columnNames[tableName]) {
            message += "[" + to_string(i++) + "] - " + columnName + "\n";
        }
        message += "[" + to_string(i) + "] - " + "All\n";
        send(clientSocket, message.c_str(), message.length(), 0);
        return i;
    }

    vector<string> getColumnNames(Configuration& configuration, const string& tableName, string& columnNumbers, int32_t Allnumber, string& columnsForOutput) const override {
        vector<string> columnNamesForPrint;
        set<int32_t> setColumnNumbers;
        istringstream columnNumbersStream(columnNumbers);
        string number;
        while (columnNumbersStream >> number) {
            setColumnNumbers.insert(stoi(number));
        }
        int32_t i = 1;
        if (setColumnNumbers.find(Allnumber) != setColumnNumbers.end()) {
            columnNamesForPrint.push_back("ALL");
            for (string columnName: configuration.columnNames[tableName]) columnsForOutput += columnName + "\t\t|\t\t";
            return columnNamesForPrint;
        }
        for (string columnName: configuration.columnNames[tableName]) {
            if (i == 1 or setColumnNumbers.find(i) != setColumnNumbers.end() ) {
                columnNamesForPrint.push_back(columnName);
                columnsForOutput += columnName + "\t\t|\t\t";
                i++;
            }
        }
        columnsForOutput += "\n";
        return columnNamesForPrint;
    }

    string receiveDataFromDBToPrint(const vector<string>& columnNamesForPrint, const string& tableName) const override {
        string result;
        if (columnNamesForPrint[0] == "ALL") {
            result = "SELECT * FROM " + tableName;
        }
        else {
            result = "SELECT";
            for (string columnName: columnNamesForPrint) {
                result += " " + tableName + "." + columnName + ",";
            }
            if (result[result.size() - 1] == ',') result.erase(result.size() - 1, 1);
            result += " FROM " + tableName; 
        }
        return result;
    }
};

class Decorator: public GuestUser {
protected:
    UserInterface* userInterface_;
public:
    Decorator(UserInterface* userInterface): userInterface_(userInterface) {}
    string printingTableData(Configuration& configuration, string& columnsForOutput) const override {
        return this->userInterface_->printingTableData(configuration, columnsForOutput);
    }
    string printingTheBestPlayers(Configuration& configuration, string& columnsForOutput) const override {
        return this->userInterface_->printingTheBestPlayers(configuration, columnsForOutput);
    }
    string playerSearch(Configuration& configuration, string& columnsForOutput) const override {
        return this->userInterface_->playerSearch(configuration, columnsForOutput);
    }
    string printStatOfTeamsPlayers(Configuration& configuration, string& columnsForOutput) const override {
        return this->userInterface_->printStatOfTeamsPlayers(configuration, columnsForOutput);
    }
    string changeTableData(Configuration& configuration) const override {
        return this->userInterface_->changeTableData(configuration);
    }
    string registration(Configuration& configuration) const override {
        return this->userInterface_->registration(configuration);
    }
private:
    virtual string deletePlayer(Configuration& configuration, const string& tableName) const = 0;
    virtual string makeInsertRequest(Configuration& configuration, const string& tableName) const = 0;
    virtual void printToSelectTable(Configuration& configuration) const = 0;
    virtual void printListOfFeatures() const = 0;
    virtual void printNumberTableForStat(Configuration& configuration) const = 0;
    virtual void printMessageToSelectTeam() const = 0;
};

class AdminUser: public Decorator {
public:
    AdminUser(UserInterface* userInterface): Decorator(userInterface) {}

    string changeTableData(Configuration& configuration) const override {
        printListOfFeatures();
        int32_t inputData = readClientsRequestNumber(clientSocket);
        if (inputData == -1) return "";
        printToSelectTable(configuration);
        int32_t tableNumber = readClientsRequestNumber(clientSocket);
        if (tableNumber == -1) return "";
        string tableName = configuration.tableNames[tableNumber - 1];
        string result;
        if (inputData == 1) result = makeInsertRequest(configuration, tableName);
        else if (inputData == 2 and tableNumber < 4) result = deletePlayer(configuration, tableName);
        else {
            createLog("Incorrect request");
            return "!";
        }
        return result;
    }

    string registration(Configuration& configuration) const override {
        return "#menu\nYou are not a Guest";
    }

private:
    string deletePlayer(Configuration& configuration, const string& tableName) const override {
        string message = "menu\nEnter the player's name";
        send(clientSocket, message.c_str(), message.length(), 0);
        string playerName = readClientsRequest(clientSocket);
        if (checkForSpecialCharacters(playerName)) return "";
        string result = "DELETE FROM " + tableName + " WHERE " + configuration.columnNames[tableName][0] + " = '" + playerName + "'";
        return result;
    }
    string makeInsertRequest(Configuration& configuration, const string& tableName) const override {
        vector<string> values;
        string result = "INSERT INTO " + tableName + "(";
        for (string columnName: configuration.columnNames[tableName]) {
            result += columnName + ", ";
            string message = "Enter a value " + columnName;
            send(clientSocket, message.c_str(), message.length(), 0);
            string value = readClientsRequest(clientSocket);
            if (checkForSpecialCharacters(value)) return "";
            values.push_back(value);
        }
        result.erase(result.size() - 2, 2);
        result += ") VALUES (";
        int32_t i = 1;
        for (string value: values) {
            if (i == 1 or i == 3) result += "'" + value + "'" + ", ";
            else result += value  + ", ";
            i++;
        }
        result.erase(result.size() - 2, 2);
        result += ")";
        return result;
    }

    void printToSelectTable(Configuration& configuration) const override {
        string message = "menu\n";
        int32_t i = 1;
        for (string tableName: configuration.tableNames) {
            if (tableName != "users") 
                message += "[" + to_string(i++) + "] - " + tableName + "\n";
            else i++;
        }
        send(clientSocket, message.c_str(), message.length(), 0);
    } 

    void printListOfFeatures() const override {
        string message = "menu\n[1] - Inserting\n[2] - Correction of player data";
        send(clientSocket, message.c_str(), message.length(), 0);
    }

    void printNumberTableForStat(Configuration& configuration) const override {}
    void printMessageToSelectTeam() const override {}

};

class RegularUser: public Decorator {
public:
    RegularUser(UserInterface* userInterface): Decorator(userInterface) {}

    string changeTableData(Configuration& configuration) const override {
        return "#menu\nYou are not an Administrator";
    }

    string printStatOfTeamsPlayers(Configuration& configuration, string& columnsForOutput) const override {
        printMessageToSelectTeam();
        string teamName = readClientsRequest(clientSocket);
        if (checkForSpecialCharacters(teamName)) return "";
        printNumberTableForStat(configuration);
        int32_t numberTable = readClientsRequestNumber(clientSocket);
        if (numberTable == -1) return "";
        if (numberTable >= 3) {
            createLog("Incorrect input data from user: client socket(" + to_string(clientSocket) + ")");
            return "!";
        }
        string tableName = configuration.tableNames[numberTable];
        for (string columnName: configuration.columnNames[tableName]) {
            columnsForOutput += columnName + "\t\t";
        }
        columnsForOutput += "\n";
        string result = "SELECT * FROM " + tableName + " WHERE " + configuration.tableNames[numberTable] + "." + configuration.columnNames[configuration.tableNames[numberTable]][2] + " = '" + teamName + "'";
        return result;
    }

    string registration(Configuration& configuration) const override {
        return "#menu\nYou are not a Guest";
    }

private:
    void printNumberTableForStat(Configuration& configuration) const override {
        string message = "menu\nSelect the position of the players\n";
        int32_t i = 1;
        for (string tableName: configuration.tableNames) {
            message += "[" + to_string(i++) + "] - " + tableName + "\n";
            if (i == 4) break;
        }
        send(clientSocket, message.c_str(), message.length(), 0);
    }

    void printMessageToSelectTeam() const override {
        string message = "menu\nEnter the name of the team";
        send(clientSocket, message.c_str(), message.length(), 0);
    }

    string deletePlayer(Configuration& configuration, const string& tableName) const override {}
    string makeInsertRequest(Configuration& configuration, const string& tableName) const override {}
    void printToSelectTable(Configuration& configuration) const override {}
    void printListOfFeatures() const override {}
};
