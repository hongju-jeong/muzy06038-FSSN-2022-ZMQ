#include <vector>
#include <thread>
#include <memory>
#include <functional>

#include<zmq.hpp>
#include"zhelpers.hpp"

class server_worker {
public:
    server_worker(zmq::context_t &ctx, int sock_type, int workerId)
        : ctx_(ctx),
        worker_(ctx_, sock_type)
    {workerId_ = workerId;}

    void work() {
        worker_.connect("inproc://backend");

        std::cout<<"worker#"<<workerId_<<" started."<<std::endl;

        try {
            while(true){
                zmq::message_t identity;
                zmq::message_t msg;
                zmq::message_t copied_id;
                zmq::message_t copied_msg;
                worker_.recv(&identity);
                worker_.recv(&msg);
                //std::cout<<"Worker#"<<workerId_<<" received "<<std::endl;
                std::cout<<"Worker#"<<workerId_<<" received "<<msg.to_string() <<" from "<<identity.to_string()<<std::endl;
                copied_id.copy(&identity);
                copied_msg.copy(&msg);
                worker_.send(copied_id, ZMQ_SNDMORE);
                worker_.send(copied_msg);
                // int replies = within(5);
                // for (int reply = 0; reply < replies; ++reply){
                //     //s_sleep(within(1000) + 1);
                    
                // }
            }
        }
        catch (std::exception &e) {}
    }
private:
    zmq::context_t &ctx_;
    zmq::socket_t worker_;
    int workerId_;
};


class server_task {
public:
    server_task(int num_server)
        : ctx_(1),
        frontend_(ctx_, ZMQ_ROUTER),
        backend_(ctx_, ZMQ_DEALER)
    {
        num_server_ = num_server;
        // std::cout<<"num_server_ : "<<num_server_<<std::endl;
    }


    void run() {
        
        frontend_.bind("tcp://*:5570");
        backend_.bind("inproc://backend");

        std::vector<server_worker *> worker;

        std::vector<std::thread *> worker_thread;
        for (int i=0; i < num_server_; ++i){
            worker.push_back(new server_worker(ctx_, ZMQ_DEALER, i));
            worker_thread.push_back(new std::thread(std::bind(&server_worker::work, worker[i])));
            worker_thread[i]->detach();
        }

        try {
            zmq::proxy(static_cast<void*>(frontend_),
                       static_cast<void*>(backend_),
                       nullptr);
        }
        catch (std::exception &e) {}

        for (int i = 0; i < num_server_; ++i) {
            delete worker[i];
            delete worker_thread[i];
        }
    }

private:
    zmq::context_t ctx_;
    zmq::socket_t frontend_;
    zmq::socket_t backend_;
    int num_server_;
};


int main (int argc, char *argv[])
{
    
    
    
    server_task st(((int)*argv[1] - 48));   //test 해보기
    std::thread t4(std::bind(&server_task::run, &st));

    
    t4.detach();

    getchar();
    return 0;
}
