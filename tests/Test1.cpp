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

#include <atomic>
#include <thread>
#include "catch.hpp"
#include "../sources/KiwiScheduler.hpp"
#define KIWI_TEST2_MAX 128

using namespace kiwi::engine;

class Counter : public Scheduler, public Scheduler::Timer
{
public:
    Counter() noexcept : m_counter(0) {}
    void callback() override { ++m_counter; }
    size_t get() const noexcept { return m_counter; }
    
private:
    std::atomic<size_t> m_counter;
};

static void producer(Counter* sch)
{
    size_t index = 0;
    std::vector<Scheduler::Task> tasks(KIWI_TEST2_MAX, *sch);
    using ms = std::chrono::milliseconds;
    using clock = std::chrono::high_resolution_clock;
    while(sch->get() < KIWI_TEST2_MAX)
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
        index = (index + 1) % KIWI_TEST2_MAX;
    }
}

static void consumer(Counter* sch)
{
    using ms = std::chrono::milliseconds;
    using clock = std::chrono::high_resolution_clock;
    while(sch->get() < KIWI_TEST2_MAX)
    {
        std::this_thread::sleep_for(ms(20));
        sch->perform(clock::now());
    }
}

TEST_CASE("Scheduler_1", "[Scheduler]")
{
    Counter sch;
    std::thread prod(producer, &sch);
    std::thread cons(consumer, &sch);
    cons.join();
    prod.join();
    
    CHECK(sch.get() >= KIWI_TEST2_MAX);
}
