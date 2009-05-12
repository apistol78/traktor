#ifndef traktor_render_Cg_H
#define traktor_render_Cg_H

namespace traktor
{
	namespace render
	{

class ShaderGraph;
class CgContext;

class Cg
{
public:
	bool generate(
		CgContext& cx,
		ShaderGraph* shaderGraph
	);
};

	}
}

#endif	// traktor_render_Cg_H
