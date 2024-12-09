#include <iostream>
#include <arpa/inet.h>
#include <unistd.h>

#include <gtk-3.0/gtk/gtk.h>

using namespace std;

GtkTextBuffer* textBuffer_1;
GtkTextBuffer* textBuffer_2;
GtkButton* button;
GtkEditable* inputText;

int32_t clientSocket = socket(AF_INET, SOCK_STREAM, 0);
sockaddr_in serverAddress;

string input;
bool isWaitStr = false;

extern "C" void topWindow_destroy(GtkWidget* w) {
    gtk_main_quit();
}

extern "C" void on_button_clicked(GtkButton* button, gpointer user_data) {
    gchar* inputtxt = gtk_editable_get_chars(inputText, 0, -1);
    input = inputtxt; 
    send(clientSocket, input.c_str(), input.length(), 0);
    gtk_editable_delete_text(inputText, 0, -1);
    char buffer[1024] = {0};
    int32_t bytes_received = read(clientSocket, buffer, sizeof(buffer));
    if (bytes_received > 0) {
        if (buffer[0] == 'm' and buffer[1] == 'e' and buffer[2] == 'n' and buffer[3] == 'u') {
            gtk_text_buffer_set_text(textBuffer_1, buffer, -1);
        }
        else gtk_text_buffer_set_text(textBuffer_2, buffer, -1);
    }
}

int main(int argc, char *argv[]) {
    try {

        gtk_init(&argc, &argv);
        GtkBuilder * ui_builder;
        GError * err = NULL;
        ui_builder = gtk_builder_new();
        if(!gtk_builder_add_from_file(ui_builder, "/home/igor/myAppGUI.glade", &err)) {
            g_critical ("Не вышло загрузить файл с UI : %s", err->message);
            g_error_free (err);
        }
        GtkWidget * window = GTK_WIDGET(gtk_builder_get_object(ui_builder, "main_window"));
        button = GTK_BUTTON(gtk_builder_get_object(ui_builder, "button"));
        inputText = GTK_EDITABLE(gtk_builder_get_object(ui_builder, "input_text"));
        textBuffer_1 = GTK_TEXT_BUFFER(gtk_builder_get_object(ui_builder, "textBuffer_1"));
        textBuffer_2 = GTK_TEXT_BUFFER(gtk_builder_get_object(ui_builder, "textBuffer_2"));
        gtk_widget_show(window);
        gtk_builder_connect_signals(ui_builder, NULL);

        serverAddress.sin_family = AF_INET;
        serverAddress.sin_port = htons(7432);
        serverAddress.sin_addr.s_addr = INADDR_ANY;
        if (connect(clientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) throw runtime_error("Error conection");
        if (clientSocket < 0) throw runtime_error("failed to accept the data");

        gtk_main();
        close(clientSocket);

         
    }
    catch (exception& e) {
        cout << e.what() << endl;
    }
}
