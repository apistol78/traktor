#ifndef traktor_render_GlslShader_H
#define traktor_render_GlslShader_H

#include <list>
#include <map>
#include <set>
#include <string>
#include "Core/RefArray.h"
#include "Core/Io/StringOutputStream.h"
#include "Render/Types.h"
#include "Render/OpenGL/GlslType.h"
#include "Render/OpenGL/GlslVariable.h"

namespace traktor
{
	namespace render
	{

class OutputPin;

/*!
 * \ingroup OGL
 */
class GlslShader
{
public:
	enum ShaderType
	{
		StVertex,
		StFragment
	};

	enum BlockType
	{
		BtUniform,
		BtInput,
		BtOutput,
		BtBody,
		BtLast
	};

	GlslShader(ShaderType shaderType);

	virtual ~GlslShader();

	void addInputVariable(const std::wstring& variableName, GlslVariable* variable);

	GlslVariable* getInputVariable(const std::wstring& variableName);

	GlslVariable* createTemporaryVariable(const OutputPin* outputPin, GlslType type);

	GlslVariable* createVariable(const OutputPin* outputPin, const std::wstring& variableName, GlslType type);

	GlslVariable* createOuterVariable(const OutputPin* outputPin, const std::wstring& variableName, GlslType type);

	GlslVariable* getVariable(const OutputPin* outputPin);

	void pushScope();

	void popScope();

	int32_t allocateInterpolator();

	void addUniform(const std::wstring& uniform);

	const std::set< std::wstring >& getUniforms() const;

	void pushOutputStream(BlockType blockType, StringOutputStream* outputStream);

	void popOutputStream(BlockType blockType);

	StringOutputStream& getOutputStream(BlockType blockType);

	std::wstring getGeneratedShader();

private:
	typedef std::map< const OutputPin*, GlslVariable* > scope_t;

	ShaderType m_shaderType;
	std::map< std::wstring, GlslVariable* > m_inputVariables;
	std::list< scope_t > m_variables;
	int32_t m_interpolatorCount;
	std::set< std::wstring > m_uniforms;
	int32_t m_nextTemporaryVariable;
	RefArray< StringOutputStream > m_outputStreams[BtLast];
};

	}
}

#endif	// traktor_render_GlslShader_H
