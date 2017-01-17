/*
 ==============================================================================
 
 This file is part of the KIWI library.
 - Copyright (c) 2016, CICM, ANR MUSICOLL, Eliott Paris, Pierre Guillot, Jean Millot.
 
 Permission is granted to use this software under the terms of the GPL v2
 (or any later version). Details can be found at: www.gnu.org/licenses
 
 KIWI is distributed in the hope that it will be useful, but WITHOUT ANY
 WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 
 ------------------------------------------------------------------------------
 
 Contact : cicm.mshparisnord@gmail.com
 
 ==============================================================================
 */

#include "KiwiScheduler.hpp"

namespace kiwi
{
    namespace scheduler
    {
        Scheduler::Scheduler() : m_main(nullptr)
        {
            
        }
        
        void Scheduler::perform(time_point_t const time)
        {
            // Retrieves the tasks to perform
            Task *head = nullptr;
            {
                std::lock_guard<std::mutex> lock(m_main_mutex);
                Task *current = m_main, *previous = nullptr;
                while(current && current->m_time <= time)
                {
                    previous = current;
                    current  = current->m_next;
                }
                if(previous)
                {
                    previous->m_next = nullptr;
                    head             = m_main;
                    m_main           = current;
                }
            }
            
            // Performs the tasks
            while(head)
            {
                head->m_method();
                head = head->m_next;
            }
            
            // Adds the tasks that wait to the list
            {
                m_futur_mutex.lock();
                while(m_futur)
                {
                    Task *current = m_futur;
                    m_futur = m_futur->m_next;
                    m_futur_mutex.unlock();
                    add(*current, current->m_time);
                    m_futur_mutex.lock();
                }
                m_futur_mutex.unlock();
            }
            
        }
        
        void Scheduler::add(Task& t, time_point_t const time)
        {
            // If we're not performing on the main list
            if(m_main_mutex.try_lock())
            {
                if(m_main)
                {
                    // First remove the task if the task is already in the main list
                    if(m_main == &t)
                    {
                        m_main = m_main->m_next;
                    }
                    else
                    {
                        Task *current = m_main->m_next, *previous = m_main;
                        while(current)
                        {
                            if(current == &t)
                            {
                                previous->m_next = current->m_next;
                                break;
                            }
                            previous = current;
                            current = current->m_next;
                        }
                    }
                    
                    // Then add the task to the main list
                    t.m_time = time;
                    if(m_main->m_time >= t.m_time)
                    {
                        t.m_next = m_main;
                        m_main   = &t;
                        return;
                    }
                    Task *previous = m_main;
                    Task *current = previous->m_next;
                    while(current)
                    {
                        if(current->m_time >= t.m_time)
                        {
                            t.m_next = current;
                            previous->m_next = &t;
                            return;
                        }
                        previous = current;
                        current = current->m_next;
                    }
                    previous->m_next = &t;
                }
                else
                {
                    m_main = &t;
                }
            }
            // Adds to task the futur list
            else
            {
                t.m_time = time;
                t.m_next = m_futur;
                std::lock_guard<std::mutex> lock(m_main_mutex);
                m_futur = &t;
            }
        }
        
        void Scheduler::remove(Task const& t)
        {
            if(m_main)
            {
                std::lock_guard<std::mutex> lock(m_main_mutex);
                if(m_main == &t)
                {
                    m_main = m_main->m_next;
                }
                else
                {
                    Task *current = m_main->m_next, *previous = m_main;
                    while(current)
                    {
                        if(current == &t)
                        {
                            previous->m_next = current->m_next;
                            return;
                        }
                        previous = current;
                        current = current->m_next;
                    }
                }
            }
        }
    }
}
