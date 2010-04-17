#ifndef traktor_render_GlslContext_H
#define traktor_render_GlslContext_H

#include <map>
#include <vector>
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

	bool defineSamplerTexture(const std::wstring& textureName, int32_t& outStage);

	const std::map< std::wstring, int32_t >& getSamplerTextures() const;

private:
	Ref< const ShaderGraph > m_shaderGraph;
	GlslShader m_vertexShader;
	GlslShader m_fragmentShader;
	GlslShader* m_currentShader;
	GlslEmitter m_emitter;
	RenderState m_renderState;
	int32_t m_nextStage;
	std::map< std::wstring, int32_t > m_samplerTextures;
};

	}
}

#endif	// traktor_render_GlslContext_H
