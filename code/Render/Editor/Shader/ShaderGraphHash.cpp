#include "Core/Serialization/DeepClone.h"
#include "Core/Serialization/DeepHash.h"
#include "Render/Shader/Node.h"
#include "Render/Shader/ShaderGraph.h"
#include "Render/Editor/Shader/ShaderGraphHash.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.ShaderGraphHash", ShaderGraphHash, Object)

uint32_t ShaderGraphHash::calculate(const Node* node)
{
	Ref< Node > nodeCopy = DeepClone(node).create< Node >();

	nodeCopy->setPosition(std::make_pair(0, 0));
	nodeCopy->setComment(L"");

	return DeepHash(nodeCopy).get();
}

uint32_t ShaderGraphHash::calculate(const ShaderGraph* shaderGraph)
{
	Ref< ShaderGraph > shaderGraphCopy = DeepClone(shaderGraph).create< ShaderGraph >();

	const RefArray< Node >& nodes = shaderGraphCopy->getNodes();
	for (RefArray< Node >::const_iterator i = nodes.begin(); i != nodes.end(); ++i)
	{
		(*i)->setPosition(std::make_pair(0, 0));
		(*i)->setComment(L"");
	}

	return DeepHash(shaderGraphCopy).get();
}

	}
}
