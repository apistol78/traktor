#include "Render/OpenGL/ES2/BlitHelper.h"
#include "Render/OpenGL/ES2/ContextOpenGLES2.h"
#include "Render/OpenGL/ES2/StateCache.h"

#if !defined(T_OFFLINE_ONLY)

namespace traktor
{
	namespace render
	{
		namespace
		{
		
struct Vertex
{
	float pos[2];
	float uv[2];
};

const Vertex c_vertices[] =
{
	{ { -1.0f,  1.0f }, { 0.0f, 0.0f } },
	{ {  1.0f,  1.0f }, { 1.0f, 0.0f } },
	{ {  1.0f, -1.0f }, { 1.0f, 1.0f } },
	{ { -1.0f,  1.0f }, { 0.0f, 0.0f } },
	{ {  1.0f, -1.0f }, { 1.0f, 1.0f } },
	{ { -1.0f, -1.0f }, { 0.0f, 1.0f } }
};
		
const char c_vertexShader[] =
{
	"precision lowp float;\n"

	"attribute vec2 in_Position;\n"
	"attribute vec2 in_TexCoord;\n"
	
	"varying vec2 v_TexCoord;\n"
	
	"void main()\n"
	"{\n"
	"	gl_Position = vec4(in_Position.x, in_Position.y, -1.0, 1.0);\n"
	"	v_TexCoord = vec2(1.0 - in_TexCoord.y, in_TexCoord.x);\n"
	"}\n"
};

const char c_fragmentShader[] =
{
	"precision lowp float;\n"

	"uniform lowp sampler2D sampler0;\n"
	
	"varying vec2 v_TexCoord;\n"

	"void main()\n"
	"{\n"
	"	vec4 color = texture2D(sampler0, v_TexCoord.st);\n"
	"	gl_FragColor = color;\n"
	"}\n"
};
		
		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.BlitHelper", BlitHelper, Object)

bool BlitHelper::create(ContextOpenGLES2* resourceContext)
{
	GLuint vertexShaderObject = resourceContext->createShaderObject(c_vertexShader, GL_VERTEX_SHADER);
	GLuint fragmentShaderObject = resourceContext->createShaderObject(c_fragmentShader, GL_FRAGMENT_SHADER);

	m_programObject = glCreateProgram();
	T_ASSERT (m_programObject != 0);

	T_OGL_SAFE(glAttachShader(m_programObject, vertexShaderObject));
	T_OGL_SAFE(glAttachShader(m_programObject, fragmentShaderObject));
	T_OGL_SAFE(glLinkProgram(m_programObject));
	
	m_attribPosition = glGetAttribLocation(m_programObject, "in_Position");
	m_attribTexCoord = glGetAttribLocation(m_programObject, "in_TexCoord");
	m_samplerIndex = glGetUniformLocation(m_programObject, "sampler0");

	T_OGL_SAFE(glGenBuffers(1, &m_vertexBuffer));
	T_OGL_SAFE(glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer));
	T_OGL_SAFE(glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(Vertex), c_vertices, GL_STATIC_DRAW));
	T_OGL_SAFE(glFlush());

	m_renderState.cullFaceEnable = GL_FALSE;
	m_renderState.blendEnable = GL_FALSE;
	m_renderState.depthTestEnable = GL_FALSE;
	m_renderState.colorMask = RenderStateOpenGL::CmAll;
	m_renderState.depthMask = GL_TRUE;
	m_renderState.alphaTestEnable = GL_FALSE;
	m_renderState.stencilTestEnable = GL_FALSE;

	return true;
}

void BlitHelper::destroy()
{
	T_OGL_SAFE(glDeleteProgram(m_programObject));
}

void BlitHelper::blit(StateCache* stateCache, GLint sourceTextureHandle)
{
	stateCache->setProgram(m_programObject);
	stateCache->setRenderState(m_renderState, false);
	stateCache->setVertexArrayObject(0);
	stateCache->setArrayBuffer(m_vertexBuffer);

	T_OGL_SAFE(glEnableVertexAttribArray(m_attribPosition));
	T_OGL_SAFE(glEnableVertexAttribArray(m_attribTexCoord));

	T_OGL_SAFE(glVertexAttribPointer(
		m_attribPosition,
		2,
		GL_FLOAT,
		GL_FALSE,
		sizeof(Vertex),
		(GLvoid*)offsetof(Vertex, pos)
	));

	T_OGL_SAFE(glVertexAttribPointer(
		m_attribTexCoord,
		2,
		GL_FLOAT,
		GL_FALSE,
		sizeof(Vertex),
		(GLvoid*)offsetof(Vertex, uv)
	));
	
	T_OGL_SAFE(glActiveTexture(GL_TEXTURE0));
	T_OGL_SAFE(glBindTexture(GL_TEXTURE_2D, sourceTextureHandle));
	T_OGL_SAFE(glUniform1i(m_samplerIndex, 0));

	T_OGL_SAFE(glDrawArrays(
		GL_TRIANGLES,
		0,
		6
	));
}
	
	}
}

#endif
