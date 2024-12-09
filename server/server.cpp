//Подключение сервера и клиента через ip
//При поиске игрока выводить что нет игрока если нет
//Удалить капс у лога
#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <arpa/inet.h>
#include <unistd.h>
#include <thread>
#include <bits/stdc++.h>
#include <pqxx/pqxx>
#include <chrono>
#include <ctime>

#include "User.h"
#include "Configuration.h"

using namespace std;

void createLog(string message) {
    auto tm = chrono::system_clock::now();
    time_t end_time = chrono::system_clock::to_time_t(tm);
    //cout << ctime(&end_time) << " LOG: " << message << endl;
    cout << " LOG: " << message << endl;
}

string startMenu() {
    string message = "menu\nPlease enter your username";
    return message;
}

string menu() {
    string message = "menu\n";
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

string createLineToSend(pqxx::result& res) {
    string result = "";
    for (auto r: res) {
        for (auto f: r){
            result += f.as<string>() + " ";
        }
        result += '\n';
    }
    return result;
}

string SendRequestInDB(const string& requestInDB, Configuration& configuration) {
    if (requestInDB == "") return "";
    string newRequestInDB = requestInDB + ";";
    pqxx::connection c("user=" + configuration.POSTGRES_USER + " password=" + configuration.POSTGRES_PASSWORD + " host=" + configuration.POSTGRES_HOST + " port=" + configuration.POSTGRES_PORT + " dbname=" + configuration.POSTGRES_USER + " target_session_attrs=read-write");
    pqxx::work db(c);
    pqxx::result res = db.exec(newRequestInDB);
    db.commit();
    string result;
    result = createLineToSend(res);
    c.close();
    return result;
}

void disconnectClient(int32_t clientSocket) {
    string message = "You are disconnected from the server";
    send(clientSocket, message.c_str(), message.length(), 0);
    close(clientSocket);
}

void processingRequests(int32_t clientSocket, unique_ptr<UserInterface>& user, Configuration& configuration) {
    while (true) {
        string message = menu();
        send(clientSocket, message.c_str(), message.length(), 0);
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
            disconnectClient(clientSocket);
            return;
        }
        /*else if (inputData == 6)*/
        else {
            disconnectClient(clientSocket);
            return;
        }
        message = SendRequestInDB(requestInDB, configuration);
        createLog("A request has been sent to the database: " + requestInDB);
        if (message == "") {
            close(clientSocket);
            return;
        }
        message += "\nTo send the next request, write \"Next\" and click \"OK\"";
        send(clientSocket, message.c_str(), message.length(), 0);
        request = readClientsRequest(clientSocket);
        if (request != "Next") {
            disconnectClient(clientSocket);
            return;
        }
    }
}

int64_t hashFunction(const string& password) {
    hash<string> hash;
    string salt = "salt";
    return hash(password + salt);
}

bool passwordVerification(const string& password, const string& username, Configuration& configuration) {
    string hash = to_string(hashFunction(password));
    cout << hash << endl;
    string request = "SELECT users.hash_pswrd FROM users WHERE users.username = '" + username + "'";
    pqxx::connection c("user=" + configuration.POSTGRES_USER + " password=" + configuration.POSTGRES_PASSWORD + " host=" + configuration.POSTGRES_HOST + " port=" + configuration.POSTGRES_PORT + " dbname=" + configuration.POSTGRES_USER + " target_session_attrs=read-write");
    pqxx::work db(c);
    for (auto [origHash]: db.query<string>(request)) if (origHash == hash) return true;
    return false;
}

bool checkAccessRights(const string& username, Configuration& configuration) {
    string request = "SELECT users.right_user FROM users WHERE users.username = '" + username + "';";
    pqxx::connection c("user=" + configuration.POSTGRES_USER + " password=" + configuration.POSTGRES_PASSWORD + " host=" + configuration.POSTGRES_HOST + " port=" + configuration.POSTGRES_PORT + " dbname=" + configuration.POSTGRES_USER + " target_session_attrs=read-write");
    pqxx::work db(c);
    string result = "";
    for (auto [right]: db.query<string>(request)) {
        result = right;
    }
    if (result == "admin") return true;
    if (result == "regular") return false;
}

void userAuthorization(int32_t clientSocket, Configuration& configuration) {
    string response;
    response = readClientsRequest(clientSocket);// изменить
    response = startMenu();
    send(clientSocket, response.c_str(), response.length(), 0);
    string userName = readClientsRequest(clientSocket);
    if (userName == "Guest") {
        unique_ptr<UserInterface> user(new GuestUser("Guest", clientSocket));
        createLog("The user logged in as a guest");
        processingRequests(clientSocket, user, configuration);
    }
    else {
        response = "Enter the password (You have 3 attempts)";
        send(clientSocket, response.c_str(), response.length(), 0);
        int32_t amountOfAttempts = 3;
        while (amountOfAttempts != 0) {
            string passsword = readClientsRequest(clientSocket);
            if (passwordVerification(passsword, userName, configuration)) break;
            amountOfAttempts--;
        }
        if (amountOfAttempts == 0) {
            throw runtime_error("The attempts are over");
        }
        UserInterface* user = new GuestUser(userName, clientSocket);
        if (checkAccessRights(userName, configuration)) {
        unique_ptr<UserInterface> admin(new AdminUser(user));
        createLog("The user " + userName + " is logged in");
        processingRequests(clientSocket, admin, configuration);
        }
        else {
        unique_ptr<UserInterface> regularUser(new RegularUser(user));
        createLog("The user " + userName + " is logged in");
        processingRequests(clientSocket, regularUser, configuration);
        }
    }
}   

void startingServer(Configuration& configuration) {
    int32_t serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == 0) throw runtime_error("Error create socket");

    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    const char* PORT_value = getenv("PORT");
    if (PORT_value == nullptr) throw runtime_error("Environment variable not found");
    serverAddress.sin_port = htons(stoi(PORT_value));
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    createLog("The server is listening");
    if (bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) throw runtime_error("Address binding error");
    if (listen(serverSocket, 3) < 0) throw runtime_error("Listening error");
    
    while (true) {
        int32_t clientSocket = accept(serverSocket, nullptr, nullptr);
        if (clientSocket < 0) throw runtime_error("failed to accept the data");

        createLog("A new client has connected: " + to_string(clientSocket));

        thread t(userAuthorization, clientSocket, ref(configuration));
		t.detach();
    }
	close(serverSocket);
}

int main() {
    try {
	    setlocale(LC_ALL, "RUSSIAN");
        const char* PATH_CONF_value = getenv("PATH_CONF");
        if (PATH_CONF_value == nullptr) throw runtime_error("Environment variable not found");
        Configuration configuration;
        configuration.readConfiguration(PATH_CONF_value);
        createLog("The server is starting");
        startingServer(configuration);
        createLog("The server is turned off");
    }
    catch (exception &e) {
		    cout << e.what();
	    }
}
