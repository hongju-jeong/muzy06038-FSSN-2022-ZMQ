#include <string>
#include <chrono>
#include <thread>
#include <iostream>
#include <zmq.hpp>
#include <random>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <sstream>
//#include"zhelpers.hpp"

#define within(num) (int) ((float) num * random() / (RAND_MAX) + 1.0)

int main(){

    zmq::context_t context{1};
    zmq::socket_t subscriber{context, zmq::socket_type::sub};
    subscriber.set(zmq::sockopt::subscribe, "");
    subscriber.connect("tcp://localhost:5557");
    zmq::socket_t publisher{context, zmq::socket_type::push};
    publisher.connect("tcp://localhost:5558");

    srandom((unsigned) time (NULL));

    zmq::pollitem_t items[] = {
        {static_cast<void*>(subscriber), 0, ZMQ_POLLIN, 0}
    };


    for(;;){
        zmq::message_t message(12);


        zmq::poll (&items [0],1,100);
        int randint;
        
        if(items[0].revents & ZMQ_POLLIN){
            subscriber.recv(message, zmq::recv_flags::none);
            std::istringstream iss(static_cast<char*>(message.data()));
            iss >> randint;
            std::cout << "I: received message " << randint << std::endl;
        }
        else{
            randint = within(100);
            if(randint<10){
                snprintf((char *) message.data(), 12, "%d", randint);
                publisher.send(message, zmq::send_flags::none);
            }
        }
    }

    return 0;
}