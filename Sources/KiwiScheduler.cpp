/*
 ==============================================================================
 
 This file is part of the KIWI library.
 - Copyright (c) 2014-2016, Pierre Guillot & Eliott Paris.
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
            Task *head = nullptr;
            {
                std::lock_guard<std::mutex> lock(m_mutex);
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
            
            // Calls the methods
            while(head)
            {
                head->m_method();
                head = head->m_next;
            }
            
            // Adds the tasks that wait to the list
            while(m_waited)
            {
                Task *next = m_waited->m_next;
                add(*m_waited, m_waited->m_time);
                m_waited = next;
            }
        }
        
        void Scheduler::add(Task& t, time_point_t const time)
        {
            t.m_time = time;
            t.m_next = nullptr;
            if(m_mutex.try_lock())
            {
                if(m_sorted)
                {
                    // First remove if the task already exists
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
            else
            {
                // dois y avoir une autre lock ici
                t.m_next = m_waited;
                m_waited = &t;
            }
        }
        
        void Scheduler::remove(Task const& t)
        {
            if(m_sorted)
            {
                std::lock_guard<std::mutex> lock(m_mutex);
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
