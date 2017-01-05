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
        //! @details A task describes ...
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
        //! @brief The container for a set of taskes.
        //! @details ...
        class Scheduler
        {
        public:
            using time_point_t = Task::time_point_t;
            
            Scheduler();
            
            void perform(time_point_t const time);
            
            void add(Task& t, time_point_t const time);
            
            void remove(Task const& t);
            
        private:
            Task*           m_head;
            Task*           m_free;
            std::mutex      m_mutex;
        };
    }
}

#endif // KIWI_SCHEDULER_HPP_INCLUDED


