// p/****************************************************************************\

#include <royale.hpp>

#include <iostream>

#include <mutex>

#include <opencv2/opencv.hpp>

#include <stdlib.h>

#include <windows.h>



using namespace std;

using namespace cv;



//global

struct kombination {

	double y_koordinate;

	double x_koordinate;

	int label;

	int height;

	int width;

	int area;

	int left;

	int top;

	Vec3b color;

	char letter;

	kombination(double y_koor, double x_koor, int lab, int hei, int wi, int ar, int le, int to) {

		y_koordinate = y_koor;

		x_koordinate = x_koor;

		label = lab;

		height = hei;

		width = wi;

		area = ar;

		left = le;

		top = to;



	}



};



bool compare_y_koordinate(kombination kombi1, kombination kombi2) {

	return (kombi1.y_koordinate < kombi2.y_koordinate);

}



bool compare_x_koordinate(kombination kombi1, kombination kombi2) {

	return (kombi1.x_koordinate < kombi2.x_koordinate);

}





class MyListener : public royale::IDepthDataListener

{



public:

	int case2 = 0;

	void onNewData(const royale::DepthData* data)

	{

		// this callback function will be called for every new depth frame



		std::lock_guard<std::mutex> lock(flagMutex);

		zImage.create(cv::Size(data->width, data->height), CV_32FC1);

		grayImage.create(cv::Size(data->width, data->height), CV_32FC1);

		zImage = 0;

		grayImage = 0;

		int k = 0;

		for (int y = 0; y < zImage.rows; y++)

		{

			for (int x = 0; x < zImage.cols; x++)

			{

				auto curPoint = data->points.at(k);

				if (curPoint.depthConfidence > 0)

				{

					// if the point is valid

					zImage.at<float>(y, x) = curPoint.z;

					grayImage.at<float>(y, x) = curPoint.grayValue;

				}

				k++;

			}

		}



		cv::Mat temp = zImage.clone();

		undistort(temp, zImage, cameraMatrix, distortionCoefficients);

		temp = grayImage.clone();

		undistort(temp, grayImage, cameraMatrix, distortionCoefficients);







		Mat dest(grayImage.size(), CV_8UC3); // Im Prak das Farbbild mit den Tasten



		if (case2 == 1) {

			Mat tempgray = grayImage.clone();


			blur(grayImage, tempgray, Size(3, 3));


			medianBlur(grayImage, grayImage, 23);

			grayImage = grayImage - tempgray;



			threshold(grayImage, grayImage, 10, 255, THRESH_BINARY_INV);



			Vec3b weiss(255, 255, 255); // a



			if (zImage2.data != NULL) {

				Mat tempzImage;
				
				Mat mask = zImage.clone();

				rectangle(mask, Rect(x_koor, y_koor, width, height), weiss, -1, 8, 0);


				subtract(zImage2, zImage, tempzImage, mask);
				 

				
				

				cout << "zImage.type: " << zImage.type() << endl;
				cout <<"zImage2.type: "<<zImage2.type() << endl;

				double zmin1, zmin2, zmax1, zmax2,difmin,difmax;
				minMaxLoc(zImage, &zmin1, &zmax1);
				minMaxLoc(zImage2, &zmin2, &zmax2);


				cout << "zImage minval:" << zmin1 << "zImage maxval:" << zmax1 << endl;
				cout << "zImage2 minval:" << zmin2 << "zImage2 maxval:" << zmax2 << endl;


				
				//tempzImage = zImage2 - zImage;
				
				

				minMaxLoc(tempzImage, &difmin, &difmax);

				std::cout << "DIFMinval= " << difmin << std::endl;
				std::cout << "DIFMaxval= " << difmax << std::endl;

				cout << "tempzImage.channels: " << tempzImage.channels() << endl;

				imshow("DIF", tempzImage);

				


				tempzImage.convertTo(tempzImage, CV_8UC1);
				cout << "LOL" << endl;

				double schwelle = 0.1; // Die Schwelle muss herausgefunden werden 

				Mat bild1 = Mat::zeros(Size(tempzImage.cols, tempzImage.rows), CV_8UC1);
				Mat bild2 = Mat::zeros(Size(tempzImage.cols, tempzImage.rows), CV_8UC1);

				cout << "bild1 data: " << "c: " << bild1.channels() << "t: " << bild1.type() << "cols: " << bild1.cols << "rows: " << bild1.rows << endl;
				cout << "bild2 data: " << "c: " << bild2.channels() << "t: " << bild2.type() << "cols: " << bild2.cols << "rows: " << bild2.rows << endl;
				cout << "tempZ data: " << "c: " << tempzImage.channels() << "t: " << tempzImage.type() << "cols: " << tempzImage.cols << "rows: " << tempzImage.rows << endl;

				
				cout << "tempZ rows: " << tempzImage.rows << endl;
				cout << "tempZ cols: " << tempzImage.cols << endl;

				for (int x = 0; x < tempzImage.rows; x++) {

				for (int y = 0; y < tempzImage.cols; y++) {

					if (tempzImage.at<uchar>(x, y) > schwelle){

						bild1.at<uchar>(x, y) = 255.0;
					
					}

					if (tempzImage.at<uchar>(x, y) < schwelle){

						bild2.at<uchar>(x, y) = 255.0;
					
					}

								}

						}

				
				//Opening auf die Binärbilder

				Mat element = getStructuringElement(MORPH_CROSS, Size(3, 3));

				morphologyEx(bild1, bild1, MORPH_OPEN, element);

				morphologyEx(bild2, bild2, MORPH_OPEN, element);

				imshow("bild1", bild1);
				imshow("bild2", bild2);


				//Einfärben der Pixel im Ergebnisbild

				/*vector<Mat>channels;
				Mat ergebnis;

				Mat green = Mat::zeros(Size(tempzImage.rows, tempzImage.cols), CV_8UC1); // Ist CV_8UC1 richtig? Auch bei binary Pictures?

				channels.push_back(bild2); // Blau

				channels.push_back(green); //Grün

				channels.push_back(bild1); // Rot


				merge(channels, ergebnis);

				imshow("Ergebnis", ergebnis);
				*/
			
			}
	
			zImage2 = zImage.clone();




			//convert

			grayImage.convertTo(grayImage, CV_8U);



			Mat labelImage(grayImage.size(), CV_32S);

			Mat stats, centroids;



			vector<kombination> labels;



			int nLabels = connectedComponentsWithStats(grayImage, labelImage, stats, centroids, 8, CV_32S);

			//cout << "Found " << nLabels << "Labels." << endl;





			vector<Vec3b>colors;





			Vec3b rot(255, 0, 0); // a

			Vec3b gelb(0, 255, 0); // b

			Vec3b blau(0, 0, 255); // c

			Vec3b gruen(0, 255, 127); // d

			Vec3b violett(138, 43, 226); //e

			Vec3b orange(255, 165, 0); //f

			Vec3b braun(139, 69, 19); //g

			Vec3b pink(255, 20, 147); //h





			colors.push_back(rot);

			colors.push_back(gelb);

			colors.push_back(blau);

			colors.push_back(gruen);

			colors.push_back(violett);

			colors.push_back(orange);

			colors.push_back(braun);

			colors.push_back(pink);







			vector<char>letter;





			char a('A');

			char b('B');

			char c('C');

			char d('D');

			char e('E');

			char f('F');

			char g('G');

			char h('H');



			letter.push_back(a);

			letter.push_back(b);

			letter.push_back(c);

			letter.push_back(d);

			letter.push_back(e);

			letter.push_back(f);

			letter.push_back(g);

			letter.push_back(h);







			for (int i = 0; i < colors.size(); i++) {

				//cout << "color" << i << ": " << colors[i] << endl;

			}





			int count = 0;

			//im Prak: Suche Tasten (Area)

			for (int i = 1; i < nLabels; i++) {

				//cout << "Label " << i << " Area: " << stats.at<int>(i, CC_STAT_AREA) << endl;

				if (stats.at<int>(i, CC_STAT_AREA) > 1000)

					count++;

			}



			if (count > 6) {

				//CCs mit dem gleichen Flächeninhalt finden und selektieren ( Prak) Bsp -> Zwischen 2 und 3,1k

				for (int i = 1; i < nLabels; i++) { // 0 = background

					if (stats.at<int>(i, CC_STAT_AREA) > 1000 && stats.at<int>(i, CC_STAT_AREA) < 3100) {

						kombination tempkombi(centroids.at<double>(i, 1), centroids.at<double>(i, 0), i, stats.at<int>(i, CC_STAT_HEIGHT), stats.at<int>(i, CC_STAT_WIDTH), stats.at<int>(i, CC_STAT_AREA), stats.at<int>(i, CC_STAT_LEFT), stats.at<int>(i, CC_STAT_TOP));

						labels.push_back(tempkombi);



					}

				}

				//cout << "---------------------------------------------------------------------------------------" << endl;

				//cout << "Labels found and pushed to vector: " << labels.size() << endl;

				/*for (int i = 0; i < labels.size(); i++) {

					cout << "Label: " << labels[i].label << endl;

					cout << "Label Area: " << labels[i].area << endl;

				}*/







				//Sortiere -

				if (labels[0].height < labels[0].width) {

					//Nach x-Koordinate

					sort(labels.begin(), labels.end(), compare_y_koordinate);

					//cout << "nach y-Koordinate sortiert" << endl;



				}

				else {

					//Nach y-Koordinate

					sort(labels.begin(), labels.end(), compare_x_koordinate);

					//cout << "Nach x-Koordinate sortiert" << endl;

				}





				x_koor = labels[0].x_koordinate;

				y_koor = labels[0].y_koordinate;

				width = labels[0].width * 8;

				height = labels[0].height;



				if (labels.size() < 9) {

					for (int i = 0; i < labels.size(); i++) {

						labels[i].color = colors[i];

						labels[i].letter = letter[i];

						//cout << "Label Color: " << labels[i].color << endl;

						rectangle(dest, Rect(labels[i].x_koordinate, labels[i].y_koordinate, labels[i].width, labels[i].height), labels[i].color, -1, 8, 0);



					}



				}

			}



		} // case



		imshow("Fenster", dest);

		waitKey(2);

		showgrayImage();

		showzImage();



		if (case2 == 2) {

			writergray.write(grayImage);

			writerdepth.write(zImage);

		}



	}



