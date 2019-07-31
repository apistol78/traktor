#pragma once

#include "Spark/Action/ActionClass.h"

namespace traktor
{
	namespace spark
	{

struct CallArgs;
class SpriteInstance;

/*! \brief MovieClipLoader class.
 * \ingroup Spark
 */
class AsMovieClipLoader : public ActionClass
{
	T_RTTI_CLASS;

public:
	AsMovieClipLoader(ActionContext* context);

	virtual void initialize(ActionObject* self) override final;

	virtual void construct(ActionObject* self, const ActionValueArray& args) override final;

	virtual ActionValue xplicit(const ActionValueArray& args) override final;

private:
	void MovieClipLoader_addListener(CallArgs& ca);

	void MovieClipLoader_getProgress(CallArgs& ca);

	bool MovieClipLoader_loadClip(ActionObject* self, const std::wstring& url, SpriteInstance* target) const;

	void MovieClipLoader_removeListener(CallArgs& ca);

	void MovieClipLoader_unloadClip(CallArgs& ca);
};

	}
}

