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

using namespace kiwi::engine;

class Application : public Scheduler
{
    using ms        = std::chrono::milliseconds;
    using clock     = std::chrono::high_resolution_clock;
    using time_t    = Scheduler::time_point_t;
    using Task      = Scheduler::Task;
    using Timer     = Scheduler::Timer;
    
    enum ThreadId : typename Scheduler::id_t
    {
        DspId       = 0,
        EngineId    = 1,
        GuiId       = 2
    };
    
    void defer(Task& task, ms const time) { Scheduler::add(task, clock::now() + time); }
    void setTicksGui(size_t const n) { m_ticks_gui = n; }
    void setTicksDsp(size_t const n) { m_ticks_dsp = n; }
    void setTicksMes(size_t const n) { m_ticks_mes = n; }
    
    class MesObject : public Timer
    {
    public:
        MesObject(Application& app) : m_master(app), m_task(*this, ThreadId::EngineId), m_nticks(0) {}
        MesObject(MesObject const& o) : m_master(o.m_master), m_task(*this, ThreadId::EngineId), m_nticks(0) {}
        void callback() override { m_master.setTicksMes(m_nticks); }
        void process() {
            ++m_nticks;
            if(!m_nticks%4)
                m_master.defer(m_task, ms(std::rand() % 100));
            else
                m_master.remove(m_task);
        }
    private:
        Application&        m_master;
        Task                m_task;
        std::atomic<size_t> m_nticks;
    };
    
    class DspObject : public Timer
    {
    public:
        DspObject(Application& app) : m_master(app), m_task(*this, ThreadId::DspId), m_nticks(0) {}
        DspObject(DspObject const& o) : m_master(o.m_master), m_task(*this, ThreadId::DspId), m_nticks(0) {}
        void callback() override { m_master.setTicksDsp(m_nticks); }
        void process() { ++m_nticks; m_master.defer(m_task, ms(0)); }
        
    private:
        Application&        m_master;
        Task                m_task;
        std::atomic<size_t> m_nticks;
    };
    
    class GuiObject : public Timer
    {
    public:
        GuiObject(Application& app) : m_master(app), m_task(*this, ThreadId::GuiId), m_nticks(0) {}
        GuiObject(GuiObject const& o) : m_master(o.m_master), m_task(*this, ThreadId::GuiId), m_nticks(0) {}
        ~GuiObject() { m_master.remove(m_task); }
        void callback() override { m_master.setTicksGui(m_nticks); }
        void process() { ++m_nticks; m_master.defer(m_task, ms(0)); }
        
    private:
        Application&        m_master;
        Task                m_task;
        std::atomic<size_t> m_nticks;
    };
    
public:
    void run()
    {
        m_ticks_dsp = 0;
        m_ticks_gui = 0;
        m_ticks_mes = 0;
        bool state_dsp = true;
        bool state_gui = true;
        std::vector<DspObject> objs_dsp(512, *this);
        std::vector<GuiObject> objs_gui(64, *this);
        std::vector<MesObject> objs_mes(128, *this);

        
        std::thread thread_dsp([this, &state_dsp, &objs_dsp]()
                               {
                                   while(state_dsp)
                                   {
                                       std::this_thread::sleep_for(ms(1));
                                       for(auto& obj : objs_dsp)
                                       {
                                           obj.process();
                                       }
                                   }
                               });
        
        std::thread thread_gui([this, &state_gui, &objs_gui]()
                               {
                                   while(state_gui)
                                   {
                                       std::this_thread::sleep_for(ms(20));
                                       for(auto& obj : objs_gui)
                                       {
                                           obj.process();
                                       }
                                   }
                               });
        
        
        while(state_dsp && state_gui)
        {
            std::this_thread::sleep_for(ms(std::rand() % 10));
            for(auto& obj : objs_mes)
            {
                obj.process();
            }
            Scheduler::perform(Scheduler::time_point_t() + ms(m_ticks_dsp));
        }
        
        thread_dsp.join();
        thread_gui.join();
    }
    
private:
    size_t  m_ticks_dsp  = 0;
    size_t  m_ticks_gui  = 0;
    size_t  m_ticks_mes = 0;
};

TEST_CASE("Scheduler 4", "[Scheduler]")
{
    Application app;
    app.run();
}
