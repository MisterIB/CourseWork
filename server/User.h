#pragma once
#include "Configuration.h"
#include <iostream>
#include <arpa/inet.h>
#include <unistd.h>
#include <set>
#include <fstream>
#include <string>

using namespace std;

bool checkForSpecialCharacters(string& inputStr) {
    for (char character: inputStr) {
        if (character < 'A' or (character > 'Z' and character < 'a') or character > 'z') return true;
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
    else throw runtime_error("The client's request is missing");
}

class UserInterface {
public:
    virtual ~UserInterface() {}
    virtual string printingTableData(Configuration& configuration, int32_t& amountOfColumns) const = 0;
    virtual string printingTheBestPlayers(Configuration& configuration) const = 0;
    virtual string playerSearch(Configuration& configuration) const = 0;
    virtual string printStatOfTeamsPlayers(Configuration& configuration) const = 0;
    virtual string changeTableData(Configuration& configuration) const = 0;
private:
    virtual string getColumnForSearch(Configuration& configuration, const string& tableName) const = 0;
    virtual void printMessageForLastNameInput() const = 0;
    virtual void printMessageForFirstNameInput() const = 0;
    virtual void printPlayerSearchMenu(Configuration& configuration) const = 0;
    virtual void printListOfPlayers(Configuration& configuration) const = 0;
    virtual string getColumnNameToDetermineBest(const string& tableName) const = 0;
    virtual void printListOfTables(Configuration& configuration) const = 0;
    virtual int32_t printListOfColumnes(Configuration& configuration, const string& tableName) const = 0;
    virtual vector<string> getColumnNames(Configuration& configuration, const string& tableName, string& columnNumbers, int32_t Allnumber) const = 0;
    virtual string receiveDataFromDBToPrint(const vector<string>& columnNamesForPrint, const string& tableName) const = 0;
};

class GuestUser: public UserInterface {
public:
    string userName;
    int32_t clientSocket;

    GuestUser(string name, int32_t socket): userName(name), clientSocket(socket) {}
    GuestUser(): userName("nobody"), clientSocket(0) {}

    string printingTableData(Configuration& configuration, int32_t& amountOfColumns) const override {
        printListOfTables(configuration);
        int32_t inputData = stoi(readClientsRequest(clientSocket));
        string tableName = configuration.tableNames[inputData - 1];
        int32_t Allnumber = printListOfColumnes(configuration, tableName);
        string columnNumbers = readClientsRequest(clientSocket);
        vector<string> columnNamesForPrint = getColumnNames(configuration, tableName, columnNumbers, Allnumber);
        string result = receiveDataFromDBToPrint(columnNamesForPrint, tableName); 
        amountOfColumns = columnNamesForPrint.size();
        return result;
    }

    string printingTheBestPlayers(Configuration& configuration) const override {
        printListOfPlayers(configuration);
        int32_t inputData = stoi(readClientsRequest(clientSocket));
        string tableName = configuration.tableNames[inputData - 1];
        string columnName = getColumnNameToDetermineBest(tableName);
        string result = "SELECT * FROM " + tableName + " ORDER BY " + columnName + " DESC LIMIT 5";
        return result;
    }

    string playerSearch(Configuration& configuration) const override {
        printPlayerSearchMenu(configuration);
        int32_t inputData = stoi(readClientsRequest(clientSocket));
        printMessageForFirstNameInput();
        string playerName = readClientsRequest(clientSocket);
        if (checkForSpecialCharacters(playerName)) return "";
        printMessageForLastNameInput();
        string playerLastName = readClientsRequest(clientSocket);
        if (checkForSpecialCharacters(playerLastName)) return "";
        string tableName = configuration.tableNames[inputData - 1];
        string columnName = getColumnForSearch(configuration, tableName);
        string result = "SELECT * FROM " + tableName + " WHERE " + tableName + "." + columnName + " = '" +  playerName + " " + playerLastName + "'";
        return result;
    }

    string printStatOfTeamsPlayers(Configuration& configuration) const override {
        string message = "You are not a regular user";
        send(clientSocket, message.c_str(), message.length(), 0);
        message = readClientsRequest(clientSocket);
        return "0";
    }

    string changeTableData(Configuration& configuration) const override {
        string message = "You are not an administrator";
        send(clientSocket, message.c_str(), message.length(), 0);
        message = readClientsRequest(clientSocket);
        return "0";
    }

private:
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

    vector<string> getColumnNames(Configuration& configuration, const string& tableName, string& columnNumbers, int32_t Allnumber) const override {
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
            return columnNamesForPrint;
        }
        for (string columnName: configuration.columnNames[tableName]) {
            if (setColumnNumbers.find(i++) != setColumnNumbers.end()) columnNamesForPrint.push_back(columnName);
        }
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
    string printingTableData(Configuration& configuration, int32_t& amountOfColumns) const override {
        return this->userInterface_->printingTableData(configuration, amountOfColumns);
    }
    string printingTheBestPlayers(Configuration& configuration) const override {
        return this->userInterface_->printingTheBestPlayers(configuration);
    }
    string playerSearch(Configuration& configuration) const override {
        return this->userInterface_->playerSearch(configuration);
    }
    string printStatOfTeamsPlayers(Configuration& configuration) const override {
        return this->userInterface_->printStatOfTeamsPlayers(configuration);
    }
    string changeTableData(Configuration& configuration) const override {
        return this->userInterface_->changeTableData(configuration);
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
        int32_t inputData = stoi(readClientsRequest(clientSocket));
        printToSelectTable(configuration);
        int32_t tableNumber = stoi(readClientsRequest(clientSocket));
        string tableName = configuration.tableNames[tableNumber - 1];
        string result;
        if (inputData == 1) result = makeInsertRequest(configuration, tableName);
        else if (inputData == 2 and tableNumber < 4) result = deletePlayer(configuration, tableName);
        else throw runtime_error("Incorrect request");
        return result;
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
        for (string columnName: configuration.columnNames[tableName]) {//sql инъекция
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
        string message = "You are not an Administrator";
        send(clientSocket, message.c_str(), message.length(), 0);
        message = readClientsRequest(clientSocket);
        return "0";
    }

    string printStatOfTeamsPlayers(Configuration& configuration) const override {
        printMessageToSelectTeam();
        string teamName = readClientsRequest(clientSocket);
        if (checkForSpecialCharacters(teamName)) return "";
        printNumberTableForStat(configuration);
        int32_t numberTable = stoi(readClientsRequest(clientSocket));
        if (numberTable >= 3) {
            throw runtime_error("Incorrect input data");
            return "";
        }
        string result = "SELECT * FROM " + configuration.tableNames[numberTable] + " WHERE " + configuration.tableNames[numberTable] + "." + configuration.columnNames[configuration.tableNames[numberTable]][2] + " = '" + teamName + "'";
        return result;
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
