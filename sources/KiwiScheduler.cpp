/*
 ==============================================================================
 
 This file is part of the KIWI library.
 Copyright (c) 2016, CICM, ANR MUSICOLL, Eliott Paris, Pierre Guillot, Jean Millot.
 
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
        Task::Task(method_t&& m, const id_t queue_id) : m_next(nullptr), m_time(), m_process_next(nullptr), m_method(m), m_futur_next(nullptr), m_futur_time(), m_queue_id(queue_id)
        {
            
        }
        
        
        // ==================================================================================== //
        //                                          QUEUE                                       //
        // ==================================================================================== //
        
        void Scheduler::Queue::perform(time_point_t const time)
        {
            // The list of tasks to perform before the given time
            Task *ready = nullptr;
            
            // ------------------------------------------//
            // Retrieves the tasks before the given time //
            // ------------------------------------------//
            {
                // Locks the mutex of the main list of tasks. If tasks are added or removed
                // during this lock, they will be added to the future list and processed after
                std::lock_guard<std::mutex> lock(m_main_mutex);
                
                // Finds the tail of the list to perform now and the head of the new list
                // We need to save the process next because, if we're out the lock the next
                // task can change
                Task *head = m_main, *tail = nullptr;
                while(head && head->m_time <= time)
                {
                    tail = head;
                    head->m_process_next = head->m_next;
                    head = head->m_process_next;
                }
                // If the tail of the list to perform isn't null:
                // • The head of the tasks to perform now is the current head of the main list
                // • The new head of the tasks is the next task of the tail
                // • The next task of the tail will be null to mark the end
                if(tail)
                {
                    tail->m_process_next = nullptr;
                    ready        = m_main;
                    m_main       = head;
                }
            }
            
            // ------------------------------------------//
            // Adds and removes the tasks that has been  //
            // added or removed during the main lock     //
            // ------------------------------------------//
            {
                // Locks the mutex of the list of futures tasks. If tasks are added or removed
                // during this lock, they managed directly by the main list or recursively
                // added to this list until the main lock is free.
                m_futur_mutex.lock();
                while(m_futur)
                {
                    Task *current = m_futur;
                    m_futur = m_futur->m_futur_next;
                    m_futur_mutex.unlock();
                    if(current->m_futur_type == Task::futur_type_t::to_add)
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
            
            
            
            // ------------------------------------------//
            // Performs the tasks                        //
            // ------------------------------------------//
            // As we don't touch the task, we can call it without locks
            while(ready)
            {
                ready->m_method();
                ready = ready->m_process_next;
            }
        }
        
        void Scheduler::Queue::add(Task& task, time_point_t const time)
        {
            // If we're not performing on the main list
            if(m_main_mutex.try_lock())
            {
                task.m_time = time;
                if(m_main)
                {
                    // First remove the task if the task is already in the main list
                    if(m_main == &task)
                    {
                        m_main = task.m_next;
                        task.m_next = nullptr;
                    }
                    else
                    {
                        Task *current = m_main->m_next, *previous = m_main;
                        while(current)
                        {
                            if(current == &task)
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
                        if(m_main->m_time > task.m_time)
                        {
                            task.m_next = m_main;
                            m_main   = &task;
                            m_main_mutex.unlock();
                            return;
                        }
                        Task *previous = m_main;
                        Task *current = previous->m_next;
                        while(current)
                        {
                            if(current->m_time > task.m_time)
                            {
                                task.m_next = current;
                                previous->m_next = &task;
                                m_main_mutex.unlock();
                                return;
                            }
                            previous = current;
                            current = current->m_next;
                        }
                        previous->m_next = &task;
                        task.m_next = nullptr;
                    }
                    else
                    {
                        m_main = &task;
                        task.m_next = nullptr;
                    }
                }
                else
                {
                    m_main = &task;
                    task.m_next = nullptr;
                }
                m_main_mutex.unlock();
            }
            // Adds to task the futur list
            else
            {
                std::lock_guard<std::mutex> lock(m_futur_mutex);
                task.m_futur_time = time;
                task.m_futur_next = m_futur;
                task.m_futur_type = Task::futur_type_t::to_add;
                m_futur = &task;
            }
        }
        
        void Scheduler::Queue::remove(Task& task)
        {
            if(m_main_mutex.try_lock())
            {
                if(m_main)
                {
                    if(m_main == &task)
                    {
                        m_main = task.m_next;
                    }
                    else
                    {
                        Task *current = m_main->m_next, *previous = m_main;
                        while(current)
                        {
                            if(current == &task)
                            {
                                previous->m_next = current->m_next;
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
                task.m_futur_time = time_point_t::min();
                task.m_futur_next = m_futur;
                task.m_futur_type = Task::futur_type_t::to_remove;
                m_futur = &task;
            }
        }
        
        void Scheduler::prepare(id_t const queue_id)
        {
            m_queues[queue_id];
        }
        
        void Scheduler::perform(time_point_t const time)
        {
            for(auto& queue : m_queues)
            {
                queue.second.perform(time);
            }
        }
        
        void Scheduler::add(Task& task, time_point_t const time)
        {
            m_queues[task.m_queue_id].add(task, time);
        }
        
        void Scheduler::remove(Task& task)
        {
            m_queues[task.m_queue_id].remove(task);
        }
    }
}
