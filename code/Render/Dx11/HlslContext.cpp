#include <cassert>
#include <sstream>
#include "Render/Dx11/Platform.h"
#include "Render/Dx11/HlslContext.h"
#include "Render/Dx11/HlslShader.h"
#include "Render/Shader/ShaderGraph.h"
#include "Render/Shader/Node.h"
#include "Render/Shader/InputPin.h"
#include "Render/Shader/OutputPin.h"

namespace traktor
{
	namespace render
	{

HlslContext::HlslContext(const ShaderGraph* shaderGraph, IProgramHints* programHints)
:	m_shaderGraph(shaderGraph)
,	m_vertexShader(HlslShader::StVertex, programHints)
,	m_pixelShader(HlslShader::StPixel, programHints)
,	m_currentShader(0)
,	m_stencilReference(0)
{
	std::memset(&m_d3dRasterizerDesc, 0, sizeof(m_d3dRasterizerDesc));
	m_d3dRasterizerDesc.FillMode = D3D11_FILL_SOLID;
	m_d3dRasterizerDesc.CullMode = D3D11_CULL_BACK;
	m_d3dRasterizerDesc.FrontCounterClockwise = FALSE;
	m_d3dRasterizerDesc.DepthBias = 0;
	m_d3dRasterizerDesc.DepthBiasClamp = 0.0f;
	m_d3dRasterizerDesc.SlopeScaledDepthBias = 0.0f;
	m_d3dRasterizerDesc.DepthClipEnable = TRUE;
	m_d3dRasterizerDesc.ScissorEnable = FALSE;
	m_d3dRasterizerDesc.MultisampleEnable = TRUE;
	m_d3dRasterizerDesc.AntialiasedLineEnable = TRUE;

	std::memset(&m_d3dDepthStencilDesc, 0, sizeof(m_d3dDepthStencilDesc));
	m_d3dDepthStencilDesc.DepthEnable = TRUE;
	m_d3dDepthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	m_d3dDepthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	m_d3dDepthStencilDesc.StencilEnable = FALSE;

	std::memset(&m_d3dBlendDesc, 0, sizeof(m_d3dBlendDesc));
	m_d3dBlendDesc.AlphaToCoverageEnable = FALSE;
	m_d3dBlendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	m_d3dBlendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
	m_d3dBlendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ZERO;
	m_d3dBlendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	m_d3dBlendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	m_d3dBlendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
	m_d3dBlendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	for (int i = 0; i < sizeof_array(m_d3dBlendDesc.RenderTarget); ++i)
		m_d3dBlendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
}

HlslVariable* HlslContext::emitInput(const InputPin* inputPin)
{
	const OutputPin* sourcePin = m_shaderGraph->findSourcePin(inputPin);
	if (!sourcePin)
		return 0;

	HlslVariable* variable = m_currentShader->getVariable(sourcePin);
	if (!variable)
	{
		m_emitter.emit(*this, sourcePin->getNode());
		variable = m_currentShader->getVariable(sourcePin);
		T_ASSERT (variable);
	}

	return variable;
}

HlslVariable* HlslContext::emitInput(Node* node, const std::wstring& inputPinName)
{
	const InputPin* inputPin = node->findInputPin(inputPinName);
	T_ASSERT (inputPin);

	return emitInput(inputPin);
}

HlslVariable* HlslContext::emitOutput(Node* node, const std::wstring& outputPinName, HlslType type)
{
	const OutputPin* outputPin = node->findOutputPin(outputPinName);
	T_ASSERT (outputPin);

	HlslVariable* out = m_currentShader->createTemporaryVariable(outputPin, type);
	T_ASSERT (out);

	return out;
}

void HlslContext::emitOutput(Node* node, const std::wstring& outputPinName, HlslVariable* variable)
{
	const OutputPin* outputPin = node->findOutputPin(outputPinName);
	T_ASSERT (outputPin);

	m_currentShader->associateVariable(outputPin, variable);
}

void HlslContext::enterVertex()
{
	m_currentShader = &m_vertexShader;
}

void HlslContext::enterPixel()
{
	m_currentShader = &m_pixelShader;
}

bool HlslContext::inVertex() const
{
	return bool(m_currentShader == &m_vertexShader);
}

bool HlslContext::inPixel() const
{
	return bool(m_currentShader == &m_pixelShader);
}

HlslShader& HlslContext::getVertexShader()
{
	return m_vertexShader;
}

HlslShader& HlslContext::getPixelShader()
{
	return m_pixelShader;
}

HlslShader& HlslContext::getShader()
{
	T_ASSERT (m_currentShader);
	return *m_currentShader;
}

HlslEmitter& HlslContext::getEmitter()
{
	return m_emitter;
}

D3D11_RASTERIZER_DESC& HlslContext::getD3DRasterizerDesc()
{
	return m_d3dRasterizerDesc;
}

D3D11_DEPTH_STENCIL_DESC& HlslContext::getD3DDepthStencilDesc()
{
	return m_d3dDepthStencilDesc;
}

D3D11_BLEND_DESC& HlslContext::getD3DBlendDesc()
{
	return m_d3dBlendDesc;
}

void HlslContext::setStencilReference(uint32_t stencilReference)
{
	m_stencilReference = stencilReference;
}

uint32_t HlslContext::getStencilReference() const
{
	return m_stencilReference;
}

	}
}
