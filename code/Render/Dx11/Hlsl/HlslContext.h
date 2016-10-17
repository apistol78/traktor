#ifndef traktor_render_HlslContext_H
#define traktor_render_HlslContext_H

#include <map>
#include "Render/Dx11/Hlsl/HlslEmitter.h"
#include "Render/Dx11/Hlsl/HlslShader.h"

namespace traktor
{
	namespace render
	{

class HlslVariable;
class InputPin;
class OutputPin;
class ShaderGraph;

/*!
 * \ingroup DX11
 */
class HlslContext
{
public:
	HlslContext(const ShaderGraph* shaderGraph);

	Node* getInputNode(const InputPin* inputPin);

	Node* getInputNode(Node* node, const std::wstring& inputPinName);

	bool emit(Node* node);

	HlslVariable* emitInput(const InputPin* inputPin);

	HlslVariable* emitInput(Node* node, const std::wstring& inputPinName);

	HlslVariable* emitOutput(Node* node, const std::wstring& outputPinName, HlslType type);

	void emitOutput(Node* node, const std::wstring& outputPinName, HlslVariable* variable);

	void findNonDependentOutputs(Node* node, const std::wstring& inputPinName, const std::vector< const OutputPin* >& dependentOutputPins, std::vector< const OutputPin* >& outOutputPins) const;

	void findCommonOutputs(Node* node, const std::wstring& inputPin1, const std::wstring& inputPin2, std::vector< const OutputPin* >& outOutputPins) const;

	void enterVertex();

	void enterPixel();

	bool inVertex() const;

	bool inPixel() const;

	bool allocateInterpolator(int32_t width, int32_t& outId, int32_t& outOffset);

	HlslShader& getVertexShader();

	HlslShader& getPixelShader();

	HlslShader& getShader();

	HlslEmitter& getEmitter();

	D3D11_RASTERIZER_DESC& getD3DRasterizerDesc();

	D3D11_DEPTH_STENCIL_DESC& getD3DDepthStencilDesc();

	D3D11_BLEND_DESC& getD3DBlendDesc();

	void setStencilReference(uint32_t stencilReference);

	uint32_t getStencilReference() const;

	const std::wstring& getError() const { return m_error; }

private:
	struct Scope
	{
		const InputPin* inputPin;
		const OutputPin* outputPin;

		Scope()
		:	inputPin(0)
		,	outputPin(0)
		{
		}

		Scope(const InputPin* inputPin_, const OutputPin* outputPin_)
		:	inputPin(inputPin_)
		,	outputPin(outputPin_)
		{
		}
	};

	Ref< const ShaderGraph > m_shaderGraph;
	HlslShader m_vertexShader;
	HlslShader m_pixelShader;
	HlslShader* m_currentShader;
	HlslEmitter m_emitter;
	std::vector< uint8_t > m_interpolatorMap;
	D3D11_RASTERIZER_DESC m_d3dRasterizerDesc;
	D3D11_DEPTH_STENCIL_DESC m_d3dDepthStencilDesc;
	D3D11_BLEND_DESC m_d3dBlendDesc;
	uint32_t m_stencilReference;
	std::list< Scope > m_emitScope;
	std::wstring m_error;
};

	}
}

#endif	// traktor_render_HlslContext_H
