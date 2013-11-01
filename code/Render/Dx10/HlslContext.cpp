#include <cassert>
#include <sstream>
#include "Render/Dx10/Platform.h"
#include "Render/Dx10/HlslContext.h"
#include "Render/Dx10/HlslShader.h"
#include "Render/Shader/ShaderGraph.h"
#include "Render/Shader/Node.h"
#include "Render/Shader/InputPin.h"
#include "Render/Shader/OutputPin.h"

// \fixme
#include "Render/Editor/Shader/ShaderGraphUtilities.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

bool doesInputPropagateToNode(const ShaderGraph* shaderGraph, const InputPin* inputPin, Node* targetNode)
{
	struct FindInputPin
	{
		const InputPin* inputPin;
		Node* targetNode;
		bool found;

		bool operator () (Node* node)
		{
			found |= bool(inputPin->getNode() == targetNode);
			return !found;
		}

		bool operator () (Edge* edge)
		{
			return true;
		}
	};

	FindInputPin visitor;
	visitor.inputPin = inputPin;
	visitor.targetNode = targetNode;
	visitor.found = false;
	ShaderGraphTraverse(shaderGraph, targetNode).preorder(visitor);

	return visitor.found;
}

		}

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

HlslVariable* HlslContext::emitInput(const InputPin* inputPin)
{
	const OutputPin* sourcePin = m_shaderGraph->findSourcePin(inputPin);
	if (!sourcePin)
		return 0;

	HlslVariable* variable = m_currentShader->getVariable(sourcePin);
	if (!variable)
	{
		if (m_emitter.emit(*this, sourcePin->getNode()))
		{
			variable = m_currentShader->getVariable(sourcePin);
			T_ASSERT (variable);
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

bool HlslContext::isPinsConnected(const OutputPin* outputPin, const InputPin* inputPin) const
{
	const OutputPin* sourceOutputPin = m_shaderGraph->findSourcePin(inputPin);
	if (!sourceOutputPin)
		return false;

	std::set< const OutputPin* > visitedOutputPins;
	visitedOutputPins.insert(sourceOutputPin);

	std::vector< const OutputPin* > outputPins;
	outputPins.push_back(sourceOutputPin);

	while (!outputPins.empty())
	{
		const OutputPin* sourceOutputPin = outputPins.back(); outputPins.pop_back();
		T_ASSERT (sourceOutputPin);

		if (sourceOutputPin == outputPin)
			return true;

		const Node* node = sourceOutputPin->getNode();
		for (int32_t i = 0; i < node->getInputPinCount(); ++i)
		{
			const InputPin* nodeInputPin = node->getInputPin(i);
			T_ASSERT (nodeInputPin);

			const OutputPin* nodeSourceOutputPin = m_shaderGraph->findSourcePin(nodeInputPin);
			if (
				nodeSourceOutputPin &&
				visitedOutputPins.find(nodeSourceOutputPin) == visitedOutputPins.end()
				)
			{
				outputPins.push_back(nodeSourceOutputPin);
				visitedOutputPins.insert(nodeSourceOutputPin);
			}
		}
	}

	return false;
}

void HlslContext::findExternalInputs(Node* node, const std::wstring& inputPinName, const std::wstring& dependentOutputPinName, std::vector< const InputPin* >& outInputPins) const
{
	const OutputPin* dependentOutputPin = node->findOutputPin(dependentOutputPinName);
	T_ASSERT (dependentOutputPin);

	std::set< const OutputPin* > visitedOutputPins;
	visitedOutputPins.insert(dependentOutputPin);

	std::vector< const InputPin* > inputPins;
	inputPins.push_back(node->findInputPin(inputPinName));

	while (!inputPins.empty())
	{
		const InputPin* inputPin = inputPins.back(); inputPins.pop_back();
		T_ASSERT (inputPin);

		if (!isPinsConnected(dependentOutputPin, inputPin))
			outInputPins.push_back(inputPin);
		else
		{
			const OutputPin* outputPin = m_shaderGraph->findSourcePin(inputPin);
			if (
				!outputPin ||
				visitedOutputPins.find(outputPin) != visitedOutputPins.end()
			)
				continue;

			visitedOutputPins.insert(outputPin);

			const Node* node = outputPin->getNode();
			for (int32_t i = 0; i < node->getInputPinCount(); ++i)
				inputPins.push_back(node->getInputPin(i));
		}
	}
}

void HlslContext::findCommonInputs(Node* node, const std::wstring& inputPin1, const std::wstring& inputPin2, std::vector< const InputPin* >& outInputPins) const
{
	struct Collect1
	{
		std::set< const OutputPin* > outputs;

		bool operator () (Node* node)
		{
			return true;
		}

		bool operator () (Edge* edge)
		{
			outputs.insert(edge->getSource());
			return true;
		}
	};

	struct Collect2
	{
		std::set< const OutputPin* >* candidates; 
		std::set< const OutputPin* > common;

		bool operator () (Node* node)
		{
			return true;
		}

		bool operator () (Edge* edge)
		{
			const OutputPin* outputPin = edge->getSource();
			if (candidates->find(outputPin) != candidates->end())
			{
				common.insert(outputPin);
				return false;
			}
			else
				return true;
		}
	};

	const OutputPin* outputPin1 = m_shaderGraph->findSourcePin(node->findInputPin(inputPin1));
	const OutputPin* outputPin2 = m_shaderGraph->findSourcePin(node->findInputPin(inputPin2));

	if (outputPin1 != outputPin2)
	{
		Collect1 visitor1;
		ShaderGraphTraverse(m_shaderGraph, outputPin1->getNode()).preorder(visitor1);

		Collect2 visitor2;
		visitor2.candidates = &visitor1.outputs;
		ShaderGraphTraverse(m_shaderGraph, outputPin2->getNode()).preorder(visitor2);

		for (std::set< const OutputPin* >::const_iterator i = visitor2.common.begin(); i != visitor2.common.end(); ++i)
		{
			std::vector< const InputPin* > inputPins;
			m_shaderGraph->findDestinationPins(*i, inputPins);

			for (std::vector< const InputPin* >::const_iterator j = inputPins.begin(); j != inputPins.end(); ++j)
			{
				if (doesInputPropagateToNode(m_shaderGraph, *j, node))
					outInputPins.push_back(*j);
			}
		}
	}
	else
	{
		// Apparently both inputs are connected to same output; thus
		// no need to traverse in order to find the intersection.
		m_shaderGraph->findDestinationPins(outputPin1, outInputPins);
	}
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
