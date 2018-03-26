//$c1 XRL 01/13/14 created.
//=========================================================================
// point cloud processing algorithms
//

#include "stdafx.h"
#include "pcp.h"

#include <iostream>
#include <pcl/point_types.h>
#include <pcl/io/pcd_io.h>
#include <pcl/kdtree/kdtree_flann.h>
#include <pcl/features/normal_3d.h>
#include <pcl/surface/gp3.h>

/*
The method works by maintaining a list of points from which the mesh can be grown (“fringe” points) 
and extending it until all possible points are connected. It can deal with unorganized points, 
coming from one or multiple scans, and having multiple connected parts. It works best if the surface 
is locally smooth and there are smooth transitions between areas with different point densities.

Triangulation is performed locally, by projecting the local neighborhood of a point along the point’s 
normal, and connecting unconnected points. Thus, the following parameters can be set:

-- setMaximumNearestNeighbors(unsigned) and setMu(double) control the size of the neighborhood. The former
   efines how many neighbors are searched for, while the latter specifies the maximum acceptable distance 
   for a point to be considered, relative to the distance of the nearest point (in order to adjust to 
   changing densities). Typical values are 50-100 and 2.5-3 (or 1.5 for grids).
-- setSearchRadius(double) is practically the maximum edge length for every triangle. This has to be set by
   the user such that to allow for the biggest triangles that should be possible.
-- setMinimumAngle(double) and setMaximumAngle(double) are the minimum and maximum angles in each triangle. 
   While the first is not guaranteed, the second is. Typical values are 10 and 120 degrees (in radians).
-- setMaximumSurfaceAgle(double) and setNormalConsistency(bool) are meant to deal with the cases where there
   are sharp edges or corners and where two sides of a surface run very close to each other. To achieve this, 
   points are not connected to the current point if their normals deviate more than the specified angle (note 
   that most surface normal estimation methods produce smooth transitions between normal angles even at sharp 
   edges). This angle is computed as the angle between the lines defined by the normals (disregarding the 
   normal’s direction) if the normal-consistency-flag is not set, as not all normal estimation methods can 
   guarantee consistently oriented normals. Typically, 45 degrees (in radians) and false works on most datasets.
*/

int pc_greedy_projection (int np, double *xyz, 
	                      double max_edge_length,
						  int *ne, int **pelems)
{
  int i, j, k, m;

  // Load data into a PointCloud<T> with an appropriate type
  pcl::PointCloud<pcl::PointXYZ>::Ptr cloud (new pcl::PointCloud<pcl::PointXYZ>);
    
  cloud->width    = np;
  cloud->height   = 1;
  cloud->is_dense = false;
  cloud->points.resize (cloud->width * cloud->height);
  for (i = 0; i < cloud->points.size (); ++i) {
    cloud->points[i].x = xyz[3*i]; 
    cloud->points[i].y = xyz[3*i + 1]; 
    cloud->points[i].z = xyz[3*i + 2]; 
  }
  //* the data should be available in cloud

  // Normal estimation*
  // the method requires normals, so they are estimated using the standard method from PCL.
  pcl::NormalEstimation<pcl::PointXYZ, pcl::Normal> n;
  pcl::PointCloud<pcl::Normal>::Ptr normals (new pcl::PointCloud<pcl::Normal>);
  pcl::search::KdTree<pcl::PointXYZ>::Ptr tree (new pcl::search::KdTree<pcl::PointXYZ>);
  tree->setInputCloud (cloud);
  n.setInputCloud (cloud);
  n.setSearchMethod (tree);
  n.setKSearch (20);
  n.compute (*normals);
  //* normals should not contain the point normals + surface curvatures

  // Concatenate the XYZ and normal fields*
  // Since coordinates and normals need to be in the same PointCloud, we create a PointNormal type point cloud.
  pcl::PointCloud<pcl::PointNormal>::Ptr cloud_with_normals (new pcl::PointCloud<pcl::PointNormal>);
  pcl::concatenateFields (*cloud, *normals, *cloud_with_normals);
  //* cloud_with_normals = cloud + normals

  // Create search tree*
  pcl::search::KdTree<pcl::PointNormal>::Ptr tree2 (new pcl::search::KdTree<pcl::PointNormal>);
  tree2->setInputCloud (cloud_with_normals);

  // Initialize objects
  pcl::GreedyProjectionTriangulation<pcl::PointNormal> gp3;
  pcl::PolygonMesh triangles;

  // Set the maximum distance between connected points (maximum edge length)
  gp3.setSearchRadius (max_edge_length);

  // Set typical values for the parameters

  // control the size of the neighborhood
  gp3.setMu (3.0);
  gp3.setMaximumNearestNeighbors (100);
  // minimum and maximum angles in each triangle
  gp3.setMinimumAngle(M_PI/36); // 10 degrees
  gp3.setMaximumAngle(160*M_PI/180); // 120 degrees
  // deal with the cases where there are sharp edges or corners
  gp3.setMaximumSurfaceAngle(M_PI/4); // 45 degrees
  gp3.setNormalConsistency(false);

  // Get result
  gp3.setInputCloud (cloud_with_normals);
  gp3.setSearchMethod (tree2);
  gp3.reconstruct (triangles);

  // Additional vertex information
  std::vector<int> parts = gp3.getPartIDs();
  std::vector<int> states = gp3.getPointStates();   // FREE, FRINGE, COMPLETED, BOUNDARY or NONE

  pcl::Vertices v;
  m = triangles.polygons.size();
  int *elems = new int[m*5];
  for(i=0; i<m; i++) {
	  v = triangles.polygons[i];
	  k = v.vertices.size();
	  elems[i*5] = k;
	  for(j=1; j<=k; j++) {
		 elems[i*5+j] = v.vertices[j-1];
	  }
  }
  *ne = m;
  *pelems = elems;

  // Finish
  return (0);
}

