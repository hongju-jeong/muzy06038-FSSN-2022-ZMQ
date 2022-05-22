#include <iostream>     ///< cout
#include <cstring>      ///< memset
#include <errno.h>      ///< errno
#include <sys/socket.h> ///< socket
#include <netinet/in.h> ///< sockaddr_in
#include <arpa/inet.h>  ///< getsockname
#include <unistd.h>     ///< close


#include <vector>
#include <sstream>
#include <thread>
#include <memory>
#include <functional>

#include<zmq.hpp>
#include"zhelpers.hpp"

#define within(num) (int) ((float) num * random() / (RAND_MAX) + 1.0)

bool global_flag_shutdown;

std::string get_local_ip(){
    const char* google_dns_server = "8.8.8.8";
    int dns_port = 53;

    struct sockaddr_in serv;
    int sock = socket(AF_INET, SOCK_DGRAM, 0);

    //Socket could not be created
    if(sock < 0)
    {
        std::cout << "Socket error" << std::endl;
    }

    memset(&serv, 0, sizeof(serv));
    serv.sin_family = AF_INET;
    serv.sin_addr.s_addr = inet_addr(google_dns_server);
    serv.sin_port = htons(dns_port);

    int err = connect(sock, (const struct sockaddr*)&serv, sizeof(serv));
    if (err < 0)
    {
        std::cout << "Error number: " << errno
            << ". Error message: " << strerror(errno) << std::endl;
    }

    struct sockaddr_in name;
    socklen_t namelen = sizeof(name);
    err = getsockname(sock, (struct sockaddr*)&name, &namelen);

    char buffer[80];
    const char* p = inet_ntop(AF_INET, &name.sin_addr, buffer, 80);
    if(p != NULL)
    {
        //std::cout << "Local IP address is: " << buffer << std::endl;
        std::string str_buffer(buffer);
        return str_buffer;
    }
    else
    {
        std::cout << "Error number: " << errno
            << ". Error message: " << strerror(errno) << std::endl;
    }

    close(sock);
}

std::vector<std::string> split(std::string input, char delimiter) {
    std::vector<std::string> answer;
    std::stringstream ss(input);
    std::string temp;
 
    while (getline(ss, temp, delimiter)) {
        answer.push_back(temp);
    }
 
    return answer;
}

std::string search_nameserver(std::string ip_mask, std::string local_ip_addr, int port_nameserver){
    // Search for P2P name server in the local network.
    zmq::context_t context;
    zmq::socket_t req{context, zmq::socket_type::sub};
    std::string target_ip_addr;
    zmq::message_t message;
    for(int last = 1;last<255;last++){
        target_ip_addr = "tcp://";
        target_ip_addr += ip_mask;
        target_ip_addr += ".";
        target_ip_addr += std::to_string(last);
        target_ip_addr += ":";
        target_ip_addr += std::to_string(port_nameserver);
        //std::cout << target_ip_addr << std::endl;
        if (target_ip_addr != local_ip_addr || target_ip_addr == local_ip_addr){
            req.connect(target_ip_addr);
            req.set(zmq::sockopt::rcvtimeo, 2000);
            req.set(zmq::sockopt::subscribe, "NAMESERVER");
        }
    }
    try {
        req.recv(message, zmq::recv_flags::none);
        
        if(message.to_string().length() > 0){
            std::vector<std::string> res_list = split(message.to_string(), ':');
            if(res_list[0] == "NAMESERVER"){
            return res_list[1];
            }
            else { return "NULL"; }   // 일단 "NULL"
        }
        else { return "NULL"; }   // 일단 "NULL"
        
        
    }
    catch (std::exception &e) {return "NULL";}   // 일단 "NULL"


}                   //아직 테스트 안됨

void beacon_nameserver(std::string local_ip_addr, int port_nameserver){
    //Generate periodic (1 second) beacon message.
    zmq::context_t context;
    zmq::socket_t socket{context, zmq::socket_type::pub};
    std::string bind_ip_addr;
    bind_ip_addr = "tcp://";
    bind_ip_addr += local_ip_addr;
    bind_ip_addr += ":";
    bind_ip_addr += std::to_string(port_nameserver);
    socket.bind(bind_ip_addr);          //bind와 connect 차이?
    std::cout <<"local p2p name server bind to "<<bind_ip_addr<<std::endl;
    for(;;){
        try
        {
            sleep(1);
            std::string msg = "NAMESERVER:";
            msg += local_ip_addr;
            socket.send(zmq::buffer(msg), zmq::send_flags::none);
        }
        catch(std::exception& e)  //exception 설정 except (KeyboardInterrupt, zmq.ContextTerminated):
        {
            break;
        }
        
    }
}

