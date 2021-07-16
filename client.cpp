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

string RouterIP;
string selected_group_name;
vector<string> group_names;
bool signed_in;
string client_IP;

void writeInrouterFileName(int fd,string msg){
    char msgChar[1024];
    strcpy(msgChar, msg.c_str());
    write(fd,msgChar,1025);
}

vector<string> remove_from_vector(vector<string> input_vec, string element){
    vector<string> output_vec;
    for (int i=0; i < input_vec.size(); i++){
        if(input_vec[i] != element)
            output_vec.push_back(input_vec[i]);
    }
    return output_vec;
}

void connect(int ClientFD,char* ans){
    bzero(ans,1025);
    read(ClientFD,ans,1025);
    string RTIP(ans);
    RouterIP = RTIP;
}

void notifyConnectedRouter(string action, string GroupName){
    string routerFileName = "cache/RT" + RouterIP;

    int fd = open(routerFileName.c_str(),O_WRONLY);
    if (fd == -1){
        cerr << "Could not open router's file!" << endl;
        return;
    }

    writeInrouterFileName(fd, action);
    writeInrouterFileName(fd, client_IP);
    writeInrouterFileName(fd, GroupName);
    close(fd);
}

void join(int ClientFD,char* ans){
    bzero(ans,1025);
    read(ClientFD,ans,1025);
    string gp_name(ans);
    group_names.push_back(ans);
    notifyConnectedRouter("new_group_join", gp_name);
    cout<<"You have joined the group "<< gp_name<<" successfuly\n";
}

void leave(int ClientFD,char* ans){
    bzero(ans,1025);
    read(ClientFD,ans,1025);
    string gp_name(ans);
    group_names = remove_from_vector(group_names, gp_name);
    notifyConnectedRouter("new_group_leave", gp_name);
    cout<<"You have left the group "<< gp_name<<" successfuly\n";
}

void select(int ClientFD,char* ans){
    bzero(ans,1025);
    read(ClientFD,ans,1025);
    string gp_name(ans);
    selected_group_name = gp_name;
    cout<<"You have selected the group "<< gp_name<<" successfuly\n";
}

void signIn(){
    if(signed_in)
        cout<<"CLient with IP " << client_IP<<" has already signed in!\n";
    else{
        signed_in = true;
        cout<<"CLient with IP " << client_IP<<" signed in successfully\n";
    }
}

void signOut(){
    if(!signed_in)
        cout<<"CLient with IP " << client_IP<<" has already signed out!\n";
    else{
        signed_in = false;
        cout<<"CLient with IP " << client_IP<<" signed out successfully\n";
    }
}

void sendFile(int ClientFD,char* ans){
    bzero(ans,1025);
    read(ClientFD,ans,1025);
    string fileName(ans);

    bzero(ans,1025);
    read(ClientFD,ans,1025);
    string gp_name(ans);

    string routerFileName = "cache/RT" + RouterIP;

    int fd = open(routerFileName.c_str(),O_WRONLY);
    if (fd == -1){
        cerr << "Could not open router's file!" << endl;
        return;
    }

    writeInrouterFileName(fd, "send");
    writeInrouterFileName(fd, fileName);
    writeInrouterFileName(fd, gp_name);
    writeInrouterFileName(fd, client_IP);
    close(fd);
}

void newMessage(int ClientFD,char* ans){
    bzero(ans,1025);
    read(ClientFD,ans,1025);
    string fileName(ans);

    bzero(ans,1025);
    read(ClientFD,ans,1025);
    string gp_name(ans);

    cout<<"client with IP "<<client_IP<<": file "<<fileName<<" received from group "<<gp_name<<endl;
}


int main(int argc,char** argv) {
    string client_name = argv[0];
    client_IP = argv[1];
    cout << "Creation of client " << client_name << " with IP "<<client_IP << " was successful!" << endl;
    signed_in = true;

    while (1) {
        int ClientFD = open(("cache/CL" + client_IP).c_str(),O_RDONLY);
        char ans[1025];
        bzero(ans,1025);
        read(ClientFD,ans,1025);
        if (strcmp(ans,"connect_client") == 0) connect(ClientFD,ans);
        else if (strcmp(ans,"join") == 0) join(ClientFD,ans);
        else if (strcmp(ans,"leave") == 0) leave(ClientFD,ans);
        else if (strcmp(ans,"select") == 0) select(ClientFD,ans);
        else if (strcmp(ans,"sign_in") == 0) signIn();
        else if (strcmp(ans,"sign_out") == 0) signOut();
        else if (strcmp(ans,"create_group") == 0) join(ClientFD,ans);
        else if (strcmp(ans,"send_file") == 0) sendFile(ClientFD,ans);
        else if (strcmp(ans,"new_message") == 0) newMessage(ClientFD,ans);

        close(ClientFD);
    }
}