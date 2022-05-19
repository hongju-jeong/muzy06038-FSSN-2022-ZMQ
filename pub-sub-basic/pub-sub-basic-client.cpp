#include <iostream>
#include <zmq.hpp>
#include <sstream>
#include <string.h>
// #include <string>

int main(int argc, char *argv[])
{
    // initialize the zmq context with a single IO thread
    zmq::context_t context{1};

    // construct a REQ (request) socket and connect to interface
    zmq::socket_t socket{context, zmq::socket_type::sub};

    std::cout << "Collecting updates from weather server..."<< std::endl;
    socket.connect("tcp://localhost:5556");

    // std::string zip_filter = 0;
    // if(argv[1]){
    //     zip_filter = atoi(argv[1]);
    // }
    // else { zip_filter = "10001"; }

    const std::string filter = (argc > 1)? argv [1]: "10001 ";

    //socket.setsockopt(ZMQ_SUBSCRIBE, filter, strlen(filter));
    socket.set(zmq::sockopt::subscribe, filter);

    int update_nbr;
    long total_temp = 0;
    for (update_nbr = 0; update_nbr < 20; update_nbr++) 
    {  
        zmq::message_t update;
        int zipcode, temperature, relhumidity;
        socket.recv(update, zmq::recv_flags::none);

        std::istringstream iss(static_cast<char*>(update.data()));
        iss >> zipcode >> temperature >> relhumidity;

        total_temp += temperature;

        std::cout << "Receive temperature for zipcode '" <<filter
              <<"' was"<<(int) (temperature) <<" F"
              << std::endl;

    }
    std::cout << "Average temperature for zipcode '" <<filter
              <<"' was"<<(int) (total_temp / update_nbr+1) <<" F"
              << std::endl;

    

    return 0;
}