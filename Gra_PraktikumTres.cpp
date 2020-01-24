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

	string letter;

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
			Vec3b weiss(255, 255, 255);

			if (zImage2.data != NULL && counter == TRUE) {

				Mat tempzImage(Size(zImage.cols, zImage.rows), CV_32FC1);
				Mat mask(Size(zImage.cols, zImage.rows), CV_32FC1, Scalar(0));

				//cout << "x: " << x_koor << endl;
				//cout << "y: " << y_koor << endl;
				//cout << "width " << width << endl;
				//cout << "height: " << height << endl;

				rectangle(mask, Rect(x_koor, y_koor, width, height), Scalar(1), -1, 8, 0);
				imshow("mask", mask);

				//subtract(zImage2, zImage, tempzImage, mask,CV_32FC1);
				tempzImage = zImage2 - zImage;
				multiply(tempzImage, mask, tempzImage);

				imshow("DIF", tempzImage);

				double zmin1, zmin2, zmax1, zmax2, difmin, difmax;
				minMaxLoc(zImage, &zmin1, &zmax1);
				minMaxLoc(zImage2, &zmin2, &zmax2);
				minMaxLoc(tempzImage, &difmin, &difmax);


				//cout << "zImage minval: " << zmin1 << " zImage maxval: " << zmax1 << endl;
				//cout << "zImage2 minval: " << zmin2 << " zImage2 maxval: " << zmax2 << endl;
				//cout << "tempzImage minval: " << difmin << " tempzImage maxval: " << difmax << endl;

				float schwelle = -0.01;
				float schwelle2 = 0.01;

				Mat bild_großer(Size(zImage.cols, zImage.rows), CV_8UC1, Scalar(0));
				Mat bild_kleiner(Size(zImage.cols, zImage.rows), CV_8UC1, Scalar(0));


				for (int x = 0; x < tempzImage.rows; x++) {

					for (int y = 0; y < tempzImage.cols; y++) {

						if (tempzImage.at<float>(x, y) > schwelle2)
							bild_großer.at<uchar>(x, y) = 255.0;

						if (tempzImage.at<float>(x, y) < schwelle)
							bild_kleiner.at<uchar>(x, y) = 255.0;

					}

				}


				//Opening auf die Bilder
				Mat element = getStructuringElement(MORPH_CROSS, Size(3, 3));
				morphologyEx(bild_großer, bild_großer, MORPH_OPEN, element);
				morphologyEx(bild_kleiner, bild_kleiner, MORPH_OPEN, element);

				imshow("bild_großer", bild_großer);
				imshow("bild_kleiner", bild_kleiner);


				//Einfärben der Pixel im Ergebnisbild
				vector<Mat>channels;
				Mat ergebnis = tempzImage.clone(); // Mit Maske
				ergebnis.convertTo(ergebnis, CV_8UC3);

				Mat green = Mat::zeros(Size(tempzImage.cols, tempzImage.rows), CV_8UC1);
				channels.push_back(bild_großer); // Blau
				channels.push_back(green); //Grün
				channels.push_back(bild_kleiner); // Rot

				merge(channels, ergebnis);
				imshow("Ergebnis", ergebnis);

				int pixel_counter = 0;
				//Mat temp_dest = dest.clone();
				for (int x = 0; x < destcopy.rows; x++) {
					for (int y = 0; y < destcopy.cols; y++) {
						if (bild_großer.at<uchar>(x, y) == 255.0) { // Markieren der Pixel in dest_temp 
							destcopy.at<Vec3b>(x, y) = Vec3b(155,155,155);
							pixel_counter++;

						}
					}
				}
				//cout << "Pixels: " << pixel_counter << endl;
				imshow("destcopy", destcopy); // Mit den markierten Pixeln

				 //color in dest
				int dest_rot = 0, dest_gelb = 0, dest_blau = 0, dest_gruen = 0, dest_violett = 0, dest_orange = 0, dest_braun = 0, dest_pink = 0;
				for (int x = 0; x < destcopy2.rows; x++) {
					for (int y = 0; y < destcopy2.cols; y++) {
						if (destcopy2.at<Vec3b>(x, y)[0] == 0 && destcopy2.at<Vec3b>(x, y)[1] == 0 && destcopy2.at<Vec3b>(x, y)[2] == 255) // BGR
							dest_rot += 1;
						if (destcopy2.at<Vec3b>(x, y)[0] == 0 && destcopy2.at<Vec3b>(x, y)[1] == 255 && destcopy2.at<Vec3b>(x, y)[2] == 0)
							dest_gelb += 1;
						if (destcopy2.at<Vec3b>(x, y)[0] == 255 && destcopy2.at<Vec3b>(x, y)[1] == 0 && destcopy2.at<Vec3b>(x, y)[2] == 0)
							dest_blau += 1;
						if (destcopy2.at<Vec3b>(x, y)[0] == 127 && destcopy2.at<Vec3b>(x, y)[1] == 255 && destcopy2.at<Vec3b>(x, y)[2] == 0)
							dest_gruen += 1;
						if (destcopy2.at<Vec3b>(x, y)[0] == 226 && destcopy2.at<Vec3b>(x, y)[1] == 43 && destcopy2.at<Vec3b>(x, y)[2] == 138)
							dest_violett += 1;
						if (destcopy2.at<Vec3b>(x, y)[0] == 0 && destcopy2.at<Vec3b>(x, y)[1] == 165 && destcopy2.at<Vec3b>(x, y)[2] == 255)
							dest_orange += 1;
						if (destcopy2.at<Vec3b>(x, y)[0] == 19 && destcopy2.at<Vec3b>(x, y)[1] == 69 && destcopy2.at<Vec3b>(x, y)[2] == 139)
							dest_braun += 1;
						if (destcopy2.at<Vec3b>(x, y)[0] == 147 && destcopy2.at<Vec3b>(x, y)[1] == 20 && destcopy2.at<Vec3b>(x, y)[2] == 255)
							dest_pink += 1;
					}
				}
				//cout << "dest rot original: " << dest_rot << endl;
				

				//count colors in temp_dest
				int rot = 0, gelb = 0, blau = 0, gruen = 0, violett = 0, orange = 0, braun = 0, pink = 0;
				for (int x = 0; x < destcopy.rows; x++) {
					for (int y = 0; y < destcopy.cols; y++) {
						if (destcopy.at<Vec3b>(x, y)[0] == 0 && destcopy.at<Vec3b>(x, y)[1] == 0 && destcopy.at<Vec3b>(x, y)[2] == 255) // BGR
							rot += 1;
						if (destcopy.at<Vec3b>(x, y)[0] == 0 && destcopy.at<Vec3b>(x, y)[1] == 255 && destcopy.at<Vec3b>(x, y)[2] == 0)
							gelb += 1;
						if (destcopy.at<Vec3b>(x, y)[0] == 255 && destcopy.at<Vec3b>(x, y)[1] == 0 && destcopy.at<Vec3b>(x, y)[2] == 0)
							blau += 1;
						if (destcopy.at<Vec3b>(x, y)[0] == 127 && destcopy.at<Vec3b>(x, y)[1] == 255 && destcopy.at<Vec3b>(x, y)[2] == 0)
							gruen += 1;
						if (destcopy.at<Vec3b>(x, y)[0] == 226 && destcopy.at<Vec3b>(x, y)[1] == 43 && destcopy.at<Vec3b>(x, y)[2] == 138)
							violett += 1;
						if (destcopy.at<Vec3b>(x, y)[0] == 0 && destcopy.at<Vec3b>(x, y)[1] == 165 && destcopy.at<Vec3b>(x, y)[2] == 255)
							orange += 1;
						if (destcopy.at<Vec3b>(x, y)[0] == 19 && destcopy.at<Vec3b>(x, y)[1] == 69 && destcopy.at<Vec3b>(x, y)[2] == 139)
							braun += 1;
						if (destcopy.at<Vec3b>(x, y)[0] == 147 && destcopy.at<Vec3b>(x, y)[1] == 20 && destcopy.at<Vec3b>(x, y)[2] == 255)
							pink += 1;
					}
				}


				//cout << "transformed rot: " << rot << endl;
			
	
				
				//sum
				int sum_rot = 0, sum_gelb = 0, sum_blau = 0, sum_gruen = 0, sum_violett = 0, sum_orange = 0, sum_braun = 0, sum_pink = 0;
				sum_rot = dest_rot - rot;
				sum_blau = dest_blau - blau;
				sum_gelb = dest_gelb - gelb;
				sum_gruen = dest_gruen - gruen;
				sum_violett = dest_violett - violett;
				sum_orange = dest_orange - orange;
				sum_braun = dest_braun - braun;
				sum_pink = dest_pink - pink;

// ... jetzt muss geguckt werden , wo die meisten markiert worden sind 
				int array[8] = {sum_rot,sum_blau,sum_gelb,sum_gruen,sum_violett,sum_orange,sum_braun,sum_pink};
				
				int groesste = 0;
				for (int i = 0; i < 8; i++) {

					if (groesste < array[i]){
						
						putText(dest, labelstemp[i].letter, Point(100, 100), FONT_HERSHEY_SIMPLEX, 1, Vec3b(255, 255, 255),2, 8, false);

					}

				}


				/*cout << "Pixel marked in label rot: " << sum_rot << endl;
				cout << "Pixel marked in label gelb: " << sum_gelb << endl;
				cout << "Pixel marked in label blau: " << sum_blau << endl;
				cout << "Pixel marked in label gruen: " << sum_gruen << endl;
				cout << "Pixel marked in label violett: " << sum_violett << endl;
				cout << "Pixel marked in label orange: " << sum_orange << endl;
				cout << "Pixel marked in label braun: " << sum_braun << endl;
				cout << "Pixel marked in label pink: " << sum_pink << endl;
				*/

			}

		

			zImage2 = zImage.clone();

			//convert
			grayImage.convertTo(grayImage, CV_8U);

			vector<kombination> labels;

			Mat labelImage(grayImage.size(), CV_32S);
			Mat stats, centroids;


			int nLabels = connectedComponentsWithStats(grayImage, labelImage, stats, centroids, 8, CV_32S);

			//cout << "Found " << nLabels << "Labels." << endl;

			vector<Vec3b>colors;
			Vec3b rot(0, 0, 255); // a
			Vec3b gelb(0, 255, 0); // b
			Vec3b blau(255, 0, 0); // c
			Vec3b gruen(127, 255, 0); // d
			Vec3b violett(226, 43, 138); //e
			Vec3b orange(0, 165, 255); //f
			Vec3b braun(19, 69, 139); //g
			Vec3b pink(147, 20, 255); //h

			colors.push_back(rot);
			colors.push_back(gelb);
			colors.push_back(blau);
			colors.push_back(gruen);
			colors.push_back(violett);
			colors.push_back(orange);
			colors.push_back(braun);
			colors.push_back(pink);

			vector<string>letter;
			string a("A");
			string b("B");
			string c("C");
			string d("D");
			string e("E");
			string f("F");
			string g("G");
			string h("H");

			letter.push_back(a);
			letter.push_back(b);
			letter.push_back(c);
			letter.push_back(d);
			letter.push_back(e);
			letter.push_back(f);
			letter.push_back(g);
			letter.push_back(h);

			int count = 0;

			for (int i = 1; i < nLabels; i++) {
				//cout << "Label " << i << " Area: " << stats.at<int>(i, CC_STAT_AREA) << endl;
				if (stats.at<int>(i, CC_STAT_AREA) > 1000)
					count++;
			}



			if (count > 6) {
				counter = TRUE;
				//CCs mit dem gleichen Flächeninhalt finden und selektieren ( Prak) Bsp -> Zwischen 2 und 3,1k
				for (int i = 1; i < nLabels; i++) { // 0 = background
					if (stats.at<int>(i, CC_STAT_AREA) > 500 && stats.at<int>(i, CC_STAT_AREA) < 3100) {
						kombination tempkombi(stats.at<int>(i, CC_STAT_TOP), stats.at<int>(i, CC_STAT_LEFT), i, stats.at<int>(i, CC_STAT_HEIGHT), stats.at<int>(i, CC_STAT_WIDTH), stats.at<int>(i, CC_STAT_AREA), stats.at<int>(i, CC_STAT_LEFT), stats.at<int>(i, CC_STAT_TOP));
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



				// Maske für Rectangle
				x_koor = labels[0].x_koordinate;
				y_koor = labels[0].y_koordinate;
				width = labels[0].width * 8;
				height = labels[0].height;



				if (labels.size() < 9) {

					for (int i = 0; i < labels.size(); i++) {
						labels[i].color = colors[i];
						labels[i].letter = letter[i];
						//cout << "Label Color: " << labels[i].color << endl;
						rectangle(dest, Rect(labels[i].x_koordinate, labels[i].y_koordinate, labels[i].width, labels[i].height), labels[i].color, -1, 8, 0); // Bei Misserfolg , die Pixel in dest anschauen
					}

				}

				labelstemp = labels;
				destcopy2 = dest.clone();
				destcopy = dest.clone();
				

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

	bool counter = FALSE;

	vector<kombination> labelstemp;
	Mat destcopy;
	Mat destcopy2;

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
