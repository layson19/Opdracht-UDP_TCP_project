#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> // for sleep function

int main()
{
    // start udp server in the background
    system("start /b udp_server.exe");
    
    // wait for the server to start
    sleep(1);
    
    // start udp client in the background
    system("start /b udp_client.exe");
    
    // wait for the client to finish communication with the server
    sleep(35); // for example, you can adjust the sleep time as per your requirement
    
    // start tcp server in the background
    system("start /b tcp_server.exe");
    
    // wait for the server to start
    sleep(1);
    
    // start tcp client in the background
    system("start /b tcp_client.exe");
    
    #ifdef _WIN32
        system("pause");
    #endif
        return 0;  
}