#include <atomic>
#include "Core/Ref.h"
#include "Core/RefArray.h"
#include "Core/Containers/SmallMap.h"
#include "Render/Editor/Node.h"
#include "Render/Editor/Shader/INodeTraits.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

std::atomic< int32_t > s_lock(0);
SmallMap< const TypeInfo*, Ref< INodeTraits > > s_traits;

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.INodeTraits", INodeTraits, Object)

const INodeTraits* INodeTraits::find(const Node* node)
{
	// Other threads should wait until s_traits are ready.
	while (s_lock == 1)
		;

	// First thread should create s_traits.
	if (s_lock++ == 0)
	{
		for (auto traitsType : type_of< INodeTraits >().findAllOf(false))
		{
			Ref< INodeTraits > tr = checked_type_cast< INodeTraits*, false >(traitsType->createInstance());
			T_ASSERT(tr);

			TypeInfoSet nodeTypes = tr->getNodeTypes();
			for (const auto& nodeType : nodeTypes)
				s_traits[nodeType] = tr;
		}

		s_lock += 2;
	}
	--s_lock;

	// Find traits from node type.
	auto it = s_traits.find(&type_of(node));
	return it != s_traits.end() ? it->second : nullptr;
}

	}
}
