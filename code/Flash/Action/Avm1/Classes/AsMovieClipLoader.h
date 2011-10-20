#ifndef traktor_flash_AsMovieClipLoader_H
#define traktor_flash_AsMovieClipLoader_H

#include "Flash/Action/Avm1/ActionClass.h"

namespace traktor
{
	namespace flash
	{

struct CallArgs;

/*! \brief MovieClipLoader class.
 * \ingroup Flash
 */
class AsMovieClipLoader : public ActionClass
{
	T_RTTI_CLASS;

public:
	AsMovieClipLoader(ActionContext* context);

	virtual void init(ActionObject* self, const ActionValueArray& args);

	virtual void coerce(ActionObject* self) const;

private:
	void MovieClipLoader_addListener(CallArgs& ca);

	void MovieClipLoader_getProgress(CallArgs& ca);

	void MovieClipLoader_loadClip(CallArgs& ca);

	void MovieClipLoader_removeListener(CallArgs& ca);

	void MovieClipLoader_unloadClip(CallArgs& ca);
};

	}
}

#endif	// traktor_flash_AsMovieClipLoader_H
