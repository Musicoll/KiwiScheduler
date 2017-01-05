//
//  main.cpp
//  KiwiScheduler
//
//  Created by Pierre on 03/01/2017.
//  Copyright © 2017 Pierre. All rights reserved.
//

#include <iostream>
#include <thread>
#include "../../Sources/KiwiScheduler.hpp"

using namespace kiwi::scheduler;

static int counter = 0;

void printo()
{
    std::cout << "zaza "<< counter++ <<"\n";
}

int main(int argc, const char * argv[]) {
    using milliseconds = std::chrono::milliseconds;
    using clock = std::chrono::high_resolution_clock;
    
    Task t1(printo);
    Task t2(printo);
    Task t3(printo);
    Task t4(printo);
    Task t5(printo);
    Scheduler sch;
    
    sch.add(t1, clock::now() + milliseconds(40));
    sch.add(t2, clock::now() + milliseconds(20));
    sch.add(t3, clock::now() + milliseconds(70));
    sch.add(t4, clock::now() + milliseconds(80));
    sch.add(t5, clock::now() + milliseconds(50));
    sch.add(t5, clock::now() + milliseconds(60));
    
    while (counter < 40)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
        sch.perform(clock::now());
        sch.add(t1, clock::now() + milliseconds(15));
    }
    
    return 0;
}
