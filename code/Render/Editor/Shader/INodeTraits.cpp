#include "Core/RefArray.h"
#include "Render/Shader/Node.h"
#include "Render/Editor/Shader/INodeTraits.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.INodeTraits", INodeTraits, Object)

const INodeTraits* findNodeTraits(const Node* node)
{
	static RefArray< INodeTraits > s_traits;
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
