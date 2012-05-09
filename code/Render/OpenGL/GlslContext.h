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

	void emitOutput(Node* node, const std::wstring& outputPinName, GlslVariable* variable);

	void enterVertex();

	void enterFragment();

	bool inVertex() const;

	bool inFragment() const;
	
	bool allocateInterpolator(int32_t width, int32_t& outId, int32_t& outOffset);

	void setRequireDerivatives();
	
	bool getRequireDerivatives() const;

	void setRequireTranspose();

	bool getRequireTranspose() const;

	GlslShader& getVertexShader();

	GlslShader& getFragmentShader();

	GlslShader& getShader();

	GlslEmitter& getEmitter();

	RenderState& getRenderState();

	void defineTexture(const std::wstring& texture);

	bool defineSampler(uint32_t stateHash, const std::wstring& texture, int32_t& outStage);

	const std::vector< std::wstring >& getTextures() const;

	const std::vector< std::pair< int32_t, int32_t > >& getSamplers() const;

private:
	Ref< const ShaderGraph > m_shaderGraph;
	GlslShader m_vertexShader;
	GlslShader m_fragmentShader;
	GlslShader* m_currentShader;
	GlslEmitter m_emitter;
	RenderState m_renderState;
	int32_t m_nextStage;
	bool m_requireDerivatives;
	bool m_requireTranspose;
	std::vector< uint8_t > m_interpolatorMap;
	std::vector< std::wstring > m_textures;
	std::vector< std::pair< int32_t, int32_t > > m_samplers;
	std::map< uint32_t, int32_t > m_samplersMap;
};

	}
}

#endif	// traktor_render_GlslContext_H
