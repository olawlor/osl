/**
 * Linear/nonlinear numerical optimization routines.
 *  Orion Sky Lawlor, olawlor@acm.org, 2003/3/29
 */
#ifndef __OSL_OPTIMIZE_H
#define __OSL_OPTIMIZE_H

namespace osl {

/**
 * Abstract representation of an "objective function"--
 * a map from unknown values to some error, to be minimized.
 */
class Objective {
public:
	
	/// Get the number of unknowns:
	virtual int getUnknowns(void) const =0;
	
	/// Return the value of the objective function at
	///  these values of the unknowns.
	virtual double getObjective(const double *unknowns) =0;

	virtual ~Objective() {}
};

/** Find the zero of a 1D function using the secant method */
double secantRoot1D(Objective &obj,double guess1,double guess2,double tol);

/**
 * A trivial, discrete nonlinear optimizer:
 *
 * while still improving:
 *   for each unknown u
 *     take steps of size step[u] until u is optimal
 *
 * This should always find a per-axis local minimum.
 */
void greedyDiscreteMin(Objective &obj,double *unknowns,double *step);

};

#endif
