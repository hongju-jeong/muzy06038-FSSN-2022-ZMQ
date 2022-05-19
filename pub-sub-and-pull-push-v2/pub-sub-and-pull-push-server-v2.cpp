#include <zmq.hpp>
#include <iostream>
#include <sstream>
#include<string.h>
using namespace std::chrono_literals;
int main(){

    zmq::context_t context{1};
    zmq::socket_t publisher{context, zmq::socket_type::pub};
    publisher.bind("tcp://*:5557");
    zmq::socket_t collector{context, zmq::socket_type::pull};
    collector.bind("tcp://*:5558");

    for(;;){
        zmq::message_t message;
        //zmq::message_t sendMessage(12);

        collector.recv(message, zmq::recv_flags::none);

        std::cout << "server: publishing update => " << message.to_string() << std::endl;


        publisher.send(zmq::buffer(message.to_string()), zmq::send_flags::none);
    }
    

    return 0;
}