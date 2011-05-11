#include "Core/Log/Log.h"
#include "Core/Misc/TString.h"
#include "Render/OpenGL/Std/Extensions.h"

namespace traktor
{
	namespace render
	{

#if !defined(__APPLE__)

// GL_ARB_shader_objects
// GL_ARB_shading_language_100
// GL_ARB_vertex_shader
// GL_ARB_fragment_shader
PFNGLCREATESHADEROBJECTARBPROC glCreateShaderObjectARB = 0;
PFNGLDELETEOBJECTARBPROC glDeleteObjectARB = 0;
PFNGLSHADERSOURCEARBPROC glShaderSourceARB = 0;
PFNGLCOMPILESHADERARBPROC glCompileShaderARB = 0;
PFNGLCREATEPROGRAMOBJECTARBPROC glCreateProgramObjectARB = 0;
PFNGLATTACHOBJECTARBPROC glAttachObjectARB = 0;
PFNGLLINKPROGRAMARBPROC glLinkProgramARB = 0;
PFNGLVALIDATEPROGRAMARBPROC glValidateProgramARB = 0;
PFNGLUSEPROGRAMOBJECTARBPROC glUseProgramObjectARB = 0;
PFNGLGETINFOLOGARBPROC glGetInfoLogARB = 0;
PFNGLGETOBJECTPARAMETERIVARBPROC glGetObjectParameterivARB = 0;
PFNGLGETACTIVEUNIFORMARBPROC glGetActiveUniformARB = 0;
PFNGLGETUNIFORMLOCATIONARBPROC glGetUniformLocationARB = 0;
PFNGLUNIFORM1IARBPROC glUniform1iARB = 0;
PFNGLUNIFORM1FARBPROC glUniform1fARB = 0;
PFNGLUNIFORM4FARBPROC glUniform4fARB = 0;
PFNGLUNIFORM1FVARBPROC glUniform1fvARB = 0;
PFNGLUNIFORM2FVARBPROC glUniform2fvARB = 0;
PFNGLUNIFORM4FVARBPROC glUniform4fvARB = 0;
PFNGLUNIFORMMATRIX4FVARBPROC glUniformMatrix4fvARB = 0;
PFNGLGETACTIVEATTRIBARBPROC glGetActiveAttribARB = 0;
PFNGLGETATTRIBLOCATIONARBPROC glGetAttribLocationARB = 0;
PFNGLBINDATTRIBLOCATIONARBPROC glBindAttribLocationARB = 0;
PFNGLENABLEVERTEXATTRIBARRAYARBPROC glEnableVertexAttribArrayARB = 0;
PFNGLVERTEXATTRIBPOINTERARBPROC glVertexAttribPointerARB = 0;

// ARB_vertex_buffer_object
PFNGLBINDBUFFERARBPROC glBindBufferARB = 0;
PFNGLGENBUFFERSARBPROC glGenBuffersARB = 0;
PFNGLBUFFERDATAARBPROC glBufferDataARB = 0;
PFNGLDELETEBUFFERSARBPROC glDeleteBuffersARB = 0;
PFNGLMAPBUFFERARBPROC glMapBufferARB = 0;
PFNGLUNMAPBUFFERARBPROC glUnmapBufferARB = 0;

// GL_EXT_???
PFNGLGENFRAMEBUFFERSEXTPROC glGenFramebuffersEXT = 0;
PFNGLDELETEFRAMEBUFFERSEXTPROC glDeleteFramebuffersEXT = 0;
PFNGLBINDFRAMEBUFFEREXTPROC glBindFramebufferEXT = 0;
PFNGLGENRENDERBUFFERSEXTPROC glGenRenderbuffersEXT = 0;
PFNGLBINDRENDERBUFFEREXTPROC glBindRenderbufferEXT = 0;
PFNGLRENDERBUFFERSTORAGEEXTPROC glRenderbufferStorageEXT = 0;
PFNGLRENDERBUFFERSTORAGEMULTISAMPLEEXTPROC glRenderbufferStorageMultisampleEXT = 0;
PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC glFramebufferRenderbufferEXT = 0;
PFNGLFRAMEBUFFERTEXTURE2DEXTPROC glFramebufferTexture2DEXT = 0;
PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC glCheckFramebufferStatusEXT = 0;
PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVEXTPROC glGetFramebufferAttachmentParameterivEXT = 0;
PFNGLBLITFRAMEBUFFEREXTPROC glBlitFramebufferEXT = 0;

// GL_???
PFNGLACTIVETEXTUREPROC glActiveTexture = 0;

// GL_EXT_???
PFNGLBLENDEQUATIONEXTPROC glBlendEquationEXT = 0;

// GL_ARB_texture_compression
PFNGLCOMPRESSEDTEXIMAGE2DPROC glCompressedTexImage2D = 0;

// ???
PFNGLDRAWRANGEELEMENTSPROC glDrawRangeElements = 0;

#endif

static struct Extension
{
	const char* name;
	bool have;
	bool use;
}
s_extensions[] =
{
	{ "GL_ARB_vertex_buffer_object", false, true },
	{ "GL_ARB_texture_non_power_of_two", false, true },
	{ "GL_ARB_texture_float", false, true },
	{ "GL_NV_float_buffer", false, true },
	{ "GL_ATI_texture_float", false, true },
	{ "GL_EXT_framebuffer_blit", false, true },
	{ "GL_EXT_framebuffer_object", false, true },
	{ "GL_EXT_framebuffer_multisample", false, true },
	{ "GL_ARB_half_float_vertex", false, true }
};

bool opengl_initialize_extensions()
{
	const char* renderer = (const char*)glGetString(GL_RENDERER);
	if (renderer)
		log::info << L"OpenGL renderer: " << mbstows(renderer) << Endl;

	for (int32_t i = 0; i < sizeof_array(s_extensions); ++i)
	{
		int32_t extensionLength = strlen(s_extensions[i].name);

		const char* supported = (const char*)glGetString(GL_EXTENSIONS);
		while (supported && *supported)
		{
			const char* end = supported;
			while (*end && *end != ' ')
			{
				// Fail safe; if extension seems to be more than 200 characters
				// long then we assume something has gone wrong and we abort.
				if ((++end - supported) >= 200)
					break;
			}

			int32_t length = end - supported;
			if (
				length == extensionLength &&
				strnicmp(supported, s_extensions[i].name, length) == 0
			)
				s_extensions[i].have = true;

			supported = end;
			while (*supported == ' ')
			{
				// Fail safe; if more than 10 white space characters then
				// we assume somethings wrong and abort.
				if ((++supported - end) >= 10)
					break;
			}
		}
	}

	for (int32_t i = 0; i < sizeof_array(s_extensions); ++i)
		log::info << L"\"" << mbstows(s_extensions[i].name) << L"\", have: " << (s_extensions[i].have ? L"yes" : L"no") << L", use: " << ((s_extensions[i].have && s_extensions[i].use) ? L"yes" : L"no") << Endl;
	
#if !defined(__APPLE__)

#	define T_WIDEN_X(x) L ## x
#	define T_WIDEN(x) T_WIDEN_X(x)

#	if defined(_WIN32)
#		define RESOLVE(fp) \
		if (!(*(PROC*)&fp = wglGetProcAddress(#fp))) { \
			log::error << L"Unable to load OpenGL extensions, \"" << T_WIDEN(#fp) << L"\" failed" << Endl; \
			return false; \
		}
#	else	// LINUX
#		define RESOLVE(fp) \
		if (!(*(size_t*)&fp = (size_t)glXGetProcAddressARB((const GLubyte*)#fp))) { \
			log::error << L"Unable to load OpenGL extensions, \"" << T_WIDEN(#fp) << L"\" failed" << Endl; \
			return false; \
		}
#	endif

	// GL_ARB_shader_objects
	// GL_ARB_shading_language_100
	// GL_ARB_vertex_shader
	// GL_ARB_fragment_shader
	RESOLVE(glCreateShaderObjectARB);
	RESOLVE(glDeleteObjectARB);
	RESOLVE(glShaderSourceARB);
	RESOLVE(glCompileShaderARB);
	RESOLVE(glCreateProgramObjectARB);
	RESOLVE(glAttachObjectARB);
	RESOLVE(glLinkProgramARB);
	RESOLVE(glValidateProgramARB);
	RESOLVE(glUseProgramObjectARB);
	RESOLVE(glGetInfoLogARB);
	RESOLVE(glGetObjectParameterivARB);
	RESOLVE(glGetActiveUniformARB);
	RESOLVE(glGetUniformLocationARB);
	RESOLVE(glUniform1iARB);
	RESOLVE(glUniform1fARB);
	RESOLVE(glUniform4fARB);
	RESOLVE(glUniform1fvARB);
	RESOLVE(glUniform2fvARB);
	RESOLVE(glUniform4fvARB);
	RESOLVE(glUniformMatrix4fvARB);
	RESOLVE(glGetActiveAttribARB);
	RESOLVE(glGetAttribLocationARB);
	RESOLVE(glBindAttribLocationARB);
	RESOLVE(glEnableVertexAttribArrayARB);
	RESOLVE(glVertexAttribPointerARB);

	// GL_ARB_vertex_buffer_object
	RESOLVE(glBindBufferARB);
	RESOLVE(glGenBuffersARB);
	RESOLVE(glBufferDataARB);
	RESOLVE(glDeleteBuffersARB);
	RESOLVE(glMapBufferARB);
	RESOLVE(glUnmapBufferARB);

	// GL_EXT_???
	RESOLVE(glGenFramebuffersEXT);
	RESOLVE(glDeleteFramebuffersEXT);
	RESOLVE(glBindFramebufferEXT);
	RESOLVE(glGenRenderbuffersEXT);
	RESOLVE(glBindRenderbufferEXT);
	RESOLVE(glRenderbufferStorageEXT);
	RESOLVE(glRenderbufferStorageMultisampleEXT);
	RESOLVE(glFramebufferRenderbufferEXT);
	RESOLVE(glFramebufferTexture2DEXT);
	RESOLVE(glCheckFramebufferStatusEXT);
	RESOLVE(glGetFramebufferAttachmentParameterivEXT);
	RESOLVE(glBlitFramebufferEXT);

	// GL_???
	RESOLVE(glActiveTexture);

	// GL_EXT_???
	RESOLVE(glBlendEquationEXT);

	// GL_ARB_texture_compression
	RESOLVE(glCompressedTexImage2D);

	// ???
	RESOLVE(glDrawRangeElements);

#endif

	return true;
}

bool opengl_have_extension(Extentions extension)
{
	T_ASSERT (extension);
	return s_extensions[int32_t(extension)].have && s_extensions[int32_t(extension)].use;
}

	}
}