void user_manager_nameserver(std::string local_ip_addr, int port_subscribe){
    //User subscription manager {ip address and user id}.
    std::vector<std::vector<std::string>> user_db;
    zmq::context_t context;
    zmq::socket_t socket{context, zmq::socket_type::rep};
    std::string bind_ip_addr;
    bind_ip_addr = "tcp://";
    bind_ip_addr += local_ip_addr;
    bind_ip_addr += ":";
    bind_ip_addr += std::to_string(port_subscribe);
    socket.bind(bind_ip_addr);
    std::cout<<"local p2p db server activated at "<<bind_ip_addr<<std::endl;

    for(;;){
        try
        {
            zmq::message_t message;
            std::vector<std::string> user_req;
            socket.recv(message, zmq::recv_flags::none);
            
            

            user_req = split(message.to_string(), ':');
            user_db.push_back(user_req);
            std::cout<<"user registration '"<<user_req[1]<<"' from '"<<user_req[0]<<"' ."<<std::endl;
            socket.send(zmq::buffer("ok"), zmq::send_flags::none);
        }
        catch(const std::exception& e)
        {
            break;
        }
        
    }
}

void relay_server_nameserver(std::string local_ip_addr, int port_chat_publisher, int port_chat_collector){
    zmq::context_t context;
    zmq::socket_t publisher{context, zmq::socket_type::pub};
    std::string pub_bind_ip_addr;
    pub_bind_ip_addr = "tcp://";
    pub_bind_ip_addr += local_ip_addr;
    pub_bind_ip_addr += ":";
    pub_bind_ip_addr += std::to_string(port_chat_publisher);
    publisher.bind(pub_bind_ip_addr);

    zmq::socket_t collector{context, zmq::socket_type::pull};
    std::string col_bind_ip_addr;
    col_bind_ip_addr = "tcp://";
    col_bind_ip_addr += local_ip_addr;
    col_bind_ip_addr += ":";
    col_bind_ip_addr += std::to_string(port_chat_collector);
    collector.bind(col_bind_ip_addr);
    std::cout<<"local p2p relay server activated at tcp://"<<local_ip_addr<<":"<<port_chat_publisher<<" & "<<port_chat_collector<<"."<<std::endl;

    for(;;){
        try
        {
            zmq::message_t message;
            collector.recv(message, zmq::recv_flags::none);
            std::cout<<"p2p-relay:<==>"<<message.to_string()<<std::endl;

            std::string send_message = "RELAY:";
            send_message += message.to_string();
            
            publisher.send(zmq::buffer(send_message), zmq::send_flags::none);
        }
        catch(const std::exception& e)
        {
            break;
        }
        
    }
}


