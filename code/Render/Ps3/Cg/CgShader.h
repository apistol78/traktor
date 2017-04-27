/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_CgShader_H
#define traktor_render_CgShader_H

#include <list>
#include <map>
#include <set>
#include <string>
#include "Core/RefArray.h"
#include "Core/Io/StringOutputStream.h"
#include "Render/Types.h"
#include "Render/Ps3/Cg/CgType.h"
#include "Render/Ps3/Cg/CgVariable.h"

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
		BtScript,
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

	void associateVariable(const OutputPin* outputPin, CgVariable* variable);

	CgVariable* getVariable(const OutputPin* outputPin) const;

	void pushScope();

	void popScope();

	bool defineScript(const std::wstring& signature);

	bool defineSamplerTexture(const std::wstring& textureName, int32_t& outStage);

	const std::map< std::wstring, int32_t >& getSamplerTextures() const;

	uint32_t addUniform(const std::wstring& uniform, CgType type, uint32_t count);

	const std::set< std::wstring >& getUniforms() const;

	void pushOutputStream(BlockType blockType, StringOutputStream* outputStream);

	void popOutputStream(BlockType blockType);

	StringOutputStream& getOutputStream(BlockType blockType);

	std::wstring getGeneratedShader(bool needVPos);

private:
	typedef std::map< const OutputPin*, Ref< CgVariable > > scope_t;

	ShaderType m_shaderType;
	std::set< std::wstring > m_inputs;
	std::list< scope_t > m_variables;
	std::set< std::wstring > m_scriptSignatures;
	std::map< std::wstring, int32_t > m_samplerTextures;
	std::set< std::wstring > m_uniforms;
	std::vector< bool > m_uniformAllocated;
	int32_t m_nextTemporaryVariable;
	int32_t m_nextStage;
	RefArray< StringOutputStream > m_outputStreams[BtLast];
};

	}
}

#endif	// traktor_render_CgShader_H
