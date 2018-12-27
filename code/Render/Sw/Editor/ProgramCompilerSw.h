/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_ProgramCompilerSw_H
#define traktor_render_ProgramCompilerSw_H

#include "Core/Thread/Semaphore.h"
#include "Render/Editor/IProgramCompiler.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_SW_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class IStream;

	namespace render
	{

/*! \brief Software program compiler.
 * \ingroup Render
 */
class T_DLLCLASS ProgramCompilerSw : public IProgramCompiler
{
	T_RTTI_CLASS;

public:
	virtual const wchar_t* getPlatformSignature() const override final;

	virtual Ref< ProgramResource > compile(
		const ShaderGraph* shaderGraph,
		const PropertyGroup* settings,
		const std::wstring& name,
		int32_t optimize,
		bool validate,
		Stats* outStats
	) const override final;

	virtual bool generate(
		const ShaderGraph* shaderGraph,
		const PropertyGroup* settings,
		const std::wstring& name,
		int32_t optimize,
		std::wstring& outVertexShader,
		std::wstring& outPixelShader,
		std::wstring& outComputeShader
	) const override final;

private:
	mutable Semaphore m_lock;
	mutable Ref< IStream > m_dump;
};

	}
}

#endif	// traktor_render_ProgramCompilerSw_H
