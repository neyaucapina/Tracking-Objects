#include <sstream>
#include <string>
#include <iostream>
#include "opencv\highgui.h"
#include "opencv\cv.h"

using namespace cv;
using namespace std;

//initial min and max HSV filter values.
//these will be changed using trackbars
int H_MIN = 0;
int H_MAX = 256;
int S_MIN = 0;
int S_MAX = 256;
int V_MIN = 0;
int V_MAX = 256;
//default capture width and height
const int FRAME_WIDTH = 640;
const int FRAME_HEIGHT = 480;
//max number of objects to be detected in frame
const int MAX_NUM_OBJECTS = 50;
//minimum and maximum object area
const int MIN_OBJECT_AREA = 1000;
const int MAX_OBJECT_AREA = FRAME_HEIGHT*FRAME_WIDTH / 1.5;
//names that will appear at the top of each window
const string windowName = "Original Image";
const string windowName1 = "HSV Image";
const string windowName2 = "Thresholded Image";
const string windowName3 = "After Morphological Operations";
const string trackbarWindowName = "Trackbars";
//Pixeles por metrica
float pixelsPerMetric = 24.26;

bool calibrationMode;//used for showing debugging windows, trackbars etc.

bool mouseIsDragging;//used for showing a rectangle on screen as user clicks and drags mouse
bool mouseMove;
bool rectangleSelected;
cv::Point initialClickPoint, currentMousePoint; //keep track of initial point clicked and current position of mouse
cv::Rect rectangleROI; //this is the ROI that the user has selected
vector<int> H_ROI, S_ROI, V_ROI;// HSV values from the click/drag ROI region stored in separate vectors so that we can sort them easily

