#ifndef traktor_spray_ClipboardData_H
#define traktor_spray_ClipboardData_H

#include "Core/RefArray.h"
#include "Core/Serialization/ISerializable.h"

namespace traktor
{
	namespace spray
	{

class EffectLayerData;

class ClipboardData : public ISerializable
{
	T_RTTI_CLASS;

public:
	void addLayer(const EffectLayerData* layer);

	const RefArray< const EffectLayerData >& getLayers() const;

	virtual void serialize(ISerializer& s);

private:
	RefArray< const EffectLayerData > m_layers;
};

	}
}

#endif	// traktor_spray_ClipboardData_H
