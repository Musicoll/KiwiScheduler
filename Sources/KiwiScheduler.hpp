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

#ifndef KIWI_SCHEDULER_HPP_INCLUDED
#define KIWI_SCHEDULER_HPP_INCLUDED

#include <iostream>
#include <chrono>
#include <mutex>

namespace kiwi
{
    namespace scheduler
    {
        // ================================================================================ //
        //                                      TASK                                        //
        // ================================================================================ //
        //! @brief The task that can be added to a scheduler.
        //! @details Perhaps we should manage a pointer to pass to the method...
        class Task
        {
        public:
            using method_t          = std::function<void()>;
            using time_point_t      = std::chrono::high_resolution_clock::time_point;

            Task(method_t&& m) : m_next(nullptr), m_method(m), m_time() {}
            
        private:
            Task*           m_next;
            method_t        m_method;
            time_point_t    m_time;
            
            friend class Scheduler;
        };
        
        
        // ==================================================================================== //
        //                                      SCHEDULER                                       //
        // ==================================================================================== //
        //! @brief The container for a set of tasks.
        //! @details The scheduler manages a set of taks for one consumer and one producer. It
        //! means that only one thread can add the tasks and only one tread can consume the
        //! tasks.
        //! @todo For the moment removing a task with the remove method isn't lock free (but it is
        //! within the add method). Perhaps we should create a lock free version of it, it would
        //! also simplify the add method.
        class Scheduler
        {
        public:
            using time_point_t = Task::time_point_t;
            
            //! @brief the constructor.
            Scheduler();
            
            //! @brief the destructor.
            ~Scheduler();
            
            //! @brief Performs the tasks until the specified time.
            //! @details The method calls all the task before the specified time and then adds
            //! tasks that could have been added during this operation.
            //! @param time The time point.
            void perform(time_point_t const time);
            
            //! @brief Adds a task at a specified time.
            //! @details Only one instance of a task can be added to the scheduler because the
            //! task owns its time point, so if the scheduler owns two instances of the same
            //! task one of these instances won't have the right time. Therefore, the task is
            //! removed from the list if it has already been added and not consumed.
            //! @param t    The task to add.
            //! @param time The time point where the task should be inserted.
            void add(Task& t, time_point_t const time);
            
            //! @brief Removes a task.
            //! @details Removing a task is not lock free.
            //! @param t The task to remove.
            void remove(Task const& t);
            
        private:
            Task*           m_main;         //! The main sorted linked list of tasks.
            Task*           m_futur;        //! The linked list of tasks that will be inserted.
            std::mutex      m_main_mutex;
            std::mutex      m_futur_mutex;
        };
    }
}

#endif // KIWI_SCHEDULER_HPP_INCLUDED


