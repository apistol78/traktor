#pragma once

#include <list>
#include <map>
#include <set>
#include <string>
#include "Core/Containers/IdAllocator.h"
#include "Core/Io/StringOutputStream.h"
#include "Render/Types.h"
#include "Render/Editor/Glsl/GlslType.h"
#include "Render/Editor/Glsl/GlslVariable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class PropertyGroup;

	namespace render
	{

class GlslLayout;
class OutputPin;

/*!
 * \ingroup Render
 */
class T_DLLCLASS GlslShader
{
public:
	enum ShaderType
	{
		StVertex,
		StFragment,
		StCompute
	};

	enum BlockType
	{
		BtInput,
		BtOutput,
		BtScript,
		BtBody,
		BtLast
	};

	explicit GlslShader(ShaderType shaderType, GlslDialect dialect);

	virtual ~GlslShader();

	void addInputVariable(const std::wstring& variableName, GlslVariable* variable);

	GlslVariable* getInputVariable(const std::wstring& variableName);

	GlslVariable* createTemporaryVariable(const OutputPin* outputPin, GlslType type);

	GlslVariable* createVariable(const OutputPin* outputPin, const std::wstring& variableName, GlslType type);

	GlslVariable* createOuterVariable(const OutputPin* outputPin, const std::wstring& variableName, GlslType type);

	GlslVariable* getVariable(const OutputPin* outputPin);

	void pushScope();

	void popScope();

	/*! \name Output streams */
	/*! \{ */

	StringOutputStream& pushOutputStream(BlockType blockType, const wchar_t* const tag);

	void popOutputStream(BlockType blockType);

	StringOutputStream& getOutputStream(BlockType blockType);

	const StringOutputStream& getOutputStream(BlockType blockType) const;

	/*! \} */

	std::wstring getGeneratedShader(const PropertyGroup* settings, const GlslLayout& layout, const GlslRequirements& requirements) const;

private:
	struct OutputPinVariable
	{
		const OutputPin* outputPin;
		Ref< GlslVariable > variable;
		int32_t index;
	};

	struct OutputStreamTuple
	{
		Ref< StringOutputStream > outputStream;
		const wchar_t* tag;
	};

	ShaderType m_shaderType;
	GlslDialect m_dialect;
	std::map< std::wstring, Ref< GlslVariable > > m_inputVariables;
	AlignedVector< OutputPinVariable > m_variables;
	AlignedVector< uint32_t > m_variableScopes;
	AlignedVector< OutputPinVariable > m_outerVariables;
	IdAllocator m_temporaryVariableAlloc;
	AlignedVector< OutputStreamTuple > m_outputStreams[BtLast];
};

	}
}

