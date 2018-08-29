#include <flycapture/FlyCapture2.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include <sl_zed/Camera.hpp>
#include <SaveDepth.hpp>
#define PORT 50210
//using namespace FlyCapture2;
//using namespace sl;
using namespace cv;

cv::Mat slMat2cvMat(Mat& input);
int main()
{
 //  *******************  SIZES  ********************************+
 Size bigimg(960,768);
 Size smallimg(320,256);
 Size window(1280,1024);
 namedWindow( "T0-R0 Camera GUI", WINDOW_AUTOSIZE );

 //  *******************  ZED section of setup code  ********************************+
  using namespace sl;
  // Create a ZED camera object
  Camera zed;

  // Set configuration parameters
  InitParameters init_params;
  init_params.camera_resolution = RESOLUTION_HD720;
  init_params.depth_mode = DEPTH_MODE_PERFORMANCE;
  init_params.coordinate_units = UNIT_METER;
  if (argc > 1) init_params.svo_input_filename.set(argv[1]);

  // Open the camera
  ERROR_CODE err = zed.open(init_params);
  if (err != SUCCESS) {
      printf("%s\n", toString(err).c_str());
      zed.close();
      return 1; // Quit if an error occurred
   }
   // Set runtime parameters after opening the camera
      RuntimeParameters runtime_parameters;
      runtime_parameters.sensing_mode = SENSING_MODE_STANDARD;

      // Prepare new image size
  Resolution image_size = zed.getResolution();
  int new_width = image_size.width ;
  int new_height = image_size.height ;
  // To share data between sl::Mat and cv::Mat, use slMat2cvMat()
  // Only the headers and pointer to the sl::Mat are copied, not the data itself
  Mat image_zed(new_width, new_height, MAT_TYPE_8U_C4);
  cv::Mat image_ocv = slMat2cvMat(image_zed);
  Mat depth_image_zed(new_width, new_height, MAT_TYPE_8U_C4);
  cv::Mat depth_image_ocv = slMat2cvMat(depth_image_zed);
  Mat image_ocv_small;
  Mat depth_image_ocv_small;

  //  *****************  Point Grey Section of setup code  **************************
      using namespace FlyCapture2;
      Error error;
      Camera camera;
      CameraInfo camInfo;
      error = ForceAllIPAddressesAutomatically();

      // Connect the camera
      error = camera.Connect( 0 );
      if ( error != PGRERROR_OK )
      {
          std::cout << "Failed to connect to camera PG" << std::endl;
          return false;
      }

      // Get the camera info and print it out
      error = camera.GetCameraInfo( &camInfo );
      if ( error != PGRERROR_OK )
      {
          std::cout << "Failed to get camera info from camera PG" << std::endl;
          return false;
      }
      std::cout << camInfo.vendorName << " "
                << camInfo.modelName << " "
                << camInfo.serialNumber << std::endl;

      error = camera.StartCapture();
      if ( error == PGRERROR_ISOCH_BANDWIDTH_EXCEEDED )
      {
          std::cout << "PG :Bandwidth exceeded" << std::endl;
          return false;
      }
      else if ( error != PGRERROR_OK )
      {
          std::cout << "PG: Failed to start image capture" << std::endl;
          return false;
      }
      Mat matPG_small;
      // ****************************  ARM camera setup section *******************************
      using namespace cv;
      VideoCapture cap(0);


      const double fps = cap.get(CAP_PROP_FPS);
      const int width  = cap.get(CAP_PROP_FRAME_WIDTH);
      const int height = cap.get(CAP_PROP_FRAME_HEIGHT);
      const int fourcc = cap.get(CAP_PROP_FOURCC );

      // printf("%d", cap.get(CV_CAP_PROP_CONVERT_RGB));
      cv::Mat matARM(width, height, CV_8UC3);


       //if not success, exit program
       if (cap.isOpened() == false)
       {
        std::cout << "Cannot open the ARM camera" << std::endl;
        return -1;
       }
         Mat matARM_small;

  // *********************** Streaming window setup and Gstreamer Pipeline *********************

       cv::Mat win_mat(window, CV_8UC3);

       cv::VideoWriter out("appsrc ! videoscale ! videoconvert ! video/x-raw,format=YUY2,width=1280,height=768, framerate=50/1 ! jpegenc quality=50 ! rtpjpegpay ! udpsink host=127.0.0.1 port=50215", fourcc, fps, cv::Size(width,height), true);

       if (out.isOpened()){
       	puts("Pipeline GST Opened");
       }

       else {
       	puts("Pipeline GST Broken");
       }
 while(true)
   {
       // ^^^^^^^ ARM camera acquisition ************************
       cap >> matARM; // get a new frame from camera
       cv::resize(matARM, matARM_small, smallimg, INTER_CUBIC );
       // ^^^^^^^^^^^ Point Grey acquisition *************
       Image raw;
       Error error = camera.RetrieveBuffer(&raw);
       if (error != PGRERROR_OK)
       {
         //std::cout << "network loss frame" << std::endl;
                     lost++;
         continue;
       }
       //printf("Frames lost: %d \n", lost);
       Image rgb;
       raw.Convert( FlyCapture2::PIXEL_FORMAT_BGR, &rgb );

       unsigned int row = (double)rgb.GetReceivedDataSize()/(double)rgb.GetRows();

       cv::Mat matPG = cv::Mat(rgb.GetRows(), rgb.GetCols(), CV_8UC3, rgb.GetData(),row);

       cv::resize(matPG, matPG_small, smallimg, INTER_CUBIC);

       // ^^^^^^^ ZED acquisition ******************************+

       if (zed.grab(runtime_parameters) == SUCCESS)
        {
                 // Retrieve the left image, depth image in resolution set
                 zed.retrieveImage(image_zed, VIEW_LEFT, MEM_CPU, new_width, new_height);
                 zed.retrieveImage(depth_image_zed, VIEW_DEPTH, MEM_CPU, new_width, new_height);
        }
        cv::resize(image_ocv, image_ocv_small, smallimg, INTER_CUBIC );
        cv::resize(depth_image_ocv, depth_image_ocv_small, smallimg, INTER_CUBIC );

        // *** Export Images to main window to stream
        Mat matbigg;
        cv::resize(matbigg, matbig, bigimg, INTER_CUBIC );

         matbig.copyTo(win_mat(cv::Rect(  0, 0, 960, 768)));
         matPG_small.copyTo(win_mat(cv::Rect(0,760,320,256)));
         matARM_small.copyTo(win_mat(cv::Rect(320,760,320,256)));
         image_ocv_small.copyTo(win_mat(cv::Rect(640,760,320,256)));
         depth_image_ocv_small.copyTo(win_mat(cv::Rect(960,760,320,256)));
         imshow( "T0-R0 Camera GUI", win_mat );
         out.write(win_mat);
         // **** CLOSING section ***********
          error = camera.StopCapture();
             if ( error != PGRERROR_OK )
             {
                 // This may fail when the camera was removed, so don't show
                 // an error message
             }
             camera.Disconnect();
             cap.release();
             zed.close;

             return 0;
         }
         cv::Mat slMat2cvMat(Mat& input) {
             // Mapping between MAT_TYPE and CV_TYPE
             int cv_type = -1;
             switch (input.getDataType()) {
                 case MAT_TYPE_32F_C1: cv_type = CV_32FC1; break;
                 case MAT_TYPE_32F_C2: cv_type = CV_32FC2; break;
                 case MAT_TYPE_32F_C3: cv_type = CV_32FC3; break;
                 case MAT_TYPE_32F_C4: cv_type = CV_32FC4; break;
                 case MAT_TYPE_8U_C1: cv_type = CV_8UC1; break;
                 case MAT_TYPE_8U_C2: cv_type = CV_8UC2; break;
                 case MAT_TYPE_8U_C3: cv_type = CV_8UC3; break;
                 case MAT_TYPE_8U_C4: cv_type = CV_8UC4; break;
                 default: break;
             }
             // Since cv::Mat data requires a uchar* pointer, we get the uchar1 pointer from sl::Mat (getPtr<T>())
             // cv::Mat and sl::Mat will share a single memory structure
             return cv::Mat(input.getHeight(), input.getWidth(), cv_type, input.getPtr<sl::uchar1>(MEM_CPU));
