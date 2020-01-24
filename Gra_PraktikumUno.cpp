/****************************************************************************\
* Vorlage fuer das Praktikum "Graphische Datenverarbeitung" WS 2018/19
* FB 03 der Hochschule Niedderrhein
* Regina Pohle-Froehlich
*
* Der Code basiert auf den c++-Beispielen der Bibliothek royale
\****************************************************************************/

#include <royale.hpp>
#include <iostream>
#include <mutex>
#include <opencv2/opencv.hpp>
#include <stdlib.h>
#include <windows.h>

using namespace std;
using namespace cv;

//global


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

		minMaxLoc(zImage, &minVal, &maxVal,0,0,temp);

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
		string namedepth = filename+ "_depth.avi";

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

				cout << "Aufruf der Auswertung" << endl;

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