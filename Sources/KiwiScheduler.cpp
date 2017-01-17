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
        Scheduler::Scheduler() : m_sorted(nullptr)
        {
            
        }
        
        void Scheduler::perform(time_point_t const time)
        {
            // Retrieves the tasks to perform
            Task *head = nullptr;
            {
                std::lock_guard<std::mutex> lock(m_sorted_mutex);
                Task *current = m_sorted, *previous = nullptr;
                while(current && current->m_time <= time)
                {
                    previous = current;
                    current  = current->m_next;
                }
                if(previous)
                {
                    previous->m_next = nullptr;
                    head             = m_sorted;
                    m_sorted           = current;
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
                m_waited_mutex.lock();
                while(m_waited)
                {
                    Task *current = m_waited;
                    m_waited = m_waited->m_next;
                    m_waited_mutex.unlock();
                    add(*current, current->m_time);
                    m_waited_mutex.lock();
                }
                m_waited_mutex.unlock();
            }
            
        }
        
        void Scheduler::add(Task& t, time_point_t const time)
        {
            // If we're not performing on the main list
            if(m_sorted_mutex.try_lock())
            {
                if(m_sorted)
                {
                    // First remove the task if the task is already in the main list
                    if(m_sorted == &t)
                    {
                        m_sorted = m_sorted->m_next;
                    }
                    else
                    {
                        Task *current = m_sorted->m_next, *previous = m_sorted;
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
                    if(m_sorted->m_time >= t.m_time)
                    {
                        t.m_next = m_sorted;
                        m_sorted   = &t;
                        return;
                    }
                    Task *previous = m_sorted;
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
                    m_sorted = &t;
                }
            }
            // Adds to task the futur list
            else
            {
                t.m_time = time;
                t.m_next = m_waited;
                std::lock_guard<std::mutex> lock(m_sorted_mutex);
                m_waited = &t;
            }
        }
        
        void Scheduler::remove(Task const& t)
        {
            if(m_sorted)
            {
                std::lock_guard<std::mutex> lock(m_sorted_mutex);
                if(m_sorted == &t)
                {
                    m_sorted = m_sorted->m_next;
                }
                else
                {
                    Task *current = m_sorted->m_next, *previous = m_sorted;
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
