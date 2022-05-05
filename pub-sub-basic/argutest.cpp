#include<iostream>
#include<string>
int main(int argc, char **argv)
{
    std::string arg1 = "0";
    if(argv[1]){
        arg1 = atoi(argv[1]);
    }
    else {arg1 = 4;}
    //int arg2 = atoi(argv[2]);

    std::string sum = arg1;//+ arg2;



    std::cout << arg1 << " " << arg1 << " "<< sum << std::endl;
    return 0;
}