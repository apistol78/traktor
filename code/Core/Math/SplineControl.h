#ifndef traktor_SplineControl_H
#define traktor_SplineControl_H

namespace traktor
{

/*! \brief Clamp time.
 * \ingroup Core
 */
template < typename Time >
struct ClampTime
{
	static inline Time t(const Time& Tat, const Time& Tfirst, const Time& Tlast, const Time& Tend)
	{
		Time T = Tat;
		if (T < Tfirst)
			return Tfirst;
		else if (T > Tlast)
			return Tlast;
		else
			return T;
	}

	static inline int index(int i, int nkeys)
	{
		if (i < 0)
			return 0;
		else if (i >= nkeys)
			return nkeys - 1;
		else
			return i;
	}
};

/*! \brief Wrap time.
 * \ingroup Core
 */
template < typename Time >
struct WrapTime
{
	static inline Time t(const Time& Tat, const Time& Tfirst, const Time& Tlast, const Time& Tend)
	{
		Time T = Tat;
		Time range = Tend - Tfirst;
		while (T < Tfirst)
			T += range;
		while (T > Tend + Tfirst)
			T -= range;
		return T;
	}

	static inline int index(int i, int nkeys)
	{
		while (i < 0)
			i += nkeys;
		while (i >= nkeys)
			i -= nkeys;
		return i;
	}
};

}

#endif	// traktor_SplineControl_H
