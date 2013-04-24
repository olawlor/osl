/**
 * Linear/nonlinear numerical optimization routines.
 *  Orion Sky Lawlor, olawlor@acm.org, 2003/3/29
 */
#include /*osl/*/ "optimize.h"
#include <math.h>
#include <algorithm>

static void copy(int n,const double *src,double *dest) {
	for (int i=0;i<n;i++) dest[i]=src[i];
}

/**
 * A trivial, discrete nonlinear optimizer:
 *
 * while still improving:
 *   for each unknown u
 *     take steps of size step[u] until u is optimal
 *
 * This should always find a per-axis local minimum.
 */
void osl::greedyDiscreteMin(Objective &obj,double *best,double *step)
{
	int n=obj.getUnknowns();
	double *test=new double[n];
	copy(n,best,test);
	double vBest=obj.getObjective(best);
	double lastBest=vBest+1;
	while (vBest<lastBest) {
		lastBest=vBest;
		for (int u=0;u<n;u++) 
		{ // Optimize unknown u:
			int nBetter=0;
			double dir=1.0;
			/// Increase until u stops improving,
			/// then decrease 
			while (1) {
				test[u]+=dir*step[u];
				double vTest=obj.getObjective(test);
				bool isBetter=(vTest<vBest);
				if (isBetter) { //Improvement--keep going
					nBetter++;
					vBest=vTest;
					best[u]=test[u];
				}
				else { //No improvement:
					test[u]=best[u]; //Go back
					if (dir>0) {
						if (nBetter==0)
							dir=-1; //Reverse direction
						else /* nBetter>0 */
							break; // Already advanced
					} else /* dir<0 */
						break; // Searched both ways
				}
			}
		}
	}
	delete test;
}


/** Optimize a 1D function using the secant method */
double osl::secantRoot1D(Objective &obj,double g1,double g2,double tol)
{
	double v1=obj.getObjective(&g1);
	double v2=obj.getObjective(&g2);
	while (fabs(g1-g2)>tol) {
		/* assume function is linear between g1 and g2:
			v = g * slope + offset
			(v - offset) / slope = g
		*/
		double slope=(g2-g1)/(v2-v1);
		double offset=v2-g2*slope;
		double gN=(0.0-offset)/slope; /* new g */
		double vN=obj.getObjective(&gN);
		if (fabs(v1)>fabs(v2)) { /* swap so g1 has lower error */
			std::swap(g1,g2);
			std::swap(v1,v2);
		}
		if (fabs(vN)>fabs(v1)) {
			/* Help!  guessed wrong!  Go back to bisection */
			g2=0.5*(g1+g2);
		}
		/* now fabs(vN)<fabs(v1)<fabs(v2), so 
		  overwrite v2 with vN */
		g2=gN;
		v2=vN;
	}
	return 0.5*(g1+g2);
}