void on_trackbar(int, void*){}
void createTrackbars(){

    //create window for trackbars
    namedWindow(trackbarWindowName, 0);
    //create memory to store trackbar name on window
    char TrackbarName[50];
    sprintf(TrackbarName, "H_MIN", H_MIN);
    sprintf(TrackbarName, "H_MAX", H_MAX);
    sprintf(TrackbarName, "S_MIN", S_MIN);
    sprintf(TrackbarName, "S_MAX", S_MAX);
    sprintf(TrackbarName, "V_MIN", V_MIN);
    sprintf(TrackbarName, "V_MAX", V_MAX);
    //create trackbars and insert them into window
    createTrackbar("H_MIN", trackbarWindowName, &H_MIN, 255, on_trackbar);
    createTrackbar("H_MAX", trackbarWindowName, &H_MAX, 255, on_trackbar);
    createTrackbar("S_MIN", trackbarWindowName, &S_MIN, 255, on_trackbar);
    createTrackbar("S_MAX", trackbarWindowName, &S_MAX, 255, on_trackbar);
    createTrackbar("V_MIN", trackbarWindowName, &V_MIN, 255, on_trackbar);
    createTrackbar("V_MAX", trackbarWindowName, &V_MAX, 255, on_trackbar);
}
void clickAndDrag_Rectangle(int event, int x, int y, int flags, void* param){
    if (calibrationMode == true){
        Mat* videoFeed = (Mat*)param;
        if (event == CV_EVENT_LBUTTONDOWN && mouseIsDragging == false){
            initialClickPoint = cv::Point(x, y);
            mouseIsDragging = true;
        }
        /* user is dragging the mouse */
        if (event == CV_EVENT_MOUSEMOVE && mouseIsDragging == true){
            currentMousePoint = cv::Point(x, y);
            mouseMove = true;
        }
        /* user has released left button */
        if (event == CV_EVENT_LBUTTONUP && mouseIsDragging == true){
            rectangleROI = Rect(initialClickPoint, currentMousePoint);
            mouseIsDragging = false;
            mouseMove = false;
            rectangleSelected = true;
        }
        if (event == CV_EVENT_RBUTTONDOWN){
            H_MIN = 0;
            S_MIN = 0;
            V_MIN = 0;
            H_MAX = 255;
            S_MAX = 255;
            V_MAX = 255;
        }
        if (event == CV_EVENT_MBUTTONDOWN){
        }
    }
}
void recordHSV_Values(cv::Mat frame, cv::Mat hsv_frame){
    //save HSV values for ROI that user selected to a vector
    if (mouseMove == false && rectangleSelected == true){
        //clear previous vector values
        if (H_ROI.size()>0) H_ROI.clear();
        if (S_ROI.size()>0) S_ROI.clear();
        if (V_ROI.size()>0 )V_ROI.clear();
        //if the rectangle has no width or height (user has only dragged a line) then we don't try to iterate over the width or height
        if (rectangleROI.width<1 || rectangleROI.height<1) cout << "Please drag a rectangle, not a line" << endl;
        else{
            for (int i = rectangleROI.x; i<rectangleROI.x + rectangleROI.width; i++){
                //iterate through both x and y direction and save HSV values at each and every point
                for (int j = rectangleROI.y; j<rectangleROI.y + rectangleROI.height; j++){
                    //save HSV value at this point
                    H_ROI.push_back((int)hsv_frame.at<cv::Vec3b>(j, i)[0]);
                    S_ROI.push_back((int)hsv_frame.at<cv::Vec3b>(j, i)[1]);
                    V_ROI.push_back((int)hsv_frame.at<cv::Vec3b>(j, i)[2]);
                }
            }
        }
        //reset rectangleSelected so user can select another region if necessary
        rectangleSelected = false;
        //set min and max HSV values from min and max elements of each array

        if (H_ROI.size()>0){
            //NOTE: min_element and max_element return iterators so we must dereference them with "*"
            H_MIN = *std::min_element(H_ROI.begin(), H_ROI.end());
            H_MAX = *std::max_element(H_ROI.begin(), H_ROI.end());
            cout << "MIN 'H' VALUE: " << H_MIN << endl;
            cout << "MAX 'H' VALUE: " << H_MAX << endl;
        }
        if (S_ROI.size()>0){
            //S_MIN = *std::min_element(S_ROI.begin(), S_ROI.end());
            //S_MAX = *std::max_element(S_ROI.begin(), S_ROI.end());
            S_MIN = 0;
            S_MAX = 255;
            cout << "MIN 'S' VALUE: " << S_MIN << endl;
            cout << "MAX 'S' VALUE: " << S_MAX << endl;
        }
        if (V_ROI.size()>0){
            //V_MIN = *std::min_element(V_ROI.begin(), V_ROI.end());
            //V_MAX = *std::max_element(V_ROI.begin(), V_ROI.end());
            V_MIN = 0;
            V_MAX = 255;
            cout << "MIN 'V' VALUE: " << V_MIN << endl;
            cout << "MAX 'V' VALUE: " << V_MAX << endl;
        }
    }
    if (mouseMove == true){
        //if the mouse is held down, we will draw the click and dragged rectangle to the screen
        rectangle(frame, initialClickPoint, cv::Point(currentMousePoint.x, currentMousePoint.y), cv::Scalar(0, 255, 0), 1, 8, 0);
    }
}
string intToString(int number){
    std::stringstream ss;
    ss << number;
    return ss.str();
}
void drawObject(int x, int y, Mat &frame){
    circle(frame, Point(x, y), 8, Scalar(0, 255, 255), -1);
    if (y - 25>0)
        line(frame, Point(x, y), Point(x, y - 25), Scalar(0, 255, 0), 2);
    else line(frame, Point(x, y), Point(x, 0), Scalar(0, 255, 0), 2);
    if (y + 25<FRAME_HEIGHT)
        line(frame, Point(x, y), Point(x, y + 25), Scalar(0, 255, 0), 2);
    else line(frame, Point(x, y), Point(x, FRAME_HEIGHT), Scalar(0, 255, 0), 2);
    if (x - 25>0)
        line(frame, Point(x, y), Point(x - 25, y), Scalar(0, 255, 0), 2);
    else line(frame, Point(x, y), Point(0, y), Scalar(0, 255, 0), 2);
    if (x + 25<FRAME_WIDTH)
        line(frame, Point(x, y), Point(x + 25, y), Scalar(0, 255, 0), 2);
    else line(frame, Point(x, y), Point(FRAME_WIDTH, y), Scalar(0, 255, 0), 2);

}
void morphOps(Mat &thresh){
    Mat erodeElement = getStructuringElement(MORPH_RECT, Size(5, 5));
    Mat dilateElement = getStructuringElement(MORPH_RECT, Size(7, 7));
    morphologyEx( thresh, thresh, MORPH_OPEN, erodeElement,Point(-1,-1),2);
    morphologyEx( thresh, thresh, MORPH_CLOSE, erodeElement,Point(-1,-1),2);
    erode(thresh, thresh, erodeElement,Point(-1,-1),2);
    dilate(thresh, thresh, dilateElement,Point(-1,-1),2);
}

