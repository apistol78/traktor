/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_HlslShader_H
#define traktor_render_HlslShader_H

#include <list>
#include <map>
#include <set>
#include <string>
#include "Core/RefArray.h"
#include "Core/Io/StringOutputStream.h"
#include "Render/Types.h"
#include "Render/Dx9/Hlsl/HlslType.h"
#include "Render/Dx9/Hlsl/HlslVariable.h"

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
		BtScript,
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

	void associateVariable(const OutputPin* outputPin, HlslVariable* variable);

	HlslVariable* getVariable(const OutputPin* outputPin) const;

	void pushScope();

	void popScope();

	bool defineTexture(const std::wstring& textureName);

	bool defineSampler(uint32_t samplerHash, const std::wstring& textureName, int32_t& outStage);

	bool defineScript(const std::wstring& signature);

	const std::set< std::wstring >& getTextures() const;

	const std::map< uint32_t, std::pair< std::wstring, int32_t > >& getSamplers() const;

	uint32_t addUniform(const std::wstring& uniform, HlslType type, uint32_t count);

	const std::set< std::wstring >& getUniforms() const;

	void pushOutputStream(BlockType blockType, StringOutputStream* outputStream);

	void popOutputStream(BlockType blockType);

	StringOutputStream& getOutputStream(BlockType blockType);

	std::wstring getGeneratedShader(bool needVPos);

private:
	typedef std::map< const OutputPin*, Ref< HlslVariable > > scope_t;

	ShaderType m_shaderType;
	std::set< std::wstring > m_inputs;
	std::list< scope_t > m_variables;
	std::set< std::wstring > m_textures;
	std::map< uint32_t, std::pair< std::wstring, int32_t > > m_samplers;
	std::set< std::wstring > m_scripts;
	std::set< std::wstring > m_uniforms;
	std::vector< bool > m_uniformAllocated;
	int32_t m_nextTemporaryVariable;
	int32_t m_nextStage;
	RefArray< StringOutputStream > m_outputStreams[BtLast];
};

	}
}

#endif	// traktor_render_HlslShader_H
