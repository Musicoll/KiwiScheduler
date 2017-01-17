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

#define CATCH_CONFIG_RUNNER
#include "catch.hpp"

#include <KiwiScheduler.hpp>

#include <thread>
#include <atomic>
#include <cstdlib>

using namespace kiwi::scheduler;

static int counter = 0;

static void printo()
{
    std::cout << "zaza "<< counter++ <<"\n";
}

TEST_CASE("Scheduler", "[Scheduler]")
{
    using milliseconds = std::chrono::milliseconds;
    using clock = std::chrono::high_resolution_clock;
    
    Task t1(printo);
    Task t2(printo);
    Task t3(printo);
    Task t4(printo);
    Task t5(printo);
    Scheduler sch;
    counter = 0;
    sch.add(t1, clock::now() + milliseconds(40));
    sch.add(t2, clock::now() + milliseconds(20));
    sch.add(t3, clock::now() + milliseconds(70));
    sch.add(t4, clock::now() + milliseconds(80));
    sch.add(t5, clock::now() + milliseconds(50));
    sch.add(t5, clock::now() + milliseconds(60));
    
    while (counter < 4)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
        sch.perform(clock::now());
        sch.add(t1, clock::now() + milliseconds(15));
    }
}

int main( int argc, char* const argv[] )
{
    std::cout << "Unit-Tests - KiwiScheduler ..." << '\n';
    return Catch::Session().run( argc, argv );
}
