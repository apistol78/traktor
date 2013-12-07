#include <cassert>
#include "Core/Log/Log.h"
#include "Render/Dx10/Platform.h"
#include "Render/Dx10/HlslContext.h"
#include "Render/Dx10/HlslShader.h"
#include "Render/Shader/ShaderGraph.h"
#include "Render/Shader/Node.h"
#include "Render/Shader/InputPin.h"
#include "Render/Shader/OutputPin.h"
#include "Render/Shader/ShaderGraphTraverse.h"

namespace traktor
{
	namespace render
	{

HlslContext::HlslContext(const ShaderGraph* shaderGraph)
:	m_shaderGraph(shaderGraph)
,	m_vertexShader(HlslShader::StVertex)
,	m_pixelShader(HlslShader::StPixel)
,	m_currentShader(0)
,	m_stencilReference(0)
{
	std::memset(&m_d3dRasterizerDesc, 0, sizeof(m_d3dRasterizerDesc));
	m_d3dRasterizerDesc.FillMode = D3D10_FILL_SOLID;
	m_d3dRasterizerDesc.CullMode = D3D10_CULL_BACK;
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
	m_d3dDepthStencilDesc.DepthWriteMask = D3D10_DEPTH_WRITE_MASK_ALL;
	m_d3dDepthStencilDesc.DepthFunc = D3D10_COMPARISON_LESS_EQUAL;
	m_d3dDepthStencilDesc.StencilEnable = FALSE;

	std::memset(&m_d3dBlendDesc, 0, sizeof(m_d3dBlendDesc));
	m_d3dBlendDesc.AlphaToCoverageEnable = FALSE;
	m_d3dBlendDesc.RenderTargetWriteMask[0] = D3D10_COLOR_WRITE_ENABLE_ALL;
	m_d3dBlendDesc.SrcBlend = D3D10_BLEND_ONE;
	m_d3dBlendDesc.DestBlend = D3D10_BLEND_ZERO;
	m_d3dBlendDesc.BlendOp = D3D10_BLEND_OP_ADD;
	m_d3dBlendDesc.SrcBlendAlpha = D3D10_BLEND_ONE;
	m_d3dBlendDesc.DestBlendAlpha = D3D10_BLEND_ONE;
	m_d3dBlendDesc.BlendOpAlpha = D3D10_BLEND_OP_ADD;
	for (int i = 0; i < sizeof_array(m_d3dBlendDesc.RenderTargetWriteMask); ++i)
		m_d3dBlendDesc.RenderTargetWriteMask[i] = D3D10_COLOR_WRITE_ENABLE_ALL;
}

Node* HlslContext::getInputNode(const InputPin* inputPin)
{
	const OutputPin* sourcePin = m_shaderGraph->findSourcePin(inputPin);
	return sourcePin ? sourcePin->getNode() : 0;
}

Node* HlslContext::getInputNode(Node* node, const std::wstring& inputPinName)
{
	const InputPin* inputPin = node->findInputPin(inputPinName);
	T_ASSERT (inputPin);

	return getInputNode(inputPin);
}

void HlslContext::emit(Node* node)
{
	int32_t outputPinCount = node->getOutputPinCount();
	for (int32_t i = 0; i < outputPinCount; ++i)
	{
		HlslVariable* variable = m_currentShader->getVariable(node->getOutputPin(i));
		if (!variable)
		{
			if (!m_emitter.emit(*this, node))
			{
				log::error << L"Failed to emit " << type_name(node) << Endl;
				log::error << L"  " << type_name(node) << L"[" << node->getOutputPin(i)->getName() << L"] " << node->getInformation() << Endl;
			}
			break;
		}
	}
}

HlslVariable* HlslContext::emitInput(const InputPin* inputPin)
{
	const OutputPin* sourcePin = m_shaderGraph->findSourcePin(inputPin);
	if (!sourcePin)
		return 0;

	HlslVariable* variable = m_currentShader->getVariable(sourcePin);
	if (!variable)
	{
		Node* node = sourcePin->getNode();
		if (m_emitter.emit(*this, node))
		{
			variable = m_currentShader->getVariable(sourcePin);
			T_ASSERT (variable);
		}
		else
		{
			log::error << L"Failed to emit " << type_name(node) << Endl;
			log::error << L"  " << type_name(node) << L"[" << sourcePin->getName() << L"] " << node->getInformation() << L" -->" << Endl;
			log::error << L"  " << type_name(inputPin->getNode()) << L"[" << inputPin->getName() << L"] " << inputPin->getNode()->getInformation() << Endl;
		}
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

void HlslContext::findNonDependentOutputs(Node* node, const std::wstring& inputPinName, const std::vector< const OutputPin* >& dependentOutputPins, std::vector< const OutputPin* >& outOutputPins) const
{
	getNonDependentOutputs(m_shaderGraph, node->findInputPin(inputPinName), dependentOutputPins, outOutputPins);
}

void HlslContext::findCommonOutputs(Node* node, const std::wstring& inputPin1, const std::wstring& inputPin2, std::vector< const OutputPin* >& outOutputPins) const
{
	std::vector< const InputPin* > inputPins(2);
	inputPins[0] = node->findInputPin(inputPin1);
	inputPins[1] = node->findInputPin(inputPin2);
	getMergingOutputs(m_shaderGraph, inputPins, outOutputPins); 
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

D3D10_RASTERIZER_DESC& HlslContext::getD3DRasterizerDesc()
{
	return m_d3dRasterizerDesc;
}

D3D10_DEPTH_STENCIL_DESC& HlslContext::getD3DDepthStencilDesc()
{
	return m_d3dDepthStencilDesc;
}

D3D10_BLEND_DESC& HlslContext::getD3DBlendDesc()
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