	void setLensParameters(const royale::LensParameters& lensParameters)

	{

		// Construct the camera matrix

		// (fx   0    cx)

		// (0    fy   cy)

		// (0    0    1 )

		cameraMatrix = (cv::Mat1d(3, 3) << lensParameters.focalLength.first, 0, lensParameters.principalPoint.first,

			0, lensParameters.focalLength.second, lensParameters.principalPoint.second,

			0, 0, 1);



		// Construct the distortion coefficients

		// k1 k2 p1 p2 k3

		distortionCoefficients = (cv::Mat1d(1, 5) << lensParameters.distortionRadial[0],

			lensParameters.distortionRadial[1],

			lensParameters.distortionTangential.first,

			lensParameters.distortionTangential.second,

			lensParameters.distortionRadial[2]);

	}

	//--------------------------------------------------------

	void showzImage() {



		double minVal, maxVal;

		Mat temp;



		compare(zImage, 0, temp, CMP_GT);



		minMaxLoc(zImage, &minVal, &maxVal, 0, 0, temp);



		double scale = 255 / (maxVal - minVal);

		double shift = -minVal * 255 / (maxVal - minVal);



		convertScaleAbs(zImage, zImage, scale, shift);



		applyColorMap(zImage, zImage, COLORMAP_RAINBOW);



		imshow("Tiefenbild", zImage);

	}



