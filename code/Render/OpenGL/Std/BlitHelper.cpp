#include "Render/OpenGL/Std/BlitHelper.h"
#include "Render/OpenGL/Std/Extensions.h"

namespace traktor
{
	namespace render
	{
		namespace
		{
		
const GLcharARB c_vertexShader[] =
{
	"void main()\n"
	"{\n"
	"	gl_Position = gl_Vertex;\n"
	"	gl_TexCoord[0] = gl_MultiTexCoord0;\n"
	"}\n"
};

const GLcharARB c_fragmentShader[] =
{
	"uniform sampler2D sampler0;\n"
	
	"void main()\n"
	"{\n"
	"	vec4 color = texture2D(sampler0, gl_TexCoord[0].st);\n"
	"	gl_FragColor = color;\n"
	"}\n"
};
		
		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.BlitHelper", BlitHelper, Object)

bool BlitHelper::create()
{
	const GLcharARB* vertexShaders[] = { c_vertexShader };
	const GLcharARB* fragmentShaders[] = { c_fragmentShader };

	GLhandleARB vertexShaderObject = glCreateShaderObjectARB(GL_VERTEX_SHADER_ARB);
	T_OGL_SAFE(glShaderSourceARB(vertexShaderObject, 1, vertexShaders, NULL));
	T_OGL_SAFE(glCompileShaderARB(vertexShaderObject));

	GLhandleARB fragmentShaderObject = glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);
	T_OGL_SAFE(glShaderSourceARB(fragmentShaderObject, 1, fragmentShaders, NULL));
	T_OGL_SAFE(glCompileShaderARB(fragmentShaderObject));
	
	m_programObject = glCreateProgramObjectARB();
	T_ASSERT (m_programObject != 0);

	T_OGL_SAFE(glAttachObjectARB(m_programObject, vertexShaderObject));
	T_OGL_SAFE(glAttachObjectARB(m_programObject, fragmentShaderObject));
	T_OGL_SAFE(glLinkProgramARB(m_programObject));

	return true;
}

void BlitHelper::destroy()
{
	T_OGL_SAFE(glDeleteObjectARB(m_programObject));
}

void BlitHelper::blit(GLint sourceTextureHandle)
{
	T_OGL_SAFE(glBindFramebufferEXT(GL_DRAW_FRAMEBUFFER_EXT, 0));

	T_OGL_SAFE(glUseProgramObjectARB(m_programObject));
	
	T_OGL_SAFE(glActiveTexture(GL_TEXTURE0));
	T_OGL_SAFE(glBindTexture(GL_TEXTURE_2D, sourceTextureHandle));
	T_OGL_SAFE(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
	T_OGL_SAFE(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));

	T_OGL_SAFE(glPushAttrib(GL_ENABLE_BIT | GL_DEPTH_BUFFER_BIT));
	T_OGL_SAFE(glDisable(GL_CULL_FACE));
	T_OGL_SAFE(glDisable(GL_BLEND));
	T_OGL_SAFE(glDisable(GL_DEPTH_TEST));
	T_OGL_SAFE(glDisable(GL_ALPHA_TEST));
	T_OGL_SAFE(glDisable(GL_STENCIL_TEST));
	T_OGL_SAFE(glDepthMask(GL_FALSE));

	(glBegin(GL_QUADS));
	(glTexCoord2f(0.0f, 0.0f));
	(glVertex3f(-1.0f, -1.0f, -1.0f));
	(glTexCoord2f(1.0f, 0.0f));
	(glVertex3f( 1.0f, -1.0f, -1.0f));
	(glTexCoord2f(1.0f, 1.0f));
	(glVertex3f( 1.0f,  1.0f, -1.0f));
	(glTexCoord2f(0.0f, 1.0f));
	(glVertex3f(-1.0f,  1.0f, -1.0f));
	(glEnd());
	
	T_OGL_SAFE(glPopAttrib());
}
	
	}
}
