//$c2 XRL 12/19/12 Added CubicMeanRatio from MeshAdapt.
//$c1 XRL 11/08/12 Created.
//========================================================================//
//              Copyright 2011 (Unpublished Material)                     //
//										                                  //
//========================================================================//
#include "stdafx.h"
#include "util.h"
#include <math.h>

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

//void crossProd(double *v1, double *v2, double *cp)
//{
//  cp[0] = v1[1]*v2[2] - v1[2]*v2[1] ;
//  cp[1] = v1[2]*v2[0] - v1[0]*v2[2] ;
//  cp[2] = v1[0]*v2[1] - v1[1]*v2[0] ;
//}

// given three points in a plane, A, B and C, compute unit normal
// return false in case of zero area. true otherwise
bool norm(double a[3], double b[3], double c[3], double *n)
{
	double ll, ac[3], bc[3], nor[3];
	int i;
	for(i=0; i<3; i++) {
		ac[i] = a[i] - c[i];
		bc[i] = b[i] - c[i];
	}
	crossProd(ac,bc,nor);

	ll = nor[0]*nor[0] + nor[1]*nor[1] + nor[2]*nor[2];
	if(ll < TOLERANCE2)
		return false;

	ll = sqrt(ll);
	for(i=0; i<3; i++) 
		n[i] = nor[i]/ll;
	return true;
}

/*----------------------------------------------------------------------------
----------------------------------------------------------------------------*/
void addVt(double *a,double *b,double *v)
{
  v[0] = a[0] + b[0] ;
  v[1] = a[1] + b[1] ;
  v[2] = a[2] + b[2] ;
}

/*----------------------------------------------------------------------------
----------------------------------------------------------------------------*/
void diffVt(double *a,double *b,double *v)
{
  v[0] = a[0] - b[0] ;
  v[1] = a[1] - b[1] ;
  v[2] = a[2] - b[2] ;
}


/*----------------------------------------------------------------------------
----------------------------------------------------------------------------*/
void scaleVt(double *a,double n)
{
  a[0] *= n ;
  a[1] *= n ;
  a[2] *= n ;
}

/*----------------------------------------------------------------------------
----------------------------------------------------------------------------*/
double dotProd(double *v1, double *v2)
{
  return v1[0]*v2[0] + v1[1]*v2[1] + v1[2]*v2[2] ;
}

/*----------------------------------------------------------------------------
----------------------------------------------------------------------------*/
void crossProd(double *v1, double *v2, double *cp)
{
  cp[0] = v1[1]*v2[2] - v1[2]*v2[1] ;
  cp[1] = v1[2]*v2[0] - v1[0]*v2[2] ;
  cp[2] = v1[0]*v2[1] - v1[1]*v2[0] ;
}

/*----------------------------------------------------------------------------
----------------------------------------------------------------------------*/
int sameSign(double x, double y)
{
  return (x<=0. && y<=0.) || ( x>=0. && y>= 0.) ;
}

/*----------------------------------------------------------------------------
----------------------------------------------------------------------------*/
double vecMag(double *vec)
{
  return sqrt((vec[0]*vec[0] + vec[1]*vec[1] + vec[2]*vec[2])) ;
}

/*----------------------------------------------------------------------------
 normalize a 3D vector
----------------------------------------------------------------------------*/
int normVt(double *v1,double *nv)
{
  double norm ;

  norm = v1[0]*v1[0] + v1[1]*v1[1] + v1[2]*v1[2] ;
//#ifdef DEBUG
//  if ( C_raneql(norm,0.,DTOLRENCE*DTOLRENCE) )
//  {
//    ErrorHandler("Division by Zero","normVt",WARN) ;
//    return(0) ;
//  }
//#endif
  norm = 1./sqrt(norm) ;
  nv[0] = v1[0]*norm ; 
  nv[1] = v1[1]*norm ; 
  nv[2] = v1[2]*norm ;

  return(1) ;
}

