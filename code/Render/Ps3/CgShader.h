#ifndef traktor_render_CgShader_H
#define traktor_render_CgShader_H

#include <list>
#include <map>
#include <set>
#include <string>
#include "Core/RefArray.h"
#include "Core/Io/StringOutputStream.h"
#include "Render/Types.h"
#include "Render/Ps3/CgType.h"
#include "Render/Ps3/CgVariable.h"

namespace traktor
{
	namespace render
	{

class OutputPin;

/*!
 * \ingroup PS3
 */
class CgShader
{
public:
	enum ShaderType
	{
		StVertex,
		StPixel
	};

	enum BlockType
	{
		BtUniform,
		BtInput,
		BtOutput,
		BtBody,
		BtLast
	};

	CgShader(ShaderType shaderType);

	virtual ~CgShader();

	bool haveInput(const std::wstring& inputName) const;

	void addInput(const std::wstring& inputName);

	CgVariable* createTemporaryVariable(const OutputPin* outputPin, CgType type);

	CgVariable* createVariable(const OutputPin* outputPin, const std::wstring& variableName, CgType type);

	CgVariable* createOuterVariable(const OutputPin* outputPin, const std::wstring& variableName, CgType type);

	CgVariable* getVariable(const OutputPin* outputPin) const;

	void pushScope();

	void popScope();

	void addSampler(const std::wstring& sampler, uint32_t stage);

	const std::map< std::wstring, uint32_t >& getSamplers() const;

	uint32_t addUniform(const std::wstring& uniform, CgType type, uint32_t count);

	const std::set< std::wstring >& getUniforms() const;

	void pushOutputStream(BlockType blockType, StringOutputStream* outputStream);

	void popOutputStream(BlockType blockType);

	StringOutputStream& getOutputStream(BlockType blockType);

	std::wstring getGeneratedShader(bool needVPos);

private:
	typedef std::map< const OutputPin*, CgVariable* > scope_t;

	ShaderType m_shaderType;
	std::set< std::wstring > m_inputs;
	std::list< scope_t > m_variables;
	std::map< std::wstring, uint32_t > m_samplers;
	std::set< std::wstring > m_uniforms;
	std::vector< bool > m_uniformAllocated;
	int32_t m_nextTemporaryVariable;
	RefArray< StringOutputStream > m_outputStreams[BtLast];
};

	}
}

#endif	// traktor_render_CgShader_H
