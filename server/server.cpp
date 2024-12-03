//Подключение сервера и клиента через ip
//Безопасностьт для user и hash
//При поиске игрока выводить что нет игрока если нет
//env - порт
//json - путь
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

string startMenu() {
    string message = "";
    message += "\t\t\t\tWelcome to the application\n";
    message += "'Manager for working with the data of hockey teams of the WDHL (World Hockey League)'\n";
    return message;
}

string authorizationMenu() {
    string message = "";
    message += "\t\t\t\tAuthorization";
    message += "Please enter your username";
    return message;
}

string menu() {
    string message = "";
    message += "Select an action\n";
    message += "[1] - Printing table data\n";
    message += "[2] - Printing statistics of the team's players (only regular user)\n";
    message += "[3] - Printing the best players\n";
    message += "[4] - Player Search\n";
    message += "[5] - Changing table data (only admin)\n";
    message += "[6] - Registration (only guest)\n";
    message += "[7] - Exit";
    return message;
}

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

    friend class UserInterface;
};

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
        return "";
    }

    string changeTableData(Configuration& configuration) const override {
        string message = "You are not an administrator";
        send(clientSocket, message.c_str(), message.length(), 0);
        return "";
    }

private:
    string getColumnForSearch(Configuration& configuration, const string& tableName) const override {
        string columnName = configuration.columnNames[tableName][0];
        return columnName;
    }

     void printMessageForFirstNameInput() const override {
        string message = "Enter the player's name";
        send(clientSocket, message.c_str(), message.length(), 0);
    }

    void printMessageForLastNameInput() const override {
        string message = "Enter the player's last name";
        send(clientSocket, message.c_str(), message.length(), 0);
    }

    void printPlayerSearchMenu(Configuration& configuration) const override {
        string message = "Choose which position the player is playing in\n";
        int32_t i = 1;
        for (string tableName: configuration.tableNames) {
            if (tableName != "coaches" and tableName != "users" and tableName != "hashes" and tableName != "teams") 
                message += "[" + to_string(i++) + "] - " + tableName + "\n";
        }
        send(clientSocket, message.c_str(), message.length(), 0);
    }

    void printListOfPlayers(Configuration& configuration) const override {
        string message = "";
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
        string message = "";
        int32_t i = 1;
        message += "Select the table to print\n";
        for (string tableName: configuration.tableNames) {
            message += "[" + to_string(i++) + "] - " + tableName + "\n";
        }
        send(clientSocket, message.c_str(), message.length(), 0);
    }

    int32_t printListOfColumnes(Configuration& configuration, const string& tableName) const override {
        string message = "";
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
    //Делегация работы
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
        string message = "Enter the player's name";
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
        string message = "";
        int32_t i = 1;
        for (string tableName: configuration.tableNames) {
            if (tableName != "users") 
                message += "[" + to_string(i++) + "] - " + tableName + "\n";
            else i++;
        }
        send(clientSocket, message.c_str(), message.length(), 0);
    } 

    void printListOfFeatures() const override {
        string message = "[1] - Inserting\n[2] - Correction of player data";
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
        return "";
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
        string message = "Select the position of the players\n";
        int32_t i = 1;
        for (string tableName: configuration.tableNames) {
            message += "[" + to_string(i++) + "] - " + tableName + "\n";
            if (i == 4) break;
        }
        send(clientSocket, message.c_str(), message.length(), 0);
    }

    void printMessageToSelectTeam() const override {
        string message = "Enter the name of the team";
        send(clientSocket, message.c_str(), message.length(), 0);
    }

    string deletePlayer(Configuration& configuration, const string& tableName) const override {}
    string makeInsertRequest(Configuration& configuration, const string& tableName) const override {}
    void printToSelectTable(Configuration& configuration) const override {}
    void printListOfFeatures() const override {}
};

/*string createLineToSend(pqxx::result& res) {
    string result = "";
    return result;
}*/

string SendRequestInDB(const string& requestInDB, int32_t amountOfColumns) {
    if (requestInDB == "") return "";
    string newRequestInDB = requestInDB + ";";
    pqxx::connection c("user=tester password=testPassword1 host=172.16.1.4 port=5432 dbname=tester target_session_attrs=read-write");//Переменные окружения
    pqxx::work db(c);
    pqxx::result res = db.exec(newRequestInDB);//Изменить имя
    db.commit();
    string result = "";
    for (auto r: res) {
        for (auto f: r){
            result += f.as<string>();
        }
        result += '\n';
    }
    cout << result << endl;
    //cout << result << endl;//Delete
    //result = createLineToSend(res, amountOfColumns);
    c.close();
    return result;
}