/*----------------------------------------------------------------------------
----------------------------------------------------------------------------*/
double det3Mat(double *v1,double *v2,double *v3) 
{
  return 
    v1[0]*v2[1]*v3[2] + v2[0]*v3[1]*v1[2] + v1[1]*v2[2]*v3[0] -
    v1[2]*v2[1]*v3[0] - v2[2]*v3[1]*v1[0] - v1[1]*v2[0]*v3[2] ;
}


void negVt(double *a) {
  a[0] = -a[0];
  a[1] = -a[1];
  a[2] = -a[2];
}

int cubicMeanRatio(double xyz[4][3],double *shape)
{
  double vol,sumSq;
  double tol=1.0e-14;
  
  /* compute the 6*volume in Euclidean space */
  double v01[3], v02[3], v03[3], normal[3];
  diffVt(xyz[1],xyz[0],v01);
  diffVt(xyz[2],xyz[0],v02);
  crossProd(v01,v02,normal);
  diffVt(xyz[3],xyz[0],v03);
  vol=dotProd(normal,v03);

  /* check if element is valid */
  if ( vol < tol*tol*tol ) 
    { *shape=0; return(0) ;}
  
  /* compute the summation of edge length square */
  sumSq=dotProd(v01,v01);
  sumSq += dotProd(v02,v02);
  sumSq += dotProd(v03,v03);

  diffVt(xyz[3],xyz[1],v01);
  sumSq += dotProd(v01,v01);
  // sumSq += XYZ_distance2(xyz[3],xyz[1]);
  diffVt(xyz[3],xyz[2],v01);
  sumSq += dotProd(v01,v01);
  // sumSq += XYZ_distance2(xyz[3],xyz[2]);
  diffVt(xyz[1],xyz[2],v01);
  sumSq += dotProd(v01,v01);
  // sumSq += XYZ_distance2(xyz[1],xyz[2]);

  /* compute cubic mean ratio */
  *shape=432.*vol*vol/(sumSq*sumSq*sumSq);

  /* check if element is acceptable */
  if( *shape<tol*tol*tol ) 
    { return (0);}
  return(1) ;
}

int fMeanRatio(double xyz[3][3],double *ratio) {

  double aSq,sumSq;
  double v01[3], v02[3], nor[3];
		
  diffVt(xyz[1],xyz[0],v01) ;
  diffVt(xyz[2],xyz[0],v02) ;
  crossProd(v01,v02,nor);
  aSq = 0.25*dotProd(nor,nor) ;
  if( aSq<=1.0e-28 ) 
  { *ratio=0; return(0); } 
    
  sumSq=dotProd(v01,v01);
  sumSq += dotProd(v02,v02);
  diffVt(xyz[1],xyz[2],v01);
  sumSq += dotProd(v01,v01);
    
  /* computer mean ratio square */
  *ratio=48.*aSq/(sumSq*sumSq);
  return 1;
}

/* given a segment defined by Lxyz0 & Lxyz1, determine the shortest
   distance square from point pxyz to that segment  11/26/01 -li */
double P_sqrDistToSegment(double L_ori[3],double L_end[3], double pxyz[3])
{
    double L_dir[3], kDiff[3];
    double fT;
    
    diffVt(L_end,L_ori,L_dir);
    diffVt(pxyz,L_ori,kDiff);
    fT=dotProd(kDiff,L_dir);
    
    if( fT > 0.0 ) {
      double fSqrLen=dotProd(L_dir,L_dir);
      if( fT>=fSqrLen ) 
		diffVt(kDiff,L_dir,kDiff);
      else {
		fT /= fSqrLen;
		L_dir[0] *= fT; 
		L_dir[1] *= fT; 
		L_dir[2] *= fT;
		diffVt(kDiff,L_dir,kDiff);
      }
    }
    
    return dotProd(kDiff,kDiff);
}

/* given a segment and a point, find parameter t at projection wrt the segment
   assume: t=0 at segment[0]
              t=1 at segment[1]
			  t>1 out of segment[1]
			  t<0 out of segment[0]
	          0<t<1 if the projection is on the segment
   for |dot1+dot2|, spr 425496 has edges that need 1.e-12; 
				    spr 425473 has edges that need 1.e-14.   */
