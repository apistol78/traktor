#include "Core/FbxLock.h"

namespace traktor
{

#if defined(_WIN32) || defined(__APPLE__) || defined(__LINUX__)
Semaphore g_fbxLock;
#endif

}
