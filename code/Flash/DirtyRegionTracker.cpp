#include "Flash/DirtyRegionTracker.h"
#include "Flash/SpriteInstance.h"
#include "Flash/Action/ActionContext.h"

namespace traktor
{
	namespace flash
	{
		namespace
		{

int32_t s_stateCounter = 0;

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.DirtyRegionTracker", DirtyRegionTracker, Object)

DirtyRegionTracker::DirtyRegionTracker()
:	m_stateCount(0)
{
}

bool DirtyRegionTracker::update(CharacterInstance* movieInstance, Aabb2& outDirtyRegion)
{
	bool forceRedraw = bool(m_stateCount != s_stateCounter);
	if (m_stateCount != s_stateCounter)
	{
		// Stage state has changed; ie instances added and/or removed thus need to redraw entire stage.
		m_stateCount = s_stateCounter;
		return false;
	}

	calculate(movieInstance, Matrix33::identity(), true, outDirtyRegion);
	return true;
}

void DirtyRegionTracker::calculate(CharacterInstance* characterInstance, const Matrix33& transform, bool visible, Aabb2& outDirtyRegion)
{
	ActionContext* context = characterInstance->getContext();
	T_ASSERT (context);

	bool instanceVisible = characterInstance->isVisible() && visible;
	if (&type_of(characterInstance) == &type_of< SpriteInstance >())
	{
		SpriteInstance* spriteInstance = static_cast< SpriteInstance* >(characterInstance);

		const Matrix33 T = transform * spriteInstance->getTransform();
		const DisplayList& displayList = spriteInstance->getDisplayList();
		const DisplayList::layer_map_t& layers = displayList.getLayers();

		for (DisplayList::layer_map_t::const_iterator i = layers.begin(); i != layers.end(); ++i)
		{
			const DisplayList::Layer& layer = i->second;
			if (layer.instance)
				calculate(
					layer.instance,
					T,
					instanceVisible,
					outDirtyRegion
				);
		}
	}
	else if (characterInstance == context->getFocus())
	{
		Aabb2 bounds = transform * characterInstance->getBounds();
		outDirtyRegion.contain(bounds);
	}
	else
	{
		State* s = static_cast< State* >(characterInstance->getCacheObject());
		if (!s)
		{
			s = new State();
			characterInstance->setCacheObject(s);
		}

		// Compare state and add to dirty region if mismatch.
		Aabb2 bounds = transform * characterInstance->getBounds();
		if (s->visible != instanceVisible)
		{
			if (s->visible)
			{
				outDirtyRegion.contain(s->bounds);
				s->visible = false;
			}
			else
			{
				outDirtyRegion.contain(bounds);
				s->visible = true;
			}
		}
		else if (instanceVisible && s->bounds != bounds)
		{
			outDirtyRegion.contain(s->bounds);
			outDirtyRegion.contain(bounds);
			s->bounds = bounds;
		}
	}
}

DirtyRegionTracker::State::State()
:	visible(false)
{
	Atomic::increment(s_stateCounter);
}

DirtyRegionTracker::State::~State()
{
	Atomic::increment(s_stateCounter);
}

	}
}