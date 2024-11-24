#include <iostream>
#include <arpa/inet.h>
#include <unistd.h>

using namespace std;

int main() {
    try {
        int32_t clientSocket = socket(AF_INET, SOCK_STREAM, 0);
        sockaddr_in serverAddress;
        serverAddress.sin_family = AF_INET;
        serverAddress.sin_port = htons(7432);
        serverAddress.sin_addr.s_addr = INADDR_ANY;
        if (connect(clientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) throw runtime_error("Error conection");
        cout << "You have connected to the server" << endl;
        while (true) {
            char buffer[1024] = {0};
            int32_t bytes_received = read(clientSocket, buffer, sizeof(buffer));
            if (bytes_received > 0) cout << buffer << endl;    
        }
        close(clientSocket);
    }
    catch (exception& e) {
        cout << e.what() << endl;
    }
}
