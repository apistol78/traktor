#pragma optimize( "", off )

#include <algorithm>
#include <limits>
#include "Core/Log/Log.h"
#include "Render/Shader/ShaderGraph.h"
#include "Render/Shader/ShaderGraphTraverse.h"
#include "Render/Sw/Emitter/EmitterContext.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

int getVariableTypeSize(EmitterVariableType variableType)
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
	enterPixel();
}

EmitterContext::~EmitterContext()
{
	for (std::map< const OutputPin*, TransientInput >::iterator i = m_states[0].inputs.begin(); i != m_states[0].inputs.end(); ++i)
		freeTemporary(i->second.var);
	for (std::map< const OutputPin*, TransientInput >::iterator i = m_states[1].inputs.begin(); i != m_states[1].inputs.end(); ++i)
		freeTemporary(i->second.var);
}

void EmitterContext::emit(Node* node)
{
	Scope scope;
	scope.node = node;
	m_scope.push_back(scope);

	m_emitter.emit(*this, node);
	
	const Scope& back = m_scope.back();
	for (std::vector< const OutputPin* >::const_iterator i = back.usedRefs.begin(); i != back.usedRefs.end(); ++i)
	{
		std::map< const OutputPin*, TransientInput >::iterator j = m_currentState->inputs.find(*i);
		T_FATAL_ASSERT(j != m_currentState->inputs.end());
		j->second.count--;
	}
	
	collectInputs(m_currentState->inputs);

	m_scope.pop_back();
}

EmitterVariable* EmitterContext::emitInput(const InputPin* inputPin)
{
	const OutputPin* sourcePin = m_shaderGraph->findSourcePin(inputPin);
	if (!sourcePin)
		return 0;

	// Emit source pin if not visited already.
	std::map< const OutputPin*, TransientInput >::iterator i = m_currentState->inputs.find(sourcePin);
	if (i == m_currentState->inputs.end() || i->second.count == 0)
	{
		emit(sourcePin->getNode());
		i = m_currentState->inputs.find(sourcePin);
		T_FATAL_ASSERT (i != m_currentState->inputs.end());
	}

	m_scope.back().usedRefs.push_back(i->first);
	return i->second.var;
}

EmitterVariable* EmitterContext::emitInput(Node* node, const std::wstring& inputPinName)
{
	const InputPin* inputPin = node->findInputPin(inputPinName);
	T_ASSERT_M (inputPin, L"Unable to find input pin");

	return emitInput(inputPin);
}

EmitterVariable* EmitterContext::emitOutput(Node* node, const std::wstring& outputPinName, EmitterVariableType type, bool force)
{
	const OutputPin* outputPin = node->findOutputPin(outputPinName);
	T_ASSERT_M (outputPin, L"Unable to find output pin");

	uint32_t count = m_shaderGraph->getDestinationCount(outputPin);
	if (count == 0 && !force)
		return 0;

	EmitterVariable* var = allocTemporary(type);
	T_FATAL_ASSERT (var);

	TransientInput& input = m_currentState->inputs[outputPin];
	input.var = var;
	input.count = count;
	input.forced = force;

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
	const EmitterVariable* dest,
	const EmitterVariable* src1,
	const EmitterVariable* src2,
	const EmitterVariable* src3,
	const EmitterVariable* src4
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
		if (opcode == OpMove && dest->reg == src1->reg)
			return getCurrentAddress();

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

EmitterVariable* EmitterContext::emitConstant(float scalar)
{
	EmitterVariable* var = new EmitterVariable();

	var->type = VtFloat;
	var->reg = m_currentState->program.addConstant(Vector4(scalar, scalar, scalar, scalar));
	var->size = 1;

	return var;
}

EmitterVariable* EmitterContext::emitConstant(const Vector4& vector)
{
	EmitterVariable* var = new EmitterVariable();

	var->type = VtFloat4;
	var->reg = m_currentState->program.addConstant(vector);
	var->size = 1;

	return var;
}

EmitterVariable* EmitterContext::emitUniform(const std::wstring& parameterName, EmitterVariableType variableType, int length)
{
	std::map< std::wstring, EmitterVariable* >::iterator i = m_parameters.uniforms.find(parameterName);
	if (i != m_parameters.uniforms.end())
		return i->second;

	EmitterVariable* var = new EmitterVariable();

	var->type = variableType;
	var->reg = m_parameters.nextUniformIndex;
	var->size = getVariableTypeSize(variableType) * length;

	m_parameters.uniforms[parameterName] = var;
	m_parameters.nextUniformIndex += var->size;

	return var;
}

EmitterVariable* EmitterContext::emitVarying(int index)
{
	EmitterVariable* var = new EmitterVariable();

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

EmitterVariable* EmitterContext::allocTemporary(EmitterVariableType variableType)
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
	{
		log::error << L"Out of registers; allocate temporary failed" << Endl;
		return 0;
	}

	EmitterVariable* var = new EmitterVariable();

	var->type = variableType;
	var->reg = reg;
	var->size = size;

	m_currentState->vars.insert(var);

	return var;
}

void EmitterContext::freeTemporary(EmitterVariable*& var)
{
	if (!var)
		return;

	for (int i = 0; i < sizeof_array(m_states); ++i)
	{
		State& state = m_states[i];
		if (state.vars.find(var) != state.vars.end())
		{
			for (int j = var->reg; j < var->reg + var->size; ++j)
			{
				T_ASSERT (!state.free[j]);
				state.free[j] = true;
			}
			state.vars.erase(var);
		}
	}

	delete var; var = 0;
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

void EmitterContext::findCommonInputs(Node* node, const std::wstring& inputPin1, const std::wstring& inputPin2, std::vector< const InputPin* >& outInputPins) const
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

void EmitterContext::collectInputs(std::map< const OutputPin*, TransientInput >& inputs)
{
	for (std::map< const OutputPin*, TransientInput >::iterator i = inputs.begin(); i != inputs.end(); ++i)
	{
		if (i->second.count > 0)
			continue;

		bool inScope = false;
		for (std::vector< Scope >::iterator j = m_scope.begin(); j != m_scope.end(); ++j)
		{
			if (j->node == i->first->getNode())
			{
				inScope = true;
				break;
			}
		}

		if (!inScope)
			freeTemporary(i->second.var);
	}
}

	}
}
