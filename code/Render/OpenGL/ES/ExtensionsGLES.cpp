// #include <cstring>
// #if defined(__ANDROID__) || defined(__LINUX__)
// #	include <dlfcn.h>
// #endif
// #include "Core/Log/Log.h"
// #include "Core/Misc/TString.h"
#include "Render/OpenGL/ES/ExtensionsGLES.h"

namespace traktor
{
	namespace render
	{

void initializeExtensions()
{
	// const char* supported = (const char*)glGetString(GL_EXTENSIONS);
	// while (supported && *supported)
	// {
	// 	const char* end = supported;
	// 	while (*end && *end != ' ')
	// 	{
	// 		if ((++end - supported) >= 200)
	// 			break;
	// 	}

	// 	int32_t length = end - supported;

	// 	char tmp[1024];
	// 	strncpy(tmp, supported, length);
	// 	tmp[length] = 0;

	// 	log::info << mbstows(tmp) << Endl;

	// 	supported = end;
	// 	while (*supported == ' ')
	// 	{
	// 		if ((++supported - end) >= 10)
	// 			break;
	// 	}
	// }
}

	}
}
