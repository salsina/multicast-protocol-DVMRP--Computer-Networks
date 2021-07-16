#include <iostream>
#include <bits/stdc++.h>
#include <vector>
#include <string>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/select.h>
#include <fstream>
#include <ftw.h>
#include <algorithm>
#include <signal.h>
#include <stdlib.h>

using namespace std;

vector<vector<int>> matrix;
vector<vector<int>> eachNodeConnections;
vector<string> routers;
vector<string> group_names;
vector<string> group_IPs;
vector<vector<string>> clientIPs_in_group;
vector<string> logs;
vector<string> client_ips;
vector<int> client_last_signout;

vector<string> splitCommand(string str) {
    vector<string> result;
    string word = "";
    for (auto x : str) {
        if (x == ' ') {
            result.push_back(word);
            word = "";
        }
        else word = word + x;
    }
    result.push_back(word);
    return result;
}

void showCmds(){
    cout << "Commands: " << endl;
    cout << "- router <number of ports> <router IP>" << endl;
    cout << "- client <client name> <client IP>" << endl;
    cout << "- sign_in <client IP>" << endl;
    cout << "- sign_out <client IP>" << endl;
    cout << "- connect_client <client IP> <router IP> <port number>" << endl;
    cout << "- connect_router <router1's IP> <router2's IP> <router1's port number> <router2's port number>" << endl;
    cout << "- disconnect_router <router1's IP> <router2's IP>" << endl;
    cout << "- get_group_list" << endl;
    cout << "- show_group <client IP>" << endl;
    cout << "- join <group name> <client IP>" << endl;
    cout << "- leave <group name> <client IP>" << endl;
    cout << "- select <group name> <client IP>" << endl;
    cout << "- create_group <group name> <multicast IP> <client IP>" << endl;
    cout << "- send_file <file name> <group name> <client IP>" << endl;
    cout << "- exit\n" << endl;
}

void writeInrouterFileName(int fd,string msg){
    char msgChar[1024];
    strcpy(msgChar, msg.c_str());
    write(fd,msgChar,1025);
}

void createrouter(vector<string> lineVec){
    for (int i=0;i<routers.size();i++)
        if (routers[i] == lineVec[2]){
            cout << "router with IP" << lineVec[2] << " has been already created!" << endl;
            return;
        }
    routers.push_back(lineVec[2]);

    int tmpIndx;
    for (int i=0;i<routers.size();i++)
        if (routers[i] == lineVec[2]){
            tmpIndx = i;
            break;
        }

    int pid = fork();
    if (pid == 0) {
        char *numOfPortsCstr = new char[lineVec[1].length() + 1];
        strcpy(numOfPortsCstr, lineVec[1].c_str());
        char *routerNumCstr = new char[lineVec[2].length() + 1];
        strcpy(routerNumCstr, lineVec[2].c_str());
        char* msg[] = {numOfPortsCstr,routerNumCstr,NULL};
        mkfifo(("cache/RT" + lineVec[2]).c_str(),0666);
        execvp("./router",msg);
    }
}

void createClient(vector<string> lineVec){
    int pid = fork();
    if (pid == 0){
        char *CliNameCstr = new char[lineVec[1].length() + 1];
        char *CliIPCstr = new char[lineVec[2].length() + 1];
        strcpy(CliNameCstr, lineVec[1].c_str());
        strcpy(CliIPCstr, lineVec[2].c_str());
        char* msg[] = {CliNameCstr,CliIPCstr,NULL};
        mkfifo(("cache/CL" + lineVec[2]).c_str(),0666);
        execvp("./client",msg);
    }
}

void connect(vector<string> lineVec){
    string routerFileName = "cache/RT" + lineVec[2];
    string clientFileName = "cache/CL" + lineVec[1];
    int routerFD = open(routerFileName.c_str(),O_WRONLY);
    if (routerFD == -1){
        cerr << "Wrong router IP. Try again!" << endl;
        return;
    }
    int clientFD = open(clientFileName.c_str(),O_WRONLY);
    if (clientFD == -1){
        cerr << "Wrong client IP. Try again!" << endl;
        return;
    }

    writeInrouterFileName(routerFD,lineVec[0]);
    writeInrouterFileName(routerFD,lineVec[1]);// client number
    writeInrouterFileName(routerFD,lineVec[3]);// port number
    close(routerFD);

    writeInrouterFileName(clientFD,lineVec[0]);
    writeInrouterFileName(clientFD,lineVec[2]);

    close(clientFD);
}


