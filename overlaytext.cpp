std::ostringstream str;
    str << "Here is some text:" << myVariable;
    cv::putText(image, cv::Point(10,10), str.str(), CV_FONT_HERSHEY_PLAIN, CV_RGB(0,0,250));
