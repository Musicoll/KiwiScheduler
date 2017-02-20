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


#ifndef KIWI_ENGINE_SCHEDULER_TEST_HPP_INCLUDED
#define KIWI_ENGINE_SCHEDULER_TEST_HPP_INCLUDED

#include <atomic>
#include <thread>
#include <vector>
#include <memory>
#include <KiwiScheduler.hpp>

namespace kiwi
{
    namespace engine
    {
        class Instance;
            
        // ================================================================================ //
        //                                      OBJECT                                      //
        // ================================================================================ //
        //! @brief The pure virtual class that generates tasks.
        //! @details The class inherits from the scheduler's timer...
        class Object : public Scheduler::Timer
        {
        public:
            using Task      = Scheduler::Task;
            using Ms        = std::chrono::milliseconds;
            
            enum Types
            {
                Main  = 0,
                Dsp   = 1,
                Gui   = 2,
                High  = 3
            };
                
            Object(Instance& instance, Types threadid);
            Object(Object const& o);
            virtual ~Object();
            void defer(Ms const time);
            void remove();
            void callback() override;
            virtual void process()  = 0;
            
        private:
            Instance& m_instance;
            Task      m_task;
            Types     m_type;
        };
        
        // ================================================================================ //
        //                                      INSTANCE                                    //
        // ================================================================================ //
        //! @brief ...
        class Instance : private Scheduler
        {
        public:
            using Task      = Object::Task;
            using Ms        = Object::Ms;
            using Clock     = std::chrono::high_resolution_clock;
            
            void defer(Task& task, Ms const time);
            void remove(Task& task);
            void access();
            void run();
        private:
            
            std::atomic<size_t> m_time;
            std::mutex          m_data_mutex;
            bool                m_data_access;
        };
    }
}

#endif // KIWI_ENGINE_SCHEDULER_HPP_INCLUDED

