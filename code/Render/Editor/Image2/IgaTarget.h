#pragma once

#include "Core/Serialization/ISerializable.h"
#include "Render/Frame/RenderGraph.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class T_DLLCLASS IgaTarget : public ISerializable
{
	T_RTTI_CLASS;

public:
	IgaTarget();

	explicit IgaTarget(const std::wstring& name);

	void setName(const std::wstring& name);

	const std::wstring& getName() const;

	void setPosition(int32_t x, int32_t y);

	const int32_t* getPosition() const;

	virtual void serialize(ISerializer& s) override final;

private:
	std::wstring m_name;
	int32_t m_position[2];
	RenderGraphTargetSetDesc m_targetSetDesc;
};

	}   
}