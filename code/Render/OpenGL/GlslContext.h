#ifndef traktor_render_GlslContext_H
#define traktor_render_GlslContext_H

#include <map>
#include <vector>
#include "Render/OpenGL/GlslEmitter.h"
#include "Render/OpenGL/GlslShader.h"
#include "Render/OpenGL/TypesOpenGL.h"

namespace traktor
{

class PropertyGroup;

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
	GlslContext(const ShaderGraph* shaderGraph, const PropertyGroup* settings);

	Node* getInputNode(const InputPin* inputPin);

	Node* getInputNode(Node* node, const std::wstring& inputPinName);

	void emit(Node* node);

	GlslVariable* emitInput(const InputPin* inputPin);

	GlslVariable* emitInput(Node* node, const std::wstring& inputPinName);

	GlslVariable* emitOutput(Node* node, const std::wstring& outputPinName, GlslType type);

	void emitOutput(Node* node, const std::wstring& outputPinName, GlslVariable* variable);

	void findNonDependentOutputs(Node* node, const std::wstring& inputPinName, const std::vector< const OutputPin* >& dependentOutputPins, std::vector< const OutputPin* >& outOutputPins) const;

	void findCommonOutputs(Node* node, const std::wstring& inputPin1, const std::wstring& inputPin2, std::vector< const OutputPin* >& outOutputPins) const;

	void enterVertex();

	void enterFragment();

	bool inVertex() const;

	bool inFragment() const;
	
	bool allocateInterpolator(int32_t width, int32_t& outId, int32_t& outOffset);

	void setRequireDerivatives();
	
	bool getRequireDerivatives() const;

	void setRequireTranspose();

	bool getRequireTranspose() const;

	const PropertyGroup* getSettings() const;

	GlslShader& getVertexShader();

	GlslShader& getFragmentShader();

	GlslShader& getShader();

	GlslEmitter& getEmitter();

	RenderStateOpenGL& getRenderState();

	void defineTexture(const std::wstring& texture);

	bool defineUniform(const std::wstring& name, GLenum type, GLuint length);

	bool defineSampler(const std::wstring& name, uint32_t stateHash, GLenum target, const std::wstring& texture, int32_t& outStage);

	const std::vector< std::wstring >& getTextures() const;

	const std::vector< NamedUniformType >& getUniforms() const;

	const std::vector< SamplerBindingOpenGL >& getSamplers() const;

private:
	Ref< const ShaderGraph > m_shaderGraph;
	Ref< const PropertyGroup > m_settings;
	GlslShader m_vertexShader;
	GlslShader m_fragmentShader;
	GlslShader* m_currentShader;
	GlslEmitter m_emitter;
	RenderStateOpenGL m_renderState;
	int32_t m_nextStage;
	bool m_requireDerivatives;
	bool m_requireTranspose;
	std::vector< uint8_t > m_interpolatorMap;
	std::vector< std::wstring > m_textures;
	std::vector< NamedUniformType > m_uniforms;
	std::vector< SamplerBindingOpenGL > m_samplers;
	std::vector< uint32_t > m_samplerStateHashes;
};

	}
}

#endif	// traktor_render_GlslContext_H
