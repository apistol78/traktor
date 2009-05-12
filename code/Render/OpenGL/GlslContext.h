#ifndef traktor_render_GlslContext_H
#define traktor_render_GlslContext_H

#include <map>
#include <vector>
#include "Core/Heap/Ref.h"
#include "Render/OpenGL/GlslEmitter.h"
#include "Render/OpenGL/GlslShader.h"
#include "Render/OpenGL/TypesOpenGL.h"

namespace traktor
{
	namespace render
	{

class ShaderGraph;
class InputPin;
class OutputPin;
class GlslVariable;

/*!
 * \ingroup OGL
 */
class GlslContext
{
public:
	GlslContext(const ShaderGraph* shaderGraph);

	GlslVariable* emitInput(const InputPin* inputPin);

	GlslVariable* emitInput(Node* node, const std::wstring& inputPinName);

	GlslVariable* emitOutput(Node* node, const std::wstring& outputPinName, GlslType type);

	void enterVertex();

	void enterFragment();

	bool inVertex() const;

	bool inFragment() const;

	GlslShader& getVertexShader();

	GlslShader& getFragmentShader();

	GlslShader& getShader();

	GlslEmitter& getEmitter();

	RenderState& getRenderState();

private:
	Ref< const ShaderGraph > m_shaderGraph;
	GlslShader m_vertexShader;
	GlslShader m_fragmentShader;
	GlslShader* m_currentShader;
	GlslEmitter m_emitter;
	RenderState m_renderState;
};

	}
}

#endif	// traktor_render_GlslContext_H
