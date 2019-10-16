#include "Core/Serialization/DeepHash.h"
#include "Core/Settings/PropertyInteger.h"
#include "Core/Test/CaseHash.h"

namespace traktor
{
	namespace test
	{
        namespace
        {



        }

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.test.CaseHash", 0, CaseHash, Case)

void CaseHash::run()
{
    // Adler32

    // Base64

    // HashStream

    // MD5

    // SHA1

    // DeepHash
    Ref< PropertyInteger > bag1 = new PropertyInteger(44246);
    Ref< PropertyInteger > bag2 = new PropertyInteger(44246);
    Ref< PropertyInteger > bag3 = new PropertyInteger(75598);
    uint32_t h1 = DeepHash(bag1).get();
    uint32_t h2 = DeepHash(bag2).get();
    uint32_t h3 = DeepHash(bag3).get();
    CASE_ASSERT(h1 == h2);
    CASE_ASSERT(h1 != h3);
}

	}
}
