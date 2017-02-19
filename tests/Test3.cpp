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
#include "catch.hpp"
#include "../sources/KiwiScheduler.hpp"

using namespace kiwi::engine;

TEST_CASE("Scheduler 3", "[Scheduler]")
{
    using ms = std::chrono::milliseconds;
    using clock = std::chrono::high_resolution_clock;
    
    enum QueueId : typename Scheduler::id_t
    {
        DspId       = 0,
        EngineId    = 1,
        GuiId       = 2
    };
    
    class Counter : public Scheduler::Timer
    {
    public:
        Counter() noexcept : m_counter(0) {}
        void callback() override { ++m_counter; }
        size_t get() const noexcept { return m_counter; }
    private:
        std::atomic<size_t> m_counter;
    };
    
    Scheduler sch;
    Counter   cnt;
    sch.prepare(QueueId::DspId);
    sch.prepare(QueueId::EngineId);
    sch.prepare(QueueId::GuiId);
    
    Scheduler::Task t1(cnt, DspId);
    Scheduler::Task t2(cnt, EngineId);
    Scheduler::Task t3(cnt, DspId);
    Scheduler::Task t4(cnt, EngineId);
    Scheduler::Task t5(cnt, GuiId);
    Scheduler::Task t6(cnt, GuiId);
    
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
    
    CHECK(cnt.get() == 8);
}
