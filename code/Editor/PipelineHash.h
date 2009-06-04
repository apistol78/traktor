#ifndef traktor_editor_PipelineHash_H
#define traktor_editor_PipelineHash_H

#include <map>
#include "Core/Serialization/Serializable.h"
#include "Core/Misc/MD5.h"
#include "Core/Date/DateTime.h"
#include "Core/Guid.h"
#include "Core/Thread/Semaphore.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_EDITOR_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace editor
	{

/*! \brief Pipeline hash.
 * \ingroup Editor
 *
 * The pipeline hash keeps records of builds
 * used by the pipeline to check if an asset
 * needs to be rebuilt.
 */
class T_DLLCLASS PipelineHash : public Serializable
{
	T_RTTI_CLASS(PipelineHash)

public:
	struct Hash
	{
		MD5 checksum;
		DateTime assetTimestamp;
		uint32_t pipelineVersion;

		Hash();

		bool serialize(Serializer& s);
	};

	void set(const Guid& guid, const Hash& hash);

	bool get(const Guid& guid, Hash& outHash) const;

	virtual int getVersion() const;

	virtual bool serialize(Serializer& s);

private:
	std::map< Guid, Hash > m_hash;
	mutable Semaphore m_lock;
};

	}
}

#endif	// traktor_editor_PipelineHash_H
