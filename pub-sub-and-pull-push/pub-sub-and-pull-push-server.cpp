#include <zmq.hpp>
#include <iostream>
#include <sstream>

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

        int randint;
        collector.recv(message, zmq::recv_flags::none);

        std::istringstream iss(static_cast<char*>(message.data()));
        iss >> randint;
        std::cout << "I: publishing update " << randint << std::endl;


        snprintf((char *) message.data(), 12, "%d", randint);
        publisher.send(message, zmq::send_flags::none);
    }
    

    return 0;
}