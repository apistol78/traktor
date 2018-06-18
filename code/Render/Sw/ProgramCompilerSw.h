/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_ProgramCompilerSw_H
#define traktor_render_ProgramCompilerSw_H

#include "Core/Thread/Semaphore.h"
#include "Render/IProgramCompiler.h"

namespace traktor
{

class IStream;

	namespace render
	{

/*! \brief Software program compiler.
 * \ingroup Render
 */
class ProgramCompilerSw : public IProgramCompiler
{
	T_RTTI_CLASS;

public:
	virtual const wchar_t* getPlatformSignature() const T_OVERRIDE T_FINAL;

	virtual Ref< ProgramResource > compile(
		const ShaderGraph* shaderGraph,
		const PropertyGroup* settings,
		int32_t optimize,
		bool validate,
		Stats* outStats
	) const T_OVERRIDE T_FINAL;

	virtual bool generate(
		const ShaderGraph* shaderGraph,
		const PropertyGroup* settings,
		int32_t optimize,
		std::wstring& outVertexShader,
		std::wstring& outPixelShader,
		std::wstring& outComputeShader
	) const T_OVERRIDE T_FINAL;

private:
	mutable Semaphore m_lock;
	mutable Ref< IStream > m_dump;
};

	}
}

#endif	// traktor_render_ProgramCompilerSw_H
