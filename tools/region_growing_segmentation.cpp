//
// Created by Masayuki IZUMI on 5/14/16.
// ref: http://pointclouds.org/documentation/tutorials/region_growing_segmentation.php
//

#include <iostream>
#include <vector>
#include <pcl/point_types.h>
#include <pcl/io/pcd_io.h>
#include <pcl/search/search.h>
#include <pcl/search/kdtree.h>
#include <pcl/features/normal_3d.h>
#include <pcl/visualization/pcl_visualizer.h>
#include <pcl/filters/passthrough.h>
#include <pcl/segmentation/region_growing.h>

int
main (int argc, char** argv)
{
  std::string pcd_file = argv[1];
  pcl::PointCloud<pcl::PointXYZRGBA>::Ptr cloud (new pcl::PointCloud<pcl::PointXYZRGBA>);

  if ( pcl::io::loadPCDFile(pcd_file, *cloud) == -1) {
    std::cout << "Cloud reading failed." << std::endl;
    return (-1);
  }

  auto tree = boost::shared_ptr<pcl::search::Search<pcl::PointXYZRGBA>> (new pcl::search::KdTree<pcl::PointXYZRGBA>);
  pcl::PointCloud <pcl::Normal>::Ptr normals (new pcl::PointCloud <pcl::Normal>);
  pcl::NormalEstimation<pcl::PointXYZRGBA, pcl::Normal> normal_estimator;
  normal_estimator.setSearchMethod(tree);
  normal_estimator.setInputCloud(cloud);
  normal_estimator.setKSearch(50);
  normal_estimator.compute(*normals);

  pcl::IndicesPtr indices (new std::vector <int>);
  pcl::PassThrough<pcl::PointXYZRGBA> pass;
  pass.setInputCloud (cloud);
  pass.setFilterFieldName ("z");
  pass.setFilterLimits (0.0, 1.0);
  pass.filter (*indices);

  pcl::RegionGrowing<pcl::PointXYZRGBA, pcl::Normal> reg;
  reg.setMinClusterSize (50);
  reg.setMaxClusterSize (1000000);
  reg.setSearchMethod (tree);
  reg.setNumberOfNeighbours (30);
  reg.setInputCloud (cloud);
  //reg.setIndices (indices);
  reg.setInputNormals (normals);
  reg.setSmoothnessThreshold (3.0 / 180.0 * M_PI);
  reg.setCurvatureThreshold (1.0);

  std::vector <pcl::PointIndices> clusters;
  reg.extract (clusters);

  std::cout << "Number of clusters is equal to " << clusters.size () << std::endl;
  std::cout << "First cluster has " << clusters[0].indices.size () << " points." << endl;
  std::cout << "These are the indices of the points of the initial" <<
    std::endl << "cloud that belong to the first cluster:" << std::endl;
  int counter = 0;
  while (counter < clusters[0].indices.size ()) {
    std::cout << clusters[0].indices[counter] << ", ";
    counter++;
    if (counter % 10 == 0) { std::cout << std::endl; }
  }
  std::cout << std::endl;

  auto colored_cloud = reg.getColoredCloudRGBA();
  pcl::visualization::PCLVisualizer viewer("Cluster viewer");
  viewer.addPointCloud(colored_cloud);

  while (!viewer.wasStopped()) {
    viewer.spinOnce(100);
  }

  return (0);
}