//Работа с файлами исправить добавление последней строки /n за /n? 
//Картинку хоккеиста побольше
//Сделать чтобы всегда выводилось имя и фамилия
//Редакция игрока
#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <arpa/inet.h>
#include <unistd.h>
#include <thread>
#include <bits/stdc++.h>
#include <pqxx/pqxx>

#include "User.h"
#include "Configuration.h"

using namespace std;

string startMenu() {
    string message = "menu\nAuthorization\nPlease enter your username and click \"OK\"\nTo log in without registration, write \"Guest\" and click \"OK\"";
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
            result += f.as<string>() + "\t";
        }
        result += '\n';
    }
    return result;
}

string SendRequestInDB(string& requestInDB, Configuration& configuration) {
    if (requestInDB == "") return "*";
    if (requestInDB == "!") return "Incorrect request";
    if (requestInDB[0] == '#') return requestInDB.erase(0, 1);
    string newRequestInDB = requestInDB + ";";
    pqxx::connection c("user=" + configuration.POSTGRES_USER + " password=" + configuration.POSTGRES_PASSWORD + " host=" + configuration.POSTGRES_HOST + " port=" + configuration.POSTGRES_PORT + " dbname=" + configuration.POSTGRES_USER + " target_session_attrs=read-write");
    pqxx::work db(c);
    pqxx::result res = db.exec(newRequestInDB);
    createLog("A request has been sent to the database: " + newRequestInDB);
    db.commit();
    string result;
    result = createLineToSend(res);
    c.close();
    return result;
}

void processingRequests(int32_t clientSocket, unique_ptr<UserInterface>& user, Configuration& configuration) {
    while (true) {
        string message = menu();
        send(clientSocket, message.c_str(), message.length(), 0);
        string request = readClientsRequest(clientSocket);
        int32_t inputData = stoi(request);
        string requestInDB;
        string columnsForOutput = "";
        if (inputData == 1) requestInDB = user->printingTableData(configuration, columnsForOutput);
        else if (inputData == 2) requestInDB = user->printStatOfTeamsPlayers(configuration, columnsForOutput);
        else if (inputData == 3) requestInDB = user->printingTheBestPlayers(configuration, columnsForOutput);
        else if (inputData == 4) requestInDB = user->playerSearch(configuration, columnsForOutput);
        else if (inputData == 5) requestInDB = user->changeTableData(configuration);
        else if (inputData == 6) requestInDB = user ->registration(configuration);
        else {
            disconnectClient(clientSocket);
            return;
        }
        message = SendRequestInDB(requestInDB, configuration);
        if (message == "*") {
            disconnectClient(clientSocket);
            return;
        }
        string fullMessage = columnsForOutput + message;
        if (message == "" and inputData != 6) fullMessage = "Nothing was found";
        if (message == "" and inputData == 6) fullMessage = "menu\nYou are registered";
        fullMessage += "\nTo send the next request, write \"Next\" and click \"OK\"";
        send(clientSocket, fullMessage.c_str(), fullMessage.length(), 0);
        request = readClientsRequest(clientSocket);
        if (request != "Next") {
            disconnectClient(clientSocket);
            return;
        }
    }
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
    response = readClientsRequest(clientSocket);
    response = startMenu();
    send(clientSocket, response.c_str(), response.length(), 0);
    string userName = readClientsRequest(clientSocket);
    if (userName == "Guest") {
        unique_ptr<UserInterface> user(new GuestUser("Guest", clientSocket));
        createLog("The user logged in as a guest");
        processingRequests(clientSocket, user, configuration);
    }
    else {
        response = "menu\nEnter the password (You have 3 attempts)";
        send(clientSocket, response.c_str(), response.length(), 0);
        int32_t amountOfAttempts = 3;
        while (amountOfAttempts != 0) {
            string passsword = readClientsRequest(clientSocket);
            if (passwordVerification(passsword, userName, configuration)) break;
            amountOfAttempts--;
            response = "menu\nThe password was entered incorrectly\nPlease try again\nThere are still attempts left: " + to_string(amountOfAttempts);
            send(clientSocket, response.c_str(), response.length(), 0);
        }
        if (amountOfAttempts == 0) {
            createLog("The attempts are over: " + userName);
            disconnectClient(clientSocket);
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
