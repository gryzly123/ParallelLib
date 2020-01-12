#pragma once

// ParallelLib.h
//   Main ParallelLib header that includes whole library functionality
//   and the #define based syntax that attempts to resemble OpenMP API.

// ------------------ REQUIRED INCLUDES ------------------

//external std dependencies
	#include <atomic>
	#include <functional>
	#include <thread>
	#include <mutex>

//helper classes
	#include "SetOnce.h"
	#include "ExecParams.h"
	#include "Singleton.h"

//main parallel structures
	#include "ParallelBase.h"
	#include "ParallelDo.h"
	#include "ParallelFor.h"
	#include "ParallelSections.h"

// -------------------- #DEFINE SYNTAX -------------------

//parallel structure constructors

	/* Creates a parallel region where each thread receives the same block of code.
	   Supported tags:
	   *  num_threads,
	   *  exec_master,
	   *  nowait
	*/
	#define parallel_do(tag, params, method) \
		pDo tag; tag params .Do([&](pExecParams ___pExecParams) method);
	
	/* Creates a parallel for loop where each thread receives the same block of code
	   but executes it for different iterator values. Supports linear incrementation (i+=n, i-=n).
	    Supported tags:
	    *  num_threads,
	    *  exec_master,
	    *  nowait,
	    *  schedule(type, chunksize)
	*/
	#define parallel_for(tag, iterator, init_val, max_val, increment, params, method) \
		pFor tag; tag params .Do(init_val, max_val, increment, [&](pExecParams ___pExecParams, int iterator) method);

	/* Creates a parallel region where each thread receives its own block of code.
	Supported tags:
	*  exec_master,
	*  nowait
	*/
#define parallel_sections(tag, params, sections) \
		pSections tag; tag params .DoIgnoreFirst({[&](pExecParams ___pExecParams){} sections });

#define parallel_section \
	,[&](pExecParams ___pExecParams)

//helper macros for ie. nowait(yes) or exec_master(no)

	#define yes true
	#define no false

//parallel structure tags

	#define num_threads(n)              .NumThreads(n)
	#define exec_master(c)              .ExecuteOnMaster(c)
	#define nowait(c)                   .NoWait(c)
	#define schedule(sched, chunksize)  .Schedule(pSchedule::sched).ChunkSize(chunksize) //schedule setter similar to OpenMP (static, dynamic, guided, except here in PascalCase)
	#define schedulev(sched, chunksize) .Schedule(sched)           .ChunkSize(chunksize) //schedule setter for situations where we want to pass the pSchedule enum value by variable

//parallel variables

	#define create_public(type, name, value) std::atomic<type> name
	#define create_private(type, name) thread_local type name

//variables available in parallel blocks

	#define THREAD_ID ___pExecParams.ThreadId

//functions available in parallel blocks

	#define PARALLEL_SLEEP_MILISECONDS(ms)   pExecParams::SleepMili(ms);
	#define PARALLEL_SLEEP_MICROSECONDS(ms)  pExecParams::SleepMicro(ms);
	#define PARALLEL_SLEEP_NANOSECONDS(ns)   pExecParams::SleepNano(ns);
