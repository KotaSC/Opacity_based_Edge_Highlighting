#include <kvs/PointObject>
#include <kvs/PolygonObject>
#include <kvs/glut/Application>
#include <kvs/glut/Screen>
#include <kvs/ParticleBasedRenderer>

#include <cstring>
#include <cstdlib>
#include <string>
#include <sstream>

#include "AlphaControlforPLY.h"
#include "ImportPointClouds.h"
#include "fileList.h"
#include "writePBRfile.h"
#include "getExtension.h"
#include "getFileName.h"
#include "alp_customize.h"
#include "alp_option.h"
#include "event_control.h"
#include "FeaturePointExtraction.h"

const double DEFAULT_CAMERA_DISTANCE = 12.0;

int main(int argc, char **argv)
{
  if (argc < 2)
  {
    std::cout << "USAGE: " << argv[0] << "lst-file or data-file (and options)." << std::endl;
    std::cout << "lst-file cannot require options \n";
    std::cout << "[Option] -a : Set the opacity ( default is " << OPACITY << " ) \n"
              << "         -l : Set the repeat level ( default is " << REPEAT_LEVEL << " ) \n"
              << "         -i : Set the image resolution ( default is " << IMAGE_RESOLUTION << " ) \n"
              << "        -fa : Set the opacity for feature extraction ( default is " << FEATURE_OPACITY << " ) \n"
              << "        -ft : Set the threshold for feature extraction ( default is " << THRESHOLD << " ) \n"
              << "[For example] " << argv[0] << " -a 0.1 -l 700 -fa 0.5 -ft 0.03 xxx.xyz"
              << std::endl;
    exit(1);
  }

  int repeatLevel     = REPEAT_LEVEL;
  int imageResolution = IMAGE_RESOLUTION;
  double alpha        = OPACITY;
  double ftAlpha      = FEATURE_OPACITY;
  double ftThresh     = THRESHOLD;

  fileList *files = new fileList(argv[1]);

  int numFiles = files->numberOfFiles();
  std::vector<std::string> inputFiles = files->plyFiles();
  std::vector<std::string> outptFiles = files->pbrFiles();
  std::vector<float> opacities = files->alphas();
  std::vector<float> opacities_ft = files->alphas_ft();
  std::vector<float> thresholds = files->thresholds();

  if (numFiles == 0)
  {
    std::vector<std::string> allowedFiles(allowedFileType, std::end(allowedFileType));
    for (int i = 1; i < argc; i++)
    {
      //---- Command-line Option
      if (!strncmp(REPEAT_LEVEL_OPTION, argv[i], strlen(REPEAT_LEVEL_OPTION)))
      {
        repeatLevel = atoi(argv[i + 1]);
        i++;
      }
      else if (!strncmp(ALPHA_OPTION, argv[i], strlen(ALPHA_OPTION)))
      {
        alpha = atof(argv[i + 1]);
        i++;
        if (alpha <= 0.0)
          alpha = 0.001;
        else if (alpha > 0.99)
          alpha = 0.99;
        std::cout << "Setting alpha " << alpha << std::endl;
      }
      else if (!strncmp(IMAGE_RESOLUTION_OPTION, argv[i], strlen(IMAGE_RESOLUTION_OPTION)))
      {
        imageResolution = atoi(argv[i + 1]);
        i++;
      }
      else if (!strncmp(FEATURE_ALPHA, argv[i], strlen(FEATURE_ALPHA)))
      {
        ftAlpha = atof(argv[i + 1]);
        i++;
      }
      else if (!strncmp(FEATURE_THRESHOLD, argv[i], strlen(FEATURE_THRESHOLD)))
      {
        ftThresh = atof(argv[i + 1]);
        i++;
      }
    }
    for (int i = 1; i < argc; i++)
    {
      for (int j = 0; j < (int)allowedFiles.size(); j++)
      {
        if (getExtension(argv[i]) == allowedFiles[j])
        {
          numFiles++;
          std::string output_tmp(OUT_PBR_FILE);
          std::string tmp;
          std::stringstream ssLR;
          std::stringstream ssAlpha;
          std::stringstream ssFtAlpha;
          std::stringstream ssTh;

          output_tmp += "LR";
          ssLR << repeatLevel;
          ssLR >> tmp;
          output_tmp += tmp;

          output_tmp += "_alpha";
          ssAlpha << alpha;
          ssAlpha >> tmp;
          tmp.erase(std::remove(tmp.begin(), tmp.end(), '.'), tmp.end());
          output_tmp += tmp;

          output_tmp += "_ftAlpha";
          ssFtAlpha << ftAlpha;
          ssFtAlpha >> tmp;
          tmp.erase(std::remove(tmp.begin(), tmp.end(), '.'), tmp.end());
          output_tmp += tmp;

          output_tmp += "_Th";
          ssTh << ftThresh;
          ssTh >> tmp;
          tmp.erase(std::remove(tmp.begin(), tmp.end(), '.'), tmp.end());
          output_tmp += tmp;

          output_tmp += "_";
          output_tmp += getFileName(argv[i]);
          output_tmp += ".spbr";

          inputFiles.push_back(argv[i]);
          outptFiles.push_back(output_tmp);
          opacities.push_back(alpha);
          opacities_ft.push_back(ftAlpha);
          thresholds.push_back(ftThresh);
        }
      }
    }
  }

  kvs::glut::Application app(argc, argv);
  kvs::glut::Screen screen(&app);

  //---- Set Camera Parameter
  if (files->isSetRepeatLevel())
    repeatLevel = files->repeatLevel();
  if (files->isImageResolution())
    imageResolution = files->imageResolution();
  if (files->isLookAt())
    screen.scene()->camera()->setLookAt(files->lookAt());
  if (files->isCameraPosition())
    screen.scene()->camera()->setPosition(files->cameraPosition());
  if (files->isViewAngle())
    screen.scene()->camera()->setFieldOfView(files->viewAngle());
  if (files->isCameraZoom())
  {
    float d0 = DEFAULT_CAMERA_DISTANCE / fabs(files->cameraZoom());
    screen.scene()->camera()->setPosition(kvs::Vector3f(0.0, 0.0, d0));
  }

  screen.setGeometry(0, 0, imageResolution, imageResolution);

  kvs::Vector3f BBMin(1.0e6, 1.0e6, 1.0e6);
  kvs::Vector3f BBMax(-1.0e6, -1.0e6, -1.0e6);
  if (files->isBoundingBox())
  {
    BBMin = files->boundingBoxMin();
    BBMax = files->boundingBoxMax();
  }
  else
  {
    for (int i = 0; i < numFiles; i++)
    {
      kvs::PolygonObject *ply = new ImportPointClouds((char *)inputFiles[i].c_str());
      ply->updateMinMaxCoords();
      if (BBMin.x() > ply->minObjectCoord().x())
        BBMin.x() = ply->minObjectCoord().x();
      if (BBMin.y() > ply->minObjectCoord().y())
        BBMin.y() = ply->minObjectCoord().y();
      if (BBMin.z() > ply->minObjectCoord().z())
        BBMin.z() = ply->minObjectCoord().z();
      if (BBMax.x() < ply->maxObjectCoord().x())
        BBMax.x() = ply->maxObjectCoord().x();
      if (BBMax.y() < ply->maxObjectCoord().y())
        BBMax.y() = ply->maxObjectCoord().y();
      if (BBMax.z() < ply->maxObjectCoord().z())
        BBMax.z() = ply->maxObjectCoord().z();

      std::cout << "Input data Size: " << i
                << ", Min: " << ply->minObjectCoord() << ", Max: " << ply->maxObjectCoord() << std::endl;
    }
  }

  std::cout << "Bounding Box size [ MIN: " << BBMin << ", MAX: " << BBMax << " ]" << std::endl;

  kvs::PointObject *object = new kvs::PointObject();

  //-- Creating points for Stochastic Point Based Rendering
  for (int i = 0; i < numFiles; i++)
  {
    ImportPointClouds *ply = new ImportPointClouds((char *)inputFiles[i].c_str());
    std::cout << "\n=============================================================" << std::endl;
    std::cout << "Creating Particles from: " << inputFiles[i] << std::endl;

    // kvs::PointObject *point =
    AlphaControlforPLY* point =
        new AlphaControlforPLY(ply, screen.scene()->camera(), BBMin, BBMax,
                               repeatLevel, opacities[i], ply->isFase());
    std::cout << "Number of Particles: " << point->numberOfVertices() << std::endl;

    object->add(*kvs::PointObject::DownCast(point));

    //---- File Output
    writePBRfile(files, repeatLevel, imageResolution, outptFiles[i], point);

    //--- Feature Visualization
    // 任意の不透明度を実現するために必要な点数・増減率を計算する
    double a_num = point->calculateRequiredPartcleNumber(opacities_ft[i], repeatLevel, BBMin, BBMax);
    double ft_ratio = point->pointRatio(a_num);

    std::cout << "Opacity for the Feature: " << opacities_ft[i] << ", " << ft_ratio << std::endl;
    std::vector<float> ft = ply->featureData();

    FeaturePointExtraction *f_point =
        new FeaturePointExtraction(ply, ft, ft_ratio, thresholds[i], repeatLevel, BBMin, BBMax);

    std::string ofname(outptFiles[i]);
    ofname += "_f.spbr";

    writePBRfile(files, repeatLevel, imageResolution, ofname, f_point);

    object->add(*kvs::PointObject::DownCast(f_point));
  }

  std::cout << "Number of Particles (Total): " << object->numberOfVertices() << std::endl;

  //--- Rendering
  object->updateMinMaxCoords();
  object->setMinMaxExternalCoords(object->minObjectCoord(), object->maxObjectCoord());
  if (files->isBoundingBox())
  {
    object->setMinMaxObjectCoords(files->boundingBoxMin(), files->boundingBoxMax());
  }

  kvs::glsl::ParticleBasedRenderer *renderer =
      new kvs::glsl::ParticleBasedRenderer();
  renderer->setRepetitionLevel(repeatLevel);
  renderer->enableLODControl();
  renderer->disableZooming();
  renderer->disableShading();

  screen.setBackgroundColor(kvs::RGBColor(0, 0, 0));
  screen.registerObject(object, renderer);

  InitializeEvent init;
  KeyPressEvent key;
  screen.addEvent(&init);
  screen.addEvent(&key);
  screen.show();
  key.displayMenu();

  return (app.run());
}