# FSSN-2022-ZMQ

# environment
ubuntu 18.04.6 LTS  
c++ 14

# Compile
g++ -std=c++14 p2p-dechat.cpp -o dechat -L/usr/local/lib -lzmq -lpthread

# require
/user/local/lib 에 zmq.hpp, zhelpers.hpp 있어야 됨  
zmq.hpp : https://github.com/zeromq/cppzmq  
zhelpers.hpp : https://github.com/booksbyus/zguide/blob/master/examples/C%2B%2B/zhelpers.hpp