void connectrouter(vector<string> lineVec){
    string routerFileName1 = "cache/RT" + lineVec[1];
    string routerFileName2 = "cache/RT" + lineVec[2];
    string port1 = lineVec[3];
    string port2 = lineVec[4];

    int fd = open(routerFileName1.c_str(),O_WRONLY);
    if (fd == -1){
        cerr << "Wrong first router IP. Try again!" << endl;
        return;
    }
    int fd2 = open(routerFileName2.c_str(),O_WRONLY);
    if (fd2 == -1){
        cerr << "Wrong second router IP. Try again!" << endl;
        return;
    }

    writeInrouterFileName(fd,lineVec[0]);
    writeInrouterFileName(fd,lineVec[2]);
    writeInrouterFileName(fd,lineVec[3]);
    close(fd);

    writeInrouterFileName(fd2,lineVec[0]);
    writeInrouterFileName(fd2,lineVec[1]);
    writeInrouterFileName(fd2,lineVec[4]);
    close(fd2);
}

void disconnectrouter(vector<string> lineVec){
    string routerFileName1 = "cache/RT" + lineVec[1];
    string routerFileName2 = "cache/RT" + lineVec[2];

    int fd = open(routerFileName1.c_str(),O_WRONLY);
    if (fd == -1){
        cerr << "Wrong first router IP. Try again!" << endl;
        return;
    }
    int fd2 = open(routerFileName2.c_str(),O_WRONLY);
    if (fd2 == -1){
        cerr << "Wrong second router IP. Try again!" << endl;
        return;
    }


    writeInrouterFileName(fd,lineVec[0]);
    writeInrouterFileName(fd,lineVec[2]);
    close(fd);

    writeInrouterFileName(fd2,lineVec[0]);
    writeInrouterFileName(fd2,lineVec[1]);
    close(fd2);
}

void getGroupList(){

    if (group_names.size() == 0){
        cout<<"No group exists!\n";
        return;
    }
    cout<<"Available groups:\n";
    for (int i = 0; i< group_names.size(); i++){
        cout<<"Group name: "<<group_names[i]<< ", multicast IP: "<<group_IPs[i]<<endl;
    }
}

void showGroup(vector<string> lineVec){
    string clientFileName = "cache/CL" + lineVec[1];
    int fd = open(clientFileName.c_str(),O_WRONLY);
    if (fd == -1){
        cerr << "Wrong client IP. Try again!" << endl;
        return;
    }
    close(fd);

    string clientIP = lineVec[1];
    vector<int> group_indexes;
    for (int i=0; i < clientIPs_in_group.size(); i++){
        for (int j=0; j < clientIPs_in_group[i].size(); j++){
            if(clientIP == clientIPs_in_group[i][j]){
                group_indexes.push_back(i);
                break;
            }
        }  
    }

    if (group_indexes.size() == 0){
        cout<<"You have not joined in any groups!\n";
        return;
    }

    cout<<"You are joined in groups below:\n";
    for (int i = 0; i< group_indexes.size(); i++){
        cout<<"- "<<group_names[group_indexes[i]]<<endl;
    }
}

int find_group_index_by_name(string name){
    for (int i=0;i<group_names.size();i++){
        if (group_names[i] == name)
            return i;
    }
    return -1;
}

