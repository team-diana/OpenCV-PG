//#include <flycapture/FlyCapture2.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "opencv2/imgproc/imgproc.hpp"
#include <iostream>
//#include <sl_zed/Camera.hpp>
//#include <SaveDepth.hpp>
#define PORT 50210
//using namespace FlyCapture2;
//using namespace sl;
using namespace cv;



int main()
{

// ****************************  ARM camera setup section *******************************
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

// *********************** Streaming window setup and Gstreamer Pipeline *********************

cv::Mat win_mat(2600, 2600, CV_8UC3);
//puts("windows");

cv::VideoWriter out("appsrc ! videoscale ! videoconvert ! video/x-raw,format=YUY2,width=1280,height=768, framerate=50/1 ! jpegenc quality=50 ! rtpjpegpay ! udpsink host=127.0.0.1 port=50215", fourcc, fps, cv::Size(width,height), true);

if (out.isOpened()){
	puts("Pipeline GST Opened");
}

else {
	puts("Pipeline GST Broken");
}
char key = 0;
int lost =0;

//--------- TCP Server Section ****************************
bool searching=false;
int mousex, mousey, xtmp, ytmp, threshold;
int set=0;
bool overlays=false;
// TcpServerx::TcpServer(PORT);
// TcpServerx::start8();
// TcpServery::TcpServer(PORT+1);
// TcpServery::start8();
// TcpServertr::TcpServer(PORT+2);
// TcpServertr::start8();
// TcpServerinput::TcpServer(PORT+3);
// TcpServerinput::start8();
// TcpServersearching::TcpServer(PORT+4);
// TcpServersearching::start8();

// ^^^^^^^^^^^^^ Acquisition Cycle **********************************
while(key != 'q'){

  // ^^^^^^^^^^^ Point Grey acquisition *************
	Mat matPG = imread("PG.jpg", CV_LOAD_IMAGE_COLOR);
  Mat image_zed = imread("ZED1.jpg", CV_LOAD_IMAGE_COLOR);
  Mat depth_image_zed = imread("ZED2.jpg", CV_LOAD_IMAGE_COLOR);

   // ^^^^^^^ ARM camera acquisition ************************

   cap >> matARM; // get a new frame from camera

   //--------- Commands receive and process ----------------------

    // mousex=TcpServerx::readLast8();
    // mousey=TcpServery::readLast8();
    // threshold=TcpServertr::readLast8();
    // searching=TcpServersearching::readLast8();
    // set=TcpServerinput::readLast8();

   // //adapt mouse values
   // xtmp=(mousex*2)-1280;
   // ytmp=(mousey*2);
   // if(xtmp>0)
   // {
   //   mousex=xtmp;
   // }
   // if (ytmp<1024)
   // {
   //   mousey=ytmp;
   // }

//apply stefano class ArmVision and CacheFinder and select various options

    // switch (set)
    //   {
    //      case 1 :
    //      matPG=matPG; //standard image Pointgrey
    //      case 2 :
    //      matARM=matARM; //standard image ARMcamera
    //      case 3 :
    //      matPG=ARMVision(matPG, mousex, mousey, searching); //ArmVision Pointgrey
    //      case 4 :
    //      matARM=ARMVision(matARM, mousex, mousey, searching); //ArmVision ARMcamera
    //      case 5 :
    //      matPG=CacheFinder(matPG); //CacheFinder Pointgrey
    //      case 6 :
    //      matARM=CacheFinder(matARM); //CacheFinder ArmCamera
    //      case 7 :
    //      image_zed=image_zed; //Standard ZED
    //      case 8 :
    //      image_zed=CacheFinder(image_zed); //ZED CacheFinder
    //      case 9 :
    //      overlays=true;      //Enables overlays
    //
    //   }

   // *** Export Images to main window to stream
    matPG.copyTo(win_mat(cv::Rect(  0, 0, 1280, 1024)));
    matARM.copyTo(win_mat(cv::Rect(1300, 300, width, height)));
    image_zed.copyTo(win_mat(cv::Rect(  0, 1024, 1280, 720)));
    depth_image_zed.copyTo(win_mat(cv::Rect(1300, 1024, 1280, 720)));
    namedWindow( "Display window", WINDOW_AUTOSIZE );

    //opportunely resized
   Size size0(2600, 2080);
   cv::resize(win_mat, win_mat, size0);
   Size size1(1280, 1024);
   Mat out1;
    cv::resize(win_mat, out1, size1, INTER_CUBIC );
   
   imshow( "Display window", out1 );
  // imshow( "Display", matARM );
   out.write(out1);
	key= cv::waitKey(10);
puts("output");
}

// **** CLOSING section ***********


    return 0;
}
