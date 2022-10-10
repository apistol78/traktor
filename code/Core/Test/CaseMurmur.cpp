#include "Core/Misc/Murmur3.h"
#include "Core/Test/CaseMurmur.h"

namespace traktor::test
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.test.CaseMurmur", 0, CaseMurmur, Case)

void CaseMurmur::run()
{
	const char text[] = "Hello world! Testing Murmur3 hash.";

	Murmur3 m;
	m.begin();
	m.feed(text, sizeof(text));
	m.end();

	const uint32_t h = m.get();
	CASE_ASSERT(h == 3720714118);
}

}
