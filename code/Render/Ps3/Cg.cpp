#include "Render/Ps3/Cg.h"
#include "Render/Ps3/CgContext.h"
#include "Render/ShaderGraph.h"
#include "Render/Nodes.h"

namespace traktor
{
	namespace render
	{

bool Cg::generate(
	CgContext& cx,
	ShaderGraph* shaderGraph
)
{
	for (RefList< Node >::const_iterator i = shaderGraph->getNodes().begin(); i != shaderGraph->getNodes().end(); ++i)
	{
		Node* node = *i;
		if (is_a< VertexOutput >(node))
			cx.getEmitter().emit(cx, node);
		else if (is_a< PixelOutput >(node))
			cx.getEmitter().emit(cx, node);
	}
	return true;
}

	}
}