void processingRequests(int32_t clientSocket, unique_ptr<UserInterface>& user, Configuration& configuration) {
    string message = menu();
    send(clientSocket, message.c_str(), message.length(), 0);
    while (true) {
        string request = readClientsRequest(clientSocket);
        int32_t inputData = stoi(request);
        string requestInDB;
        int32_t amountOfColumns;
        if (inputData == 1) requestInDB = user->printingTableData(configuration, amountOfColumns);
        else if (inputData == 2) requestInDB = user->printStatOfTeamsPlayers(configuration);
        else if (inputData == 3) requestInDB = user->printingTheBestPlayers(configuration);
        else if (inputData == 4) requestInDB = user->playerSearch(configuration);
        else if (inputData == 5) requestInDB = user->printStatOfTeamsPlayers(configuration);
        else if (inputData == 7) {
            close(clientSocket);
            return;
        }
        cout << requestInDB << endl;//Delete
        /*else if (inputData == 6)*/
        message = SendRequestInDB(requestInDB, amountOfColumns);
        if (message == "") {
            close(clientSocket);
            return;
        }
        message += menu();
        send(clientSocket, message.c_str(), message.length(), 0);
    }
}

int64_t hashFunction(const string& password) {
    hash<string> hash;
    string salt = "salt";
    return hash(password + salt);
}

bool passwordVerification(const string& password, const string& username) {
    string hash = to_string(hashFunction(password));
    cout << hash << endl;
    string request = "SELECT users.hash_pswrd FROM users WHERE users.username = '" + username + "'";
    pqxx::connection c("user=tester password=testPassword1 host=172.16.1.4 port=5432 dbname=tester target_session_attrs=read-write");//Переменные окружения
    pqxx::work db(c);
    for (auto [origHash]: db.query<string>(request)) if (origHash == hash) return true;
    return false;
}

bool checkAccessRights(const string& username) {
    string request = "SELECT users.right_user FROM users WHERE users.username = '" + username + "';";
    pqxx::connection c("user=tester password=testPassword1 host=172.16.1.4 port=5432 dbname=tester target_session_attrs=read-write");//Переменные окружения
    pqxx::work db(c);
    string result = "";
    for (auto [right]: db.query<string>(request)) {
        result = right;
    }
    if (result == "admin") return true;
    if (result == "regular") return false;
}

void userAuthorization(int32_t clientSocket, Configuration& configuration) {
    string response = startMenu();
    response += authorizationMenu();
    send(clientSocket, response.c_str(), response.length(), 0);//Свернуть в одну функцию
    string userName = readClientsRequest(clientSocket);
    if (userName == "Guest") {
        unique_ptr<UserInterface> user(new GuestUser("Guest", clientSocket));
        processingRequests(clientSocket, user, configuration);
    }
    else {
        response = "Enter the password (You have 3 attempts)";
        send(clientSocket, response.c_str(), response.length(), 0);
        int32_t amountOfAttempts = 3;
        while (amountOfAttempts != 0) {
            string passsword = readClientsRequest(clientSocket);
            if (passwordVerification(passsword, userName)) break;
            amountOfAttempts--;
        }
        if (amountOfAttempts == 0) {
            throw runtime_error("The attempts are over");
        }
        UserInterface* user = new GuestUser(userName, clientSocket);
        if (checkAccessRights(userName)) {
        unique_ptr<UserInterface> admin(new AdminUser(user));
        processingRequests(clientSocket, admin, configuration);
        }
        else {
        unique_ptr<UserInterface> regularUser(new RegularUser(user));
        processingRequests(clientSocket, regularUser, configuration);
        }
    }
}   

void startingServer(Configuration& configuration) {
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

        thread t(userAuthorization, clientSocket, ref(configuration));
		t.detach();
    }
	close(serverSocket);
}

int main() {
    try {
	    setlocale(LC_ALL, "RUSSIAN");
        Configuration configuration;
        configuration.readConfiguration("/configuration/configuration.json");//путь сделать в переменную среду
        startingServer(configuration);
    }
    catch (exception &e) {
		    cout << e.what();
	    }
}