int find_group_index_by_IP(string IP){
    for (int i=0;i<group_IPs.size();i++){
        if (group_IPs[i] == IP)
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

vector<string> remove_from_vector(vector<string> input_vec, int idx){
    vector<string> output_vec;
    for (int i=0; i < input_vec.size(); i++){
        if(i!=idx)
            output_vec.push_back(input_vec[i]);
    }
    return output_vec;
}

vector<int> remove_from_vector_int(vector<int> input_vec, int idx){
    vector<int> output_vec;
    for (int i=0; i < input_vec.size(); i++){
        if(i!=idx)
            output_vec.push_back(input_vec[i]);
    }
    return output_vec;
}

int remove_client_from_group(int idx, string clientIP){
    for(int i=0; i < clientIPs_in_group[idx].size(); i++){
        if(clientIPs_in_group[idx][i] == clientIP){
            clientIPs_in_group[idx] = remove_from_vector(clientIPs_in_group[idx], i);
            return 0;
        }
    }
    return -1;
}

void signInClient(vector<string> lineVec){
    string clientIP = lineVec[1];
    string clientFileName = "cache/CL" + clientIP;

    int fd = open(clientFileName.c_str(),O_WRONLY);
    if (fd == -1){
        cerr << "Wrong client IP. Try again!" << endl;
        return;
    }

    writeInrouterFileName(fd,lineVec[0]);
    close(fd);
}

void signOutClient(vector<string> lineVec){
    string clientIP = lineVec[1];
    string clientFileName = "cache/CL" + clientIP;

    int fd = open(clientFileName.c_str(),O_WRONLY);
    if (fd == -1){
        cerr << "Wrong client IP. Try again!" << endl;
        return;
    }

    writeInrouterFileName(fd,lineVec[0]);

    client_ips.push_back(clientIP);
    client_last_signout.push_back(logs.size() - 1);

    close(fd);
}

void joinGroup(vector<string> lineVec){
    string groupName = lineVec[1];
    string clientIP = lineVec[2];
    string clientFileName = "cache/CL" + clientIP;

    int fd = open(clientFileName.c_str(),O_WRONLY);
    if (fd == -1){
        cerr << "Wrong client IP. Try again!" << endl;
        return;
    }

    int group_index = find_group_index_by_name(groupName);
    if (group_index == -1){
        cout<<"Wrong group name\n";
        return;
    }

    if (add_client_to_group(group_index, clientIP) == -1){
        cout<<"Client with IP "<<clientIP<<" has already joined in group "<< groupName << endl;
        return;
    }

    writeInrouterFileName(fd,lineVec[0]);
    writeInrouterFileName(fd,groupName);
    close(fd);
}

void leaveGroup(vector<string> lineVec){
    string groupName = lineVec[1];
    string clientIP = lineVec[2];
    string clientFileName = "cache/CL" + clientIP;

    int fd = open(clientFileName.c_str(),O_WRONLY);
    if (fd == -1){
        cerr << "Wrong client IP. Try again!" << endl;
        return;
    }

    int group_index = find_group_index_by_name(groupName);
    if (group_index == -1){
        cout<<"Wrong group name\n";
        return;
    }

    if (remove_client_from_group(group_index, clientIP) == -1){
        cout<<"Client with IP "<<clientIP<<" is not in group "<< groupName << endl;
        return;
    }

    writeInrouterFileName(fd,lineVec[0]);
    writeInrouterFileName(fd,groupName);
    close(fd);
}

bool is_client_in_group(string clientIP, int groupIndex){
    vector<string> members = clientIPs_in_group[groupIndex];
    for (int i=0; i<members.size(); i++){
        if(members[i] == clientIP)
            return true;
    }
    return false;
}

void selectGroup(vector<string> lineVec){
    string groupName = lineVec[1];
    string clientIP = lineVec[2];
    string clientFileName = "cache/CL" + clientIP;

    int fd = open(clientFileName.c_str(),O_WRONLY);
    if (fd == -1){
        cerr << "Wrong client IP. Try again!" << endl;
        return;
    }

    int group_index = find_group_index_by_name(groupName);
    if (group_index == -1){
        cout<<"Wrong group name\n";
        return;
    }

    if(!is_client_in_group(clientIP, group_index)){
        cout<<"Client with IP "<<clientIP<<" is not in group "<<groupName<<endl;
        return;
    }

    writeInrouterFileName(fd,lineVec[0]);
    writeInrouterFileName(fd,groupName);
    close(fd);
}

void print_logs(int start){
    for(int i=start; i< logs.size(); i++)
        cout<<logs[i]<<endl;
}
void sync(vector<string> lineVec){
    string clientIP = lineVec[1];
    string clientFileName = "cache/CL" + clientIP;

    int fd = open(clientFileName.c_str(),O_WRONLY);
    if (fd == -1){
        cerr << "Wrong client IP. Try again!" << endl;
        return;
    }
    close(fd);

    for(int i=0;i<client_ips.size();i++){
        if(client_ips[i] == clientIP){
            print_logs(client_last_signout[i]);
            client_ips = remove_from_vector(client_ips , i);
            client_last_signout = remove_from_vector_int(client_last_signout , i);
        }
    }
}


void createGroup(vector<string> lineVec){
    string groupName = lineVec[1];
    string groupMultIP = lineVec[2];
    string clientIP = lineVec[3];
    

    if (find_group_index_by_name(groupName) != -1){
        cout<<"Group with name "<<groupName<<" already exists!\n";
        return;
    }   

    if (find_group_index_by_IP(groupMultIP) != -1){
        cout<<"Group with IP "<<groupMultIP<<" already exists!\n";
        return;
    }    

    string clientFileName = "cache/CL" + clientIP;
    int fd = open(clientFileName.c_str(),O_WRONLY);
    if (fd == -1){
        cerr << "Wrong client IP. Try again!" << endl;
        return;
    }

    group_names.push_back(groupName);
    group_IPs.push_back(groupMultIP);
    vector<string> members;
    clientIPs_in_group.push_back(members);

    writeInrouterFileName(fd,lineVec[0]);
    writeInrouterFileName(fd,groupName);
    close(fd);


    int group_index = find_group_index_by_name(groupName);
    if (group_index == -1){
        cout<<"Wrong group name\n";
        return;
    }

    if (add_client_to_group(group_index, clientIP) == -1){
        cout<<"Client with IP "<<clientIP<<" has already joined in group "<< groupName << endl;
        return;
    }

    cout<<"Group "<<groupName<<" with multicast IP "<<groupMultIP<<" was created successfully by client IP "<<clientIP<<endl;
}

void sendFile(vector<string> lineVec){
    string fileName = lineVec[1];
    string groupName = lineVec[2];
    string clientIP = lineVec[3];

    string clientFileName = "cache/CL" + clientIP;

    int fd = open(clientFileName.c_str(),O_WRONLY);
    if (fd == -1){
        cerr << "Wrong client IP. Try again!" << endl;
        return;
    }

    int group_index = find_group_index_by_name(groupName);
    if (group_index == -1){
        cout<<"Wrong group name\n";
        return;
    }

    if(!is_client_in_group(clientIP, group_index)){
        cout<<"Client with IP "<<clientIP<<" is not in group "<<groupName<<endl;
        return;
    }

    writeInrouterFileName(fd,lineVec[0]);
    writeInrouterFileName(fd,fileName);
    writeInrouterFileName(fd,groupName);
    logs.push_back("client with IP "+ clientIP+" sent file " + fileName +" to group " + groupName);
    close(fd);
}

static int rmFiles(const char *pathname, const struct stat *sbuf, int type, struct FTW *ftwb) {
    if(remove(pathname) < 0) {
        cout<< "ERROR: remove" << endl;
        return -1;
    }
    return 0;
}

void Exit(int s){
    if (nftw("cache", rmFiles,10, FTW_DEPTH|FTW_MOUNT|FTW_PHYS) < 0){
        cout << "Couldnt exit!" << endl;
        return;
    }
    exit(-1);
}

bool checkEmptyLine(string line){
    for (int i=0;i<line.size();i++)
        if (line[i] != ' ') return false;
    return true;
}


int main(int argc,char** argv){
    string server_ip = argv[1];
    string line;
    vector<string> lineVec;
    mkdir("cache",0777);
    showCmds();
    while (getline(cin,line)) {
        lineVec = splitCommand(line);
        if (checkEmptyLine(line)) continue;
        if (lineVec[0] == "router" && lineVec.size() == 3) createrouter(lineVec);
        else if (lineVec[0] == "client" && lineVec.size() == 3) createClient(lineVec);
        else if (lineVec[0] == "sign_in" && lineVec.size() == 2) signInClient(lineVec);
        else if (lineVec[0] == "sign_out" && lineVec.size() == 2) signOutClient(lineVec);
        else if (lineVec[0] == "connect_client" && lineVec.size() == 4) connect(lineVec);
        else if (lineVec[0] == "get_group_list" && lineVec.size() == 1) getGroupList();
        else if (lineVec[0] == "show_group" && lineVec.size() == 2) showGroup(lineVec);
        else if (lineVec[0] == "sync" && lineVec.size() == 2) sync(lineVec);
        else if (lineVec[0] == "join" && lineVec.size() == 3) joinGroup(lineVec);
        else if (lineVec[0] == "leave" && lineVec.size() == 3) leaveGroup(lineVec);
        else if (lineVec[0] == "select" && lineVec.size() == 3) selectGroup(lineVec);
        else if (lineVec[0] == "create_group" && lineVec.size() == 4) createGroup(lineVec);
        else if (lineVec[0] == "send_file" && lineVec.size() == 4) sendFile(lineVec);            
        else if (lineVec[0] == "connect_router" && lineVec.size() == 5) connectrouter(lineVec);
        else if (lineVec[0] == "disconnect_router" && lineVec.size() == 3) disconnectrouter(lineVec);
        else if (lineVec[0] == "help" && lineVec.size() == 1) showCmds();  
        else if (lineVec[0] == "exit" && lineVec.size() == 1) Exit(1);  
        else cout << "Invalid command!" << endl;

        struct sigaction sigIntHandler;
        sigIntHandler.sa_handler = Exit;
        sigemptyset(&sigIntHandler.sa_mask);
        sigIntHandler.sa_flags = 0;
        sigaction(SIGINT, &sigIntHandler, NULL);
    }
}