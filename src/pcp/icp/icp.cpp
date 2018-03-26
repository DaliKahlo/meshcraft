//$c1 XRL 01/13/14 created.
//=========================================================================
// point cloud processing algorithms
//

#include "stdafx.h"
#include "pcp.h"

#include <iostream>
#include <pcl/point_types.h>
#include <pcl/registration/icp.h>
//#include <pcl/registration/transformation_estimation_point_to_plane_lls.h>

//typedef pcl::registration::TransformationEstimationPointToPlaneLLS<pcl::PointNormal, pcl::PointNormal> PointToPlane; 

// return 0 if converged
//        1    not converged
//        2    unable to open file
int pc_align(int nsrc, double *xyzsrc, 
	         int ntgt, int nf, double *xyztgt, int *triatgt,
			 double *score, double trans[4][4], char *fpfname)
{
  size_t i;
  pcl::PointCloud<pcl::PointXYZ>::Ptr cloud_in (new pcl::PointCloud<pcl::PointXYZ>);
  pcl::PointCloud<pcl::PointXYZ>::Ptr cloud_out (new pcl::PointCloud<pcl::PointXYZ>);

  // Fill in the CloudIn data from source mesh
  cloud_in->width    = nsrc;
  cloud_in->height   = 1;
  cloud_in->is_dense = false;
  cloud_in->points.resize (cloud_in->width * cloud_in->height);

  for (i = 0; i < cloud_in->points.size (); ++i)
  {
    cloud_in->points[i].x = xyzsrc[3*i]; 
    cloud_in->points[i].y = xyzsrc[3*i + 1]; 
    cloud_in->points[i].z = xyzsrc[3*i + 2]; 
  }
  //std::cout << "Set " << cloud_in->points.size () << " source data points."
  //    << std::endl;

 // Fill in the CloudOut data from target mesh
  cloud_out->width    = ntgt;
  cloud_out->height   = 1;
  cloud_out->is_dense = false;
  cloud_out->points.resize (cloud_out->width * cloud_out->height);
  for (i = 0; i < cloud_out->points.size (); ++i)
  {
    cloud_out->points[i].x = xyztgt[3*i]; 
    cloud_out->points[i].y = xyztgt[3*i + 1]; 
    cloud_out->points[i].z = xyztgt[3*i + 2]; 
  }
  //std::cout << "Set " << cloud_out->points.size () << " target data points."
  //    << std::endl;
  //for (i = 0; i < cloud_out->points.size (); ++i) std::cout << "    " <<
  //    cloud_out->points[i].x << " " << cloud_out->points[i].y << " " <<
  //    cloud_out->points[i].z << std::endl;

  // ICP
  pcl::IterativeClosestPoint<pcl::PointXYZ, pcl::PointXYZ> icp;
  
  //PointToPlane *pPP = new PointToPlane;
  //boost::shared_ptr<PointToPlane> point_to_plane(pPP); 
  //icp.setTransformationEstimation(point_to_plane);

  icp.setInputCloud(cloud_in);
  icp.setInputTarget(cloud_out);

	//// Set the max correspondence distance to 5cm (e.g., correspondences with higher distances will be ignored)
	//icp.setMaxCorrespondenceDistance (6);
	//// Set the maximum number of iterations (criterion 1)
	//icp.setMaximumIterations (10);
	//// Set the transformation epsilon (criterion 2)
	//icp.setTransformationEpsilon (1e-8);
	//// Set the euclidean distance difference epsilon (criterion 3)
	//icp.setEuclideanFitnessEpsilon (1);

  pcl::PointCloud<pcl::PointXYZ> Final;
  //std::cout << "Aligning source to target...... " << std::endl;
  icp.align(Final);
  //std::cout << "Has converged:" << icp.hasConverged() << " score: " <<
  //icp.getFitnessScore() << std::endl;
  //std::cout << icp.getFinalTransformation() << std::endl;

  *score = icp.getFitnessScore();  //sum of squared distances from the source to the target

  Eigen::Matrix4f matx= icp.getFinalTransformation();
  for(i=0; i<4; i++) {
	  for(int j=0; j<4; j++) 
		trans[j][i] = matx(i,j);
  }

  if(fpfname) {
	  //char *ffname = "Final.pts";
	  FILE *fp = NULL ;
	  fp = fopen(fpfname,"w") ;
	  if ( fp == NULL )
		  return 2;
	  for (i = 0; i < Final.points.size (); ++i) {
		  fprintf(fp,"%le %le %le\n",Final.points[i].x, Final.points[i].y, Final.points[i].z);
	  }
	  fclose(fp);
	  //std::cout << "Please check Final.pts for result." << std::endl;
  }

  if(icp.hasConverged())
	  return 0;
  return 1 ;
}
