//$c1 XRL 11/08/12 Created.
//////////////////////////////////////////////////////////
///
///

#if !defined(MW_UTIL_H__E1C38130_3A61_11D5_9107_00105AA7521E__INCLUDED_)
#define MW_UTIL_H__E1C38130_3A61_11D5_9107_00105AA7521E__INCLUDED_

#define MAX(x,y) ((x)<(y) ? (y) : (x))
#define MIN(x,y) ((x)>(y) ? (y) : (x))
#define ABS(x) ((x) < 0 ? -(x) : (x))
#define SWAP(T, a, b) { T save = (a); (a) = (b); (b) = save; }

#define TOLERANCE2	1.0e-28
#define MW_TOL		1.e-6
#define VALUE_NOT_EXIST 1.0e14

#define RESERVED_VERTEX_TEMP_ID_1	-9999999
#define RESERVED_VERTEX_TEMP_ID_2	-9999998
#define DELETE_VERTEX_TEMP_ID		-9999997

void diffVt(double *a,double *b,double *v);
double dotProd(double *v1, double *v2);
void crossProd(double *v1, double *v2, double *cp);
int normVt(double *v1,double *nv);
bool norm(double a[3], double b[3], double c[3], double *n);
int cubicMeanRatio(double xyz[4][3],double *shape);
int fMeanRatio(double xyz[3][3],double *ratio);
void shrink(int nv, double fxyz[4][3], double k);

double P_sqrDistToSegment(double L_ori[3],double L_end[3], double pxyz[3]);
double ParOnLinearEdge(double segment[2][3], double xyz[3]);

char * CString2char(const CString& cszData);
bool str2floatInMeter(CString cs, double *val);
void boundingbox(int nn, double *nodes, double minPt[3], double maxPt[3]);

int circle(double[3][3], double[3], double *r, double[3]);
int sphere(double[4][3], double (*)[3], double *r);

#endif
