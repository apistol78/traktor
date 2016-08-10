#ifndef traktor_render_GlslShader_H
#define traktor_render_GlslShader_H

#include <list>
#include <map>
#include <set>
#include <string>
#include "Core/RefArray.h"
#include "Core/Io/StringOutputStream.h"
#include "Render/Types.h"
#include "Render/Vulkan/Glsl/GlslType.h"
#include "Render/Vulkan/Glsl/GlslVariable.h"

namespace traktor
{
	namespace render
	{

class OutputPin;

/*!
 * \ingroup Vulkan
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
		BtScript,
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

	void associateVariable(const OutputPin* outputPin, GlslVariable* variable);

	GlslVariable* getVariable(const OutputPin* outputPin);

	void pushScope();

	void popScope();

	void addUniform(const std::wstring& uniform);

	const std::set< std::wstring >& getUniforms() const;

	bool defineScript(const std::wstring& signature);
    
	void pushOutputStream(BlockType blockType, StringOutputStream* outputStream);

	void popOutputStream(BlockType blockType);

	StringOutputStream& getOutputStream(BlockType blockType);

	const StringOutputStream& getOutputStream(BlockType blockType) const;

	std::wstring getGeneratedShader() const;

private:
	typedef std::map< const OutputPin*, Ref< GlslVariable > > scope_t;

	ShaderType m_shaderType;
	std::map< std::wstring, GlslVariable* > m_inputVariables;
	std::list< scope_t > m_variables;
	std::set< std::wstring > m_uniforms;
	std::set< std::wstring > m_scriptSignatures;
	int32_t m_nextTemporaryVariable;
	RefArray< StringOutputStream > m_outputStreams[BtLast];
};

	}
}

#endif	// traktor_render_GlslShader_H
