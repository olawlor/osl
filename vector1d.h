/*
Orion's Standard Library
written by 
Orion Sky Lawlor, olawlor@acm.org, 7/18/2001

Rather complete interval and box classes.
*/
#ifndef __OSL_VECTOR1D_H
#define __OSL_VECTOR1D_H

#include <limits>

namespace osl {

/// A closed span of 1d space-- [min,max]
template <class T>
class seg1dT {
	typedef seg1dT<T> seg1d;
	T min,max;
public:
	seg1dT(void) {empty();}
	seg1dT(T val) :min(val),max(val) {}
	seg1dT(T Nmin,T Nmax) :min(Nmin),max(Nmax) {}
	
	inline void init(T a) {min=max=a;}
	inline void init(T a,T b) { set(a,b); }
	inline void init(T a,T b,T c) {init(a,b); add(c);}
	inline T getMin(void) const {return min;}
	inline T getMax(void) const {return max;}
	inline void setMin(T m) {min=m;}
	inline void setMax(T m) {max=m;}
	inline void setMinMax(T lo,T hi) {min=lo; max=hi;}
	inline void shift(T by) {min+=by; max+=by;}
	inline T getLength(void) const {return max-min;}
	
	inline T getHiValue(void) const {return std::numeric_limits<T>::max();}
	inline T getLoValue(void) const {
		/* I just want the smallest representable value, but 
		   the definition for numeric_limits is screwed up: */
		if (std::numeric_limits<T>::is_integer)
			return std::numeric_limits<T>::min();
		else
			return -std::numeric_limits<T>::max();
	}
	
	/// Set this span to contain no points
	inline seg1d &empty(void) {
		min=getHiValue(); max=getLoValue(); return *this;
	}
	/// Set this span to contain all points
	inline seg1d &infinity(void) {
		min=getLoValue(); max=getHiValue(); return *this;
	}
	
	/// Return true if this span is empty.
	inline bool isEmpty(void) const { return max<min;}
	
	/// Set this span to contain only this point
	inline seg1d &set(T b) {min=max=b; return *this;}
	/// Set this span to contain these two points and everything in between
	inline seg1d &set(T a,T b) {
		if (a<b) {min=a;max=b;}
		else     {min=b;max=a;}
		return *this;
	}
	
	inline void expandMin(T b) {if (min>b) min=b;}
	inline void expandMax(T b) {if (max<b) max=b;}
	
	/// Expand this span to contain this point
	inline seg1d &add(T b) {
		expandMin(b);expandMax(b);
		return *this;
	}
	/// Expand this span to contain this span (union)
	inline seg1d &add(const seg1d &b) {
		expandMin(b.min);expandMax(b.max);
		return *this;
	}
	
	/// Return the intersection of this and that seg
	seg1d getIntersection(const seg1d &b) const {
		return seg1d(min>b.min?min:b.min, max<b.max?max:b.max);
	}
	/// Return the union of this and that seg
	seg1d getUnion(const seg1d &b) const {
		return seg1d(min<b.min?min:b.min, max>b.max?max:b.max);
	}
	
	/// Return true if this seg contains this point
	/// in its interior or boundary (closed interval)
	inline bool contains(T b) const {
		return (min<=b)&&(b<=max);
	}
	/// Return true if this seg contains this point 
	/// in its interior (open interval)
	inline bool containsOpen(T b) const {
		return (min<b)&&(b<max);
	}
	/// Return true if this seg contains this point 
	/// in its interior or left endpoint (half-open interval)
	inline bool containsHalf(T b) const {
		return (min<=b)&&(b<max);
	}
	
	/// Return true if this seg contains the segment b--
	///  if the intersection of us and b is equal to b.
	inline bool contains(const seg1d &b) const {
		return (min<=b.min)&&(b.max<=max);
	}
	
	/// Return true if this seg and that share any points
	inline bool intersects(const seg1d &b) const {
		return contains(b.min)||b.contains(min);
	}
	/// Return true if this seg and that share any interior points
	inline bool intersectsOpen(const seg1d &b) const {
		return containsHalf(b.min)||b.containsOpen(min);
	}
	/// Return true if this seg and that share any half-open points
	inline bool intersectsHalf(const seg1d &b) const {
		return containsHalf(b.min)||b.containsHalf(min);
	}
	
#ifdef __CK_PUP_H
	void pup(PUP::er &p) {p|min;p|max;}
#endif
};
typedef seg1dT<double> seg1d;
typedef seg1dT<int> seg1i;

}; /* end namespace osl */

#endif
