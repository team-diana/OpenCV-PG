
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>

using namespace cv;
using namespace std;

// Mouse callback function
void CallBack(int event, int x, int y, int flags, void* userdata)
{
  if(event == EVENT_LBUTTONDOWN)
  {
    mousex = x;
    mousey = y;
    searching = true;
  }
  else if(event == EVENT_MBUTTONDOWN)
    searching = false;
}

// Trackbar callback function
void trackbarCallBack( int, void* )
{
  tgap = slider_val * 5;
}

int main( int argc, char** argv )
{

  namedWindow("Vision", WINDOW_AUTOSIZE);
  setMouseCallback("ArmVision", CallBack, NULL);
  createTrackbar( "Threshold", "ArmVision", &slider_val, 20, trackbarCallBack );

    // The sink caps for the 'rtpjpegdepay' need to match the src caps of the 'rtpjpegpay' of the sender pipeline
    // Added 'videoconvert' at the end to convert the images into proper format for appsink, without
    // 'videoconvert' the receiver will not read the frames, even though 'videoconvert' is not present
    // in the original working pipeline
    VideoCapture cap("udpsrc port=50205 ! application/x-rtp, media=video, clock-rate=90000, encoding-name=JPEG, payload=30 ! rtpjpegdepay ! jpegdec ! videoconvert ! appsink ",CAP_GSTREAMER);

    if(!cap.isOpened())
    {
        cout<<"VideoCapture not opened"<<endl;
        exit(-1);
    }

    Mat frame;

    while(true) {

        cap.read(frame);

        if(frame.empty())
            break;

        imshow("Vision", frame);
        if(waitKey(1) == 'r')
            break;
    }
    destroyWindow("Vision");
}
