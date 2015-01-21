#ifndef traktor_render_EmitterContext_H
#define traktor_render_EmitterContext_H

#include <bitset>
#include <map>
#include <stack>
#include "Core/Math/Vector4.h"
#include "Render/Sw/Emitter/Emitter.h"
#include "Render/Sw/Emitter/EmitterVariable.h"
#include "Render/Sw/Core/IntrProgram.h"
#include "Render/Sw/RenderStateDesc.h"
#include "Render/Types.h"

namespace traktor
{
	namespace render
	{

class ShaderGraph;
class Node;
class InputPin;
class OutputPin;

/*!
 * \ingroup SW
 */
class EmitterContext
{
public:
	struct Parameters
	{
		std::map< std::wstring, EmitterVariable* > uniforms;
		std::map< std::wstring, int > samplers;
		uint32_t nextUniformIndex;

		Parameters()
		:	nextUniformIndex(0)
		{
		}
	};

	EmitterContext(const ShaderGraph* shaderGraph, Parameters& parameters);

	virtual ~EmitterContext();

	/*! \name Node emitter */
	//@{

	void emit(Node* node);

	EmitterVariable* emitInput(const InputPin* inputPin);

	EmitterVariable* emitInput(Node* node, const std::wstring& inputPinName);

	EmitterVariable* emitOutput(Node* node, const std::wstring& outputPinName, EmitterVariableType type, bool force = false);

	//@}

	/*! \name Constant branch evaluation */
	//@{

	bool evaluateConstant(Node* node, const std::wstring& inputPinName, float& outValue);

	//@}

	/*! \name Instruction emitter */
	//@{

	uint32_t getCurrentAddress() const;

	uint32_t emitInstruction(const Instruction& inst);

	void emitInstruction(uint32_t offset, const Instruction& inst);

	uint32_t emitInstruction(
		unsigned char opcode,
		const EmitterVariable* dest = 0,
		const EmitterVariable* src1 = 0,
		const EmitterVariable* src2 = 0,
		const EmitterVariable* src3 = 0,
		const EmitterVariable* src4 = 0
	);

	//@}

	EmitterVariable* emitConstant(float scalar);

	EmitterVariable* emitConstant(const Vector4& vector);

	EmitterVariable* emitUniform(const std::wstring& parameterName, EmitterVariableType variableType, int length = 1);

	EmitterVariable* emitVarying(int index);

	uint32_t allocInterpolator();

	uint32_t allocSampler(const std::wstring& parameterName);

	EmitterVariable* allocTemporary(EmitterVariableType variableType);

	void freeTemporary(EmitterVariable*& var);

	void setRenderState(const RenderStateDesc& renderState);

	void enterVertex();

	void enterPixel();

	bool inVertex() const;

	bool inPixel() const;

	void findCommonInputs(Node* node, const std::wstring& inputPin1, const std::wstring& inputPin2, std::vector< const InputPin* >& outInputPins) const;

	Emitter& getEmitter();

	IntrProgram& getVertexProgram();

	IntrProgram& getPixelProgram();

	IntrProgram& getProgram();

	const RenderStateDesc& getRenderState() const;

	uint32_t getInterpolatorCount() const;

private:
	struct Scope
	{
		Node* node;
		std::vector< const OutputPin* > usedRefs;
	};

	struct TransientInput
	{
		EmitterVariable* var;
		int32_t count;
		bool forced;
	};

	struct State
	{
		IntrProgram program;
		std::bitset< 256 > free;
		std::set< EmitterVariable* > vars;
		std::map< const OutputPin*, TransientInput > inputs;
	};

	Emitter m_emitter;
	Ref< const ShaderGraph > m_shaderGraph;
	Parameters& m_parameters;
	State m_states[2];
	State* m_currentState;
	std::vector< Scope > m_scope;
	uint32_t m_interpolatorCount;
	uint32_t m_samplerCount;
	RenderStateDesc m_renderState;

	void collectInputs(std::map< const OutputPin*, TransientInput >& inputs);
};

	}
}

#endif	// traktor_render_EmitterContext_H
