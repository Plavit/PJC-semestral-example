#include <iostream>
#include <fstream>
#include <iomanip>
#include <vector>
#include <chrono>
#include <thread>
#include <mutex>

#include "controller.cpp"

// WordCount 1.0
// Semestral work for B6B36PJC
// Created with love and eggnog by @Author Marek Szeles

int main() {
    static std::string version = "1.0";

    std::cout << "###############################" << std::endl;
    std::cout << "# " << "WordCount v" << version << " initialized." << " #" << std::endl;
    std::cout << "###############################" << std::endl;

    auto *c=new Controller();

    c->print_commands();
    //start controller instance
    c->start();

    return 0;
};