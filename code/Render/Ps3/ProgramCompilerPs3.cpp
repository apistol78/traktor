#include "Core/Log/Log.h"
#include "Core/Misc/TString.h"
#include "Core/Thread/Acquire.h"
#include "Core/Thread/Semaphore.h"
#include "Render/Ps3/PlatformPs3.h"
#include "Render/Ps3/Cg.h"
#include "Render/Ps3/CgProgram.h"
#include "Render/Ps3/ProgramResourcePs3.h"
#include "Render/Ps3/ProgramCompilerPs3.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ProgramCompilerPs3", 0, ProgramCompilerPs3, IProgramCompiler)

Ref< ProgramResource > ProgramCompilerPs3::compile(const ShaderGraph* shaderGraph, int32_t optimize, bool validate) const
{
	CgProgram cgProgram;
	if (Cg().generate(shaderGraph, cgProgram))
		return compile(cgProgram);
	else
		return 0;
}

Ref< ProgramResource > ProgramCompilerPs3::compile(const CgProgram& cgProgram) const
{
	static Semaphore s_globalLock;
	static Semaphore s_errorLock;

	const char* argv[] = { 0 };
	CGCstatus status;

	T_ANONYMOUS_VAR(Acquire< Semaphore >)(s_globalLock);
	
	const std::wstring& vertexShader = cgProgram.getVertexShader();
	const std::wstring& pixelShader = cgProgram.getPixelShader();

	CGCcontext* cgc = sceCgcNewContext();
	if (!cgc)
		return 0;

	CGCbin* msg = sceCgcNewBin();
	if (!msg)
		return 0;

	CGCbin* vertexShaderBin = sceCgcNewBin();
	if (!vertexShaderBin)
		return 0;

	status = sceCgcCompileString(
		cgc,
		wstombs(vertexShader).c_str(),
		"sce_vp_rsx",
		"main",
		argv,
		vertexShaderBin,
		msg
	);
	if (status != SCECGC_OK)
	{
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(s_errorLock);
		log::error << L"Compile CG vertex shader failed" << Endl;
		if (sceCgcGetBinSize(msg) > 1)
			log::error << mbstows((char*)sceCgcGetBinData(msg)) << Endl;
		FormatMultipleLines(log::error, vertexShader);
		return 0;
	}

	CGCbin* pixelShaderBin = sceCgcNewBin();
	if (!pixelShaderBin)
		return 0;

	status = sceCgcCompileString(
		cgc,
		wstombs(pixelShader).c_str(),
		"sce_fp_rsx",
		"main",
		argv,
		pixelShaderBin,
		msg
	);
	if (status != SCECGC_OK)
	{
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(s_errorLock);
		log::error << L"Compile CG fragment shader failed" << Endl;
		if (sceCgcGetBinSize(msg) > 1)
			log::error << mbstows((char*)sceCgcGetBinData(msg)) << Endl;
		FormatMultipleLines(log::error, pixelShader);
		return 0;
	}

	sceCgcDeleteContext(cgc);
	sceCgcDeleteBin(msg);

	return new ProgramResourcePs3(vertexShaderBin, pixelShaderBin);
}

	}
}
