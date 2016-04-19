#include <opencv2/core/utility.hpp>
#include <opencv2/saliency.hpp>
#include <opencv2/highgui.hpp>
#include <iostream>

using namespace std;
using namespace cv;
using namespace saliency;

static const char* keys =
{ "{@saliency_algorithm | | Saliency algorithm <saliencyAlgorithmType.[saliencyAlgorithmTypeSubType]> }"
    "{@video_name      | | video name            }"
    "{@start_frame     |1| Start frame           }"
    "{@training_path   |1| Path of the folder containing the trained files}" };

static void help()
{
  cout << "\nThis example shows the functionality of \"Saliency \""
       "Call:\n"
       "./example_saliency_computeSaliency <saliencyAlgorithmSubType> <video_name> <start_frame> \n"
       << endl;
}

int main( int argc, char** argv )
{
  CommandLineParser parser( argc, argv, keys );

  String saliency_algorithm = parser.get<String>( 0 );
  String video_name = parser.get<String>( 1 );
  int start_frame = parser.get<int>( 2 );
  String training_path = parser.get<String>( 3 );

  if( saliency_algorithm.empty() || video_name.empty() )
  {
    help();
    return -1;
  }

  //open the capture
  VideoCapture cap;
  cap.open( video_name );
  cap.set( CAP_PROP_POS_FRAMES, start_frame );

  if( !cap.isOpened() )
  {
    help();
    cout << "***Could not initialize capturing...***\n";
    cout << "Current parameter's value: \n";
    parser.printMessage();
    return -1;
  }

  Mat frame;

  //instantiates the specific Saliency
  Ptr<Saliency> saliencyAlgorithm = Saliency::create( saliency_algorithm );

  if( saliencyAlgorithm == NULL )
  {
    cout << "***Error in the instantiation of the saliency algorithm...***\n";
    return -1;
  }

  Mat binaryMap;
  Mat image;

  cap >> frame;
  if( frame.empty() )
  {
    return 0;
  }

  frame.copyTo( image );

  VideoWriter* outputVideo = new VideoWriter( "salinecy.avi", CV_FOURCC('D', 'I', 'V', 'X'), cap.get(CV_CAP_PROP_FPS), frame.size() );
  printf("%d %d\n", frame.cols, frame.rows);

  if( saliency_algorithm.find( "SPECTRAL_RESIDUAL" ) == 0 )
  {
    Mat saliencyMap;
    if( saliencyAlgorithm->computeSaliency( image, saliencyMap ) )
    {
      StaticSaliencySpectralResidual spec;
      spec.computeBinaryMap( saliencyMap, binaryMap );

      imshow( "Saliency Map", saliencyMap );
      imshow( "Original Image", image );
      imshow( "Binary Map", binaryMap );
      waitKey( 0 );
    }

  }
  else if( saliency_algorithm.find( "BING" ) == 0 )
  {
    if( training_path.empty() )
    {

      cout << "Path of trained files missing! " << endl;
      return -1;
    }

    else
    {
      vector<Vec4i> saliencyMap;
      saliencyAlgorithm.dynamicCast<ObjectnessBING>()->setTrainingPath( training_path );
      saliencyAlgorithm.dynamicCast<ObjectnessBING>()->setBBResDir( training_path + "/Results" );

      if( saliencyAlgorithm->computeSaliency( image, saliencyMap ) )
      {
        std::cout << "Objectness done" << std::endl;
      }
    }

  }
  else if( saliency_algorithm.find( "BinWangApr2014" ) == 0 )
  {

    //Ptr<Size> size = Ptr<Size>( new Size( image.cols, image.rows ) );
    saliencyAlgorithm.dynamicCast<MotionSaliencyBinWangApr2014>()->setImagesize( image.cols, image.rows );
    saliencyAlgorithm.dynamicCast<MotionSaliencyBinWangApr2014>()->init();

    bool paused = false;
    for ( ;; )
    {
      if( !paused )
      {

        cap >> frame;
        cvtColor( frame, frame, COLOR_BGR2GRAY );

        Mat saliencyMap;
        if( saliencyAlgorithm->computeSaliency( frame, saliencyMap ) )
        {
          std::cout << "current frame motion saliency done" << std::endl;
        }

        imshow( "image", frame );
        imshow( "saliencyMap", saliencyMap * 255 );
        saliencyMap = saliencyMap * 255;
        saliencyMap.convertTo(saliencyMap, CV_8UC3);
        Mat output = Mat(saliencyMap.rows, saliencyMap.cols, CV_8UC3);
        for (int y=0; y<saliencyMap.rows; y++)
          for (int x=0; x<saliencyMap.cols; x++) {
            if (saliencyMap.at<uchar>(y, x) == 0)
              output.at<Vec3b>(y, x) = Vec3b(0, 0, 0);
            else
              output.at<Vec3b>(y, x) = Vec3b(255, 255, 255);
          }
        
        (*outputVideo) << output;
      }

      char c = (char) waitKey( 2 );
      if( c == 'q' )
        break;
      if( c == 'p' )
        paused = !paused;

    }
  }

  return 0;
}