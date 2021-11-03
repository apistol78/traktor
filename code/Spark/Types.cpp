#include <atomic>
#include "Spark/Types.h"

namespace traktor
{
	namespace spark
	{
		namespace
		{

std::atomic< int32_t > s_tag(0);

		}

int32_t allocateCacheTag()
{
	return ++s_tag;
}

	}
}
