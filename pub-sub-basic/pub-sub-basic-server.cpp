//server.cpp

#include <string>
#include <chrono>
#include <thread>
#include <iostream>
#include <zmq.hpp>
#include <random>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#if (defined (WIN32))
#include <zhelpers.hpp>
#endif

#define within(num) (int) ((float) num * random() / (RAND_MAX) + 1.0)


using namespace std::chrono_literals;

/*
std::string randrange(int min, int max){
    std::random_device rd;  //시드
    std::mt19937 gen(rd());  // 난수 생성 엔진 초기화
    std::uniform_int_distribution<int> dis(min,max); // 균등 분포
    return std::to_string(dis(gen));
}
*/

int main() 
{

    // initialize the zmq context with a single IO thread
    zmq::context_t context{1};

    // construct a REP (reply) socket and bind to interface
    zmq::socket_t socket{context, zmq::socket_type::pub};
    socket.bind("tcp://*:5556");
    socket.bind("ipc://weather.ipc");

    srandom((unsigned) time (NULL));
    // std::string zipcode;
    // std::string temperature;
    // std::string relhumidity;
    // std::string allstr;
    // char ch[100];


    for (;;) 
    {
        int zipcode, temperature, relhumidity;

        zipcode = within (100000);
        temperature = within (215) - 80;
        relhumidity = within (50) +10;
        zmq::message_t message(20);
        // zipcode = randrange(1, 100000);
        // temperature = randrange(-80, 135);
        // relhumidity = randrange(10,60);

        // allstr = zipcode + " " + temperature + " " + relhumidity;
        // strcpy(ch,allstr.c_str());
        // send the reply to the client

        snprintf((char *) message.data(), 20, 
            "%05d %d %d", zipcode, temperature, relhumidity);
        socket.send(message, zmq::send_flags::none);
        
    }

    return 0;
}