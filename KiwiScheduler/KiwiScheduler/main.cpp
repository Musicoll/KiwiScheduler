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

static int counter = 0;

void printo()
{
    std::cout << "zaza "<< counter++ <<"\n";
}

class MyClock : public kiwi::Scheduler::Master
{
public:
    kiwi::Scheduler::time_point_t getTime() const noexcept override {return std::chrono::high_resolution_clock::now();}
};


void tester()
{
    MyClock c;
    kiwi::Scheduler sch(c);
    
    class Tacher
    {
    public:
        Tacher(kiwi::Scheduler& sc) : m_sch(sc) {}
        kiwi::Scheduler& m_sch;
    };
}

int main(int argc, const char * argv[]) {
    MyClock c;
    kiwi::Scheduler::Task t1(printo);
    kiwi::Scheduler::Task t2(printo);
    kiwi::Scheduler::Task t3(printo);
    kiwi::Scheduler::Task t4(printo);
    kiwi::Scheduler::Task t5(printo);
    kiwi::Scheduler sch(c);
    
    sch.add(t1, kiwi::Scheduler::milliseconds_t(40));
    sch.add(t2, kiwi::Scheduler::milliseconds_t(20));
    sch.add(t3, kiwi::Scheduler::milliseconds_t(70));
    sch.add(t4, kiwi::Scheduler::milliseconds_t(80));
    sch.add(t5, kiwi::Scheduler::milliseconds_t(50));
    sch.add(t5, kiwi::Scheduler::milliseconds_t(60));
    
    while (counter < 40)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
        sch.perform();
        sch.add(t1, kiwi::Scheduler::milliseconds_t(15));
    }
    
    return 0;
}
