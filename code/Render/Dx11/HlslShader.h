#ifndef traktor_render_HlslShader_H
#define traktor_render_HlslShader_H

#include <list>
#include <map>
#include <set>
#include <string>
#include "Core/RefArray.h"
#include "Core/Io/StringOutputStream.h"
#include "Render/Types.h"
#include "Render/Dx11/HlslType.h"
#include "Render/Dx11/HlslVariable.h"

namespace traktor
{
	namespace render
	{

class IProgramHints;
class OutputPin;

/*!
 * \ingroup DX11
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

	HlslShader(ShaderType shaderType, IProgramHints* programHints);

	virtual ~HlslShader();

	void addInputVariable(const std::wstring& variableName, HlslVariable* variable);

	HlslVariable* getInputVariable(const std::wstring& variableName);

	HlslVariable* createTemporaryVariable(const OutputPin* outputPin, HlslType type);

	HlslVariable* createVariable(const OutputPin* outputPin, const std::wstring& variableName, HlslType type);

	HlslVariable* createOuterVariable(const OutputPin* outputPin, const std::wstring& variableName, HlslType type);

	void associateVariable(const OutputPin* outputPin, HlslVariable* variable);

	HlslVariable* getVariable(const OutputPin* outputPin);

	void pushScope();

	void popScope();

	int32_t allocateInterpolator();

	int32_t allocateBooleanRegister();

	void allocateVPos();

	void allocateTargetSize();

	void addSampler(const std::wstring& sampler, const D3D11_SAMPLER_DESC& dsd);

	const std::map< std::wstring, D3D11_SAMPLER_DESC >& getSamplers() const;

	void addUniform(const std::wstring& uniform);

	const std::set< std::wstring >& getUniforms() const;

	void pushOutputStream(BlockType blockType, StringOutputStream* outputStream);

	void popOutputStream(BlockType blockType);

	StringOutputStream& getOutputStream(BlockType blockType);

	std::wstring getGeneratedShader();

private:
	typedef std::map< const OutputPin*, Ref< HlslVariable > > scope_t;

	ShaderType m_shaderType;
	IProgramHints* m_programHints;
	std::map< std::wstring, HlslVariable* > m_inputVariables;
	std::list< scope_t > m_variables;
	int32_t m_interpolatorCount;
	int32_t m_booleanRegisterCount;
	std::map< std::wstring, D3D11_SAMPLER_DESC > m_samplers;
	std::set< std::wstring > m_uniforms;
	int32_t m_nextTemporaryVariable;
	RefArray< StringOutputStream > m_outputStreams[BtLast];
	bool m_needVPos;
	bool m_needTargetSize;
};

	}
}

#endif	// traktor_render_HlslShader_H
