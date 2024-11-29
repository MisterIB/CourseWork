//Подключение сервера и клиента через ip
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
    message += "[2] - Printing player statistics\n";
    message += "[3] - Printing the best players\n";
    message += "[4] - Player Search\n";
    message += "[5] - Changing table data (only admin)\n";
    message += "[6] - Add a favorite player (only regular user)\n";
    message += "[7] - Registration (only guest)\n";
    message += "[8] - Exit\n";
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
private:
    virtual void printListOfTables(Configuration& configuration) const = 0;
    virtual void printListOfColumnes(Configuration& configuration, const string& tableName) const = 0;
    virtual vector<string> getColumnNames(Configuration& configuration, const string& tableName, string& columnNumbers) const = 0;
    virtual string receiveDataFromDBToPrint(const vector<string>& columnNamesForPrint, const string& tableName) const = 0;
};

class GuestUser: public UserInterface {
public:
    string userName;
    int32_t clientSocket;

    GuestUser(string name, int32_t socket): userName(name), clientSocket(socket) {}
    GuestUser(): userName("nobody"), clientSocket(0) {}//???

    string printingTableData(Configuration& configuration, int32_t& amountOfColumns) const override{
        printListOfTables(configuration);
        int32_t inputData = stoi(readClientsRequest(clientSocket));
        string tableName = configuration.tableNames[inputData - 1];
        printListOfColumnes(configuration, tableName);
        string columnNumbers = readClientsRequest(clientSocket);
        vector<string> columnNamesForPrint = getColumnNames(configuration, tableName, columnNumbers);
        string result = receiveDataFromDBToPrint(columnNamesForPrint, tableName); 
        amountOfColumns = columnNamesForPrint.size();
        return result;
    }

private:
    void printListOfTables(Configuration& configuration) const override {
        string message = "";
        int32_t i = 1;
        message += "Select the table to print\n";
        for (string tableName: configuration.tableNames) {
            message += "[" + to_string(i++) + "] - " + tableName + "\n";
        }
        send(clientSocket, message.c_str(), message.length(), 0);
    }

    void printListOfColumnes(Configuration& configuration, const string& tableName) const override {
        string message = "";
        message += "Select the columns to print\n";
        int32_t i = 1;
        for (string columnName: configuration.columnNames[tableName]) {
            message += "[" + to_string(i++) + "] - " + columnName + "\n";
        }
        message += "[" + to_string(i++) + "] - " + "All\n";
        send(clientSocket, message.c_str(), message.length(), 0);
    }

    vector<string> getColumnNames(Configuration& configuration, const string& tableName, string& columnNumbers) const override {
        vector<string> columnNamesForPrint;
        set<int32_t> setColumnNumbers;
        istringstream columnNumbersStream(columnNumbers);
        string number;
        while (columnNumbersStream >> number) {
            setColumnNumbers.insert(stoi(number));
        }
        int32_t i = 1;
        if (setColumnNumbers.find(7) != setColumnNumbers.end()) {
            columnNamesForPrint.push_back("ALL");
            return columnNamesForPrint;

        }
        for (string columnName: configuration.columnNames[tableName]) {
            if (setColumnNumbers.find(i) != setColumnNumbers.end()) columnNamesForPrint.push_back(columnName);
        }
        return columnNamesForPrint;
    }

    string receiveDataFromDBToPrint(const vector<string>& columnNamesForPrint, const string& tableName) const override {
        string result;
        if (columnNamesForPrint[0] == "ALL") {
            //pqxx::result res{workWithDB.exec("SELECT * FROM forwards")};
            //workWithDB.exec("SELECT * FROM forwards");
            result = "SELECT * FROM " + tableName;
        }
        return result;
    }
};

class Decorator: public GuestUser {
protected:
    UserInterface* userInterface_;
public:
    Decorator(UserInterface* userInterface): userInterface_(userInterface) {}
    //Делегировать работу
};

class AdminUser: public Decorator {
public:
    AdminUser(UserInterface* userInterface): Decorator(userInterface) {}
};

class RegularUser: public Decorator {
public:
    RegularUser(UserInterface* userInterface): Decorator(userInterface) {}
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
    pqxx::connection c("user=tester password=testPassword1 host=172.16.1.4 port=5432 dbname=tester target_session_attrs=read-write");
    pqxx::work db(c);
    //pqxx::result res = db.exec(NrequestInDB);
    pqxx::result res = db.exec("SELECT team FROM forwards");
    c.close();
    string result;
    result = createLineToSend(res, amountOfColumns);
    return result;
}

void processingRequests(int32_t clientSocket, unique_ptr<UserInterface>& user, Configuration& configuration) {
    string message = "You have successfully logged in\n";
    message += menu();
    send(clientSocket, message.c_str(), message.length(), 0);
    string request = readClientsRequest(clientSocket);
    int32_t inputData = stoi(request);
    cout << inputData << endl;//Delete
    string requestInDB;
    int32_t amountOfColumns;
    if (inputData == 1) {
        cout << "Зашел" << endl;//delete
        requestInDB = user->printingTableData(configuration, amountOfColumns);
    }
    cout << requestInDB << endl;//Delete
    /*else if (inputData == 2)
    else if (inputData == 3)
    else if (inputData == 4)
    else if (inputData == 5)
    else if (inputData == 6)
    else if (inputData == 7)
    else if (inputData == 8)*/
    message = SendRequestInDB(requestInDB, amountOfColumns);
    send(clientSocket, message.c_str(), message.length(), 0);
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
        //pqxx::connection conn("user=tester password=testPassword1 host=172.16.1.4 port=5432 dbname=tester1 target_session_attrs=read-write");//Через переменные среды
        //pqxx::work db(conn);
        Configuration configuration;
        configuration.readConfiguration("/configuration/configuration.json");//путь сделать в переменную среду
        //createDataBase(db, configuration);
        //conn.close();
        startingServer(configuration);
    }
    catch (exception &e) {
		    cout << e.what();
	    }
}
