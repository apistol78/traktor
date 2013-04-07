#ifndef traktor_SplineControl_H
#define traktor_SplineControl_H

namespace traktor
{

/*! \brief Clamp time.
 * \ingroup Core
 */
struct ClampTime
{
	static inline float t(float Tat, float Tfirst, float Tlast, float Tend)
	{
		float T = Tat;
		if (T < Tfirst)
			return Tfirst;
		else if (T > Tlast)
			return Tlast;
		else
			return T;
	}

	static inline int index(int i, int last)
	{
		if (i < 0)
			return 0;
		else if (i > last)
			return last;
		else
			return i;
	}
};

/*! \brief Wrap time.
 * \ingroup Core
 */
struct WrapTime
{
	static inline float t(float Tat, float Tfirst, float Tlast, float Tend)
	{
		float T = Tat;
		float range = Tend - Tfirst;
		while (T < Tfirst)
			T += range;
		while (T > Tend + Tfirst)
			T -= range;
		return T;
	}

	static inline int index(int i, int last)
	{
		while (i < 0)
			i += last + 1;
		while (i > last)
			i -= last + 1;
		return i;
	}
};

}

#endif	// traktor_SplineControl_H
