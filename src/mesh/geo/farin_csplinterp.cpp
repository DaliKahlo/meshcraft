// c2_bspline.cpp : Defines the entry point for the console application.
//

#include "farin.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>

namespace farin {

//::::::::::::::
//set_up_system.
//::::::::::::::

/*	given the knot sequence, the linear system 
	for clamped end condition B-spline interpolation
	is set up.
input:	knot:	knot sequence
	points:	points to be interpolated
	l:	number of intervals
output:	alpha,beta,gamma: 1-D arrays that constitute
		the elements of the interpolation matrix.

	Note: no data points needed so far! 
*/
void set_up_system(double knot[], int l, double alpha[], double beta[], double gamma[])
{
	int i,l1;
	double delta_im2,delta_im1, delta_i, delta_ip1,sum;
	l1=l-1;

	/* some special cases: */
	if(l==1)
	{alpha[0]=0.0; alpha[1]=0.0; beta[0]=1.0; beta[1]=1.0;
	 gamma[0]=0.0; gamma[1]=0.0; return;}

	if(l==2)
	{
		beta[0]=1.0;
		delta_im1=(knot[1]-knot[0]);
		delta_i  =(knot[2]-knot[1]);
		delta_ip1=(knot[3]-knot[2]);
		sum = delta_im1+delta_i;

		alpha[1]=delta_i*delta_i/sum;
		beta[1] =(delta_i*delta_im1)/sum
                + delta_im1*delta_i/ sum;
		gamma[1]=delta_im1*delta_im1/sum;

		alpha[1]=alpha[1]/sum;
		beta[1] =beta[1]/sum;
		gamma[1]=gamma[1]/sum;

		beta[2]=1.0;
		alpha[2]=0.0;
		gamma[2]=0.0;
		return;
	}

	/* the rest does the cases l>2.  */

	delta_im1=(knot[1]-knot[0]);
	delta_i  =(knot[2]-knot[1]);
	delta_ip1=(knot[3]-knot[2]);
	sum = delta_im1+delta_i;

	beta[0]=1.0; gamma[0]=0.0;

	alpha[1]=delta_i*delta_i/sum;
	beta[1] =(delta_i*delta_im1)/sum
                + delta_im1*(delta_i+delta_ip1)
		/ (sum+delta_ip1);
	gamma[1]=delta_im1*delta_im1/(sum+delta_ip1);

	alpha[1]=alpha[1]/sum;
	beta[1] =beta[1]/sum;
	gamma[1]=gamma[1]/sum;

	/*Now for the main loop:   */
	for(i=2; i<l1; i++)
	{
		/* compute delta_i_minus_2,...  */
		delta_im2=(knot[i-1]-knot[i-2]);
		delta_im1=(knot[i]  -knot[i-1]);
		delta_i  =(knot[i+1]-knot[i]);
		delta_ip1=(knot[i+2]-knot[i+1]);
	
		sum = delta_im1+delta_i;

		alpha[i]=delta_i*delta_i/(delta_im2 +sum);
		beta[i] =delta_i*(delta_im2+delta_im1) 
			/(delta_im2 + sum)
			+
			 delta_im1*(delta_i+delta_ip1)
			/(sum + delta_ip1);
		gamma[i]=delta_im1*delta_im1
			/(sum + delta_ip1);
		
		alpha[i]=alpha[i]/sum;
		beta[i] =beta[i]/sum;
		gamma[i]=gamma[i]/sum;
	}

	/*  special care at the end:  */
	delta_im2=knot[l-2]-knot[l-3];
	delta_im1=knot[l1]-knot[l-2];
	delta_i  =knot[l]-knot[l1];
	sum=delta_im1+delta_i;

	alpha[l1]=delta_i*delta_i/(delta_im2+sum);
	beta[l1] =delta_i*(delta_im2+delta_im1)/(delta_im2  + sum)
                 +
		  delta_im1*delta_i / sum;
	gamma[l1]=delta_im1*delta_im1/sum;

	alpha[l1]=alpha[l1]/sum;
	beta[l1] =beta[l1]/sum;
	gamma[l1]=gamma[l1]/sum;


	alpha[l]=0.0; beta[l]=1.0; gamma[l]=0.0;


}

//::::::::::::::
//l_u_system.c
//::::::::::::::

/*	perform LU decomposition of tridiagonal system with
	lower diagonal alpha, diagonal beta, upper diagonal gamma.

input:	alpha,beta,gamma: the coefficient matrix entries
	l:	matrix size [0,l]x[0,l]
	low:	L-matrix entries
	up:	U-matrix entries
*/

void l_u_system(double alpha[], double beta[], double gamma[], int l, double up[], double low[])
{
	int i;

	up[0]=beta[0];
	for(i=1; i<=l; i++)
	{
		low[i]=alpha[i]/up[i-1];
		up[i] =beta[i]-low[i]*gamma[i-1];
	}
}

//::::::::::::::
//bessel_ends.c
//::::::::::::::

/*	Computes B-spline points data[1] and data[l+]
	according to Bessel end condition.

input:	data:	sequence of data coordinates data[0] to data[l+2].
		Note that data[1] and data[l+1] are expected to
		be empty, as they will be filled by this routine.
	knot:	knot sequence
	l:	number of intervals
 
output:	data:	completed, as above.
*/
void bessel_ends(double data[], double knot[], int l)
{
	double alpha, beta; 
	
	if (l==1)
	{/*  This is not really Bessel, but then what do you do
	     when you have only one interval? -- make it linear!
	 */
		data[1]= (2.0*data[0] + data[3])/3.0;
		data[2]= (2.0*data[3] + data[0])/3.0;
	}

	else if (l==2)
	{
		/* beginning:    */
		alpha= (knot[2]-knot[1])/(knot[2]-knot[0]);
		beta = 1.0 - alpha;

		data[1]=(data[2]-alpha*alpha*data[0]-beta*beta*data[4])
		/(2.0*alpha*beta);
		data[1]=2.0*(alpha*data[0]+beta*data[1])/3.0 + data[0]/3.0;

		/* end:  */
		alpha= (knot[2]-knot[1])/(knot[2]-knot[0]);
		beta = 1.0 - alpha;

		data[3]=(data[2]-alpha*alpha*data[0]-beta*beta*data[4])
		/(2.0*alpha*beta);

		data[3]=2.0*(alpha*data[3]+beta*data[4])/3.0+data[4]/3.0;	
	}
	
	else

	{

		/* beginning:    */
		alpha= (knot[2]-knot[1])/(knot[2]-knot[0]);
		beta = 1.0 - alpha;

		data[1]=(data[2]-alpha*alpha*data[0]-beta*beta*data[3])
		/(2.0*alpha*beta);
		data[1]=2.0*(alpha*data[0]+beta*data[1])/3.0 + data[0]/3.0;


		/* end:  */
		alpha= (knot[l]-knot[l-1])/(knot[l]-knot[l-2]);
		beta = 1.0 - alpha;

		data[l+1]=(data[l]-alpha*alpha*data[l-1]-beta*beta*data[l+2])
		/(2.0*alpha*beta);

		data[l+1]=2.0*(alpha*data[l+1]+beta*data[l+2])/3.0+data[l+2]/3.0;

	}

}

//::::::::::::::
//solve_system.c
//::::::::::::::

/*	solve  tridiagonal linear system
	of size (l+1)(l+1) whose LU decompostion has entries up and low,
	and whose right hand side is rhs, and whose original matrix
	had upper diagonal gamma. Solution is d[0],...,d[l+2];
Input: up,low,gamma:  as above.
       l:             size of system: l+1 eqs in l+1 unknowns.
       rhs:           right hand side, i.e, data points with end
                      `tangent Bezier points' in rhs[1] and rhs[l+1].
Output:d:             solution vector.

Note:	Both rhs and d are from 0 to l+2.
*/
 void solve_system(double up[], double low[], double gamma[], int l, double rhs[], double d[])
{
	int i;
	double aux[100];

	d[0] = rhs[0];
	d[1] = rhs[1];

	/* forward substitution:  */
	aux[0]=rhs[1];
	for(i=1; i<=l; i++) aux[i]=rhs[i+1]-low[i]*aux[i-1];
	
	/* backward substitution:  */
	d[l+1]=aux[l]/up[l];
	for(i=l-1; i>0; i--) d[i+1]=(aux[i]-gamma[i]*d[i+2])/up[i];
	d[l+2]=rhs[l+2];
}

//::::::::::::::
//parameters.c
//::::::::::::::


/*	Finds a centripetal parametrization for a given set
	of 2D data points. 
Input:	data_x, data_y:	input points, numbered from 0 to l+2. 
	l:		number of intervals.
Output:	knot:		knot sequence. Note: not (knot[l]=1.0)!
Note:	data_x[1], data_x[l+1] are not used! Same for data_y.
*/

double* parameters(double data_x[], double data_y[], int l, int degree)
{
	double* knots = NULL;
	int i,  l1,l2;     /* In the following, special care must be    */
	double delta;       /* at the ends because of the data structure */
	                   /* used. See note above.                     */
	l1 = l-1; 
	l2 = l+2;

	if (!(knots = (double*) malloc((l + 2 * degree - 1) * sizeof(double)))) {
		return NULL;
	}
	for (i = 0; i < degree; i++) {
		knots[i] = 0.0;
    }

	delta=sqrt((data_x[2]-data_x[0])*(data_x[2]-data_x[0])
		         +(data_y[2]-data_y[0])*(data_y[2]-data_y[0]));
	knots[degree]= sqrt(delta);       

	for(i=2; i<l; i++)
	{
		delta=sqrt((data_x[i+1]-data_x[i])*(data_x[i+1]-data_x[i])
		          +(data_y[i+1]-data_y[i])*(data_y[i+1]-data_y[i]));

		knots[i+degree-1]=knots[i+degree-2]+sqrt(delta);
	}

	delta=sqrt((data_x[l2]-data_x[l])*(data_x[l2]-data_x[l])
		         +(data_y[l2]-data_y[l])*(data_y[l2]-data_y[l]));
	knots[l+degree-1]= knots[l+degree-2]+sqrt(delta);     
		                               
	for (i = l + degree; i < l + 2 * degree - 1; i++) {
		knots[i] = knots[l+degree-1];
	}
	return (knots);

}

double* create_centripetal_knotsequence(double* data_x, double* data_y, int l, int degree) {
  double* knots = NULL;
  int i, l1, l2; // In the following, special care must be
  double delta; // at the ends because of the data structure
  // used. See note above.

  l1 = l - 1;
  l2 = l + 2;

  if (!(knots = (double*) malloc((l + 2 * degree - 1) * sizeof(double)))) {
    return (NULL);
  }

  // Erste Knoten bei Parameter 0
  for (i = 0; i < degree; i++) {
    knots[i] = 0.0;
  }

  delta = sqrt((data_x[2] - data_x[0]) * (data_x[2] - data_x[0]) + (data_y[2] - data_y[0]) * (data_y[2] - data_y[0]));

  knots[degree] = sqrt(delta);

  for (i = 2; i < l; i++) {
    delta = sqrt((data_x[i + 1] - data_x[i]) * (data_x[i + 1] - data_x[i]) + (data_y[i + 1] - data_y[i]) * (data_y[i
        + 1] - data_y[i]));
    knots[i + degree - 1] = knots[i + degree - 2] + sqrt(delta);
  }

  delta = sqrt((data_x[l2] - data_x[l]) * (data_x[l2] - data_x[l]) + (data_y[l2] - data_y[l])
      * (data_y[l2] - data_y[l]));
  knots[l + degree - 1] = knots[l + degree - 2] + sqrt(delta);

  // Knotensequenz normieren
  for (i = degree; i < l + degree; i++) {
    knots[i] /= knots[l + degree - 1];
  };
  for (i = l + degree; i < l + 2 * degree - 1; i++) {
    knots[i] = 1.0;
  }
  return (knots);
}

//double* create_centripetal_knotsequence_3D(int np, double* data) {
//  double* knots = NULL;
//  int i, j;	
//  double delta; 
//
//  if (!(knots = (double*) malloc(np * sizeof(double)))) 
//    return (NULL);
//
//  knots[0] = 0.0;
//
//  for (i = 1; i < np; i++) {
//	j=i-1;
//    delta = sqrt((data[3*i] - data[3*j]) * (data[3*i] - data[3*j]) + 
//		         (data[3*i + 1] - data[3*j + 1]) * (data[3*i + 1] - data[3*j + 1]) +
//				 (data[3*i + 2] - data[3*j + 2]) * (data[3*i + 2] - data[3*j + 2]));
//    knots[i] = knots[j] + sqrt(delta);   // sqrt since we uses centripetal
//  }
//
//  // Knotensequenz normieren
//  for (i=1; i < np; i++) {
//    knots[i] /= knots[np-1];
//  };
//  return (knots);
//}

double* create_centripetal_knotsequence_3D_farin(int np, double* data, int l, int degree) {
  double* knots = NULL;
  int i, j, l1, l2; // In the following, special care must be
  double delta; // at the ends because of the data structure
  // used. See note above.

  l1 = l - 1;
  l2 = l + 2;

  if (!(knots = (double*) malloc((l + 2 * degree - 1) * sizeof(double)))) {
    return (NULL);
  }

  // Erste Knoten bei Parameter 0
  for (i = 0; i < degree; i++) {
    knots[i] = 0.0;
  }

  // distance btw 0 and 2 points
  delta = sqrt((data[6] - data[0]) * (data[6] - data[0]) + (data[7] - data[1]) * (data[7] - data[1]) + (data[8] - data[2]) * (data[8] - data[2]));

  knots[degree] = sqrt(delta);

  for (i = 2; i < l; i++) {
	j=i+1;
    delta = sqrt((data[3*i] - data[3*j]) * (data[3*i] - data[3*j]) + 
		         (data[3*i + 1] - data[3*j + 1]) * (data[3*i + 1] - data[3*j + 1]) +
				 (data[3*i + 2] - data[3*j + 2]) * (data[3*i + 2] - data[3*j + 2]));
    knots[i + degree - 1] = knots[i + degree - 2] + sqrt(delta);		// sqrt since we uses centripetal
  }

  delta = sqrt((data[3*l2] - data[3*l]) * (data[3*l2] - data[3*l]) + (data[3*l2+1] - data[3*l+1])* (data[3*l2+1] - data[3*l+1])
	                                                              + (data[3*l2+2] - data[3*l+2])* (data[3*l2+2] - data[3*l+2]));
  knots[l + degree - 1] = knots[l + degree - 2] + sqrt(delta);

  // Knotensequenz normieren
  for (i = degree; i < l + degree; i++) {
    knots[i] /= knots[l + degree - 1];
  };
  for (i = l + degree; i < l + 2 * degree - 1; i++) {
    knots[i] = 1.0;
  }
  return (knots);
}


//::::::::::::::
//c2_spline.c
//::::::::::::::

/* Finds the C2 cubic spline interpolant to the data points in data_x, data_y.
Input:	knot:	the knot sequence  knot[0], ..., knot[l]
		l:		the number of intervals
		data_x, data_y: 
				the data points data_x[0], ..., data[l+2].
				Attention: data_x[1] and data_x[l+1] are filled by Bessel end 
						   conditions and are thus ignored on input. 
				Same for data_y.
Output:	bspl_x, bspl_y: 
				the B-spline control polygon of the interpolant. 
				Dimensions: bspl_x[0], ..., bspl_x[l+2]. 
				Same for bspl_y.
*/

double* c2_spline_interp(int l,						// in
	           int degree,							// in
			   double data_x[], double data_y[],	// in
			   double bspl_x[], double bspl_y[])	// out
{
	
	double alpha[temp_buf_size_], beta[temp_buf_size_], gamma[temp_buf_size_], up[temp_buf_size_], low[temp_buf_size_];

	double* knots = parameters(data_x, data_y, l, degree);
	if(!knots)
		return NULL;

	set_up_system(&knots[degree - 1],l,alpha,beta,gamma);

	l_u_system(alpha,beta,gamma,l,up,low);

	bessel_ends(data_x,&knots[degree - 1],l);
	bessel_ends(data_y,&knots[degree - 1],l);

	solve_system(up,low,gamma,l,data_x,bspl_x);
	solve_system(up,low,gamma,l,data_y,bspl_y);

	return knots;
}

void free_knot_vector(double *knots)
{
	free(knots);
}

double* create_centripetal_knotsequence_3D(int np, double* data) {
  double* knots = NULL;
  int i, j;	
  double delta; 

  if (!(knots = (double*) malloc(np * sizeof(double)))) 
    return (NULL);

  knots[0] = 0.0;

  for (i = 1; i < np; i++) {
	j=i-1;
    delta = sqrt((data[3*i] - data[3*j]) * (data[3*i] - data[3*j]) + 
		         (data[3*i + 1] - data[3*j + 1]) * (data[3*i + 1] - data[3*j + 1]) +
				 (data[3*i + 2] - data[3*j + 2]) * (data[3*i + 2] - data[3*j + 2]));
    knots[i] = knots[j] + sqrt(delta);   // sqrt since we uses centripetal
  }

  // Knotensequenz normieren
  for (i=1; i < np; i++) {
    knots[i] /= knots[np-1];
  };
  return (knots);
}

void sample_as_cbspline_interp_3D(int np, double* data, int newnp, double* newdata)
{
	double alpha[temp_buf_size_], beta[temp_buf_size_], gamma[temp_buf_size_], up[temp_buf_size_], low[temp_buf_size_];
	double u;
	int i,j;
	int degree = 3;	
	int l = np - 1;

	/// compute knot vector
	double* knots = create_centripetal_knotsequence_3D_farin(np, data,l,degree);

	/// set up
	set_up_system(&knots[degree - 1],l,alpha,beta,gamma);
	l_u_system(alpha,beta,gamma,l,up,low);

	/// re-sample 
	double *fx = (double*) malloc(np * sizeof(double));
	double *bspl = (double*) malloc(np * sizeof(double));
	double *newfx = (double*) malloc(newnp * sizeof(double));
	for(i=0; i<3; i++) {
		for(j=0; j<np; j++) 
			fx[j] = data[3*j+i];
		// compute b-spline
		bessel_ends(fx,&knots[degree - 1],l);
		solve_system(up,low,gamma,l,fx,bspl);
		// evaluate b-spline
		newdata[i] = fx[0];
		newdata[3*newnp+i] = fx[np - 1];
		for(j=1; j<newnp; j++) {
			u = (double) j / (newnp);
			bspl_evaluate(degree, l, bspl, knots, u, &(newdata[3*j+i]));
		}
	}
	free(fx);
	free(bspl);
	free(newfx);
	free_knot_vector(knots);
	return;
}

void filter_interp_points(int np, double* data, int max_allowed_points, int newnp, double* newdata)
{
	int nskp = (np - 1) / max_allowed_points;
	int k = nskp + 1;
	newnp = np / k;
	for(int i=0; i<newnp; i++)
		printf("%d ",k*i);

	printf("\n");
	return;
}

} // namespace farin