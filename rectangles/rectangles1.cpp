
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "opencv2/imgproc/imgproc.hpp"
#include <iostream>
#include <time.h>

#define PORT 50210
//using namespace FlyCapture2;
//using namespace sl;
using namespace cv;

int main()
{
 // TIME -------------
  time_t _tm =time(NULL );


 //  *******************  SIZES  ********************************+
 Size bigimg(960,768);
 Size smallimg(320,256);
 Size window(1280,1024);
int set = 1;
char sett;
std::cin >> sett ;
set = sett -'0';
 namedWindow( "T0-R0 Camera GUI", WINDOW_AUTOSIZE );

  Mat image_ocv_small;
  Mat depth_ocv_small;
  Mat image_ocv;
  Mat depth_ocv;
  Mat matbig;
  Mat logo, logoo;
  cv::Mat win_mat(window, CV_8UC3);
  Mat matPG = imread("PG.jpg", CV_LOAD_IMAGE_COLOR);
  image_ocv = imread("ZED1.jpg", CV_LOAD_IMAGE_COLOR);
  depth_ocv = imread("ZED1.jpg", CV_LOAD_IMAGE_COLOR);
  logo = imread ("logo.jpg", CV_LOAD_IMAGE_COLOR);
  cv::resize(logo, logoo, smallimg, INTER_CUBIC );
  cv::resize(image_ocv, image_ocv_small, smallimg, INTER_CUBIC );
  cv::resize(depth_ocv, depth_ocv_small, smallimg, INTER_CUBIC );

      Mat matPG_small;
      // ****************************  ARM camera setup section *******************************

      VideoCapture cap(0);


      const double fps = cap.get(CAP_PROP_FPS);
      const int width  = cap.get(CAP_PROP_FRAME_WIDTH);
      const int height = cap.get(CAP_PROP_FRAME_HEIGHT);
      const int fourcc = cap.get(CAP_PROP_FOURCC );

      cv::Mat matARM(width, height, CV_8UC3);


       //if not success, exit program
       if (cap.isOpened() == false)
       {
        std::cout << "Cannot open the ARM camera" << std::endl;
        return -1;
       }
         Mat matARM_small;

  // *********************** Streaming window setup and Gstreamer Pipeline *********************



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
       cv::resize(matARM, matbig, bigimg, INTER_CUBIC );
       // ^^^^^^^^^^^ Point Grey acquisition *************


       cv::resize(matPG, matPG_small, smallimg, INTER_CUBIC);



        // *** Export Images to main window to stream
        Mat matbiGa;

        if (set==1)
        matbiGa=matPG;
        else if (set==2)
        matbiGa=matARM;
        else if (set==3)
        matbiGa=image_ocv;
        else if (set==4)
        matbiGa=depth_ocv;

        cv::resize(matbiGa , matbig, bigimg, INTER_CUBIC );
         matbig.copyTo(win_mat(cv::Rect(  0, 0, 960, 768)));
         matPG_small.copyTo(win_mat(cv::Rect(0,760,320,256)));
         matARM_small.copyTo(win_mat(cv::Rect(320,760,320,256)));
         image_ocv_small.copyTo(win_mat(cv::Rect(640,760,320,256)));
         depth_ocv_small.copyTo(win_mat(cv::Rect(960,760,320,256)));
         logoo.copyTo(win_mat(cv::Rect(960,0,320,256)));
         struct tm * curtime = localtime ( &_tm );

         String str1 = asctime(curtime);
         String str = str1 + " FPS: "+ std::to_string(int(fps)) ;


         imshow( "T0-R0 Camera GUI", win_mat );
        int  key= cv::waitKey(10);
        cv::putText(win_mat, str, Point(970,300), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(255,255,255), 1);

         //out.write(win_mat);
         // **** CLOSING section ***********



        }
        cap.release();

        return 0;
      }
