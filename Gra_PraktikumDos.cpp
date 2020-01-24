// p/****************************************************************************\
* Vorlage fuer das Praktikum "Graphische Datenverarbeitung" WS 2018/19

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
	kombination(double y_koor, double x_koor, int lab, int hei, int wi, int ar, int le, int to, Vec3b col) {
		y_koordinate = y_koor;
		x_koordinate = x_koor;
		label = lab;
		height = hei;
		width = wi;
		area = ar;
		left = le;
		top = to;
		color = col;
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
			
			//convert
			grayImage.convertTo(grayImage, CV_8U );

			Mat labelImage(grayImage.size(), CV_32S);
			Mat stats, centroids;
			vector<kombination> labels;

			int nLabels = connectedComponentsWithStats(grayImage, labelImage, stats, centroids, 8, CV_32S);
			cout << "Found " << nLabels << "Labels." << endl;

			vector<Vec3b>colors;


		Vec3b rot(255,0,0);
		Vec3b gelb(0,255,0);
		Vec3b blau(0,0,255);
		Vec3b gruen(0,255,127);
		Vec3b violett(138,43,226);
		Vec3b orange(255,165,0);
		Vec3b braun(139,69,19);
		Vec3b pink(255,20,147);

		colors.push_back(rot);
		colors.push_back(gelb);
		colors.push_back(blau);
		colors.push_back(gruen);
		colors.push_back(violett);
		colors.push_back(orange);
		colors.push_back(braun);
		colors.push_back(pink);

		for (int i = 0; i < colors.size(); i++) {
			cout << "color" << i << ": " << colors[i] << endl;
		}


			int count = 0;
			//im Prak: Suche Tasten (Area)
			for (int i = 1; i < nLabels; i++) {
				cout << "Label " << i << " Area: " << stats.at<int>(i, CC_STAT_AREA) << endl;
				if (stats.at<int>(i, CC_STAT_AREA) > 1000)
					count++;
			}

			if (count > 6) {
				//CCs mit dem gleichen Flächeninhalt finden und selektieren ( Prak) Bsp -> Zwischen 2 und 3,1k
				for (int i = 1; i < nLabels; i++) { // 0 = background
				if (stats.at<int>(i, CC_STAT_AREA) > 1000 && stats.at<int>(i, CC_STAT_AREA) < 3100) {
				kombination tempkombi(centroids.at<double>(i, 1), centroids.at<double>(i, 0), i, stats.at<int>(i, CC_STAT_HEIGHT), stats.at<int>(i, CC_STAT_WIDTH), stats.at<int>(i, CC_STAT_AREA), stats.at<int>(i, CC_STAT_LEFT), stats.at<int>(i, CC_STAT_TOP), Vec3b(rand() % 255, rand() % 255, rand() % 255));
				labels.push_back(tempkombi);
			
				}
				}
				cout << "---------------------------------------------------------------------------------------" << endl;
				cout << "Labels found and pushed to vector: " << labels.size() << endl;
				for (int i = 0; i < labels.size(); i++) {
				cout << "Label: " << labels[i].label << endl;
				cout << "Label Area: " << labels[i].area << endl;
				}

				//Sortiere -
				if (labels[0].height < labels[0].width) {
				//Nach x-Koordinate
				sort(labels.begin(), labels.end(), compare_y_koordinate);
				cout << "nach y-Koordinate sortiert" << endl;
				
				}
				else {
				//Nach y-Koordinate
				sort(labels.begin(), labels.end(), compare_x_koordinate);
				cout << "Nach x-Koordinate sortiert" << endl;
				}

				for (int i = 0; i < labels.size(); i++) {
					labels[i].color = colors[i];
					cout << "Label Color: " << labels[i].color << endl;
					rectangle(dest,Rect(labels[i].x_koordinate,labels[i].y_koordinate, labels[i].width, labels[i].height),labels[i].color, -1, 8, 0);

				}

				
				cout << "Malen nach Zahlen start" << endl;
				cout << "Dest cols:" << dest.cols << endl;
				cout << "Dest rows: " << dest.rows << endl;

				//Malen nach Zahlen
				/*for (int i = 0; i < dest.cols; i++)
				for (int k = 0; k < dest.rows; k++) {
				int label = labelImage.at<int>(k, i);

				for (int m = 0; m < labels.size(); m++) {
				if (label == labels[m].label)
				dest.at<Vec3b>(k, i) = labels[m].color;
				}
				}*/
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

private:
	VideoWriter writergray;		//Aufgabe 4, Variablen für die neue Methode
	VideoWriter writerdepth;
	VideoCapture capturegray;
	VideoCapture capturedepth;
	cv::Mat zImage, grayImage;
	cv::Mat cameraMatrix, distortionCoefficients;
	std::mutex flagMutex;


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