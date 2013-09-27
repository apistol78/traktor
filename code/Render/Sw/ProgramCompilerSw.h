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
	virtual const wchar_t* getPlatformSignature() const;

	virtual Ref< ProgramResource > compile(
		const ShaderGraph* shaderGraph,
		int32_t optimize,
		bool validate,
		IProgramHints* hints,
		Stats* outStats
	) const;

	virtual bool generate(
		const ShaderGraph* shaderGraph,
		int32_t optimize,
		std::wstring& outShader
	) const;

private:
	mutable Semaphore m_lock;
	mutable Ref< IStream > m_dump;
};

	}
}

#endif	// traktor_render_ProgramCompilerSw_H
