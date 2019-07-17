#pragma once

#include <string>
#include "Core/Containers/AlignedVector.h"
#include "Render/Types.h"
#include "Render/Resource/ProgramResource.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_OPENGL_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

/*!
 * \ingroup OGL
 */
class T_DLLCLASS ProgramResourceOpenGL : public ProgramResource
{
	T_RTTI_CLASS;

public:
	struct ParameterDesc
	{
		std::wstring name;
		uint32_t buffer;	//!< Index of which uniform buffer (0-2), or texture index.
		uint32_t offset;	//!< Offset in 4-byte floats.
		uint32_t size;		//!< Size in 4-byte floats.

		ParameterDesc()
		:	buffer(0)
		,	offset(0)
		,	size(0)
		{
		}

		ParameterDesc(const std::wstring& name_, uint32_t buffer_, uint32_t offset_, uint32_t size_)
		:	name(name_)
		,	buffer(buffer_)
		,	offset(offset_)
		,	size(size_)
		{
		}

		void serialize(ISerializer& s);
	};

	struct SamplerDesc
	{
		uint32_t unit;
		SamplerState state;
		uint32_t textureIndex;	//!< Index into texture parameter list.

		SamplerDesc()
		:	unit(0)
		,	textureIndex(0)
		{
		}

		SamplerDesc(uint32_t unit_, const SamplerState& state_, uint32_t textureIndex_)
		:	unit(unit_)
		,	state(state_)
		,	textureIndex(textureIndex_)
		{
		}

		void serialize(ISerializer& s);
	};

	ProgramResourceOpenGL();

	virtual void serialize(ISerializer& s) override final;

private:
	friend class ProgramOpenGL;
	friend class ProgramCompilerOpenGL;

	RenderState m_renderState;

	std::string m_vertexShader;
	std::string m_fragmentShader;
	std::string m_computeShader;

	AlignedVector< ParameterDesc > m_parameters;
	uint32_t m_uniformBufferSizes[3];	// Once(0), Frame(1) and Draw(2)
	uint32_t m_texturesCount;
	uint32_t m_sbufferCount;

	AlignedVector< SamplerDesc > m_samplers;
	uint32_t m_hash;
};

	}
}
