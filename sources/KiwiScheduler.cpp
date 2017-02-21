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
    namespace engine
    {
        
        // ================================================================================ //
        //                                  SCHEDULER QUEUE                                 //
        // ================================================================================ //
        
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
                    Task::Status operation = current->m_status;
                    current->m_status = Task::Status::available;
                    current->m_futur_next = nullptr;
                    m_futur_mutex.unlock();
                    if(operation == Task::Status::to_add)
                    {
                        add(*current, current->m_futur_time);
                    }
                    else if(operation == Task::Status::to_remove)
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
                ready->m_timer.callback();
                ready = ready->m_process_next;
            }
        }
        
        void Scheduler::Queue::add(Task& task, time_point_t const time)
        {
            // If we're not performing on the main list
            if(m_main_mutex.try_lock())
            {
                task.m_time = time;
                task.m_status = Task::Status::available;
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
                if(task.m_status == Task::Status::available)
                {
                    task.m_futur_next = m_futur;
                    m_futur = &task;
                }
                task.m_futur_time = time;
                task.m_status = Task::Status::to_add;
            }
        }
        
        void Scheduler::Queue::remove(Task& task)
        {
            if(m_main_mutex.try_lock())
            {
                task.m_status = Task::Status::available;
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
                if(task.m_status == Task::Status::available)
                {
                    task.m_futur_next = m_futur;
                    m_futur = &task;
                }
                task.m_futur_time = 0;
                task.m_status = Task::Status::to_remove;
            }
        }
        
        // ================================================================================ //
        //                                      SCHEDULER                                   //
        // ================================================================================ //
        
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
        
        
        
        
        // ================================================================================ //
        //                                  SCHEDULER LIST                                  //
        // ================================================================================ //
        
        void List::perform()
        {
            // Retrieves the list of tasks in a
            // local tempory pointer and sets
            // the member list to null.
            Command* head = m_list.exchange(double_ptr_list_t{nullptr, nullptr}).head;
            while(head)
            {
                // Calls the callback method of the
                // task if its status is inserted,
                // otherwise set the status available
                Command* next = head->m_next;
                Command::Status state_inserted  = Command::Status::inserted;
                if(head->m_state.compare_exchange_strong(state_inserted, Command::Status::available))
                {
                    head->m_callback.call();
                }
                else
                {
                    head->m_state.store(Command::Status::available);
                }
                // Retrieves the next task
                head = next;
            }
        }
        
        void List::add(Command& task)
        {
            Command::Status state_available = Command::Status::available;
            Command::Status state_removed   = Command::Status::removed;
            if(task.m_state.compare_exchange_strong(state_available, Command::Status::inserted))
            {
                task.m_next = nullptr;
                double_ptr_list_t empty_list{nullptr, nullptr};
                if(!m_list.compare_exchange_strong(empty_list, double_ptr_list_t({&task, &task})))
                {
                    // Perhaps here it's not lock free.
                    m_list.load().tail->m_next = &task;
                }
            }
            else
            {
                task.m_state.compare_exchange_strong(state_removed, Command::Status::inserted);
            }
        }
        
        void List::remove(Command& task, bool sequential)
        {
            Command::Status state_inserted = Command::Status::inserted;
            task.m_state.compare_exchange_strong(state_inserted, Command::Status::removed);
        }
    }
}