void trackFilteredObject(int &x, int &y, Mat threshold, Mat &cameraFeed){

    Mat temp;
    threshold.copyTo(temp);
    //these two vectors needed for output of findContours
    vector< vector<Point> > contours;
    vector<Vec4i> hierarchy;
    //find contours of filtered image using openCV findContours function
    findContours(temp, contours, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE);
    //use moments method to find our filtered object
    double refArea = 0;
    int largestIndex = 0;
    bool objectFound = false;
    if (hierarchy.size() > 0) {
        int numObjects = hierarchy.size();
        //if number of objects greater than MAX_NUM_OBJECTS we have a noisy filter
        if (numObjects<MAX_NUM_OBJECTS){
            for (int index = 0; index >= 0; index = hierarchy[index][0]) {
                Moments moment = moments((cv::Mat)contours[index]);
                double area = moment.m00;
                if (area>MIN_OBJECT_AREA && area<MAX_OBJECT_AREA && area>refArea){
                    x = moment.m10 / area;
                    y = moment.m01 / area;
                    objectFound = true;
                    refArea = area;
                    //save index of largest contour to use with drawContours
                    largestIndex = index;
                }
                else objectFound = false;
            }
            //let user know you found an object
            if (objectFound == true){
                putText(cameraFeed,"Objeto Encontrado", Point(0, 20), FONT_HERSHEY_COMPLEX_SMALL,1.1, Scalar(0,0,255),2, CV_AA);
                putText(cameraFeed,"x: "+to_string(x), Point(0, 40), FONT_HERSHEY_COMPLEX_SMALL,1, Scalar(0, 255, 0),1, CV_AA);
                putText(cameraFeed,"y: "+to_string(y), Point(0, 60), FONT_HERSHEY_COMPLEX_SMALL,1, Scalar(0, 255, 0),1, CV_AA);
                //draw object location on screen
                drawObject(x, y, cameraFeed);
                //draw largest contour
                //drawContours(cameraFeed, contours, largestIndex, Scalar(0, 255, 255), 2);
            }else
                putText(cameraFeed,"No hay objeto con ese color HSV", Point(0, 20), FONT_HERSHEY_COMPLEX_SMALL,1.1, Scalar(0, 0, 0),2, CV_AA);
        }
        else putText(cameraFeed, "TOO MUCH NOISE! ADJUST FILTER", Point(0, 50), FONT_HERSHEY_COMPLEX_SMALL,0.75, Scalar(0, 0, 0), 1, CV_AA);
    }
}
int main(int argc, char* argv[]){
    //program
    bool trackObjects = true;
    bool useMorphOps = true;
    calibrationMode = true;
    //Matrix to store each frame of the webcam feed
    Mat cameraFeed;
    //matrix storage for HSV image
    Mat HSV;
    //matrix storage for binary threshold image
    Mat threshold;
    //x and y values for the location of the object
    int x = 0, y = 0;
    //video capture object to acquire webcam feed
    VideoCapture capture;
    //open capture object at location zero (default location for webcam)
    capture.open(1);
    //set height and width of capture frame
    capture.set(CV_CAP_PROP_FRAME_WIDTH, FRAME_WIDTH);
    capture.set(CV_CAP_PROP_FRAME_HEIGHT, FRAME_HEIGHT);
    //must create a window before setting mouse callback
    cv::namedWindow(windowName);
    cv::setMouseCallback(windowName, clickAndDrag_Rectangle, &cameraFeed);
    //initiate mouse move and drag to false
    mouseIsDragging = false;
    mouseMove = false;
    rectangleSelected = false;
    while (1){
        //store image to matrix
        capture.read(cameraFeed);
        flip(cameraFeed,cameraFeed,1);
        //convert frame from BGR to HSV colorspace
        cvtColor(cameraFeed, HSV, COLOR_BGR2HSV);
        //set HSV values from user selected region
        recordHSV_Values(cameraFeed, HSV);
        //filter HSV image between values and store filtered image to
        //threshold matrix
        inRange(HSV, Scalar(H_MIN, S_MIN, V_MIN), Scalar(H_MAX, S_MAX, V_MAX), threshold);
        //perform morphological operations on thresholded image to eliminate noise
        //and emphasize the filtered object(s)
        if (useMorphOps)
            morphOps(threshold);
        //pass in thresholded frame to our object tracking function
        //this function will return the x and y coordinates of the
        //filtered object
        if (trackObjects)
            trackFilteredObject(x, y, threshold, cameraFeed);

        //show frames
        if (calibrationMode == true){
            //create slider bars for HSV filtering
            createTrackbars();
            imshow(windowName1, HSV);
            imshow(windowName2, threshold);
        }
        else{
            destroyWindow(windowName1);
            destroyWindow(windowName2);
            destroyWindow(trackbarWindowName);
        }
        imshow(windowName, cameraFeed);
        //delay 30ms so that screen can refresh.
        if (waitKey(30) == 99) calibrationMode = !calibrationMode;//if user presses 'c', toggle calibration mode
    }
    return 0;
}
