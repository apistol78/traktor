#include <limits>
#include <algorithm>
#include "Render/Sw/Emitter/EmitterContext.h"
#include "Render/ShaderGraph.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

int getVariableTypeSize(VariableType variableType)
{
	switch (variableType)
	{
	case VtFloat:
	case VtFloat2:
	case VtFloat3:
	case VtFloat4:
		return 1;
	case VtFloat4x4:
		return 4;
	}
	return 0;
}

		}

EmitterContext::EmitterContext(const ShaderGraph* shaderGraph, Parameters& parameters)
:	m_shaderGraph(shaderGraph)
,	m_parameters(parameters)
,	m_currentState(0)
,	m_interpolatorCount(0)
,	m_samplerCount(0)
{
	for (int i = 0; i < 256; ++i)
	{
		m_states[0].free[i] =
		m_states[1].free[i] = true;
	}
}

void EmitterContext::emit(Node* node)
{
	m_emitter.emit(*this, node);

	// Clean all used inputs.
	for (std::map< const OutputPin*, TransientInput >::iterator i = m_inputs.begin(); i != m_inputs.end(); )
	{
		if (!i->second.count)
		{
			freeTemporary(i->second.var);
			i = m_inputs.erase(i);
		}
		else
			++i;
	}
}

Variable* EmitterContext::emitInput(const InputPin* inputPin)
{
	const OutputPin* sourcePin = m_shaderGraph->findSourcePin(inputPin);
	if (!sourcePin)
		return 0;

	// Emit source pin if not visited already.
	std::map< const OutputPin*, TransientInput >::iterator i = m_inputs.find(sourcePin);
	if (i == m_inputs.end())
	{
		m_emitter.emit(*this, sourcePin->getNode());

		// The source pin should now have been emitted and thus be in the m_temporaries map.
		i = m_inputs.find(sourcePin);
		T_ASSERT (i != m_inputs.end());
	}

	// Consume a reference.
	T_ASSERT (i->second.count > 0);
	i->second.count--;

	return i->second.var;
}

Variable* EmitterContext::emitInput(Node* node, const std::wstring& inputPinName)
{
	const InputPin* inputPin = node->findInputPin(inputPinName);
	T_ASSERT_M (inputPin, L"Unable to find input pin");

	return emitInput(inputPin);
}

Variable* EmitterContext::emitOutput(Node* node, const std::wstring& outputPinName, VariableType type)
{
	const OutputPin* outputPin = node->findOutputPin(outputPinName);
	T_ASSERT_M (outputPin, L"Unable to find output pin");

	std::vector< const InputPin* > destinationPins;
	m_shaderGraph->findDestinationPins(outputPin, destinationPins);
	if (destinationPins.empty())
		return 0;

	Variable* var = allocTemporary(type);
	T_ASSERT (var);

	TransientInput input;
	input.var = var;
	input.count = uint32_t(destinationPins.size());
	m_inputs[outputPin] = input;

	return var;
}

bool EmitterContext::evaluateConstant(Node* node, const std::wstring& inputPinName, float& outValue)
{
	const InputPin* inputPin = node->findInputPin(inputPinName);
	T_ASSERT_M (inputPin, L"Unable to find input pin");

	const OutputPin* sourcePin = m_shaderGraph->findSourcePin(inputPin);
	if (!sourcePin)
		return false;

	Ref< const Scalar > scalarNode = dynamic_type_cast< const Scalar* >(sourcePin->getNode());
	if (!scalarNode)
		return false;

	outValue = scalarNode->get();
	return true;
}

uint32_t EmitterContext::getCurrentAddress() const
{
	return uint32_t(m_currentState->program.getInstructions().size());
}

uint32_t EmitterContext::emitInstruction(const Instruction& inst)
{
	return m_currentState->program.addInstruction(inst);
}

void EmitterContext::emitInstruction(uint32_t offset, const Instruction& inst)
{
	m_currentState->program.setInstruction(offset, inst);
}

