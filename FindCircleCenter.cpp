#pragma comment(lib, "Ws2_32.lib")  //connect this lib

//C++ Basic Lib
#include<iostream>
#include<math.h>
#include <string>
#include <fstream>
#include <sstream>
#include <time.h>
//Socket Lib
#include <Ws2tcpip.h>
#include <windows.h>
#include <WinSock2.h>
//OpenCV Lib
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/dnn.hpp>
#include <opencv2/core/utility.hpp>
#include <opencv2/tracking.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/core/types.hpp>



using namespace std;
using namespace cv;


int main() {
	//************************************//
	//          OpenCV setting            //
	//************************************//	


	ofstream write;
	
	write.open("Communication.csv");
	write << 0 << ",";    //X
	write << 0 << ",";   //Y
	write << 0 << endl;     //Speed
	write.close();

	clock_t t1, t2=0;

	float distance, time_interval, speed;

	float controller_x, controller_y, controller_speed;

	bool tracking = 0, get_center = 0;

	int width_S, height_S,
		width_M = 0, height_M = 0,
		width, height;

	int temp_width_S = 0, temp_height_S = 0, temp_height_M = 0;
	int last_Diameter = 0, last_height_M = 0;
	int last_judge_left = 0, last_judge_right = 0;

	int count_y = 0;
	int judge_left = 0, judge_right = 0;
	int change1		 ,change2;
	



	//setting camera && viedo frame
	VideoCapture video(0);
	Mat frame, Gray_frame, BW_frame, roi_frame;

	//fail to open camera
	if (!video.isOpened()) {
		cout << "\nfail to read camera\n";
		return 1;
	}

	video >> frame;

	if (!video.read(frame)) {
		cout << "\nfail to read camera\n";
	}


	//       input, output    ,Conversion method
	cvtColor(frame, Gray_frame, COLOR_BGR2GRAY);//   transfer to gray
	//        
	//adaptiveThreshold(Gray_frame,BW_frame,255, ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY,201,0);
	threshold(Gray_frame, BW_frame, 130, 255, THRESH_BINARY);

	//use double to store rectangle template
	Rect2d roi, roi_init;

	roi = selectROI("target", BW_frame);
	//quit if ROI was not selected
	if (roi.width == 0 || roi.height == 0)
		return 0;

	destroyWindow("target");

	// create a tracker object KCF ,MIL, TLD
	Ptr<TrackerTLD> tracker = TrackerTLD::create();


	// initialize the tracker
	tracker->init(Gray_frame, roi);
	// perform the tracking process
	cout << "Start the tracking process, press ESC to quit.\n";


	//************************************//
	//          While(1)                  //
	//************************************//	

	while (1) {

		video >> frame;

		cvtColor(frame, Gray_frame, COLOR_BGR2GRAY);//transfer to gray
		threshold(Gray_frame, BW_frame, 130, 255, THRESH_BINARY);
		

		if (!video.read(Gray_frame)) {
			cout << "\nfail to read camera\n";
		}

		// update tracker to find target   tracking: it is caught or not
		tracking = tracker->update(frame, roi);

		// draw the tracked object
		//		  input,range, color of rim          ,Thickness of rim , type of line
		rectangle(frame, roi, Scalar(255, 0, 0), 2       , 1);

		//target position(now)
		width_S = roi.x;
		width = roi.width;
		height_S = roi.y;
		height = roi.height;

	    //it is in workspace
		if (height_S < 210 && width_S>226 && width_S < 396) {
			
			//center of target
			for (int y = height_S ; y <= height_S + height; y++) {
				for (int x = width_S; x <=width_S + width + 1; x++) {
					if (y < 0||x<0)
						break;
					change1 = int(BW_frame.at<uchar>(y, x));
					change2 = int(BW_frame.at<uchar>(y, x + 1));
					
					//if(change1 == 0|| change2 ==0)
					//cout << "\nchange1 " << change1 << "\tchange2 " << change2;
					
					if (change1 - change2 < 0) {
						judge_left = x;
					}
					if (change1 - change2 > 0) {
						judge_right = x;
						
						break;
					}
				}

				if (last_Diameter == 0)//init
					last_Diameter = judge_right - judge_left;

				if ((last_Diameter - (judge_right - judge_left)) > 0) {
					width_M = (last_judge_left + last_judge_right) / 2;
					height_M = y - 1;
					get_center = 1;
					/*
					cout << "last_Diameter " << last_Diameter
						<< " last_judge_left " << last_judge_left
						<< " last_judge_right " << last_judge_right << endl;
					cout << "width_M :" << width_M
						<< "height_M :" << height_M << endl << endl;
					*/

					break;
				}
				else {
					last_Diameter = judge_right - judge_left;
					last_judge_left = judge_left;
					last_judge_right = judge_right;
				}
			}

			//when it move, give data to PY
			if (abs(temp_height_M - height_M) >100 && get_center&&  height_M>0) {

				t1 = time(0);// Timing begins

				distance = temp_height_M - height_M;
				time_interval = t1-t2 ;   //ms -> s
				speed = distance/time_interval ;   // pixel/s		



				cout << "\ntemp_height_M :" << temp_height_M
					 << "height_M :" << height_M << endl;


				if (t2 != 0 && distance > 0) { 

					controller_y = (width_M - 226) * 23.5294;
					controller_x = (height_M) * 22.2929 + 9000;
					controller_speed = speed * 22.2929;

					controller_y = int(controller_y);
					controller_x = int(controller_x);
					controller_speed = int(controller_speed);

					write.open("Communication.csv");
					write << controller_x << ",";    //X
					write << controller_y << ",";    //Y
					write << controller_speed << endl;          //Speed
					write.close();

					cout << "\n------------------------------------"
						<< "\nwidth_M :" << width_M
						<< "\nheight_M :" << height_M;
					cout << "\ndistance :" << distance
					//	<<"\nt1 "<<t1<<"\t t2 "<<t2
						<< "\ntime_interval :" << time_interval
						<< "\nspeed :" << speed
						<< "\nwidth_S :" << width_S
						<< "\nwidth :" << width
						<< "\nheight_S :" << height_S
						<< "\nheight :" << height << endl;

					cout << "\ncontroller_x :" << controller_x
						<< "\ncontroller_y :" << controller_y
						<< "\ncontroller_speed :" << controller_speed
						<< "\n------------------------------------" ;
				}

				temp_height_M = height_M;
				t2 = t1;

			}
		}


		cv::imshow("tracking ", frame);
		cv::imshow("BW_frame", BW_frame);
		//quit on ESC button
		if (waitKey(1) == 27) {
			break;
		}


	}

	system("PAUSE");
	return 0;
}