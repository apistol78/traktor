#include "Core/RefArray.h"
#include "Core/Thread/Acquire.h"
#include "Core/Thread/Semaphore.h"
#include "Render/Shader/Node.h"
#include "Render/Editor/Shader/INodeTraits.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

RefArray< INodeTraits > s_traits;
Semaphore s_lock;

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.INodeTraits", INodeTraits, Object)

const INodeTraits* INodeTraits::find(const Node* node)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(s_lock);

	if (s_traits.empty())
	{
		std::vector< const TypeInfo* > traitsTypes;
		type_of< INodeTraits >().findAllOf(traitsTypes, false);

		for (std::vector< const TypeInfo* >::const_iterator i = traitsTypes.begin(); i != traitsTypes.end(); ++i)
		{
			Ref< INodeTraits > traits = checked_type_cast< INodeTraits*, false >((*i)->createInstance());
			s_traits.push_back(traits);
		}
	}

	const TypeInfo& nodeType = type_of(node);
	for (RefArray< INodeTraits >::const_iterator i = s_traits.begin(); i != s_traits.end(); ++i)
	{
		TypeInfoSet nodeTypes = (*i)->getNodeTypes();
		if (nodeTypes.find(&nodeType) != nodeTypes.end())
			return *i;
	}

	return 0;
}

	}
}
