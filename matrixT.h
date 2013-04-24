/*
Orion's Standard Library
Orion Sky Lawlor, 9/23/1999
NAME:		osl/matrix.h

DESCRIPTION:	Templatized C++ Matrix library

This file provides routines for creating, reading in,
adding, multiplying, and inverting NxM matrices.

They're not super-duper C++-ish (overloaded operators,
etc.) but do have the required operations and can be
allocated completely on the stack.  See also osl::Matrix.
*/
#ifndef __OSL_MATRIXT_H
#define __OSL_MATRIXT_H
#include "math_header.h"

namespace osl {

template <class real,int rows,int cols> class MatrixT {
public:
	/// Provide outside access to our template parameters.
	typedef real real_t; enum {n_rows=rows}; enum {n_cols=cols};
	real data[rows][cols];//The data contained in this Matrix.  

/// Access matrix entries using m(r,c) syntax.
        real &operator() (int r,int c) {return data[r][c];}
        const real &operator() (int r,int c) const {return data[r][c];}
/// Access entries using m[r][c] syntax.
        real *operator[] (int r) {return data[r];}
        const real *operator[] (int r) const {return data[r];}
	
/// Creation routines:
	MatrixT() {} //Uninitialized
	MatrixT(real x) {identity(x);}//Create scaling Matrix by x
	
	//Default copy constructor, assignment operator
	void identity(real x=1.0);
	
	bool operator==(const MatrixT<real,rows,cols> &to) const;
	bool operator!=(const MatrixT<real,rows,cols> &to) const {return !(*this==to);}

	MatrixT<real,rows,cols> &operator*=(real r);
	
//Row operations:
	real *getRow(int row) {return &data[row][0];}
	void setRow(int row,const real *toWhat);//Set the values in this row to these values
	void swapRow(int row1,int row2);//Swap these two rows
	void scaleRow(int row,const real scaleBy);//data[row][*]*=scale
	void scaleAddRow(int rowSrc,const real scaleBy,int rowSum);//data[rowSum][*]+=data[rowSrc][*]*scale

//Column operations:
	void setColumn(int col,const real *toWhat);

//Matrix operations:
//Add m to this Matrix.
	void add(const MatrixT<real,rows,cols> &m);
	
//Solve this Matrix using (non-naive) gaussian elimination.  
//  Returns true if the solution was sucessful (non-singular), false otherwise.
// This Matrix must have more columns than rows.
	bool solve(void);
	
//Invert this Matrix using non-naive gaussian elimination.
// Inverse is allowed to be this Matrix.
//  This Matrix must be square.
//  Returns true if the solution was sucessful.
	bool invert(MatrixT<real,rows,cols> &inverse) const;
	MatrixT<real,rows,cols> inverse(void) const {
		MatrixT<real,rows,cols> ret;
		invert(ret);
		return ret;
	}
	
//Transpose this matrix to this destination
	void transpose(MatrixT<real,cols,rows /* note: backwards! */> &dest) const {
		for (int r=0;r<rows;r++) for (int c=0;c<cols;c++)
			dest.data[c][r]=data[r][c];
	}
	
//Set dest=this * by
/*	template<int byRows,int byCols>
	void product(const MatrixT<real,byRows,byCols> &by,
		MatrixT<real,rows,byCols> &dest) const;*/
	void product(const MatrixT<real,cols,rows> &by,
		MatrixT<real,rows,rows> &dest) const;
//Apply this Matrix to the given values
	void apply(const real *in,real *out) const;
};



//The template definitions are right here-- because they have to be, to compile.
#define TI(ret) template<class real,int rows,int cols> inline ret MatrixT<real,rows,cols>
#define TIX template<class real,int rows,int cols> inline MatrixT<real,rows,cols>
#define TN(ret) template<class real,int rows,int cols> ret MatrixT<real,rows,cols>

// Matrix operations:
TIX operator+(const MatrixT<real,rows,cols> &a,const MatrixT<real,rows,cols> &b)
{
	MatrixT<real,rows,cols> ret=a; ret.add(b);
	return ret;
}
TIX operator*(const MatrixT<real,rows,cols> &a,const MatrixT<real,rows,cols> &b)
{
	MatrixT<real,rows,cols> ret;
	a.product(b,ret);
	return ret;
}
TIX inverse(const MatrixT<real,rows,cols> &a)
{
	MatrixT<real,rows,cols> ret;
	a.invert(ret);
	return ret;
}


//Row operations:
TI(void)::setRow(int r,const real *toWhat)//Set the values in this row to these values
{
	for (int c=0;c<cols;c++)
		data[r][c]=toWhat[c];
}
TI(void)::swapRow(int r1,int r2)//Swap these two rows
{
	for (int c=0;c<cols;c++)
	{
		real tmp=data[r1][c];
		data[r1][c]=data[r2][c];
		data[r2][c]=tmp;
	}
}
template<class real,int rows,int cols> 
MatrixT<real,rows,cols> &MatrixT<real,rows,cols>::operator*=(real sclBy) {
	for (int r=0;r<rows;r++) for (int c=0;c<cols;c++)
		data[r][c]*=sclBy;
	return *this;
}

TI(void)::scaleRow(int r,const real scaleBy)//data[rowSum][*]*=scale
{
	for (int c=0;c<cols;c++)
		data[r][c]*=scaleBy;
}
TI(void)::scaleAddRow(int rSrc,const real scaleBy,int rDest)//data[rowSum][*]+=data[rowSrc][*]*scale
{
	for (int c=0;c<cols;c++)
		data[rDest][c]+=data[rSrc][c]*scaleBy;
}

TI(void)::setColumn(int c,const real *toWhat)
{
	for (int r=0;r<rows;r++)
		data[r][c]=toWhat[r];
}

//Matrix operations:
//Add m to this Matrix.  m must be the same size as this Matrix
TN(void)::add(const MatrixT<real,rows,cols> &m)
{
	for (int r=0;r<rows;r++) for (int c=0;c<cols;c++)
		data[r][c]+=m(r,c);
}

//Solve this Matrix using (non-naive) gaussian elimination.  
//  Returns true if the solution was sucessful (non-singular), false otherwise.
// This Matrix must have more columns than rows.
TN(bool)::solve(void)
{
	int pivotCol,r,c;
	for (pivotCol=0;pivotCol<rows;pivotCol++)
	{
		//Find a suitable row to pivot on
		int pivotRow=-1;//Row to pivot on
		double pivotVal=0.0; //Magnitude of pivot value
		//Pivot on largest value
		for (r=pivotCol;r<rows;r++) {
			double val=(double)fabs(data[r][pivotCol]);
			if (pivotVal<val) {pivotVal=val;pivotRow=r;}
		}
		if (pivotRow==-1) return false;//We only found zeros in a pivot column-- singular Matrix!
		//Swap that row into the pivot position
		if (pivotRow!=pivotCol)
			swapRow(pivotRow,pivotCol);
		pivotRow=pivotCol;
		scaleRow(pivotRow,((real)1.0)/data[pivotRow][pivotCol]);
		
		//Scale away the rest of the Matrix
		for (r=0;r<rows;r++)
			if (r!=pivotRow)
			{
				real scale=data[r][pivotCol];
				data[r][pivotCol]=0.0;
				for (c=pivotCol+1;c<cols;c++)
					data[r][c]-=data[pivotRow][c]*scale;
			}
	}
	return true;//It worked!
}

//Invert this Matrix using non-naive gaussian elimination.
//  This Matrix must be square.
//  Returns true if the solution was sucessful.
TN(bool)::invert(MatrixT<real,rows,cols> &inv) const
{
	MatrixT<real,rows,cols*2> aug;
	int r,c;
	for (r=0;r<rows;r++)
	{
		for (c=0;c<cols;c++)
			aug(r,c)=data[r][c];//Us on the left;
		for (c=0;c<cols;c++)
			aug(r,cols+c)=(c==r)?(real)1:(real)0;//Identity on the right
	}
	//Row-reduce the augmented Matrix
	if (!aug.solve()) return false;//Row-reduction failed!  We're singular.
	for (r=0;r<rows;r++) for (c=0;c<cols;c++)
		inv(r,c)=aug(r,cols+c);
	return true;//It worked!
}

//Set dest=this * by
//  Dest will be re-allocated to the appropriate size (this->rows x by->cols).
//  The number of rows of "by" must equal the number of columns of this Matrix.

/*
template<class real,int rows,int cols>
 template<int byRows,int byCols> 
*/
TN(void)::product(const MatrixT<real,cols,rows> &by,MatrixT<real,rows,rows> &dest)  const
{
	int r,c,inner;
	for (r=0;r<rows;r++)
		for (c=0;c<rows;c++)
		{
			real sum=0.0;
			for (inner=0;inner<cols;inner++)
				sum+=data[r][inner]*by(inner,c);
			dest(r,c)=sum;
		}
}

//Apply this Matrix to this data (in[cols],out[rows])
TI(void)::apply(const real *in,real *out) const
{
	for (int r=0;r<rows;r++) {
		real sum=0;
		for (int c=0;c<cols;c++) sum+=data[r][c]*in[c];
		out[r]=sum;
	}
}

TN(void)::identity(real x)
{
	for (int r=0;r<rows;r++) for (int c=0;c<cols;c++)
		data[r][c]=(r==c)?x:0;
}
TN(bool)::operator==(const MatrixT<real,rows,cols> &to) const
{
	for (int r=0;r<rows;r++) for (int c=0;c<cols;c++)
		if (fabs(data[r][c]-to(r,c))>0.00001)
			return false;
	return true;
}

/**
 * Copy the portion of src that overlaps with dest.
 * This can be used to, for example, convert a double matrix
 * to a float matrix; or extract the topleft 2x2 portion 
 * of a 3x3 matrix.
 */
template <class sreal,int srows,int scols,
          class dreal,int drows,int dcols>
inline void copy(const MatrixT<sreal,srows,scols> &s,
                       MatrixT<dreal,drows,dcols> &d) {
	int rows=srows; if (rows>drows) rows=drows;
	int cols=scols; if (cols>dcols) cols=dcols;
	for (int r=0;r<rows;r++) for (int c=0;c<cols;c++)
		d(r,c)=dreal(s(r,c));
}

/**
 * Copy the portion of src that overlaps with dest, applying the 
 * given conversion function.
 * This can be used to, for example, convert a complex matrix
 * to a float matrix.
 */
template <class fn_t,
          class sreal,int srows,int scols,
          class dreal,int drows,int dcols>
inline void copy(fn_t fn,
                 const MatrixT<sreal,srows,scols> &s,
                       MatrixT<dreal,drows,dcols> &d) {
	int rows=srows; if (rows>drows) rows=drows;
	int cols=scols; if (cols>dcols) cols=dcols;
	for (int r=0;r<rows;r++) for (int c=0;c<cols;c++)
		d(r,c)=fn(s(r,c));
}

/**
 * Return the largest value in this matrix.
 */
template <class real,int rows,int cols>
inline real matrixMax(const MatrixT<real,rows,cols> &m) {
	real ret=0;
	for (int r=0;r<rows;r++) for (int c=0;c<cols;c++)
		if (ret<m(r,c)) ret=m(r,c);
	return ret;
}



#undef TN
#undef TI
}; //end namespace osl
#endif