	void showgrayImage() {



		double minVal, maxVal;



		Mat temp;



		minMaxLoc(grayImage, &minVal, &maxVal);



		double scale = 255 / (maxVal - minVal);

		double shift = -minVal * 255 / (maxVal - minVal);



		convertScaleAbs(grayImage, grayImage, scale, shift);

		imshow("Grauwertbild", grayImage);



	}

	//--------------------------------------------------------------------------

	void openvideowriter(string name, uint16_t high, uint16_t width, uint16_t framerate) {							//Aufgabe 4 neue Methode



		string gray = name + "_gray.avi";																			//Aufgabe 5

		string depth = name + "_depth.avi";

		Size size = Size(width, high);



		cout << "Framerate =" << framerate << endl;

		cout << "size = " << size << endl;

		writergray.open(gray, cv::VideoWriter::fourcc('M', 'P', 'E', 'G'), framerate, size, false);

		writerdepth.open(depth, cv::VideoWriter::fourcc('M', 'P', 'E', 'G'), framerate, size, true);				//öffnen der 2 Videowriter



		if (!writergray.isOpened() && !writerdepth.isOpened()) {

			cerr << "Cant open videowriter!" << endl;

			system("pause");

			exit(44);

		}



	}



	void closewriter() {



		writerdepth.release();

		writergray.release();

	}

	//-----------------------------------------------------------------------------------------------------------



	void openvideocapture(string filename) {



		Mat showgraymat;

		Mat showdepthmat;



		string namegray = filename + "_gray.avi";

		string namedepth = filename + "_depth.avi";



		cout << "Files: " << namegray << ", " << namedepth << endl;



		capturegray.open(namegray);

		capturedepth.open(namedepth);



		if (!capturegray.isOpened() && !capturedepth.isOpened()) {

			cerr << "cant open VideoCapture..." << endl;

			exit(1);



		}



		cout << "Capture successfully open..." << endl;

		while (capturedepth.read(showdepthmat) && capturegray.read(showgraymat)) {



			imshow("Tiefenbild", showdepthmat);	// Frames ausgegeben

			imshow("Grauwertbild", showgraymat);

			waitKey(150);



		}

		capturedepth.release();

		capturegray.release();

		system("Pause");

	}

	//---------------------------------------------------------------------------------------------------------



	void evaluation_gray() {



		blur(grayImage, grayImage, Size(3, 3));





	}



	void evalúation_depth() {







	}



private:

	VideoWriter writergray;		//Aufgabe 4, Variablen für die neue Methode

	VideoWriter writerdepth;

	VideoCapture capturegray;

	VideoCapture capturedepth;

