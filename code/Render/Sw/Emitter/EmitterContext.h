#ifndef traktor_render_EmitterContext_H
#define traktor_render_EmitterContext_H

#include <map>
#include <bitset>
#include "Core/Math/Vector4.h"
#include "Render/Sw/Emitter/Emitter.h"
#include "Render/Sw/Emitter/Variable.h"
#include "Render/Sw/Core/IntrProgram.h"
#include "Render/Sw/RenderStateDesc.h"
#include "Render/Types.h"

namespace traktor
{
	namespace render
	{

class ShaderGraph;
class ShaderGraphAdjacency;
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
		std::map< std::wstring, Variable* > uniforms;
		std::map< std::wstring, int > samplers;
		uint32_t nextUniformIndex;

		Parameters()
		:	nextUniformIndex(0)
		{
		}
	};

	EmitterContext(const ShaderGraph* shaderGraph, Parameters& parameters);

	/*! \name Node emitter */
	//@{

	void emit(Node* node);

	Variable* emitInput(const InputPin* inputPin);

	Variable* emitInput(Node* node, const std::wstring& inputPinName);

	Variable* emitOutput(Node* node, const std::wstring& outputPinName, VariableType type);

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
		const Variable* dest = 0,
		const Variable* src1 = 0,
		const Variable* src2 = 0,
		const Variable* src3 = 0,
		const Variable* src4 = 0
	);

	//@}

	Variable* emitConstant(float scalar);

	Variable* emitConstant(const Vector4& vector);

	Variable* emitUniform(const std::wstring& parameterName, VariableType variableType, int length = 1);

	Variable* emitVarying(int index);

	uint32_t allocInterpolator();

	uint32_t allocSampler(const std::wstring& parameterName);

	Variable* allocTemporary(VariableType variableType);

	void freeTemporary(Variable*& var);

	void setRenderState(const RenderStateDesc& renderState);

	void enterVertex();

	void enterPixel();

	bool inVertex() const;

	bool inPixel() const;

	Emitter& getEmitter();

	IntrProgram& getVertexProgram();

	IntrProgram& getPixelProgram();

	IntrProgram& getProgram();

	const RenderStateDesc& getRenderState() const;

	uint32_t getInterpolatorCount() const;

private:
	struct State
	{
		IntrProgram program;
		std::bitset< 256 > free;
	};

	struct TransientInput
	{
		Variable* var;
		uint32_t count;
	};

	Emitter m_emitter;
	Ref< const ShaderGraph > m_shaderGraph;
	Ref< ShaderGraphAdjacency > m_shaderGraphAdj;
	Parameters& m_parameters;
	State m_states[2];
	State* m_currentState;
	std::map< const OutputPin*, TransientInput > m_inputs;
	uint32_t m_interpolatorCount;
	uint32_t m_samplerCount;
	RenderStateDesc m_renderState;
};

	}
}

#endif	// traktor_render_EmitterContext_H
