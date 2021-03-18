#pragma once

#include <string>
#include "Core/Containers/AlignedVector.h"
#include "Render/Types.h"
#include "Render/Resource/ProgramResource.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_OPENGL_ES_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

/*!
 */
class T_DLLCLASS ProgramResourceOpenGLES : public ProgramResource
{
	T_RTTI_CLASS;

public:
	struct ParameterDesc
	{
		std::wstring name;
		uint32_t buffer = 0;	//!< Index of which uniform buffer (0-2), or texture index.
		uint32_t offset = 0;	//!< Offset in 4-byte floats.
		uint32_t size = 0;		//!< Size in 4-byte floats.

		ParameterDesc() = default;

		explicit ParameterDesc(const std::wstring& name_, uint32_t buffer_, uint32_t offset_, uint32_t size_)
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
		uint32_t unit = 0;
		SamplerState state;
		uint32_t textureIndex = 0;	//!< Index into texture parameter list.

		SamplerDesc() = default;

		explicit SamplerDesc(uint32_t unit_, const SamplerState& state_, uint32_t textureIndex_)
		:	unit(unit_)
		,	state(state_)
		,	textureIndex(textureIndex_)
		{
		}

		void serialize(ISerializer& s);
	};

	virtual void serialize(ISerializer& s) override final;

private:
	friend class ProgramOpenGLES;
	friend class ProgramCompilerOpenGLES;

	RenderState m_renderState;

	std::string m_vertexShader;
	std::string m_fragmentShader;
	std::string m_computeShader;

	AlignedVector< ParameterDesc > m_parameters;
	uint32_t m_uniformBufferSizes[3] = { 0, 0, 0 };	// Once(0), Frame(1) and Draw(2)
	uint32_t m_texturesCount = 0;
	uint32_t m_sbufferCount = 0;

	AlignedVector< SamplerDesc > m_samplers;
	uint32_t m_hash = 0;
};

	}
}
