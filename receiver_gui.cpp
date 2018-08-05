
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#define PORT 50210
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

  int mousex, mousey;
  int slider_val;
  char input;
  bool searching = false;
//Open Client TcpClient
TcpClientx::TcpClient(IP_ROVER, PORT);
TcpClienty::TcpClient(IP_ROVER, PORT+1);
TcpClienttr::TcpClient(IP_ROVER, PORT+2);
TcpClientinput::TcpClient(IP_ROVER, PORT+3);
TcpClientsearching::TcpClient(IP_ROVER, PORT+4);
//Return true if the client is connected to the server, false if something went wrong
if(!TcpClientx::isConnected()){
  std::cout << "Connection Error Mouse x\n";
  return -1;
}
if(!TcpClienty::isConnected()){
  std::cout << "Connection Error Mouse y\n";
  return -1;
}
if(!TcpClienttr::isConnected()){
  std::cout << "Connection Error threshold\n";
  return -1;
}
if(!TcpClientinput::isConnected()){
  std::cout << "Connection Error keyboard input\n";
  return -1;
}
if(!TcpClientsearching::isConnected()){
  std::cout << "Connection Error keyboard input\n";
  return -1;
}
// Create Windows and mouse input
  namedWindow("Vision", WINDOW_AUTOSIZE);
  setMouseCallback("Vision", CallBack, NULL);
  createTrackbar( "Threshold", "Vision", &slider_val, 20, trackbarCallBack );


    // Receive from Gstreamer -----------------------------------

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
      TcpClientx::send8(mousex);
      TcpClienty::send8(mousey);
      TcpClienttr::send8(slider_val);
      cin>>input;
      if (isdigit(input))
      {
          TcpClientinput::send8(input);
      }
      TcpClientsearching::send8(searching);

    }

    destroyWindow("Vision");
}
