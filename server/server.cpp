//Подключение сервера и клиента через ip
//При добавлении через админа изменять главную таблицу
//Безопасностьт для user и hash
//При поиске игрока выводить что нет игрока если нет
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
    message += "[2] - Printing statistics of the team's players\n";
    message += "[3] - Printing the best players\n";
    message += "[4] - Player Search\n";
    message += "[5] - Changing table data (only admin)\n";
    message += "[6] - Add a favorite player (only regular user)\n";
    message += "[7] - Registration (only guest)\n";
    message += "[8] - Exit";
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
    virtual string PrintingTheBestPlayers(Configuration& configuration) const = 0;
    virtual string playerSearch(Configuration& configuration) const = 0;
    virtual string printStatOfTeamsPlayers(Configuration& configuration) const = 0;
    virtual string changeTableData(Configuration& configuration) const = 0;
    virtual string addFavoritePlayer(Configuration& configuration) const = 0;
private:
    virtual void printNumberTableForStat(Configuration& configuration) const = 0;
    virtual void printMessageToSelectTeam() const = 0;
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
    GuestUser(): userName("nobody"), clientSocket(0) {}//???

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

    string PrintingTheBestPlayers(Configuration& configuration) const override {
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
        printMessageForLastNameInput();
        string playerLastName = readClientsRequest(clientSocket);
        string tableName = configuration.tableNames[inputData - 1];
        string columnName = getColumnForSearch(configuration, tableName);
        string result = "SELECT * FROM " + tableName + " WHERE " + tableName + "." + columnName + " = '" +  playerName + " " + playerLastName + "'";
        return result;
    }

    string printStatOfTeamsPlayers(Configuration& configuration) const override {
        printMessageToSelectTeam();
        string teamName = readClientsRequest(clientSocket);
        int32_t numberTable = stoi(readClientsRequest(clientSocket));
        if (numberTable >= 3) {
            throw runtime_error("Incorrect input data");
            return "";
        }
        string result = "SELECT * FROM " + configuration.tableNames[numberTable] + " WHERE " + configuration.tableNames[numberTable] + "." + configuration.columnNames[configuration.tableNames[numberTable]][2] + " = '" + teamName + "'";
        return result;
    }

    string changeTableData(Configuration& configuration) const override {
        string message = "You are not an administrator";
        send(clientSocket, message.c_str(), message.length(), 0);
        return "";
    }

    string addFavoritePlayer(Configuration& configuration) const override {
        string message = "You are not logged in";
        send(clientSocket, message.c_str(), message.length(), 0);
        return "";
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
        message += "Select the columns to print\n";
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
    string PrintingTheBestPlayers(Configuration& configuration) const override {
        return this->userInterface_->PrintingTheBestPlayers(configuration);
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
    string addFavoritePlayer(Configuration& configuration) const override {
        return this->userInterface_->addFavoritePlayer(configuration);
    }
};

class AdminUser: public Decorator {
public:
    AdminUser(UserInterface* userInterface): Decorator(userInterface) {}

    string changeTableData(Configuration& configuration) const override {
        return "";
    }

    string addFavoritePlayer(Configuration& configuration) const override {
        string message = "You are not logged in";
        send(clientSocket, message.c_str(), message.length(), 0);
        return "";
    }
};

class RegularUser: public Decorator {
public:
    RegularUser(UserInterface* userInterface): Decorator(userInterface) {}

    string changeTableData(Configuration& configuration) const override {
        string message = "You are not an Administrator";
        send(clientSocket, message.c_str(), message.length(), 0);
        return "";
    }

    string addFavoritePlayer(Configuration& configuration) const override {
        return "";
    }
};

void createDataBase(pqxx::work& db, Configuration& configuration) {
    pqxx::result r{db.exec("SELECT table_name FROM information_schema.tables WHERE table_schema = '" + configuration.dataBaseName + "' AND table_name = '" + configuration.tableNames[0] + "';")};
    if (!r.empty()) cout << "таблица уже создана" << endl;
    else {
        r = db.exec("SELECT * FROM forwards");
        db.commit();
        for (auto const &row: r) {
            for (auto const &field: row) std::cout << field.c_str() << '\t';
            std::cout << '\n';
        }
    }
    
}

string createLineToSend(pqxx::result& res, int32_t amountOfColumn) {//возможно удалить количество 
    string result = "";
    for (auto const &row: res) {
            for (auto const &field: row)  {
                result += field.c_str() + '\t';
                cout << field.c_str() + '\t';
            }
            result += '\n';

            cout << endl;
        }
    return result;
}

string SendRequestInDB(const string& requestInDB, int32_t amountOfColumns) {
    string NrequestInDB = requestInDB + ";";//Delete
    pqxx::connection c("user=tester password=testPassword1 host=172.16.1.4 port=5432 dbname=tester target_session_attrs=read-write");//Переменные окружения
    pqxx::work db(c);
    pqxx::result res = db.exec(NrequestInDB);//Изменить имя
    //pqxx::result r = db.exec("SELECT * FROM forwards");
    db.commit();
    string result = "";
    for (auto row = begin(res); row != end(res); row++) {
        for (auto field = begin(row); field != end(row); field++) cout << field->c_str() << '\t';
    cout << '\n';
    }
    cout << "уэээээээээ" << endl;//Delete
    //cout << result << endl;//Delete
    //result = createLineToSend(res, amountOfColumns);
    c.close();
    return result;
}

void processingRequests(int32_t clientSocket, unique_ptr<UserInterface>& user, Configuration& configuration) {
    //string message = "You have successfully logged in\n";
    string message = menu();
    send(clientSocket, message.c_str(), message.length(), 0);
    while (true) {
        string request = readClientsRequest(clientSocket);
        int32_t inputData = stoi(request);
        cout << inputData << endl;//Delete
        string requestInDB;
        int32_t amountOfColumns;
        if (inputData == 1) requestInDB = user->printingTableData(configuration, amountOfColumns);
        else if (inputData == 2) requestInDB = user->printStatOfTeamsPlayers(configuration);
        else if (inputData == 3) requestInDB = user->PrintingTheBestPlayers(configuration);//Убрать большую букву
        else if (inputData == 4) requestInDB = user->playerSearch(configuration);
        else if (inputData == 8) break;
        cout << requestInDB << endl;//Delete
        /*else if (inputData == 5)
        else if (inputData == 6)
        else if (inputData == 7)*/
        message = SendRequestInDB(requestInDB, amountOfColumns);
        message += menu();
        send(clientSocket, message.c_str(), message.length(), 0);
    }
}

int64_t hashFunction(const string& password) {
    hash<string> hash;
    string salt = "salt";
    return hash(password + salt);
}

void userAuthorization(int32_t clientSocket, Configuration& configuration) {
    string response = startMenu();
    send(clientSocket, response.c_str(), response.length(), 0);
    int32_t amountOfAttempts = 3;
    while (amountOfAttempts != 0) {
        amountOfAttempts--;
    }
    string request = readClientsRequest(clientSocket);//Delete
    //UserInterface* user = new GuestUser("aboba", clientSocket);
    unique_ptr<UserInterface> user(new GuestUser("aboba", clientSocket));

    processingRequests(clientSocket, user, configuration);
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
        //createDataBase(db, configuration);
        startingServer(configuration);
    }
    catch (exception &e) {
		    cout << e.what();
	    }
}
