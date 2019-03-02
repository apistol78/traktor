#pragma once

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

	virtual void serialize(ISerializer& s) override final;

private:
	RefArray< const EffectLayerData > m_layers;
};

	}
}

