#ifndef traktor_render_HlslShader_H
#define traktor_render_HlslShader_H

#include <list>
#include <map>
#include <set>
#include <string>
#include "Core/RefArray.h"
#include "Core/Io/StringOutputStream.h"
#include "Render/Types.h"
#include "Render/Dx9/HlslType.h"
#include "Render/Dx9/HlslVariable.h"

namespace traktor
{
	namespace render
	{

class OutputPin;

/*!
 * \ingroup DX9 Xbox360
 */
class HlslShader
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

	HlslShader(ShaderType shaderType);

	virtual ~HlslShader();

	bool haveInput(const std::wstring& inputName) const;

	void addInput(const std::wstring& inputName);

	HlslVariable* createTemporaryVariable(const OutputPin* outputPin, HlslType type);

	HlslVariable* createVariable(const OutputPin* outputPin, const std::wstring& variableName, HlslType type);

	HlslVariable* createOuterVariable(const OutputPin* outputPin, const std::wstring& variableName, HlslType type);

	HlslVariable* getVariable(const OutputPin* outputPin) const;

	void pushScope();

	void popScope();

	uint32_t addSamplerTexture(const std::wstring& textureName);

	const std::vector< std::wstring >& getSamplerTextures() const;

	uint32_t addUniform(const std::wstring& uniform, HlslType type, uint32_t count);

	const std::set< std::wstring >& getUniforms() const;

	void pushOutputStream(BlockType blockType, StringOutputStream* outputStream);

	void popOutputStream(BlockType blockType);

	StringOutputStream& getOutputStream(BlockType blockType);

	std::wstring getGeneratedShader(bool needVPos);

private:
	typedef std::map< const OutputPin*, HlslVariable* > scope_t;

	ShaderType m_shaderType;
	std::set< std::wstring > m_inputs;
	std::list< scope_t > m_variables;
	std::vector< std::wstring > m_samplerTextures;
	std::set< std::wstring > m_uniforms;
	std::vector< bool > m_uniformAllocated;
	int32_t m_nextTemporaryVariable;
	RefArray< StringOutputStream > m_outputStreams[BtLast];
};

	}
}

#endif	// traktor_render_HlslShader_H
