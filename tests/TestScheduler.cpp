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

#include <iostream>
#include <cassert>
#include "TestScheduler.hpp"

namespace kiwi
{
    namespace engine
    {
        // ================================================================================ //
        //                                      OBJECT                                      //
        // ================================================================================ //
        Object::Object(Instance& instance, Types threadid) :
        m_instance(instance), m_task(*this, threadid), m_type(threadid)
        {
            
        }
        
        Object::Object(Object const& o) :
        m_instance(o.m_instance), m_task(*this, o.m_type), m_type(o.m_type)
        {
            
        }
        
        Object::~Object()
        {
            m_instance.remove(m_task);
        }
       
        void Object::callback()
        {
            m_instance.access();
        }
        
        void Object::defer(Ms const time)
        {
            m_instance.defer(m_task, time);
        }
        
        void Object::remove()
        {
            m_instance.remove(m_task);
        }
        
        
        
        class Message : public Object
        {
        public:
            Message(Instance& instance) : Object(instance, Types::Main) {}
            void process() override {
                if((m_state = !m_state)) {
                    defer(Ms(std::rand() % 20));
                }
                else {
                    remove();
                }
            }
        private:
            bool m_state;
        };
        
        class Dsp : public Object
        {
        public:
            Dsp(Instance& instance) : Object(instance, Types::Dsp) {}
            void process() override {
                defer(Ms(0));
            }
        };
        
        class Gui : public Object
        {
        public:
            Gui(Instance& instance) : Object(instance, Types::Gui) {}
            void process() override {
                defer(Ms(0));
            }
        };
        
        class High : public Object
        {
        public:
            High(Instance& instance) : Object(instance, Types::High) {}
            void process() override {
                if((m_state = !m_state)) {
                    defer(Ms(std::rand() % 20));
                }
                else {
                    remove();
                }
            }
        private:
            bool m_state;
        };

        
        // ================================================================================ //
        //                                      INSTANCE                                    //
        // ================================================================================ //
        void Instance::defer(Task& task, Ms const time)
        {
            Scheduler::add(task, m_time.load() + time.count());
        }
        
        void Instance::remove(Task& task)
        {
            Scheduler::remove(task);
        }
        
        void Instance::access()
        {
            assert(m_data_mutex.try_lock() && "can't access mutex");
            m_data_access = !m_data_access;
            m_data_mutex.unlock();
        }
        
        void Instance::run()
        {
            m_time      = 0;
            
            std::atomic<bool> state, valid;
            valid = state = true;
            std::vector<Dsp>        objs_dsp(64, *this);
            std::vector<Gui>        objs_gui(256, *this);
            std::vector<Message>    objs_mes(128, *this);
            std::vector<High>       objs_hig(64, *this);
            
            Scheduler::prepare(Object::Types::Dsp);
            Scheduler::prepare(Object::Types::Gui);
            Scheduler::prepare(Object::Types::Main);
            Scheduler::prepare(Object::Types::High);
            
            //auto start_time = Clock::now();
            
            std::thread thread_dsp([this, &state, &objs_dsp]()
                                   {
                                       while(state)
                                       {
                                           for(auto& obj : objs_dsp)
                                           {
                                               obj.process();
                                           }
                                           ++m_time;
                                           state = m_time.load() < 1000;
                                           std::this_thread::sleep_for(Ms(1));
                                       }
                                   });
            
            std::thread thread_gui([this, &state, &objs_gui]()
                                   {
                                       while(state)
                                       {
                                           for(auto& obj : objs_gui)
                                           {
                                               obj.process();
                                           }
                                           std::this_thread::sleep_for(Ms(20));
                                       }
                                   });
            
            
            std::thread thread_high([this, &state, &objs_hig]()
                                   {
                                       while(state)
                                       {
                                           for(auto& obj : objs_hig)
                                           {
                                               obj.process();
                                           }
                                           std::this_thread::sleep_for(Ms(1));
                                       }
                                   });
            
        
            
            while(state)
            {
                for(auto& obj : objs_mes)
                {
                    obj.process();
                }
                Scheduler::perform(m_time.load());
                std::this_thread::sleep_for(Ms(std::rand() % 10));
            }
            
            thread_dsp.join();
            thread_gui.join();
            thread_high.join();
        }
    }
}



int main(int argc, char* const argv[])
{
    std::cout << "running Unit-Tests - KiwiScheduler ...";
    kiwi::engine::Instance instance;
    instance.run();
    instance.run();
    instance.run();
    instance.run();
    instance.run();
    std::cout << "ok\n";
    return 0;
}

