/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_PsslShader_H
#define traktor_render_PsslShader_H

#include <list>
#include <map>
#include <set>
#include <string>
#include "Core/RefArray.h"
#include "Core/Io/StringOutputStream.h"
#include "Render/Types.h"
#include "Render/Ps4/PsslType.h"
#include "Render/Ps4/PsslVariable.h"

namespace traktor
{
	namespace render
	{

class OutputPin;

/*!
 * \ingroup GNM
 */
class PsslShader
{
public:
	enum ShaderType
	{
		StVertex,
		StPixel
	};

	enum BlockType
	{
		BtCBufferOnce,
		BtCBufferFrame,
		BtCBufferDraw,
		BtTextures,
		BtSamplers,
		BtInput,
		BtOutput,
		BtScript,
		BtBody,
		BtLast
	};

	PsslShader(ShaderType shaderType);

	virtual ~PsslShader();

	bool haveInput(const std::wstring& inputName) const;

	void addInput(const std::wstring& inputName);

	PsslVariable* createTemporaryVariable(const OutputPin* outputPin, PsslType type);

	PsslVariable* createVariable(const OutputPin* outputPin, const std::wstring& variableName, PsslType type);

	PsslVariable* createOuterVariable(const OutputPin* outputPin, const std::wstring& variableName, PsslType type);

	void associateVariable(const OutputPin* outputPin, PsslVariable* variable);

	PsslVariable* getVariable(const OutputPin* outputPin) const;

	void pushScope();

	void popScope();

	int32_t allocateInterpolator();

	int32_t allocateBooleanRegister();

	void allocateVPos();

	void allocateVFace();

	void allocateInstanceID();

	void allocateTargetSize();

	bool defineScript(const std::wstring& signature);

	//void addSampler(const std::wstring& sampler, const D3D11_SAMPLER_DESC& dsd);

	//const std::map< std::wstring, D3D11_SAMPLER_DESC >& getSamplers() const;

	void addUniform(const std::wstring& uniform);

	const std::set< std::wstring >& getUniforms() const;

	void pushOutputStream(BlockType blockType, StringOutputStream* outputStream);

	void popOutputStream(BlockType blockType);

	StringOutputStream& getOutputStream(BlockType blockType);

	std::wstring getGeneratedShader();

private:
	typedef std::map< const OutputPin*, Ref< PsslVariable > > scope_t;

	ShaderType m_shaderType;
	std::set< std::wstring > m_inputs;
	std::list< scope_t > m_variables;
	int32_t m_interpolatorCount;
	int32_t m_booleanRegisterCount;
	std::set< std::wstring > m_scripts;
	//std::map< std::wstring, D3D11_SAMPLER_DESC > m_samplers;
	std::set< std::wstring > m_uniforms;
	int32_t m_nextTemporaryVariable;
	RefArray< StringOutputStream > m_outputStreams[BtLast];
	bool m_needVPos;
	bool m_needVFace;
	bool m_needTargetSize;
	bool m_needInstanceID;
};

	}
}

#endif	// traktor_render_PsslShader_H
