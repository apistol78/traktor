/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#if defined(_WIN32)
#	include "Render/Vulkan/Win32/ApiLoader.h"
#elif defined(__ANDROID__)
#	include "Render/Vulkan/Android/ApiLoader.h"
#endif

