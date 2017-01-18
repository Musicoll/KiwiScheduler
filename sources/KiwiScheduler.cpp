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
        Task::Task(method_t&& m) : m_next(nullptr), m_time(), m_futur_next(nullptr), m_futur_time(), m_method(m)
        {
            
        }
        
        
        Scheduler::Scheduler() : m_main(nullptr), m_futur(nullptr)
        {
            
        }
        
        Scheduler::~Scheduler()
        {
            
        }
        
        void Scheduler::perform(time_point_t const time)
        {
            Task *head = nullptr;
            // Retrieves the tasks to perform
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
                    m_futur = m_futur->m_futur_next;
                    m_futur_mutex.unlock();
                    if(current->m_futur_time != time_point_t::min())
                    {
                        add(*current, current->m_futur_time);
                    }
                    else
                    {
                        remove(*current);
                    }
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
                t.m_time = time;
                if(m_main)
                {
                    // First remove the task if the task is already in the main list
                    if(m_main == &t)
                    {
                        m_main = t.m_next;
                        t.m_next = nullptr;
                    }
                    else
                    {
                        Task *current = m_main->m_next, *previous = m_main;
                        while(current)
                        {
                            if(current == &t)
                            {
                                previous->m_next = current->m_next;
                                current->m_next = nullptr;
                                break;
                            }
                            previous = current;
                            current = current->m_next;
                        }
                    }
                    // Then add the task to the main list
                    if(m_main)
                    {
                        if(m_main->m_time > t.m_time)
                        {
                            t.m_next = m_main;
                            m_main   = &t;
                            m_main_mutex.unlock();
                            return;
                        }
                        Task *previous = m_main;
                        Task *current = previous->m_next;
                        while(current)
                        {
                            if(current->m_time > t.m_time)
                            {
                                t.m_next = current;
                                previous->m_next = &t;
                                m_main_mutex.unlock();
                                return;
                            }
                            previous = current;
                            current = current->m_next;
                        }
                        previous->m_next = &t;
                        t.m_next = nullptr;
                    }
                    else
                    {
                        m_main = &t;
                        t.m_next = nullptr;
                    }
                }
                else
                {
                    m_main = &t;
                    t.m_next = nullptr;
                }
                m_main_mutex.unlock();
            }
            // Adds to task the futur list
            else
            {
                std::lock_guard<std::mutex> lock(m_futur_mutex);
                t.m_futur_time = time;
                t.m_futur_next = m_futur;
                m_futur = &t;
            }
        }
        
        void Scheduler::remove(Task& t)
        {
            if(m_main_mutex.try_lock())
            {
                if(m_main)
                {
                    if(m_main == &t)
                    {
                        m_main = t.m_next;
                        t.m_next = nullptr;
                    }
                    else
                    {
                        Task *current = m_main->m_next, *previous = m_main;
                        while(current)
                        {
                            if(current == &t)
                            {
                                previous->m_next = current->m_next;
                                current->m_next = nullptr;
                                m_main_mutex.unlock();
                                return;
                            }
                            previous = current;
                            current = current->m_next;
                        }
                    }
                }
                m_main_mutex.unlock();
            }
            else
            {
                std::lock_guard<std::mutex> lock(m_futur_mutex);
                t.m_futur_time = time_point_t::min();
                t.m_futur_next = m_futur;
                m_futur = &t;
            }
        }
    }
}
