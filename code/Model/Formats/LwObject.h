#ifndef traktor_model_LwObject_H
#define traktor_model_LwObject_H

#include "Core/Object.h"
#include "Core/RefArray.h"
#include "Core/Containers/AlignedVector.h"
#include "Core/Math/Vector4.h"

namespace traktor
{

class IStream;

	namespace model
	{

class LwClip;
class LwLayer;
class LwSurface;

class LwObject : public Object
{
	T_RTTI_CLASS;

public:
	bool read(IStream* stream);

	const RefArray< LwLayer >& getLayers() const { return m_layers; }

	const RefArray< LwSurface >& getSurfaces() const { return m_surfaces; }

	const RefArray< LwClip >& getClips() const { return m_clips; }

	const std::vector< std::wstring >& getTags() const { return m_tags; }

private:
	int32_t m_objectType;
	RefArray< LwLayer > m_layers;
	RefArray< LwSurface > m_surfaces;
	RefArray< LwClip > m_clips;
	std::vector< std::wstring > m_tags;
};

	}
}

#endif	// traktor_model_LwObject_H
