#if defined(_DEBUG)
#	if defined(_WIN32)
#		include "Core/Thread/Win32/Atomic.inl"
#	elif defined(__APPLE__)
#		include "Core/Thread/OsX/Atomic.inl"
#	else
#		include "Core/Thread/Linux/Atomic.inl"
#	endif
#endif
