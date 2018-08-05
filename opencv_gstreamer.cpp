#include "FlyCapture2.h"
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include <sl_zed/Camera.hpp>
#include <SaveDepth.hpp>
using namespace FlyCapture2;
using namespace sl;

cv::Mat slMat2cvMat(Mat& input);
int main()
{
 //  *******************  ZED section of setup code  ********************************+

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



//  *****************  Point Grey Section of setup code  **************************

    Error error;
    Camera camera;
    CameraInfo camInfo;

    // Connect the camera
    error = camera.Connect( 0 );
    if ( error != PGRERROR_OK )
    {
        std::cout << "Failed to connect to camera" << std::endl;
        return false;
    }

    // Get the camera info and print it out
    error = camera.GetCameraInfo( &camInfo );
    if ( error != PGRERROR_OK )
    {
        std::cout << "Failed to get camera info from camera" << std::endl;
        return false;
    }
    std::cout << camInfo.vendorName << " "
              << camInfo.modelName << " "
              << camInfo.serialNumber << std::endl;

    error = camera.StartCapture();
    if ( error == PGRERROR_ISOCH_BANDWIDTH_EXCEEDED )
    {
        std::cout << "Bandwidth exceeded" << std::endl;
        return false;
    }
    else if ( error != PGRERROR_OK )
    {
        std::cout << "Failed to start image capture" << std::endl;
        return false;
    }


// ****************************  ARM camera setup section *******************************

VideoCapture cap(0);

 // if not success, exit program
 if (cap.isOpened() == false)
 {
  cout << "Cannot open the video camera" << endl;
  return -1;
 }
   Mat matARM;

// *********************** Streaming window setup and Gstreamer Pipeline *********************
   Mat win_mat;
cv::Mat win_mat(cv::Size(2560, 1774), CV_8UC3);

cv::VideoWriter out("appsrc ! videoscale ! videoconvert ! video/x-raw,format=YUY2,width=1280,height=887, framerate=50/1 ! jpegenc quality=50! rtpjpegpay ! udpsink host=10.0.0.102 port=50205", 1800,0,50, cv::Size(2560,1774), true);

if (out.isOpened()){
	puts("Pipeline Opened");
}

else {
	puts("Pipeline Broken");
}
char key = 0;
int lost =0;

// ^^^^^^^^^^^^^ Acquisition Cycle **********************************
while(key != 'q'){

  // ^^^^^^^^^^^ Point Grey acquisition *************
	Image raw;
	Error error = camera.RetrieveBuffer(&raw);
	if (error != PGRERROR_OK){
		//std::cout << "network loss frame" << std::endl;
                lost++;
		continue;
	}
	//printf("Frames lost: %d \n", lost);
	Image rgb;
	raw.Convert( FlyCapture2::PIXEL_FORMAT_BGR, &rgb );

	unsigned int row = (double)rgb.GetReceivedDataSize()/(double)rgb.GetRows();

	cv::Mat matPG = cv::Mat(rgb.GetRows(), rgb.GetCols(), CV_8UC3, rgb.GetData(),row);

   // ^^^^^^^ ARM camera acquisition ************************
   cap >> matARM; // get a new frame from camera

   // ^^^^^^^ ZED acquisition ******************************+

   if (zed.grab(runtime_parameters) == SUCCESS) {

             // Retrieve the left image, depth image in resolution set
             zed.retrieveImage(image_zed, VIEW_LEFT, MEM_CPU, new_width, new_height);
             zed.retrieveImage(depth_image_zed, VIEW_DEPTH, MEM_CPU, new_width, new_height);

             // Display image and depth using cv:Mat which share sl:Mat data
           //  cv::imshow("Image", image_ocv);
           //  cv::imshow("Depth", depth_image_ocv);

             // Handle key event
             //key = cv::waitKey(10);
             //processKeyEvent(zed, key);
        }
   // *** Export Images to main window to stream
  matPG.copyTo(win_mat(cv::Rect(  0, 0, 1280, 1024)));
  matARM.copyTo(win_mat(cv::Rect(1280, 0, 1280, 1024)));
  image_zed.copyTo(win_mat(cv::Rect(  0, 1024, 1280, 720)));
  depth_image_zed.copyTo(win_mat(cv::Rect(1280, 1024, 1280, 720)));
	out.write(win_mat);
	key= cv::waitKey(30);
}

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