double ParOnLinearEdge(double segment[2][3], double xyz[3])
{
	double AB[3], AC[3], CB[3], dot1, dot2;
	diffVt(segment[1],segment[0],AB);
	diffVt(segment[1],xyz,CB);
	diffVt(xyz,segment[0],AC);
	dot1=dotProd(CB,AB);
	dot2=dotProd(AC,AB);
#ifdef _DEBUG
	if(ABS(dot1+dot2)<1.e-14)  // true only |AB|-->0
		printf("ParOnLinearEdge: demoninator equals to %le\n",dot1+dot2);
#endif
	return dot2/(dot1+dot2);
}

void center(int nv, double fxyz[4][3], double c[3])
{
	c[0]=fxyz[0][0];
	c[1]=fxyz[0][1];
	c[2]=fxyz[0][2];
	for(int i=1; i<nv; i++) {
		c[0]+=fxyz[i][0];
		c[1]+=fxyz[i][1];
		c[2]+=fxyz[i][2];
	}
	c[0] /= nv;
	c[1] /= nv;
	c[2] /= nv;
	return;
}

void shrink(int nv, double fxyz[4][3], double k)
{
	double c[3];
	center(nv,fxyz,c);

	k = 1.0 - k;
	for(int i=0; i<nv; i++) {
		fxyz[i][0] += k*(c[0] - fxyz[i][0]);
		fxyz[i][1] += k*(c[1] - fxyz[i][1]);
		fxyz[i][2] += k*(c[2] - fxyz[i][2]);
	}
}

//char * CString2char(const CString& cszData)
//{
//  USES_CONVERSION;
//  char* pchTmp(T2A(cszData));
//  return pchTmp;
//}

bool str2floatInMeter(CString cs, double *val)
{
	if( !cs.IsEmpty() ) {
		int pos = cs.Find(_T("mm"));
		if(pos != -1) {
			CString left = cs.Left(pos);
			*val = _ttof(left) * 0.001;
			return true;
		}
		pos = cs.Find(_T("m"));
		if(pos != -1) {
			CString left = cs.Left(pos);
			*val = _ttof(left);
			return true;
		}
	}
	return false;
}

void boundingbox(int nn, double *nodes, double minPt[3], double maxPt[3])
{
	if(nn>0) {
		double coord[3];
		int i,j;
		minPt[0] = nodes[0];
		minPt[1] = nodes[1];
		minPt[2] = nodes[2];
		maxPt[0] = minPt[0];
		maxPt[1] = minPt[1];
		maxPt[2] = minPt[2];
		for(i=1; i<nn; i++) {
			for( j=0; j<3; j++) {
				coord[j] = nodes[3*i + j];
				if(minPt[j] > coord[j]) minPt[j] = coord[j];
				if(maxPt[j] < coord[j]) maxPt[j] = coord[j];
			}
		}
	}
	return;
}

//
int circle(double xyz[3][3], double *center, double *r, double *axis)
{
	// triangle edges
	double t[3], u[3], v[3];
	diffVt(xyz[1],xyz[0],t);
	diffVt(xyz[2],xyz[0],u);
	diffVt(xyz[2],xyz[1],v);

	// triangle normal
	double w[3], wsl, iwsl2;
	crossProd(t,u,w);
	wsl = dotProd(w,w);
	if(wsl<1.0e-14)
		return 1;  // area of the triangle is too small

	// helpers
	iwsl2 = 1.0 / (2.0*wsl);
	double tt = dotProd(t,t);
	double uu = dotProd(u,u);
	double vv = dotProd(v,v);
	double uv = dotProd(u,v);
	double tv = dotProd(t,v);

	// result circle
	for(int i=0; i<3; i++) {
		center[i] = xyz[0][i] + (u[i]*tt*uv - t[i]*uu*tv) * iwsl2;
		axis[i] = w[i] / wsl;
	}
	*r = sqrt(tt*uu*vv*iwsl2*0.5);

	return 0;
}

int sphere(double[4][3], double **, double *r)
{
	return 0;
}