int main(int argc, char *argv[]){            //argument, don't forget this.

    char *username = argv[1];
    std::string str_username(username);
    
    std::string ip_addr_p2p_server;
    int port_nameserver = 9001;
    int port_chat_publisher = 9002;
    int port_chat_collector = 9003;
    int port_subscribe = 9004;
    
    std::string str_ip_arr = get_local_ip();
    //std::string str_ip_arr(ip_addr);
    
    

    
    std::vector<std::string> result = split(str_ip_arr, '.');
    std::string str_ip_mask = result[0] ;
    str_ip_mask += ".";
    str_ip_mask += result[1];
    str_ip_mask += ".";
    str_ip_mask += result[2];

    

    std::cout<< "searching for p2p server." <<std::endl;

    std::string name_server_ip_addr = search_nameserver(str_ip_mask, str_ip_arr, port_nameserver);

    int flag = 0;

    if(name_server_ip_addr == "NULL"){
        

        ip_addr_p2p_server = str_ip_arr;
        std::cout<<"p2p server is not found, and p2p server mode is activated."<<std::endl;
        std::thread beacon_thread(beacon_nameserver,str_ip_arr, port_nameserver);
        beacon_thread.detach();
        std::cout<<"p2p beacon server is activated."<<std::endl;
        std::thread db_thread(user_manager_nameserver,str_ip_arr,port_subscribe);
        db_thread.detach();
        std::cout<<"p2p subscriber database server is activated."<<std::endl;
        std::thread relay_thread(relay_server_nameserver,str_ip_arr,port_chat_publisher,port_chat_collector);
        relay_thread.detach();
        std::cout<<"p2p message relay server is activated."<<std::endl;

    }
    else{
        ip_addr_p2p_server = name_server_ip_addr;
        std::cout<<"p2p server fount at"<<ip_addr_p2p_server<<", and p2p client mode is activated."<<std::endl;
    }

    

    sleep(5);

    std::cout<<"starting user registration procedure."<<std::endl;

    

    zmq::context_t db_client_context;
    zmq::socket_t db_client_socket{db_client_context, zmq::socket_type::req};
    std::string req_connect_ip_addr;
    req_connect_ip_addr = "tcp://";
    req_connect_ip_addr += ip_addr_p2p_server;
    req_connect_ip_addr += ":";
    req_connect_ip_addr += std::to_string(port_subscribe);

    

    db_client_socket.connect(req_connect_ip_addr);
    
    std::string send_message = str_ip_arr+":"+str_username;
    
    db_client_socket.send(zmq::buffer(send_message), zmq::send_flags::none);

    zmq::message_t recv_message;
    db_client_socket.recv(recv_message, zmq::recv_flags::none);

    if(recv_message.to_string().compare("ok")){
        std::cout<<"user registration to p2p server completed."<<std::endl;
    }
    else{
        std::cout<<"user registation to p2p server failed."<<std::endl;
    }

    std::cout<<"starting message transfer procedure."<<std::endl;

    

    zmq::context_t relay_client;
    zmq::socket_t p2p_rx{relay_client, zmq::socket_type::sub};
    const std::string filter = "RELAY";
    p2p_rx.set(zmq::sockopt::subscribe, filter);
    std::string pub_connect_ip_addr;
    pub_connect_ip_addr = "tcp://";
    pub_connect_ip_addr += ip_addr_p2p_server;
    pub_connect_ip_addr += ":";
    pub_connect_ip_addr += std::to_string(port_chat_publisher);
    p2p_rx.connect(pub_connect_ip_addr);


    zmq::socket_t p2p_tx{relay_client, zmq::socket_type::push};
    std::string push_connect_ip_addr;
    push_connect_ip_addr = "tcp://";
    push_connect_ip_addr += ip_addr_p2p_server;
    push_connect_ip_addr += ":";
    push_connect_ip_addr += std::to_string(port_chat_collector);
    p2p_tx.connect(push_connect_ip_addr);

    std::cout<<push_connect_ip_addr<<std::endl;

    std::cout<<"starting autonomous message transmit and receive scenario."<<std::endl;

    zmq::pollitem_t items[] = {
        {static_cast<void*>(p2p_rx), 0, ZMQ_POLLIN, 0}
    };

    for(;;){
        try
        {
            zmq::poll (&items [0],1,100);
            zmq::message_t message;
            std::string msg;
            if(items[0].revents & ZMQ_POLLIN){   //&&
                p2p_rx.recv(message, zmq::recv_flags::none);
                std::vector<std::string> message_split = split(message.to_string(), ':');
                std::cout<<"p2p-recv::<<== "<<message_split[1]<<":"<<message_split[2]<<"\""<<std::endl;
            }
            else{
                int rand = within(100);
                if (rand <10){
                    sleep(3);
                    msg = "(" + str_username + "," + str_ip_arr + ":ON)";
                    p2p_tx.send(zmq::buffer(msg), zmq::send_flags::none);
                    std::cout<<"p2p-send::==>>"<<msg<<std::endl;
                }
                else if(rand > 90){
                    sleep(3);
                    msg = "(" + str_username + "," + str_ip_arr + ":OFF)";
                    p2p_tx.send(zmq::buffer(msg), zmq::send_flags::none);
                    std::cout<<"p2p-send::==>>"<<msg<<std::endl;
                }

            }
            
        }
        catch(const std::exception& e)
        {
            break;
        }
        
    }

    std::cout<<"closing p2p chatting program."<<std::endl;


    global_flag_shutdown = true;
    db_client_socket.close();
    p2p_rx.close();
    p2p_tx.close();
    db_client_context.close();
    relay_client.close();



    return 0;
}