	cv::Mat zImage, grayImage, zImage2;

	cv::Mat cameraMatrix, distortionCoefficients;

	std::mutex flagMutex;

	Mat maske;

	double x_koor, y_koor;

	int width, height;

};



int main(int argc, char* argv[])

{



	//Argumentcheck

	cout << "\nArgc:" << argc << endl;

	cout << "\nArgv[1]:" << *argv[1] << endl;





	namedWindow("Grauwertbild", WINDOW_AUTOSIZE);

	namedWindow("Tiefenbild", WINDOW_AUTOSIZE);







	MyListener listener;

	bool control = 0;

	int entertaste = 0;

	string videodateiname;

	string videodateisuche;

	uint16_t high, width, framerate;





	if (argc > 1) {

		if (*argv[1] == '1' || *argv[1] == '2') {



			//----------------------------------------------------------------------------------------------------

			// this represents the main camera device object

			std::unique_ptr<royale::ICameraDevice> cameraDevice;



			// the camera manager will query for a connected camera

			{



				royale::CameraManager manager;



				// try to open the first connected camera

				royale::Vector<royale::String> camlist(manager.getConnectedCameraList());

				std::cout << "Detected " << camlist.size() << " camera(s)." << std::endl;



				if (!camlist.empty())

				{

					cameraDevice = manager.createCamera(camlist[0]);

				}

				else

				{

					std::cerr << "No suitable camera device detected." << std::endl

						<< "Please make sure that a supported camera is plugged in, all drivers are "

						<< "installed, and you have proper USB permission" << std::endl;

					return 1;

				}



				camlist.clear();



			}

			// the camera device is now available and CameraManager can be deallocated here



			if (cameraDevice == nullptr)

			{

				// no cameraDevice available

				if (argc > 1)

				{

					std::cerr << "Could not open " << argv[1] << std::endl;

					return 1;

				}

				else

				{

					std::cerr << "Cannot create the camera device" << std::endl;

					return 1;

				}

			}



			// call the initialize method before working with the camera device

			auto status = cameraDevice->initialize();

			if (status != royale::CameraStatus::SUCCESS)

			{

				std::cerr << "Cannot initialize the camera device, error string : " << getErrorString(status) << std::endl;

				return 1;

			}



			// retrieve the lens parameters from Royale

			royale::LensParameters lensParameters;

			status = cameraDevice->getLensParameters(lensParameters);

			if (status != royale::CameraStatus::SUCCESS)

			{

				std::cerr << "Can't read out the lens parameters" << std::endl;

				return 1;

			}



			listener.setLensParameters(lensParameters);



			// register a data listener

			if (cameraDevice->registerDataListener(&listener) != royale::CameraStatus::SUCCESS)

			{

				std::cerr << "Error registering data listener" << std::endl;

				return 1;

			}



			//------------------------------------------------------------------



			switch (*argv[1]) {

			case '1':

				listener.case2 = 1;

				cout << "Aufruf der Auswertung" << endl;



				cameraDevice->setExposureMode(royale::ExposureMode::AUTOMATIC);



				if (cameraDevice->startCapture() != royale::CameraStatus::SUCCESS)

				{

					std::cerr << "Error starting the capturing" << std::endl;

					return 1;

				}





				while (1) {

					if (waitKey(0) == 13)

						break;

				}



				if (cameraDevice->stopCapture() != royale::CameraStatus::SUCCESS)

				{

					//listener.closewriter;

					std::cerr << "Error stopping the capturing" << std::endl;

					return 1;

				}



				system("pause");

				break;



			case '2':



				listener.case2 = 2;

				cout << "Name eingeben" << endl;				//Präfix der Videodatei angeben

				cin >> videodateiname;

				cameraDevice->getMaxSensorHeight(high);

				cameraDevice->getMaxSensorWidth(width);

				cameraDevice->setExposureMode(royale::ExposureMode::AUTOMATIC);

				cameraDevice->getMaxFrameRate(framerate);

				listener.openvideowriter(videodateiname, high, width, framerate);

				// start capture mode

				if (cameraDevice->startCapture() != royale::CameraStatus::SUCCESS)

				{

					std::cerr << "Error starting the capturing" << std::endl;

					return 1;

				}



				while (1) {

					if (waitKey(0) == 13)

						break;

				}



				listener.closewriter();



				if (cameraDevice->stopCapture() != royale::CameraStatus::SUCCESS)

				{

					//listener.closewriter;

					std::cerr << "Error stopping the capturing" << std::endl;

					return 1;

				}

				break;

			}

		}

		else if (*argv[1] == '3') {



			cout << "Geben sie hier den Namen des Videos folgenden Formates ein [Name]_gray.avi : " << endl;

			cin >> videodateisuche;

			listener.openvideocapture(videodateisuche);

			system("Pause");

		}

	}





	return 0;

}
