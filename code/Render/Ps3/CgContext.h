#ifndef traktor_render_CgContext_H
#define traktor_render_CgContext_H

#include <map>
#include "Core/Heap/Ref.h"
#include "Render/Ps3/CgEmitter.h"
#include "Render/Ps3/CgShader.h"
#include "Render/Ps3/TypesPs3.h"

namespace traktor
{
	namespace render
	{

class ShaderGraph;
class OutputPin;
class CgVariable;

class CgContext
{
public:
	CgContext(ShaderGraph* shaderGraph);

	CgVariable* emitInput(Node* node, const std::wstring& inputPinName);

	CgVariable* emitOutput(Node* node, const std::wstring& outputPinName, CgType type);

	void enterVertex();

	void enterPixel();

	bool inVertex() const;

	bool inPixel() const;

	CgShader& getVertexShader();

	CgShader& getPixelShader();

	CgShader& getShader();

	CgEmitter& getEmitter();

	RenderState& getRenderState();

private:
	Ref< ShaderGraph > m_shaderGraph;
	CgShader m_vertexShader;
	CgShader m_pixelShader;
	CgShader* m_currentShader;
	CgEmitter m_emitter;
	RenderState m_renderState;
};

	}
}

#endif	// traktor_render_CgContext_H
