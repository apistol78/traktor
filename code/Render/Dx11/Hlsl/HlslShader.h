#ifndef traktor_render_HlslShader_H
#define traktor_render_HlslShader_H

#include <list>
#include <map>
#include <set>
#include <string>
#include "Core/RefArray.h"
#include "Core/Io/StringOutputStream.h"
#include "Render/Types.h"
#include "Render/Dx11/Hlsl/HlslType.h"
#include "Render/Dx11/Hlsl/HlslVariable.h"

namespace traktor
{
	namespace render
	{

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

	int32_t allocateInterpolator();

	int32_t allocateBooleanRegister();

	void allocateVPos();

	void allocateVFace();

	void allocateInstanceID();

	void allocateTargetSize();

	bool defineScript(const std::wstring& signature);

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
	std::set< std::wstring > m_inputs;
	std::list< scope_t > m_variables;
	int32_t m_interpolatorCount;
	int32_t m_booleanRegisterCount;
	std::set< std::wstring > m_scripts;
	std::map< std::wstring, D3D11_SAMPLER_DESC > m_samplers;
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

#endif	// traktor_render_HlslShader_H
