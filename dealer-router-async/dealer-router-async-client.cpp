#include <vector>
#include <thread>
#include <memory>
#include <functional>

#include<zmq.hpp>
#include"zhelpers.hpp"

class client_task {
public:
    client_task(char *clientId)
        : ctx_(1),
          client_socket_(ctx_, ZMQ_DEALER)
    {clientId_ = clientId;}

    void start() {
        // generate random identity

        char identity[10] = {};
        sprintf(identity, "%s", clientId_);
        
        client_socket_.setsockopt(ZMQ_IDENTITY, identity, strlen(identity));
        client_socket_.connect("tcp://localhost:5570");

        printf("Client %s started\n", identity);

        zmq::pollitem_t items[] = {
            { client_socket_, 0, ZMQ_POLLIN, 0 } };
        int request_nbr = 0;
        try {
            while (true) {
                zmq::message_t msg;
                for (int i = 0; i < 100; ++i) {
                    // 10 milliseconds
                    zmq::poll(items, 1, 10);
                    if (items[0].revents & ZMQ_POLLIN) {
                        //printf("\n%s ", identity);
                        //client_socket_.recv(&identity);
                        client_socket_.recv(&msg);
                        //std::cout<<"Worker#"<<workerId_<<" received "<<std::endl;
                        std::cout<<identity<<" received: "<<msg.to_string()<<std::endl;
                        //s_dump(client_socket_);
                    }
                }
                char request_string[16] = {};
                sprintf(request_string, "request #%d", ++request_nbr);
                std::cout << "Req #"<<request_nbr<<" sent.."<<std::endl;
                client_socket_.send(request_string, strlen(request_string));
            }
        }
        catch (std::exception &e) {}
    }

private:
    zmq::context_t ctx_;
    zmq::socket_t client_socket_;
    char *clientId_;
};

int main(int argc, char *argv[]){
    client_task ct1(argv[1]);

    std::thread t1(std::bind(&client_task::start, &ct1));

    t1.detach();

    getchar();
    return 0;
}