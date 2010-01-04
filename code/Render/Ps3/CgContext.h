#ifndef traktor_render_CgContext_H
#define traktor_render_CgContext_H

#include "Render/Ps3/TypesPs3.h"
#include "Render/Ps3/CgEmitter.h"
#include "Render/Ps3/CgShader.h"

namespace traktor
{
	namespace render
	{

class ShaderGraph;
class InputPin;
class OutputPin;
class CgVariable;

/*!
 * \ingroup DX9 Xbox360
 */
class CgContext
{
public:
	CgContext(const ShaderGraph* shaderGraph);

	CgVariable* emitInput(const InputPin* inputPin);

	CgVariable* emitInput(Node* node, const std::wstring& inputPinName);

	CgVariable* emitOutput(Node* node, const std::wstring& outputPinName, CgType type);

	void enterVertex();

	void enterPixel();

	bool inVertex() const;

	bool inPixel() const;

	int32_t allocateInterpolator();

	int32_t allocateBooleanRegister();

	void allocateVPos();

	inline CgShader& getVertexShader() { return m_vertexShader; }

	inline CgShader& getPixelShader() { return m_pixelShader; }

	inline CgShader& getShader() { return *m_currentShader; }

	inline CgEmitter& getEmitter() { return m_emitter; }

	inline RenderState& getRenderState() { return m_renderState; }

	inline bool needVPos() const { return m_needVPos; }

private:
	Ref< const ShaderGraph > m_shaderGraph;
	CgShader m_vertexShader;
	CgShader m_pixelShader;
	CgShader* m_currentShader;
	CgEmitter m_emitter;
	RenderState m_renderState;
	int32_t m_interpolatorCount;
	int32_t m_booleanRegisterCount;
	bool m_needVPos;
};

	}
}

#endif	// traktor_render_CgContext_H
