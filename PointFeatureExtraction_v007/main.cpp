#include <iostream>
#include <cstdlib>
#include <vector>
#include "importPointClouds.h"
#include "calculateFeature.h"
#include "writeFeature.h"

#include <kvs/PolygonObject>
#include <kvs/PointObject>
#include <kvs/glut/Application>
#include <kvs/glut/Screen>
#include <kvs/PointRenderer>
#include <kvs/Coordinate>
#include <kvs/ColorMap>

const char OUT_FILE[] = "../XYZ_DATA/out.xyz";

int main( int argc, char** argv )
{
  char outXYZfile[512];
  strcpy( outXYZfile, OUT_FILE );
  if( argc < 2 ) {
    std::cout << "USAGE   : " << argv[0] << " [input_point_cloud_data] [output_point_cloud_data]" << std::endl;
    std::cout << "EXAMPLE : " << argv[0] << " [input_point_cloud.ply] [output_point_cloud.xyz]" << std::endl;
    exit( 1 );
  } else if( argc == 3 ) {
    strcpy( outXYZfile, argv[2] );
  }
  //--- Inheritance of KVS::PolygonObject
  ImportPointClouds *ply = new ImportPointClouds( argv[1] );
  ply->updateMinMaxCoords();
  std::cout << "PLY Mim, Max Coords:" << std::endl;
  std::cout << "Min : " << ply->minObjectCoord() << std::endl;
  std::cout << "Max : " << ply->maxObjectCoord() << std::endl;
  std::cout << std::endl;

  //--- Set up for calculating feature
  calculateFeature *ft = new calculateFeature();

  //--- Select type of Feature Calculation
  int featureCalculationID;

  std::cout << "Feature calculation type" << std::endl;
  std::cout << "Point PCA: " << calculateFeature::PointPCA << ", ";
  std::cout << "Minimum entropy PCA: " << calculateFeature::MinimumEntropyFeature << std::endl;

  std::cout << "Select an ID >> ";
  std::cin >> featureCalculationID;

  std::cout << "Feature calculation type ==> " << featureCalculationID << std::endl;
  std::cout << std::endl;

  if ( featureCalculationID == calculateFeature::PointPCA )
    ft->setFeatureType( calculateFeature::PointPCA );
  else if ( featureCalculationID == calculateFeature::MinimumEntropyFeature )
    ft->setFeatureType( calculateFeature::MinimumEntropyFeature );

  // ft->setFeatureType( calculateFeature::PlaneBasedFeature );

  // ft->addNoise( 0.1 );
  // ft->setFeatureType( calculateFeature::NormalPCA );
  // ft->setFeatureType( calculateFeature::NormalDispersion );

  //---- Calculation
  int featureValueID;

  std::cout << "Feature value type" << std::endl;
  std::cout << "Change of curvature: " << calculateFeature::CHANGE_OF_CURVATURE_ID << ", ";
  std::cout << "Aplanarity: " << calculateFeature::APLANARITY_ID << ", ";
  std::cout << "Linearity: " << calculateFeature::LINEARITY_ID << ", ";
  std::cout << "Eigentropy: " << calculateFeature::EIGENTROPY_ID << std::endl;

  std::cout << "Select an ID >> ";
  std::cin >> featureValueID;

  std::cout << "Feature value type ==> " << featureValueID << std::endl;
  std::cout << std::endl;

  if ( featureValueID == calculateFeature::CHANGE_OF_CURVATURE_ID )
    ft->setFeatureValueID( calculateFeature::CHANGE_OF_CURVATURE_ID );
  else if ( featureValueID == calculateFeature::APLANARITY_ID )
    ft->setFeatureValueID( calculateFeature::APLANARITY_ID );
  else if ( featureValueID == calculateFeature::LINEARITY_ID )
    ft->setFeatureValueID( calculateFeature::LINEARITY_ID );
  else if ( featureValueID == calculateFeature::EIGENTROPY_ID )
    ft->setFeatureValueID( calculateFeature::EIGENTROPY_ID );

  ft->calc( ply );

  //--- Getting Feature value
  std::vector<float> ftvec = ft->feature( );

  //-- Output File for "xyzrgbf"
  WritingDataType type = Ascii; // Writing data as ascii
  //  WritingDataType type = Binary;    // Writing data as Binary
  writeFeature( ply, ftvec, outXYZfile, type );

  //--- Convert PolygonObject to PointObject
  kvs::PointObject* object = new kvs::PointObject( *ply );

  float ftMax = (float)ft->maxFeature();
  // float ftMin = (float)ft->minFeature();
  kvs::ColorMap cmap( 256.0, 0.0, ftMax );
  // kvs::ColorMap cmap( 256, 0.0175976, 0.021997 );
  // kvs::ColorMap cmap( 256, ftMin, ftMax );
  cmap.create();

  std::vector<unsigned char> cl;
  for( size_t i=0; i<ply->numberOfVertices(); i++ ) {
    kvs::RGBColor color( cmap.at( ftvec[i] ) );
    cl.push_back( color.r() );
    cl.push_back( color.g() );
    cl.push_back( color.b() );
    // cl.push_back( color.r() );
    // cl.push_back( color.g() );
    // cl.push_back( 0 );
  }
  object->setColors( kvs::ValueArray<kvs::UInt8>( cl ) );

  // object->setSize( 5 );
  object->setSize( 1 );
  object->updateMinMaxCoords();

  kvs::glut::Application app( argc, argv );
  kvs::glut::Screen screen( &app );

  kvs::PointRenderer* renderer = new kvs::PointRenderer();
  renderer->enableTwoSideLighting();

  screen.setSize( 1000, 1000 );
  screen.setTitle( "Point Object" );

  // Box
  // kvs::Vector3f cam_pos(-8, 8, 7);
  // kvs::Vector3f cam_up(0, 0, 1);

  // kvs::Vector3f cam_pos(0, 11, 0);
  // kvs::Vector3f cam_up(0, 0, 1);

  // screen.scene()->camera()->setPosition(cam_pos);
  // screen.scene()->camera()->setUpVector(cam_up);

  screen.registerObject( object, renderer );

  screen.show();

  return app.run();

}