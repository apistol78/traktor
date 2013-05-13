#ifndef traktor_render_HlslContext_H
#define traktor_render_HlslContext_H

#include <map>
#include "Render/Dx11/HlslEmitter.h"
#include "Render/Dx11/HlslShader.h"

namespace traktor
{
	namespace render
	{

class HlslVariable;
class InputPin;
class IProgramHints;
class OutputPin;
class ShaderGraph;

/*!
 * \ingroup DX11
 */
class HlslContext
{
public:
	HlslContext(const ShaderGraph* shaderGraph, IProgramHints* programHints);

	Node* getInputNode(const InputPin* inputPin);

	Node* getInputNode(Node* node, const std::wstring& inputPinName);

	HlslVariable* emitInput(const InputPin* inputPin);

	HlslVariable* emitInput(Node* node, const std::wstring& inputPinName);

	HlslVariable* emitOutput(Node* node, const std::wstring& outputPinName, HlslType type);

	void emitOutput(Node* node, const std::wstring& outputPinName, HlslVariable* variable);

	bool isPinsConnected(const OutputPin* outputPin, const InputPin* inputPin) const;

	void findExternalInputs(Node* node, const std::wstring& inputPinName, const std::wstring& dependentOutputPinName, std::vector< const InputPin* >& outInputPins) const;

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

private:
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
};

	}
}

#endif	// traktor_render_HlslContext_H
