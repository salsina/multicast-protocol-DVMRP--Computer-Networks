#include <iostream>
#include <vector>
#include <string>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h> 
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <sys/time.h>
#include <bits/stdc++.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/select.h>

using namespace std;

vector<vector<string>> lookupTable;
vector<string> connections;
vector<bool> enables;
vector<string> routerOrClient;
string routerIP;

vector<string> group_names;
vector<vector<string>> clientIPs_in_group;

void initialLookUpTable(int numOfPort){
    for (int i=0;i < numOfPort;i++){
        lookupTable.push_back(vector<string> ());
        connections.push_back("");
        routerOrClient.push_back("client");
        enables.push_back(false); 
    }
}

int find_group_index_by_name(string name){
    for (int i=0;i<group_names.size();i++){
        if (group_names[i] == name)
            return i;
    }
    return -1;
}

int add_client_to_group(int idx, string clientIP){
    for(int i=0; i < clientIPs_in_group[idx].size(); i++)
        if(clientIPs_in_group[idx][i] == clientIP)
            return -1;

    clientIPs_in_group[idx].push_back(clientIP);
    return 0;
}

void setSrcPort(int portNumber,string src,string determine = "client"){ 
    connections[portNumber - 1] = src;
    routerOrClient[portNumber - 1] = determine;
}

void writeInFileName(int fd,string msg){
    char msgChar[1024];
    strcpy(msgChar, msg.c_str());
    write(fd,msgChar,1025);
}

vector<string> remove_from_vector(vector<string> input_vec, string element){
    vector<string> output_vec;
    bool firstTimeDel = false;
    for (int i=0; i < input_vec.size(); i++){
        if(input_vec[i] != element || firstTimeDel)
            output_vec.push_back(input_vec[i]);
        else if(!firstTimeDel)
            firstTimeDel = true;
    }
    return output_vec;
}

void connect(int routerfd,char* ans){
    bzero(ans,1025);
    read(routerfd,ans,1025);
    string client_ip(ans);
    bzero(ans,1025);
    read(routerfd,ans,1025);
    int port = atoi(ans);

    setSrcPort(port, client_ip);
    
    cout << "Connection of client with IP " << client_ip << " to router " << routerIP << " by port " << port
            << " was successful!" << endl;
}

void connectrouter(int routerfd,char* ans){
    bzero(ans,1025);
    read(routerfd,ans,1025);
    string routerIP2 (ans);
    bzero(ans,1025);
    read(routerfd,ans,1025);
    int port = atoi(ans);

    setSrcPort(port,routerIP2,"router");

    cout << "Router " << routerIP << ": router with IP " << routerIP2 << " is connected at port " << port<<endl;
}

void disconnectrouter(int routerfd,char* ans){
    bzero(ans,1025);
    read(routerfd,ans,1025);
    string routerIP2 (ans);
    
    for(int i=0; i<connections.size(); i++){
        if (connections[i] == routerIP2){
            connections[i] = "";
            vector<string> tmp;
            lookupTable[i] =  tmp;
            routerOrClient[i] = "client";
            enables[i] = false;
            cout << "Router " << routerIP << ": router with IP " << routerIP2 << " is now disconnected\n";
            return;
        }
    }
    cout << "Router " << routerIP << ": router with IP " << routerIP2 << " not found!\n";
}

void notifyConnectedRouters(string action, string groupName, int port){
    for (int i=0;i<connections.size(); i++){
        if(i == port) continue;
        if(connections[i] == "") continue;
        if(routerOrClient[i] == "client") continue;


        string routerFileName = "cache/RT" + connections[i];

        int fd = open(routerFileName.c_str(),O_WRONLY);
        if (fd == -1){
            cerr << "Could not open router's file!" << endl;
            return;
        }

        writeInFileName(fd, action);
        writeInFileName(fd, routerIP);
        writeInFileName(fd, groupName);
        close(fd);
    }
}

