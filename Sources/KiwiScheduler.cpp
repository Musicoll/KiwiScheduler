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
        Scheduler::Scheduler() : m_head(nullptr)
        {
            
        }
        
        void Scheduler::perform(time_point_t const time)
        {
            Task *head = nullptr;
            {
                std::lock_guard<std::mutex> lock(m_mutex);
                Task *current = m_head, *previous = nullptr;
                while(current && current->m_time <= time)
                {
                    previous = current;
                    current  = current->m_next;
                }
                if(previous)
                {
                    previous->m_next = nullptr;
                    head             = m_head;
                    m_head           = current;
                }
            }
            while(head)
            {
                head->m_method();
                head = head->m_next;
            }
        }
        
        void Scheduler::add(Task& t, time_point_t const time)
        {
            t.m_time = time;
            t.m_next = nullptr;
            
            remove(t);
            
            std::lock_guard<std::mutex> lock(m_mutex);
            if(m_head)
            {
                if(m_head->m_time >= t.m_time)
                {
                    t.m_next = m_head;
                    m_head   = &t;
                    return;
                }
                Task *previous = m_head;
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
                m_head = &t;
            }
            
            
        }
        
        void Scheduler::remove(Task const& t)
        {
            if(m_head)
            {
                std::lock_guard<std::mutex> lock(m_mutex);
                if(m_head == &t)
                {
                    m_head = m_head->m_next;
                }
                else
                {
                    Task *current = m_head->m_next, *previous = m_head;
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
