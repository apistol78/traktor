#include "Core/Ref.h"
#include "Core/RefArray.h"
#include "Core/Containers/SmallMap.h"
#include "Core/Thread/Acquire.h"
#include "Core/Thread/Semaphore.h"
#include "Render/Editor/Node.h"
#include "Render/Editor/Shader/INodeTraits.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

Semaphore s_lock;
SmallMap< const TypeInfo*, Ref< INodeTraits > > s_traits;

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.INodeTraits", INodeTraits, Object)

const INodeTraits* INodeTraits::find(const Node* node)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(s_lock);

	// Should we create s_traits.
	if (s_traits.empty())
	{
		// Find all concrete INodeTraits classes.
		TypeInfoSet traitsTypes;
		type_of< INodeTraits >().findAllOf(traitsTypes, false);

		// Instantiate traits.
		for (const auto& traitsType : traitsTypes)
		{
			Ref< INodeTraits > tr = checked_type_cast< INodeTraits*, false >(traitsType->createInstance());
			T_ASSERT(tr);

			TypeInfoSet nodeTypes = tr->getNodeTypes();
			for (const auto& nodeType : nodeTypes)
				s_traits[nodeType] = tr;
		}
	}

	// Find traits from node type.
	auto it = s_traits.find(&type_of(node));
	return it != s_traits.end() ? it->second : nullptr;
}

	}
}
