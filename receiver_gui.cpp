
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>

using namespace cv;
using namespace std;

int main( int argc, char** argv )
{
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

        imshow("Receiver", frame);
        if(waitKey(1) == 'r')
            break;
    }
    destroyWindow("Receiver");
}
