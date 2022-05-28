# FSSN-2022-ZMQ

# Compile
g++ -std=c++14 p2p-dechat.cpp -o dechat -L/usr/local/lib -lzmq -lpthread

# require
/user/local/lib 에 zmq.hpp, zhelpers.hpp 있어야 됨
https://github.com/zeromq/cppzmq

