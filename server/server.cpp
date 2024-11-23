#include <iostream>
#include <arpa/inet.h>
#include <unistd.h>
#include <thread>

#include <pqxx/pqxx>
#include <fstream>

using namespace std;

void helloFunc() {
    ifstream conf("/configuration/configuration.json");
    if (conf.is_open()) {
        string str;
        cout << "start" << endl;
        while (conf >> str) {
            cout << str;
        }
        cout << "end" << endl;
    }
    conf.close();
}

int main() {
    try {
	    setlocale(LC_ALL, "RUSSIAN");
        cout << "start listen sql" << endl;
        pqxx::connection c("user=tester password=testPassword1 host=172.16.1.4 port=5432 dbname=tester target_session_attrs=read-write");
        pqxx::work w(c);
        pqxx::row r = w.exec1("SELECT 1");
        w.commit();
        std::cout << r[0].as<int>() << std::endl;

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
            thread t(helloFunc);
			t.detach();
        }
		close(serverSocket);
    }
    catch (exception &e) {
		    cout << e.what();
	    }
}
