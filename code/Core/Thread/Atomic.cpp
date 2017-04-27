/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Thread/Atomic.h"
#if defined(_DEBUG)
#	if defined(_WIN32)
#		include "Core/Thread/Win32/Atomic.inl"
#	elif defined(__APPLE__)
#		include "Core/Thread/OsX/Atomic.inl"
#	elif defined(_PS3)
#		include "Core/Thread/Ps3/Atomic.inl"
#	else
#		include "Core/Thread/Linux/Atomic.inl"
#	endif
#endif
