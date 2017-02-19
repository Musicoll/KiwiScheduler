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

static std::atomic<size_t> counter;
static void increment()
{
    counter++;
}

enum QueueId : typename Scheduler::id_t
{
    DspId       = 0,
    EngineId    = 1,
    GuiId       = 2
};

extern int perform_test4()
{
    using ms = std::chrono::milliseconds;
    using clock = std::chrono::high_resolution_clock;
    
    Scheduler sche;
    sche.prepare(QueueId::DspId);
    sche.prepare(QueueId::EngineId);
    sche.prepare(QueueId::GuiId);
    sche.prepare(4);
    
    counter = 0;
    Scheduler sch;
    
    Task t1(increment, DspId);
    Task t2(increment, EngineId);
    Task t3(increment, DspId);
    Task t4(increment, EngineId);
    Task t5(increment, GuiId);
    Task t6(increment, GuiId);
    
    sch.add(t1, clock::now() + ms(40));
    sch.add(t2, clock::now() + ms(20));
    sch.add(t3, clock::now() + ms(70));
    sch.add(t4, clock::now() + ms(80));
    sch.add(t5, clock::now() + ms(50));
    sch.add(t5, clock::now() + ms(60));
    sch.add(t1, clock::now() + ms(40));
    sch.add(t6, clock::now() + ms(30));
    
    sch.perform(clock::now() + ms(40));
    sch.add(t2, clock::now() + ms(20));
    sch.add(t6, clock::now() + ms(30));
    sch.perform(clock::now() + ms(80));
    
    return counter != 8;
}
