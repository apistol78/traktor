#ifndef traktor_HeapConfig_H
#define traktor_HeapConfig_H

/*! \ingroup Core */
//@{

/*!
 * T_HEAP_FAST_ALLOCATOR		Use FastAllocator as primary memory allocator.
 * T_HEAP_CONCURRENT_COLLECT	Use multiple threads to collect objects.
 * T_HEAP_THREAD_SAFE			Serialize access to heap
 * T_HEAP_DEBUG_ALLOCATOR		Use DebugAllocator as primary memory allocator, T_HEAP_FAST_ALLOCATOR mustn't be defined.
 */

#if defined(_DEBUG)
#	if !defined(WINCE)
#		define T_HEAP_FAST_ALLOCATOR
#		define T_HEAP_CONCURRENT_COLLECT
#	endif
#	define T_HEAP_THREAD_SAFE
#else	// _RELEASE
#	if !defined(WINCE)
#		define T_HEAP_CONCURRENT_COLLECT
#	endif
#	define T_HEAP_FAST_ALLOCATOR
#	define T_HEAP_THREAD_SAFE
#endif

//@}

#endif	// traktor_HeapConfig_H