uint32_t EmitterContext::emitInstruction(
	unsigned char opcode,
	const Variable* dest,
	const Variable* src1,
	const Variable* src2,
	const Variable* src3,
	const Variable* src4
)
{
	Instruction inst(opcode, 0, 0, 0, 0, 0);

	if (opcode == OpFetchUniform)
	{
		inst.dest = dest->reg;
		inst.src[0] = src1->reg;
		inst.src[1] = dest->size;
	}
	else if (opcode == OpFetchIndexedUniform)
	{
		inst.dest = dest->reg;
		inst.src[0] = src1->reg;
		inst.src[1] = dest->size;
		inst.src[2] = src2->reg;
	}
	else
	{
		if (dest)
			inst.dest = dest->reg;
		if (src1)
			inst.src[0] = src1->reg;
		if (src2)
			inst.src[1] = src2->reg;
		if (src3)
			inst.src[2] = src3->reg;
		if (src4)
			inst.src[3] = src4->reg;
	}

	return emitInstruction(inst);
}

Variable* EmitterContext::emitConstant(float scalar)
{
	Variable* var = new Variable();

	var->type = VtFloat;
	var->reg = m_currentState->program.addConstant(Vector4(scalar, scalar, scalar, scalar));
	var->size = 1;

	return var;
}

Variable* EmitterContext::emitConstant(const Vector4& vector)
{
	Variable* var = new Variable();

	var->type = VtFloat4;
	var->reg = m_currentState->program.addConstant(vector);
	var->size = 1;

	return var;
}

Variable* EmitterContext::emitUniform(const std::wstring& parameterName, VariableType variableType, int length)
{
	std::map< std::wstring, Variable* >::iterator i = m_parameters.uniforms.find(parameterName);
	if (i != m_parameters.uniforms.end())
		return i->second;

	Variable* var = new Variable();

	var->type = variableType;
	var->reg = m_parameters.nextUniformIndex;
	var->size = getVariableTypeSize(variableType) * length;

	m_parameters.uniforms[parameterName] = var;
	m_parameters.nextUniformIndex += var->size;

	return var;
}

Variable* EmitterContext::emitVarying(int index)
{
	Variable* var = new Variable();

	var->type = VtFloat4;
	var->reg = index;
	var->size = 1;

	return var;
}

uint32_t EmitterContext::allocInterpolator()
{
	return m_interpolatorCount++;
}

uint32_t EmitterContext::allocSampler(const std::wstring& parameterName)
{
	uint32_t samplerIndex = m_samplerCount++;
	m_parameters.samplers[parameterName] = samplerIndex;
	return samplerIndex;
}

Variable* EmitterContext::allocTemporary(VariableType variableType)
{
	int size = getVariableTypeSize(variableType);
	int reg = -1;

	for (int i = 0; i < 256 - size; ++i)
	{
		bool free = true;
		for (int j = i; j < i + size; ++j)
			free &= m_currentState->free[j];
		if (free)
		{
			reg = i;
			for (int j = i; j < i + size; ++j)
				m_currentState->free[j] = false;
			break;
		}
	}

	if (reg < 0)
		return 0;

	Variable* var = new Variable();

	var->type = variableType;
	var->reg = reg;
	var->size = size;

	return var;
}

void EmitterContext::freeTemporary(Variable*& var)
{
	if (!var)
		return;

	for (int i = var->reg; i < var->reg + var->size; ++i)
		m_currentState->free[i] = true;

	delete var;
	var = 0;
}

void EmitterContext::setRenderState(const RenderStateDesc& renderState)
{
	m_renderState = renderState;
}

void EmitterContext::enterVertex()
{
	m_currentState = &m_states[0];
}

void EmitterContext::enterPixel()
{
	m_currentState = &m_states[1];
}

bool EmitterContext::inVertex() const
{
	return bool(m_currentState == &m_states[0]);
}

bool EmitterContext::inPixel() const
{
	return bool(m_currentState == &m_states[1]);
}

Emitter& EmitterContext::getEmitter()
{
	return m_emitter;
}

IntrProgram& EmitterContext::getVertexProgram()
{
	return m_states[0].program;
}

IntrProgram& EmitterContext::getPixelProgram()
{
	return m_states[1].program;
}

IntrProgram& EmitterContext::getProgram()
{
	T_ASSERT (m_currentState);
	return m_currentState->program;
}

const RenderStateDesc& EmitterContext::getRenderState() const
{
	return m_renderState;
}

uint32_t EmitterContext::getInterpolatorCount() const
{
	return m_interpolatorCount;
}

	}
}
