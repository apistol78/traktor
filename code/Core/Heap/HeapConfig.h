#ifndef traktor_HeapConfig_H
#define traktor_HeapConfig_H

/*! \ingroup Core */
//@{

/*!
 * T_HEAP_COLLECT_CYCLES		Enable cycle collector; will leak memory of reference cycles are constructed.
 * T_HEAP_CONCURRENT_COLLECT	Use multiple threads to collect objects.
 * T_HEAP_DEBUG_KEEP_DEAD		Instead of freeing object memory, tag object header as dead and zero-out object.
 * T_HEAP_DEBUG_ALLOCATOR		Use DebugAllocator as primary memory allocator, T_HEAP_FAST_ALLOCATOR mustn't be defined.
 * T_HEAP_FAST_ALLOCATOR		Use FastAllocator as primary memory allocator.
 * T_HEAP_THREAD_SAFE			Serialize access to heap
 */

#if defined(_DEBUG)
#	define T_HEAP_COLLECT_CYCLES
#	if !defined(WINCE)
#		define T_HEAP_CONCURRENT_COLLECT
#	endif
#	define T_HEAP_DEBUG_KEEP_DEAD
#	define T_HEAP_THREAD_SAFE
#else	// _RELEASE
#	define T_HEAP_COLLECT_CYCLES
#	if !defined(WINCE)
#		define T_HEAP_CONCURRENT_COLLECT
#	endif
#	define T_HEAP_FAST_ALLOCATOR
#	define T_HEAP_THREAD_SAFE
#endif

//@}

#endif	// traktor_HeapConfig_H
