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

#ifndef KIWI_ENGINE_SCHEDULER_HPP_INCLUDED
#define KIWI_ENGINE_SCHEDULER_HPP_INCLUDED

#include <chrono>
#include <mutex>
#include <map>

namespace kiwi
{
    namespace engine
    {
        // ================================================================================ //
        //                                      SCHEDULER                                   //
        // ================================================================================ //
        //! @brief The manager of tasks.
        //! @details The scheduler manages a set of tasks inside several queues. It a single
        //! consumer but it can accepts several producer defined by ids, that match with
        //! queues. So to add several events in a concurrency context, you need a use a
        //! different id for each thread. And of course, you can use several ids inside the
        //! same thread.
        //! @todo later we can add priorities.
        class Scheduler
        {
        public:
            using id_t              = uint32_t;
            using time_point_t      = size_t;
            
            // ============================================================================ //
            //                                      TIMER                                   //
            // ============================================================================ //
            //! @brief The timer is the pure virtual class that used by task.
            class Timer
            {
            public:
                
                //! @brief The destructor
                virtual ~Timer() {}
                
                //! @brief The callback method
                virtual void callback() = 0;
            };
            
            // ============================================================================ //
            //                                      TASK                                    //
            // ============================================================================ //
            //! @brief The task that can be added to a scheduler.
            //! @todo Perhaps we should manage a pointer to pass to the method...
            class Task
            {
            public:
                
                //! @brief the constructor.
                //! @param master The method to call.
                //! @param queue_id The id of the queue in wich it will be added.
                Task(Timer& master, const id_t queue_id = 0) : m_timer(master), m_queue_id(queue_id) {}
                
            private:
                enum Status : unsigned
                {
                    available = 0,
                    inserted  = 1,
                    to_add    = 2,
                    to_remove = 3
                };
                
                Task*           m_next = nullptr;           //!< The next task in the queue.
                time_point_t    m_time;                     //!< The current time of the task.
                
                Task*           m_process_next = nullptr;   //!< The next future task in the queue.
                Timer&          m_timer;                    //!< The method to call.
                
                Task*           m_futur_next  = nullptr;    //!< The next future task in the queue.
                time_point_t    m_futur_time;   //!< The future time if it waits for the insertion.
                Status          m_status      = Status::available;   //!< The future action.
                const id_t      m_queue_id;     //!< The id of the queue.
                friend class Scheduler;
            };
            
            //! @brief Prepare the scheduler for a specific queue.
            //! @details If a queue has never been used, the first use of queue calls its
            //! allocator. If for some case, you want to avoid this allocation, for example in
            //! the audio thead, you can this method to pre-allocate the queue.
            //! @param queue_id The id of the queue to prepare.
            void prepare(id_t const queue_id);
            
            //! @brief Performs the tasks until the specified time.
            //! @details The method performs all the tasks of all the queues, until the
            //! defined time point. So for each queue, the method calls all the task before
            //! the specified time and then adds tasks that could have been added during this
            //! operation.
            //! @param time The time point.
            //! @todo We can add another time point that specified when we should avoid to
            //! continue the process.
            void perform(time_point_t const time);
            
            //! @brief Adds a task at a specified time.
            //! @details The method performs adds a task of to its queues, and allocate a new
            //! queue if needed. Only one instance of a task can be added to a queue because
            //! the task owns its time point, so if the queue owns two instances of the same
            //! task one of these instances won't have the right time. Therefore, the task is
            //! removed from the queue if it has already been added and not consumed. The task
            //! is already defined by a queue's id, so at the end only one instance of a task
            //! can be added to a scheduler.
            //! @param task The task to add.
            //! @param time The time point where the task should be inserted.
            void add(Task& task, time_point_t const time);
            
            //! @brief Removes a task.
            //! @details This method removes a task from its queue. 
            //! @param task The task to remove.
            void remove(Task& task);
            
        private:
            
            // ============================================================================ //
            //                                  SCHEDULER QUEUE                             //
            // ============================================================================ //
            //! @brief The container for a set of tasks.
            //! @details The queue manages a set of taks for one consumer and one producer. It
            //! means that only one thread can add or remove the tasks and only one tread can
            //! consume the tasks. If only one thread can add or remove a task, it means that
            //! these two methods can only be called sequentially but the perform method can be
            //! called in concurence.
            class Queue
            {
            public:                
                //! @brief Performs the tasks until the specified time.
                //! @details The method calls all the task before the specified time and then
                //! adds tasks that could have been added during this operation.
                //! @param time The time point.
                void perform(time_point_t const time);
                
                //! @brief Adds a task at a specified time.
                //! @details Only one instance of a task can be added to the queue because the
                //! task owns its time point, so if the queue owns two instances of the same
                //! task one of these instances won't have the right time. Therefore, the task
                //! is removed from the queue if it has already been added and not consumed.
                //! @param task The task to add.
                //! @param time The time point where the task should be inserted.
                void add(Task& task, time_point_t const time);
                
                //! @brief Removes a task.
                //! @details This method is also lock free but for lock reasons, the method
                //! can't be used by the add method.
                //! @param task The task to remove.
                void remove(Task& task);
                
            private:
                Task*           m_main  = nullptr;  //!< The main sorted linked list of tasks.
                Task*           m_futur = nullptr;  //!< The linked list of tasks that will be inserted.
                std::mutex      m_main_mutex;       //!< The main list mutex.
                std::mutex      m_futur_mutex;      //!< The futur list mutex.
            };
            
            std::map<id_t, Queue> m_queues; //!< The list of queues.
        };
        
        
        class Callback
        {
        public:
            
            //! @brief The destructor
            virtual ~Callback() {}
            
            //! @brief The callback method
            virtual void call() = 0;
        };
        
        class Command
        {
        public:
            
            Command(Callback& c) : m_callback(c), m_state(Status::available), m_next(nullptr) {}
        private:
            enum Status : unsigned
            {
                available = 0,
                inserted  = 1,
                removed   = 2
            };
            
            Callback&           m_callback;
            std::atomic<Status> m_state;
            Command*            m_next;
            friend class List;
        };
    
        // ============================================================================ //
        //                                  SCHEDULER LIST                              //
        // ============================================================================ //
        class List
        {
            // Une tache n'est pas vraiment enlevée avant la fonction perform
        public:
            //! @brief Performs all the tasks.
            //! @details The method calls all the task before.
            void perform();
            
            //! @brief Adds a task.
            //! @details Only one instance of a task can be added to the list to ensure
            //! consistency with the queu. Therefore, the task is removed from the queue
            //! if it has already been added and not consumed.
            //! @param task The task to add.
            void add(Command& task);
            
            //! @brief Removes a task.
            //! @details This method is also lock free but for lock reasons, the method
            //! can't be used by the add method.
            //! @param task The task to remove.
            void remove(Command& task, bool sequential);
        private:
            struct double_ptr_list_t
            {
                Command* head;
                Command* tail;
            };
            
            std::atomic<double_ptr_list_t> m_list;
        };
    }
}

#endif // KIWI_ENGINE_SCHEDULER_HPP_INCLUDED


