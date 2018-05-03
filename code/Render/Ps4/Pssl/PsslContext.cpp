/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <cassert>
#include "Core/Log/Log.h"
//#include "Render/Ps4/Platform.h"
#include "Render/Ps4/Pssl/PsslContext.h"
#include "Render/Ps4/Pssl/PsslShader.h"
#include "Render/Shader/InputPin.h"
#include "Render/Shader/Node.h"
#include "Render/Shader/OutputPin.h"
#include "Render/Shader/ShaderGraph.h"
#include "Render/Shader/ShaderGraphTraverse.h"

namespace traktor
{
	namespace render
	{

PsslContext::PsslContext(const ShaderGraph* shaderGraph)
:	m_shaderGraph(shaderGraph)
,	m_vertexShader(PsslShader::StVertex)
,	m_pixelShader(PsslShader::StPixel)
,	m_currentShader(0)
//,	m_stencilReference(0)
{
	//std::memset(&m_d3dRasterizerDesc, 0, sizeof(m_d3dRasterizerDesc));
	//m_d3dRasterizerDesc.FillMode = D3D11_FILL_SOLID;
	//m_d3dRasterizerDesc.CullMode = D3D11_CULL_BACK;
	//m_d3dRasterizerDesc.FrontCounterClockwise = FALSE;
	//m_d3dRasterizerDesc.DepthBias = 0;
	//m_d3dRasterizerDesc.DepthBiasClamp = 0.0f;
	//m_d3dRasterizerDesc.SlopeScaledDepthBias = 0.0f;
	//m_d3dRasterizerDesc.DepthClipEnable = TRUE;
	//m_d3dRasterizerDesc.ScissorEnable = FALSE;
	//m_d3dRasterizerDesc.MultisampleEnable = FALSE;
	//m_d3dRasterizerDesc.AntialiasedLineEnable = TRUE;

	//std::memset(&m_d3dDepthStencilDesc, 0, sizeof(m_d3dDepthStencilDesc));
	//m_d3dDepthStencilDesc.DepthEnable = TRUE;
	//m_d3dDepthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	//m_d3dDepthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	//m_d3dDepthStencilDesc.StencilEnable = FALSE;

	//std::memset(&m_d3dBlendDesc, 0, sizeof(m_d3dBlendDesc));
	//m_d3dBlendDesc.AlphaToCoverageEnable = FALSE;
	//m_d3dBlendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	//m_d3dBlendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
	//m_d3dBlendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ZERO;
	//m_d3dBlendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	//m_d3dBlendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	//m_d3dBlendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
	//m_d3dBlendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	//for (int i = 0; i < sizeof_array(m_d3dBlendDesc.RenderTarget); ++i)
	//	m_d3dBlendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
}

Node* PsslContext::getInputNode(const InputPin* inputPin)
{
	const OutputPin* sourcePin = m_shaderGraph->findSourcePin(inputPin);
	return sourcePin ? sourcePin->getNode() : 0;
}

Node* PsslContext::getInputNode(Node* node, const std::wstring& inputPinName)
{
	const InputPin* inputPin = node->findInputPin(inputPinName);
	T_ASSERT (inputPin);

	return getInputNode(inputPin);
}

void PsslContext::emit(Node* node)
{
	int32_t outputPinCount = node->getOutputPinCount();
	for (int32_t i = 0; i < outputPinCount; ++i)
	{
		PsslVariable* variable = m_currentShader->getVariable(node->getOutputPin(i));
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

PsslVariable* PsslContext::emitInput(const InputPin* inputPin)
{
	const OutputPin* sourcePin = m_shaderGraph->findSourcePin(inputPin);
	if (!sourcePin)
		return 0;

	PsslVariable* variable = m_currentShader->getVariable(sourcePin);
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

PsslVariable* PsslContext::emitInput(Node* node, const std::wstring& inputPinName)
{
	const InputPin* inputPin = node->findInputPin(inputPinName);
	T_ASSERT (inputPin);

	return emitInput(inputPin);
}

PsslVariable* PsslContext::emitOutput(Node* node, const std::wstring& outputPinName, PsslType type)
{
	const OutputPin* outputPin = node->findOutputPin(outputPinName);
	T_ASSERT (outputPin);

	PsslVariable* out = m_currentShader->createTemporaryVariable(outputPin, type);
	T_ASSERT (out);

	return out;
}

void PsslContext::emitOutput(Node* node, const std::wstring& outputPinName, PsslVariable* variable)
{
	const OutputPin* outputPin = node->findOutputPin(outputPinName);
	T_ASSERT (outputPin);

	m_currentShader->associateVariable(outputPin, variable);
}

void PsslContext::findNonDependentOutputs(Node* node, const std::wstring& inputPinName, const std::vector< const OutputPin* >& dependentOutputPins, std::vector< const OutputPin* >& outOutputPins) const
{
	getNonDependentOutputs(m_shaderGraph, node->findInputPin(inputPinName), dependentOutputPins, outOutputPins);
}

void PsslContext::findCommonOutputs(Node* node, const std::wstring& inputPin1, const std::wstring& inputPin2, std::vector< const OutputPin* >& outOutputPins) const
{
	std::vector< const InputPin* > inputPins(2);
	inputPins[0] = node->findInputPin(inputPin1);
	inputPins[1] = node->findInputPin(inputPin2);
	getMergingOutputs(m_shaderGraph, inputPins, outOutputPins); 
}

void PsslContext::enterVertex()
{
	m_currentShader = &m_vertexShader;
}

void PsslContext::enterPixel()
{
	m_currentShader = &m_pixelShader;
}

bool PsslContext::inVertex() const
{
	return bool(m_currentShader == &m_vertexShader);
}

bool PsslContext::inPixel() const
{
	return bool(m_currentShader == &m_pixelShader);
}

bool PsslContext::allocateInterpolator(int32_t width, int32_t& outId, int32_t& outOffset)
{
	int32_t lastId = int32_t(m_interpolatorMap.size());

	for (int32_t i = 0; i < lastId; ++i)
	{
		uint8_t& occupied = m_interpolatorMap[i];
		if (width <= 4 - occupied)
		{
			outId = i;
			outOffset = occupied;

			occupied += width;

			return false;
		}
	}

	outId = lastId;
	outOffset = 0;

	m_interpolatorMap.push_back(width);

	return true;
}

PsslShader& PsslContext::getVertexShader()
{
	return m_vertexShader;
}

PsslShader& PsslContext::getPixelShader()
{
	return m_pixelShader;
}

PsslShader& PsslContext::getShader()
{
	T_ASSERT (m_currentShader);
	return *m_currentShader;
}

PsslEmitter& PsslContext::getEmitter()
{
	return m_emitter;
}

//D3D11_RASTERIZER_DESC& PsslContext::getD3DRasterizerDesc()
//{
//	return m_d3dRasterizerDesc;
//}
//
//D3D11_DEPTH_STENCIL_DESC& PsslContext::getD3DDepthStencilDesc()
//{
//	return m_d3dDepthStencilDesc;
//}
//
//D3D11_BLEND_DESC& PsslContext::getD3DBlendDesc()
//{
//	return m_d3dBlendDesc;
//}
//
//void PsslContext::setStencilReference(uint32_t stencilReference)
//{
//	m_stencilReference = stencilReference;
//}
//
//uint32_t PsslContext::getStencilReference() const
//{
//	return m_stencilReference;
//}

	}
}
