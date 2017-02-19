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

extern int perform_test3()
{
    using ms = std::chrono::milliseconds;
    using clock = std::chrono::high_resolution_clock;
    
    counter = 0;
    Scheduler sch;
    
    Task t1(increment);
    Task t2(increment);
    Task t3(increment);
    Task t4(increment);
    Task t5(increment);
    Task t6(increment);
    
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
