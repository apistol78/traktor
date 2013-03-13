#ifndef traktor_editor_PipelineFactory_H
#define traktor_editor_PipelineFactory_H

#include <vector>
#include "Core/Object.h"
#include "Core/Ref.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class PropertyGroup;

	namespace editor
	{

class IPipeline;

class T_DLLCLASS PipelineFactory : public Object
{
	T_RTTI_CLASS;

public:
	PipelineFactory(const PropertyGroup* settings);

	virtual ~PipelineFactory();

	/*! \brief Find pipeline from source type.
	 *
	 * \param sourceType Type of source asset.
	 * \param outPipeline Pipeline.
	 * \param outPipelineHash Hash of pipeline settings.
	 * \return True if pipeline found.
	 */
	bool findPipeline(const TypeInfo& sourceType, Ref< IPipeline >& outPipeline, uint32_t& outPipelineHash) const;

private:
	std::vector< std::pair< Ref< IPipeline >, uint32_t > > m_pipelines;
};

	}
}

#endif	// traktor_editor_PipelineFactory_H
