/*
 ==============================================================================
 
 This file is part of the KIWI library.
 Copyright (c) 2016, CICM, ANR MUSICOLL, Eliott Paris, Pierre Guillot, Jean Millot.
 
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

#include "TestScheduler.hpp"

using namespace kiwi::engine;
#define MAX_COUNT 128
static std::atomic<size_t> counter;

class Counter
{
    using ms = std::chrono::milliseconds;
    using clock = std::chrono::high_resolution_clock;
    
    Counter() : m_counter(0) {}
    
private:
    Scheduler           m_scheduler;
    static const size_t m_max = 128;
    std::atomic<size_t> m_counter;
};


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

extern int perform_test1()
{
    counter = 0;
    Scheduler sch;
    std::thread prod(producer, &sch);
    std::thread cons(consumer, &sch);
    cons.join();
    prod.join();
    
    return counter < MAX_COUNT;
}
