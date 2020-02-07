#pragma once

#include <string>
#include "Core/Serialization/ISerializable.h"
#include "Render/Frame/RenderGraph.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class ImageTargetSet;

/*!
 * \ingroup Render
 */
class T_DLLCLASS ImageTargetSetData : public ISerializable
{
	T_RTTI_CLASS;

public:
	Ref< const ImageTargetSet > createInstance() const;

	virtual void serialize(ISerializer& s) override final;

private:
	friend class ImageGraphPipeline;

	std::wstring m_targetSetId;
	RenderGraphTargetSetDesc m_targetSetDesc;
};

	}
}
