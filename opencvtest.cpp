// CameraTest.cpp : Defines entry point for console application.
 
#include "stdafx.h"
#include "FlyCapture2.h"
#include <cv.h>
#include <highgui.h>  
 
// Disable  C4996 warnings
#pragma warning(disable: 4996)
#define SOFTWARE_TRIGGER_CAMERA
 
using namespace FlyCapture2;
 
const int col_size   = 24;
const int row_size   = 480;
const int data_size  = row_size * col_size;
 
// Forward declarations
bool CheckSoftwareTriggerPresence( Camera* pCam );
bool PollForTriggerReady( Camera* pCam );
bool FireSoftwareTrigger( Camera* pCam );
void PrintError( Error error );
void ReleaseImage( IplImage* pimg,
               IplImage* pimg_bw,
               CvMemStorage* pstorage );
 
void GrabImages( Camera* pcam,
                 IplImage* pimg,
                 IplImage* pimg_bw,
                 CvMemStorage* pstorage,
                 int cnt );
 
// Main Control Loop
int _tmain(int argc, _TCHAR* argv[])
{
    Camera cam;
    CameraInfo camInfo;
    Error error;
    BusManager busMgr;
    PGRGuid guid;
    Format7PacketInfo fmt7PacketInfo;
    Format7ImageSettings fmt7ImageSettings;
    CvMemStorage* storage = NULL;
    IplImage* img    = NULL;
    IplImage* img_bw = NULL;
    TriggerMode triggerMode;
 
    // Create OpenCV structs for grayscale image
    img = cvCreateImage( cvSize( col_size, row_size ),
             IPL_DEPTH_8U,
             1 );
    img_bw = cvCloneImage( img );   
 
    storage = cvCreateMemStorage( 0 );
 
    // Get Flea2 camera
    error = busMgr.GetCameraFromIndex( 0, &guid );  
 
    if ( error != PGRERROR_OK )
    {
    PrintError( error );
        return -1;
    }   
 
    // Connect to the camera
    error = cam.Connect( &guid );
    if ( error != PGRERROR_OK )
    {
        PrintError( error );
        return -1;
    }   
 
    // Get camera information
    error = cam.GetCameraInfo(&camInfo);
    if ( error != PGRERROR_OK )
    {
        PrintError( error );
        return -1;
    }
 
#ifndef SOFTWARE_TRIGGER_CAMERA
    // Check for external trigger support
    TriggerModeInfo triggerModeInfo;
    error = cam.GetTriggerModeInfo( &triggerModeInfo ;
 
    if ( error != PGRERROR_OK )
    {
        PrintError( error );
    return -1;
    }
 
    if ( triggerModeInfo.present != true )
    {
        printf( "Camera doesn't support external trigger!\n" );
    return -1;
    }
 
#endif
    // Get current trigger settings
    error = cam.GetTriggerMode( &triggerMode );
    if ( error != PGRERROR_OK )
    {
        PrintError( error );
        return -1;
    }
 
    // Set camera to trigger mode 0
    triggerMode.onOff = true;
    triggerMode.mode = 0;
    triggerMode.parameter = 0;
#ifdef SOFTWARE_TRIGGER_CAMERA
 
    // A source of 7 means software trigger
    triggerMode.source = 7;
#else
 
    // Triggering the camera externally using source 0.
    triggerMode.source = 0;
#endif
 
    // Set camera triggering mode
    error = cam.SetTriggerMode( &triggerMode );
    if ( error != PGRERROR_OK )
    {
        PrintError( error );
        return -1;
    }
 
    // Power on the camera
    const unsigned int k_cameraPower = 0x610;
    const unsigned int k_powerVal = 0x80000000;
    error  = cam.WriteRegister( k_cameraPower, k_powerVal );
 
    if ( error != PGRERROR_OK )
    {
        PrintError( error );
        return -1;
    }
 
    // Poll to ensure camera is ready
    bool retVal = PollForTriggerReady( &cam );
    if( !retVal )
    {
        PrintError( error );
        return -1;
    }   
 
    // Set camera configuration: region of 24 x 480 pixels
    // greyscale image mode
    fmt7ImageSettings.width   = col_size;
    fmt7ImageSettings.height  = row_size;
    fmt7ImageSettings.mode    = MODE_0;
    fmt7ImageSettings.offsetX = 312;
    fmt7ImageSettings.offsetY = 0;
    fmt7ImageSettings.pixelFormat = PIXEL_FORMAT_MONO8;
 
    // Validate Format 7 settings
    bool valid;
    error = cam.ValidateFormat7Settings( &fmt7ImageSettings,
                                 &valid,
                     &fmt7PacketInfo );
    unsigned int num_bytes =
        fmt7PacketInfo.recommendedBytesPerPacket;   
 
    // Set Format 7 (partial image mode) settings
    error = cam.SetFormat7Configuration( &fmt7ImageSettings,
                                         num_bytes );
    if ( error != PGRERROR_OK)
    {
        PrintError( error );
        return -1;
    }
 
    // Start capturing images
    error = cam.StartCapture();
    if ( error != PGRERROR_OK )
    {
        PrintError( error );
        return -1;
    }
 
    #ifdef SOFTWARE_TRIGGER_CAMERA
 
    if ( !CheckSoftwareTriggerPresence( &cam ) )
    {
        printf( "SOFT_ASYNC_TRIGGER not implemented on "
                "this camera! Stopping application\n" );
        return -1;
    }
 
    #else
    printf( "Trigger the camera by sending a trigger pulse"
            " to GPIO%d.\n",
            triggerMode.source );
    #endif
 
    error = cam.StartCapture();
 
    // Warm up - necessary to get decent images.
    // See Flea2 Technical Ref.: camera will typically not
    // send first 2 images acquired after power-up
    // It may therefore take several (n) images to get
    // satisfactory image, where n is undefined
    for ( int i = 0; i < 30; i++ )
    {
        // Check that the trigger is ready
        PollForTriggerReady( &cam );
 
        // Fire software trigger
        FireSoftwareTrigger( &cam );
 
        Image im;
 
        // Retrieve image before starting main loop
        Error error = cam.RetrieveBuffer( &im );
        if ( error != PGRERROR_OK )
        {
            PrintError( error );
            return -1;
        }
    }
 
    #ifdef SOFTWARE_TRIGGER_CAMERA
 
    if ( !CheckSoftwareTriggerPresence( &cam ) )
    {
        printf( "SOFT_ASYNC_TRIGGER not implemented on this"
                " camera! Stopping application\n");
        return -1;
    }
    #else
 
    printf( "Trigger camera by sending trigger pulse to"
            " GPIO%d.\n",
            triggerMode.source );
    #endif
 
    // Grab images acc. to number of hw/sw trigger events
    for ( int i = 0; i < 25; i++ )
    {
        GrabImages( &cam, img, img_bw, storage, i );
    } 
 
    // Turn trigger mode off.
    triggerMode.onOff = false;
    error = cam.SetTriggerMode( &triggerMode );
    if ( error != PGRERROR_OK )
    {
        PrintError( error );
        return -1;
    } 
 
    printf( "\nFinished grabbing images\n" ); 
 
    // Stop capturing images error = cam.StopCapture();
    if ( error != PGRERROR_OK )
    {
        PrintError( error );
        return -1;
    } 
 
    // Disconnect the camera
    error = cam.Disconnect();
    if ( error != PGRERROR_OK )
    {
        PrintError( error );
        return -1;
    } 
 
    ReleaseImage( img, img_bw, storage);
    return 0;
} 
 
// Print error trace
void PrintError( Error error )
{
    error.PrintErrorTrace();
} 
 
// Check for the presence of software trigger
bool CheckSoftwareTriggerPresence( Camera* pCam )
{
    const unsigned int k_triggerInq = 0x530;
    Error error;
    unsigned int regVal = 0;
    error = pCam->ReadRegister( k_triggerInq, &regVal );
    if ( error != PGRERROR_OK )
    {
        // TODO
    }
 
    if( ( regVal & 0x10000 ) != 0x10000 )
    {
        return false;
    }
 
    return true;
}
 
// Start polling for trigger ready
bool PollForTriggerReady( Camera* pCam )
{
    const unsigned int k_softwareTrigger = 0x62C;
    Error error;
    unsigned int regVal = 0;
 
    do
    {
        error = pCam->ReadRegister( k_softwareTrigger,
                                       regVal );
 
        if ( error != PGRERROR_OK )
        {
            // TODO
        }
    } while ( (regVal >> 31) != 0 );
 
    return true;
}
 
// Launch the software trigger event
bool FireSoftwareTrigger( Camera* pCam )
{
    const unsigned int k_softwareTrigger = 0x62C;
    const unsigned int k_fireVal = 0x80000000;
    Error error;
 
    error = pCam->WriteRegister( k_softwareTrigger,
                                    k_fireVal );
 
    if ( error != PGRERROR_OK )
    {
        // TODO
    }
 
    return true;
}
 
// Tidy up memory allocated for images etc
void ReleaseImage( IplImage* pimg,
                   IplImage* pimg_bw,
                   CvMemStorage* pstorage )
{
    cvReleaseImage( &pimg );
    cvReleaseImage( &pimg_bw );
    cvClearMemStorage( pstorage );
}
 
// Grab camera grayscale image and convert into
// an OpenCV image
void GrabImages( Camera* pcam,
                 IplImage* pimg,
                 IplImage* pimg_bw,
                 CvMemStorage* pstorage,
                 int cnt )
{
    Image image;
 
    #ifdef SOFTWARE_TRIGGER_CAMERA
 
    // Check that the trigger is ready
    PollForTriggerReady( pcam );
 
    printf( "Press Enter to initiate software trigger.\n" );
 
    getchar();
 
    // Fire software trigger
    bool retVal = FireSoftwareTrigger( pcam );
 
    if ( !retVal )
    {
        // TODO.
    }
 
    #endif
 
    // Retrieve image before starting main loop
    Error error = pcam->RetrieveBuffer( &image );
 
    if ( error != PGRERROR_OK )
    {
        // TODO.
    }
 
    // Copy FlyCapture2 image into OpenCV struct
    memcpy( pimg->imageData,
    image.GetData(),
    data_size );
 
    // Save the bitmap to file
    cvSaveImage( "orig.bmp", pimg );
 
    // Threshold to convert image into binary (B&W)
    cvThreshold( pimg,    // source image
                 pimg_bw, // destination image
                 145,     // threhold val.
                 255,     // max. val
                 CV_THRESH_BINARY ); // binary type );
 
    // Save the bitmap to file
    char buffer[ 20 ];
    sprintf( buffer, "%d", cnt );
    std::string mess = "B_W.bmp_";
    mess.append( buffer );
    mess.append( ".bmp" );
    cvSaveImage( mess.c_str(), pimg_bw );
 
    // Find connected components using OpenCV
    // for black spots against a white background
    CvSeq* seq;
    int num_blobs = cvFindContours( pimg_bw,
                                    pstorage,
                                    &seq,
                                    sizeof( CvContour ),
                                    CV_RETR_LIST,
                                    CV_CHAIN_APPROX_NONE,
                                    cvPoint( 0, 0 ) ) - 1;
}


