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
#include <cstdlib>

using namespace kiwi::engine;

class MessageDspObject
{
public:
    MessageDspObject() : m_task([this](){callback();})
    {
        srand(time(NULL));
    }
    
    void callback()
    {
        m_message = m_value;
        m_counter++;
    }
    
    bool run()
    {
        using ms    = std::chrono::milliseconds;
        
        m_ndspticks = 0;
        m_counter   = 0;
        m_state     = true;
        
        // This thread represents the DSP thread, every each 2 ms the fake perform method
        // generates a random value and add a task to perform as soon as possible.
        std::thread dsp([this]()
                         {
                             while(m_state)
                             {
                                 std::this_thread::sleep_for(ms(1));
                                 m_value = std::rand() / float(RAND_MAX) * 2.f + 1.f;
                                 m_scheduler.add(m_task, Scheduler::time_point_t() + ms(m_ndspticks));
                                 ++m_ndspticks;
                             }
                         });
        
        // This thread represents the message thread, it ticks between 0 and 10 ms and try to
        // retrieves the informations of the DSP thread through the callback method.
        std::thread message([this]()
                         {
                             while(m_state)
                             {
                                 std::this_thread::sleep_for(ms(std::rand() % 10));
                                 m_scheduler.perform(Scheduler::time_point_t() + ms(m_ndspticks));
                             }
                         });
        
       
        std::this_thread::sleep_for(ms(1000));
        m_state = false;
        dsp.join();
        message.join();
        m_scheduler.perform(Scheduler::time_point_t() + ms(m_ndspticks));
        
        return m_ndspticks < m_counter;
    }
    
private:
    std::atomic<bool>   m_state;
    std::atomic<float>  m_value;
    float               m_message;
    size_t              m_counter;
    std::atomic<size_t> m_ndspticks;
    Scheduler           m_scheduler;
    Task                m_task;
};

extern int perform_test2()
{
    MessageDspObject obj;
    return obj.run();
}