void new_group_join(int routerfd,char* ans){
    bzero(ans,1025);
    read(routerfd,ans,1025);
    string IP(ans);
    bzero(ans,1025);
    read(routerfd,ans,1025);
    string groupName(ans);

    for (int i=0;i<connections.size();i++){
        if (connections[i] == IP && routerOrClient[i] == "client"){
            int port = i;
            lookupTable[port].push_back(groupName);
            cout << "Router " << routerIP << ": client with IP " << IP <<" on port "<< port + 1 << " joined group " <<groupName<<endl;
            notifyConnectedRouters("new_group_join",groupName, port);
            return;
        }
        else if(connections[i] == IP && routerOrClient[i] == "router"){
            int port = i;
            lookupTable[port].push_back(groupName);
            cout << "Router " << routerIP << ": router with IP " << IP <<" on port "<< port + 1 << " joined group " <<groupName<<endl;
            notifyConnectedRouters("new_group_join", groupName, port);
            return;
        }
    }
    cout << "Router " << routerIP << ": could not join client/router with IP " << IP << " to the group " << groupName <<endl;
}


void new_group_leave(int routerfd,char* ans){
    bzero(ans,1025);
    read(routerfd,ans,1025);
    string IP(ans);
    bzero(ans,1025);
    read(routerfd,ans,1025);
    string groupName(ans);

    for (int i=0;i<connections.size();i++){
        if (connections[i] == IP && routerOrClient[i] == "client"){
            int port = i;
            lookupTable[port] = remove_from_vector(lookupTable[port], groupName);
            cout << "Router " << routerIP << ": client with IP " << IP <<" on port "<< port + 1 << " left group " <<groupName<<endl;
            notifyConnectedRouters("new_group_leave", groupName, port);
            return;
        }
        else if(connections[i] == IP && routerOrClient[i] == "router"){
            int port = i;
            lookupTable[port] = remove_from_vector(lookupTable[port], groupName);
            cout << "Router " << routerIP << ": router with IP " << IP <<" on port "<< port + 1 << " left group " <<groupName<<endl;
            notifyConnectedRouters("new_group_leave", groupName, port);
            return;
        }
    }
    cout << "Router " << routerIP << ": could not join client/router with IP " << IP << " to the group " << groupName <<endl;
}


void send_to_port(int idx, string fileName, string groupName){
    string IP = connections[idx];

    string destFileName ;

    if(routerOrClient[idx] == "router")
        destFileName = "cache/RT" + IP;
    else 
        destFileName = "cache/CL" + IP;

    int fd = open(destFileName.c_str(),O_WRONLY);
    if (fd == -1){
        cerr << "Could not open dest file!" << endl;
        return;
    }

    if(routerOrClient[idx] == "router"){
        writeInFileName(fd, "send");
        writeInFileName(fd, fileName);
        writeInFileName(fd, groupName);
        writeInFileName(fd, routerIP);
    }
    else{
        writeInFileName(fd, "new_message");
        writeInFileName(fd, fileName);
        writeInFileName(fd, groupName);
    }
    close(fd);
}

int find_port_by_IP(string IP){
    for (int i=0;i<connections.size(); i++){
        if (connections[i] == IP)
            return i;
    }
    return -1;
}

void send(int routerfd,char* ans){
    bzero(ans,1025);
    read(routerfd,ans,1025);
    string fileName(ans);
    bzero(ans,1025);
    read(routerfd,ans,1025);
    string groupName(ans);
    bzero(ans,1025);
    read(routerfd,ans,1025);
    string clientIP(ans);

    int port = find_port_by_IP(clientIP);

    for (int i =0; i<lookupTable.size(); i++){
        if (i == port) continue;
        for(int j = 0; j< lookupTable[i].size(); j++){
            if (lookupTable[i][j] == groupName){
                send_to_port(i, fileName, groupName);
                break;
            }
        }
    }
}

int main(int argc,char** argv) {
    int numOfPorts;
    numOfPorts = atoi(argv[0]);
    routerIP = argv[1];
    cout << "Creation of router with IP " << routerIP <<" with "<< numOfPorts<<" ports was successful!" << endl;
    initialLookUpTable(numOfPorts);

    while(1) {
        int routerfd = open(("cache/RT" + routerIP).c_str(),O_RDONLY);
        char ans[1025];
        bzero(ans,1025);
        read(routerfd,ans,1025);
        if (strcmp(ans,"connect_client") == 0) connect(routerfd,ans);
        else if (strcmp(ans,"connect_router") == 0) connectrouter(routerfd,ans);
        else if (strcmp(ans,"disconnect_router") == 0) disconnectrouter(routerfd,ans);
        else if (strcmp(ans,"new_group_join") == 0) new_group_join(routerfd,ans);
        else if (strcmp(ans,"new_group_leave") == 0) new_group_leave(routerfd,ans);
        else if (strcmp(ans,"send") == 0) send(routerfd,ans);
        close(routerfd);
    }
}