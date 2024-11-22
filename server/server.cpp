#include <iostream>
#include <arpa/inet.h>
#include <unistd.h>
#include <thread>

using namespace std;

void helloFunc() {
    cout << "Hello world!!!" << endl;
}

int main() {
    try {
	    setlocale(LC_ALL, "RUSSIAN");

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
