/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/FbxLock.h"

namespace traktor
{

#if defined(_WIN32) || defined(__APPLE__) || defined(__LINUX__)
Semaphore g_fbxLock;
#endif

}
