#ifndef traktor_render_GlslContext_H
#define traktor_render_GlslContext_H

#include <map>
#include <vector>
#include "Render/Vulkan/Glsl/GlslEmitter.h"
#include "Render/Vulkan/Glsl/GlslShader.h"

namespace traktor
{
	namespace render
	{

class ShaderGraph;
class InputPin;
class OutputPin;
class GlslVariable;

/*!
 * \ingroup Vulkan
 */
class GlslContext
{
public:
	GlslContext(const ShaderGraph* shaderGraph);

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

	GlslShader& getVertexShader();

	GlslShader& getFragmentShader();

	GlslShader& getShader();

	GlslEmitter& getEmitter();

private:
	Ref< const ShaderGraph > m_shaderGraph;
	GlslShader m_vertexShader;
	GlslShader m_fragmentShader;
	GlslShader* m_currentShader;
	GlslEmitter m_emitter;
	std::vector< uint8_t > m_interpolatorMap;
};

	}
}

#endif	// traktor_render_GlslContext_H
