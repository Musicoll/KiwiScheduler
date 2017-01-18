/*
 ==============================================================================
 
 This file is part of the KIWI library.
 Copyright (c) 2014 Pierre Guillot & Eliott Paris.
 
 Permission is granted to use this software under the terms of either:
 a) the GPL v2 (or any later version)
 b) the Affero GPL v3
 
 Details of these licenses can be found at: www.gnu.org/licenses
 
 KIWI is distributed in the hope that it will be useful, but WITHOUT ANY
 WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
 
 ------------------------------------------------------------------------------
 
 To release a closed-source product which uses KIWI, contact : guillotpierre6@gmail.com
 
 ==============================================================================
 */

// Catch main function

//#define CATCH_CONFIG_RUNNER
//#include "catch.hpp"

#include <KiwiScheduler.hpp>

#include <thread>
#include <atomic>
#include <cstdlib>
#include <vector>

using namespace kiwi::scheduler;
#define MAX_COUNT 128
static std::atomic<size_t> counter;

static void increment()
{
    counter++;
}

static void producer(Scheduler* sch)
{
    size_t index = 0;
    std::vector<Task> tasks(MAX_COUNT, Task(increment));
    using ms = std::chrono::milliseconds;
    using clock = std::chrono::high_resolution_clock;
    while(counter < MAX_COUNT)
    {
        std::this_thread::sleep_for(ms(10));
        if(index%2)
        {
            sch->add(tasks[index], clock::now() + ms(5));
        }
        else
        {
            sch->add(tasks[index], clock::now() + ms(17));
        }
        index = (index + 1) % MAX_COUNT;
    }
}

static void consumer(Scheduler* sch)
{
    using ms = std::chrono::milliseconds;
    using clock = std::chrono::high_resolution_clock;
    while(counter < MAX_COUNT)
    {
        std::this_thread::sleep_for(ms(20));
        sch->perform(clock::now());
    }
}

int main( int argc, char* const argv[] )
{
    std::cout << "Unit-Tests - KiwiScheduler ...\n";
    using ms = std::chrono::milliseconds;
    using clock = std::chrono::high_resolution_clock;
    counter = 0;
    Scheduler sch;
    Task t1(increment);
    Task t2(increment);
    Task t3(increment);
    Task t4(increment);
    Task t5(increment);
    
    sch.add(t1, clock::now() + ms(40));
    sch.add(t2, clock::now() + ms(20));
    sch.add(t3, clock::now() + ms(70));
    sch.add(t4, clock::now() + ms(80));
    sch.add(t5, clock::now() + ms(50));
    sch.add(t5, clock::now() + ms(60));
    
    std::thread prod(producer, &sch);
    std::thread cons(consumer, &sch);
    cons.join();
    prod.join();
    
    return 0;
}
