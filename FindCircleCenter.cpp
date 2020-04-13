#pragma comment(lib, "Ws2_32.lib")  //連結此lib

//C++基本程式庫
#include<iostream>
#include<math.h>
#include <string>
#include <fstream>
#include <sstream>
#include <time.h>
//Socket程式庫
#include <Ws2tcpip.h>
#include <windows.h>
#include <WinSock2.h>
//OpenCV相關程式庫
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
	//            OpenCV設定              //
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
	



	//設定相機 && 存影像的frame
	VideoCapture video(0);
	Mat frame, Gray_frame, BW_frame, roi_frame;

	//未成功開啟相機
	if (!video.isOpened()) {
		cout << "\n開啟相機失敗\n";
		return 1;
	}

	video >> frame;

	if (!video.read(frame)) {
		cout << "\n擷取畫面失敗\n";
	}


	//       input,output, 轉換方式
	cvtColor(frame, Gray_frame, COLOR_BGR2GRAY);//將frame之圖像轉為灰階
	//        
	//adaptiveThreshold(Gray_frame,BW_frame,255, ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY,201,0);
	threshold(Gray_frame, BW_frame, 130, 255, THRESH_BINARY);

	//以double形式用2D矩形template
	Rect2d roi, roi_init;

	roi = selectROI("想追蹤之物品", BW_frame);
	//quit if ROI was not selected
	if (roi.width == 0 || roi.height == 0)
		return 0;

	destroyWindow("想追蹤之物品");

	// create a tracker object KCF ,MIL, TLD(lag、大小不定，但可解決problem)
	Ptr<TrackerTLD> tracker = TrackerTLD::create();


	// initialize the tracker
	tracker->init(Gray_frame, roi);
	// perform the tracking process
	cout << "Start the tracking process, press ESC to quit.\n";


	//************************************//
	//          While(1)迴圈              //
	//************************************//	

	while (1) {

		video >> frame;

		cvtColor(frame, Gray_frame, COLOR_BGR2GRAY);//將frame之圖像轉為灰階
		threshold(Gray_frame, BW_frame, 130, 255, THRESH_BINARY);
		

		if (!video.read(Gray_frame)) {
			cout << "\n擷取畫面失敗\n";
		}

		// 更新tracker 找到目標新位置的邊界   tracking:判斷是否抓到
		tracking = tracker->update(frame, roi);

		// draw the tracked object
		//		  input     ,範圍, 邊框顏色         ,邊框粗度 ,線型
		rectangle(frame, roi, Scalar(255, 0, 0), 2       , 1);

		//目前追蹤物位置
		width_S = roi.x;
		width = roi.width;
		height_S = roi.y;
		height = roi.height;

	    //在履帶中時
		if (height_S < 210 && width_S>226 && width_S < 396) {
			
			//目標物中心
			for (int y = height_S ; y <= height_S + height; y++) {//每次讀一列
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

				if (last_Diameter == 0)//給初始值
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

			//物體在履帶中移動時，輸出座標給PY
			if (abs(temp_height_M - height_M) >100 && get_center&&  height_M>0) {

				t1 = time(0);//計時開始

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


		cv::imshow("追蹤中", frame);
		cv::imshow("BW_frame", BW_frame);
		//quit on ESC button
		if (waitKey(1) == 27) {
			break;
		}


	}

	system("PAUSE");
	return 0;
}