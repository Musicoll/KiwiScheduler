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

#ifndef KIWI_SCHEDULER_TEST_HPP_INCLUDED
#define KIWI_SCHEDULER_TEST_HPP_INCLUDED

#include <KiwiScheduler.hpp>
#include <thread>
#include <atomic>
#include <cstdlib>
#include <vector>
#include <cassert>

// A concurency test.
int perform_test1();


int perform_test2();

// A test where several tasks are inserted, duplicated in different orders.
int perform_test3();

int perform_test4();

// Todo A test where the callback re-add the task.

#endif // KIWI_SCHEDULER_TEST_HPP_INCLUDED


