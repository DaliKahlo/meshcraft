#ifndef CAGD_FARIN_H_
#define CAGD_FARIN_H_

namespace farin {

  const int temp_buf_size_ = 1024;
  double *c2_spline_interp(int l, int degree, double data_x[], double data_y[], double bspl_x[], double bspl_y[]);
  void free_knot_vector(double *);
  double deboor(int degree,double coeff[],double knot[],double u, int i);
  void bspl_to_points(int degree, int l, double coeff_x[], double knot[], int dense, double points_x[], int *point_num);
  void bspl_evaluate(int degree, int l, double coeff_x[], double knot[], double u, double *points_x);

  double* create_centripetal_knotsequence_3D(int np, double* data);
  double* create_centripetal_knotsequence_3D_farin(int np, double* data, int l, int degree);
  void sample_as_cbspline_interp_3D(int np, double* data, int np2, double* data2);

}  // namespace farin

#endif /*CAGD_FARIN_H_*/