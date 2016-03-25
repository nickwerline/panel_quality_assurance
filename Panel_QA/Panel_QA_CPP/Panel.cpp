// This is the main DLL file.

#include "Panel.h"
#include "Calibrate.h"

Panel::Panel()
{
}

Panel::~Panel()
{
	delete m_pPanel;
}

// Camera Calibration Constants
Mat mainMap1, mainMap2;

// Helper function to display a message box
void ShowMessage(string message)
{
	char buff[100];
	sprintf_s(buff, "%s", message.c_str());
	MessageBoxA(NULL, (LPCSTR)buff, (LPCSTR)"Panel_QA_CPP.dll", MB_OK);
}

///////////////////////////////////////////////////////////////////////
// Helper Function for Cascade Classifier				       ////////
///////////////////////////////////////////////////////////////////////

void detectAndDisplay(Mat image, string panel_cascade_name)
{
	CascadeClassifier panel_cascade;
	if (!panel_cascade.load(panel_cascade_name)){ printf("--(!)Error loading\n"); return; };

	std::vector<Rect> detectedPanels;
	Mat frame_gray;

	cvtColor(image, frame_gray, CV_BGR2GRAY);
	equalizeHist(frame_gray, frame_gray);

	//-- Detect faces
	panel_cascade.detectMultiScale(frame_gray, detectedPanels, 1.1, 20, 0 | CV_HAAR_SCALE_IMAGE, Size(200, 200));

	for (size_t i = 0; i < detectedPanels.size(); i++)
	{
		Point topLeft(detectedPanels[i].x, detectedPanels[i].y);
		Point botRight(detectedPanels[i].x + detectedPanels[i].width, detectedPanels[i].y + detectedPanels[i].height);
		rectangle(image, topLeft, botRight, Scalar(0, 0, 255), 4);
}
	//-- Show what you got
	imshow("Classifier Result", image);
}

///////////////////////////////////////////////////////////////////////
// End of Helper Function for Cascade Classifier			   ////////
///////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////
///// Helper functions for Finding Contours                   /////////
///////////////////////////////////////////////////////////////////////
void morphIt(Mat &img)
{
	cvtColor(img, img, CV_RGB2GRAY);
	threshold(img, img, 50.0, 255.0, THRESH_BINARY);
	erode(img, img, Mat());
	dilate(img, img, Mat());
}

void blurthresh(Mat &img)
	{
	int kblur = 1;
	int threshval = 0;
	//medianBlur(img,img,kblur%2+3+kblur);
	blur(img, img, Size(kblur, kblur), Point(-1, -1), BORDER_DEFAULT);
	// threshold(img, img, threshval, 255, THRESH_BINARY_INV);
	}

void showimgcontours(Mat &threshedimg, Mat &original)
{
	vector<vector<Point> > contours;
	RotatedRect rect;
	vector<Vec4i> hierarchy;
	Point2f rectPoints[4];
	Scalar color = Scalar(255, 0, 0);
	int largest_area = 0;
	int largest_contour_index = 0;
	findContours(threshedimg, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE);
	//this will find largest contour
	for (int i = 0; i< contours.size(); i++) // iterate through each contour. 
	{
		double a = contourArea(contours[i], false);  //  Find the area of contour
		if (a>largest_area)
	{
			largest_area = a;
			largest_contour_index = i;                //Store the index of largest contour
	}

}
	//search for largest contour has end

	if (contours.size() > 0)
{
		drawContours(original, contours, largest_contour_index, CV_RGB(0, 255, 0), 2, 8, hierarchy);
		//if want to show all contours use below one
		//drawContours(original,contours,-1, CV_RGB(0, 255, 0), 2, 8, hierarchy);

		// Find and add bounding rectangle
		rect = minAreaRect(contours[largest_contour_index]);
		rect.points(rectPoints);
		int j = 0;
		for (j; j < 4; j++)
			line(original, rectPoints[j], rectPoints[(j + 1) % 4], color, 1, 8);
		string strDim = "Min Area Rectangle Dimensions:\n" + 
			to_string(rect.size.height) + " x " + to_string(rect.size.width);
		imshow("Largest Contour", original);
		ShowMessage(strDim);
	}
}

//////////////////////////////////////////////////////////////////////////////
///// End of Helper functions for Finding Contours					 /////////
//////////////////////////////////////////////////////////////////////////////

// Helper function to replace characters in a string
void strReplace(string& source, string const& find, string const& replace)
{
	for (string::size_type i = 0; (i = source.find(find, i)) != string::npos;)
	{
		source.replace(i, find.length(), replace);
		i += replace.length();
	}
}

// On click listener to display the HSV value of the point clicked
static void onMouse(int event, int x, int y, int f, void *ptr)
{
	if (event == EVENT_LBUTTONDOWN)
	{
		Panel *pPanel = static_cast<Panel*>(ptr);
		Point clickPoint(x, y);
		pPanel->ColorAtPoint(clickPoint);
	}
}

// Function to display the specified image and add an 
//	on click listener (onMouse())
bool Panel::ShowImage(string sImgPath, string windowTitle)
{
	m_pPanel = new Panel;
	// read specified image
	m_pPanel->m_Image = imread(sImgPath, IMREAD_COLOR);

	if (m_pPanel->m_Image.empty()) // Check for invalid input
	{
		ShowMessage("Could not open or find the image");
		return false;
	}

	// resize the image to have 1000 width, keeping the aspect ratio
	float r = 750.0 / m_pPanel->m_Image.cols;
	Size dim = Size(750.0, int(m_pPanel->m_Image.rows * r));
	resize(m_pPanel->m_Image, m_pPanel->m_Image, dim);

	// Find the ROI
	const Rect roi(0,0,650,m_pPanel->m_Image.rows);
	m_pPanel->m_Image(roi);

	// Show the image
	namedWindow(windowTitle, CV_WINDOW_KEEPRATIO);
	imshow(windowTitle, m_pPanel->m_Image);
	// Set mouse callback to show the color of the point clicked
	setMouseCallback(windowTitle, onMouse, static_cast<void*>(&m_pPanel));

	return true;
}

// Function to display the specified image and add an 
//	on click listener (onMouse())
bool Panel::ShowImageWithCalibration(string sImgPath, string windowTitle)
{
	m_pPanel = new Panel;
	// read specified image
	m_pPanel->m_Image = imread(sImgPath, IMREAD_COLOR);

	if (m_pPanel->m_Image.empty()) // Check for invalid input
	{
		ShowMessage("Could not open or find the image");
		return false;
	}

	// resize the image to have 1000 width, keeping the aspect ratio
	//float r = 750.0 / m_pPanel->m_Image.cols;
	//Size dim = Size(750.0, int(m_pPanel->m_Image.rows * r));
	//resize(m_pPanel->m_Image, m_pPanel->m_Image, dim);

	// Find the ROI
	//const Rect roi(0, 0, 650, m_pPanel->m_Image.rows);
	//m_pPanel->m_Image(roi);

	// Calibrate
	Mat rview;
	remap(m_pPanel->m_Image, rview, mainMap1, mainMap2, INTER_LINEAR);

	// Show the image
	namedWindow(windowTitle, CV_WINDOW_KEEPRATIO);
	imshow(windowTitle, rview);
	// Set mouse callback to show the color of the point clicked
	setMouseCallback(windowTitle, onMouse, static_cast<void*>(&m_pPanel));

	return true;
}


void Panel::FixPath(string &path)
{
	strReplace(path, "\\" , "\\\\");
}

string Panel::ColorName(Vec3b color)
{
	uchar hue = color.val[0];
	uchar sat = color.val[1];
	uchar val = color.val[2];
	if (sat >= 100 && val >= 30)
	{
		if (hue >= 105 && hue <= 131)
			return "Blue";
		else if (hue <= 20 && (val <= 160 && val >= 55))
			return "Brown";
		else if (hue >= 160 || hue <= 10)
			return "Red";
		else if (hue >= 15 && hue <= 35)
			return "Yellow";
		else if (hue >= 50 && hue <= 70)
			return "Green";
	}
	
	else if (val < 30)
		return "Black";
	else if (val >= 190)
		return "White";

	return "Unknown";
}

void Panel::ColorAtPoint(Point point)
{
	Mat HSV;
	cvtColor(m_pPanel->m_Image, HSV, CV_BGR2HSV);
	Vec3b BGRpix, HSVpix;
	BGRpix = m_pPanel->m_Image.at<Vec3b>(Point(point.x, point.y));
	HSVpix = HSV.at<Vec3b>(Point(point.x, point.y));
	uchar blue, green, red;
	uchar hue, sat, val;
	blue = BGRpix.val[0];
	green = BGRpix.val[1];
	red = BGRpix.val[2];
	hue = HSVpix.val[0];
	sat = HSVpix.val[1];
	val = HSVpix.val[2];
	// Convert to string
	string RGBcolorStr("(" + to_string(red) + "," + to_string(green) + "," + to_string(blue) + ")");
	string HSVcolorStr("(" + to_string(hue) + "," + to_string(sat) + "," + to_string(val) + ")");
	// Display the color strings
	ShowMessage("Point: (" + to_string(point.x) + "," + to_string(point.y) +
		") \nRGB Value: " + RGBcolorStr + "\nHSV Value: " + HSVcolorStr + "\nColor: " + m_pPanel->ColorName(HSVpix));
}

void Panel::MaskWithColor(string sImgPath, string color)
{
	if(!ShowImage(sImgPath, "Original"))
		return;

	Mat HSV, Mask, BGR, MaskResult;
	cvtColor(m_pPanel->m_Image, HSV, CV_BGR2HSV);
	if (color == "blue")
	{
		inRange(HSV, Scalar(105, 100, 30), Scalar(131, 255, 255), Mask);
	}
	else if (color == "red")
	{
		Mat Mask1, Mask2;
		inRange(HSV, Scalar(0, 100, 30), Scalar(10, 255, 255), Mask1);
		inRange(HSV, Scalar(160, 100, 30), Scalar(180, 255, 255), Mask2);
		bitwise_or(Mask1, Mask2, Mask);
	}
	else if (color == "panel")
	{
		int h1Lo = 0, s1Lo = 55, v1Lo = 115;
		int h1Hi = 20, s1Hi = 120, v1Hi = 210;
		int h2Lo = 0, s2Lo = 0, v2Lo = 0;
		int h2Hi = 0, s2Hi = 0, v2Hi = 0;

		/*
		namedWindow("InRange Tester", CV_WINDOW_AUTOSIZE);
		cvCreateTrackbar("Hue Lo 1:", "InRange Tester", &h1Lo, 180);
		cvCreateTrackbar("Hue Hi 1:", "InRange Tester", &h1Hi, 180);
		cvCreateTrackbar("Sat Lo 1", "InRange Tester", &s1Lo, 255);
		cvCreateTrackbar("Sat Hi 1", "InRange Tester", &s1Hi, 255);
		cvCreateTrackbar("Val Lo 1", "InRange Tester", &v1Lo, 255);
		cvCreateTrackbar("Val Hi 1", "InRange Tester", &v1Hi, 255);
		cvCreateTrackbar("Hue Lo 2:", "InRange Tester", &h2Lo, 180);
		cvCreateTrackbar("Hue Hi 2:", "InRange Tester", &h2Hi, 180);
		cvCreateTrackbar("Sat Lo 2", "InRange Tester", &s2Lo, 255);
		cvCreateTrackbar("Sat Hi 2", "InRange Tester", &s2Hi, 255);
		cvCreateTrackbar("Val Lo 2", "InRange Tester", &v2Lo, 255);
		cvCreateTrackbar("Val Hi 2", "InRange Tester", &v2Hi, 255);
		*/

		Mat Mask1, Mask2;
		// while (true)
		{
			inRange(HSV, Scalar(h1Lo, s1Lo, v1Lo), Scalar(h1Hi, s1Hi, v1Hi), Mask1);
			inRange(HSV, Scalar(h2Lo, s2Lo, v2Lo), Scalar(h2Hi, s2Hi, v2Hi), Mask2);
			bitwise_or(Mask1, Mask2, Mask);

			m_pPanel->m_Image.copyTo(MaskResult, Mask);

			// namedWindow("Mask Result", CV_WINDOW_AUTOSIZE);
			// imshow("Mask Result", MaskResult);

			// CannyDetection(Result);

			// if (waitKey(30) == 27)
			//	break;
		}
	}

	m_pPanel->m_Image.copyTo(MaskResult, Mask);
	morphIt(MaskResult);
	blurthresh(MaskResult);
	// imshow("Morphed and Blurred", MaskResult);
	Mat modified;
	m_pPanel->m_Image.copyTo(modified, MaskResult);
	// imshow("Masked Original", modified);

	if (color == "red" || color == "blue")
	{
		DetectBlob(MaskResult);
}
	else if (color == "panel")
	{
		FindContours(modified);
	}
}

void Panel::DetectEdges(string sImgPath)
{
	if (!ShowImage(sImgPath, "Original"))
		return;

	// Canny Edge and Hough Line Detection
	Mat edges = CannyDetection(m_pPanel->m_Image);
}

Mat Panel::CannyDetection(Mat image)
{
	Mat greyImage;
	cvtColor(image, greyImage, CV_BGR2GRAY);

	Mat thresh, blurredThresh, edges, edgesGray;
	int low = 80, high = 255;
	int sigmaX = 2, sigmaY = 2; 
	int cannyLow = 100, ratio = 1, aperture = 3;
	/*	This code is for testing different values of various functions
		Uncomment if you want to test different values than the ones given
	*/
	namedWindow("Sliders", CV_WINDOW_AUTOSIZE);
	cvCreateTrackbar("Threshhold", "Sliders", &low, 255);
	//cvCreateTrackbar("High", "Sliders", &high, 255);
	cvCreateTrackbar("SigmaX", "Sliders", &sigmaX, 500);
	cvCreateTrackbar("SigmaY", "Sliders", &sigmaY, 500);
	cvCreateTrackbar("Low Threshold", "Sliders", &cannyLow, 100);
	cvCreateTrackbar("Ratio", "Sliders", &ratio, 5);

	while (true)
	{
		threshold(greyImage, thresh, low, 255, THRESH_BINARY);
		namedWindow("Threshhold", CV_WINDOW_AUTOSIZE);
		imshow("Threshhold", thresh);

		GaussianBlur(thresh, blurredThresh, Size(7, 7), sigmaX, sigmaY);
		namedWindow("Blurred", CV_WINDOW_AUTOSIZE);
		imshow("Blurred", blurredThresh);

		Canny(blurredThresh, edges, cannyLow, cannyLow*ratio, 3);
		namedWindow("Canny Edges", CV_WINDOW_AUTOSIZE);
		imshow("Canny Edges", edges);

		if (waitKey(30) == 27)
			break;
	}
	return edges;
}

void Panel::FindContours(Mat image)
{
	int low = 60;
	Mat grayImage;
	cvtColor(image, grayImage, CV_BGR2GRAY);
	Mat dilated;
	dilate(grayImage, dilated, Mat());
	Mat blurred;
	GaussianBlur(dilated, blurred, Size(7, 7), 0, 0);
	Mat thresh;
	threshold(blurred, thresh, low, 255, THRESH_BINARY);
	// namedWindow("Threshold", CV_WINDOW_AUTOSIZE);
	// imshow("Threshold", thresh);
	showimgcontours(thresh, image);
	// namedWindow("Contours", CV_WINDOW_AUTOSIZE);
	// imshow("Contours", thresh);
	// imshow("Largest Contour", image);
}

void Panel::DetectBlob(Mat image)
{
	Mat grayImage, dilatedEroded, dilated, blurred;
	cvtColor(image, grayImage, CV_BGR2GRAY);

	// Setup SimpleBlobDetector parameters.
	Ptr<SimpleBlobDetector> detector;
	SimpleBlobDetector::Params params;
	std::vector<KeyPoint> keypoints;
	Mat im_with_keypoints, thresh;

	int low = 60;
	int blobArea = 325;
	int sigmaX = 0, sigmaY = 0;
	/*	This is Test Code
		Uncomment this and the waiteKey() code and add closing bracket after
		break to run this portion of code withs trackbars

	namedWindow("Blob", CV_WINDOW_NORMAL);
	cvCreateTrackbar("Blob Area", "Blob", &blobArea, 2000);
	cvCreateTrackbar("Threshhold", "Blob", &low, 255);
	while (true)
	{
	*/
	dilate(grayImage, dilated, Mat());

	GaussianBlur(dilated, blurred, Size(7, 7), 0, 0);

	threshold(blurred, thresh, low, 255, THRESH_BINARY);

	// Filter by Area.
	params.filterByArea = true;
	params.filterByColor = false;
	params.filterByConvexity = false;
	params.filterByCircularity = false;
	params.filterByInertia = false;
	params.minArea = blobArea;

	// Set up the detector with default parameters.
	detector = SimpleBlobDetector::create(params);

	// Detect blobs.
	detector->detect(thresh, keypoints);

	// Draw detected blobs as red circles.
	// DrawMatchesFlags::DRAW_RICH_KEYPOINTS flag ensures the size of the circle corresponds to the size of blob
	drawKeypoints(thresh, keypoints, im_with_keypoints, Scalar(0, 0, 255), DrawMatchesFlags::DRAW_RICH_KEYPOINTS);

	// Show blobs
	imshow("keypoints", im_with_keypoints);

	// Pass/Fail Message
	if (!keypoints.empty())
		ShowMessage("Tag detected");
	else
		ShowMessage("No tag detected");
		/*
		if (waitKey(30) == 27)
			break;
			}
		*/
}

void Panel::CascadeClassify(string sImgPath, string sClassPath)
{
	if (!ShowImage(sImgPath, "Original"))
		return;

	detectAndDisplay(m_pPanel->m_Image, sClassPath);
}

void Panel::CalibrateCamera(string sFilePath)
{
	help();

	//! [file_read]
	Settings s;
	const string inputSettingsFile = sFilePath;
	FileStorage fs(inputSettingsFile, FileStorage::READ); // Read the settings
	if (!fs.isOpened())
	{
		cout << "Could not open the configuration file: \"" << inputSettingsFile << "\"" << endl;
//		return -1;
	}
	fs["Settings"] >> s;
	fs.release();                                         // close Settings file
	//! [file_read]

	//FileStorage fout("settings.yml", FileStorage::WRITE); // write config as YAML
	//fout << "Settings" << s;

	if (!s.goodInput)
	{
		cout << "Invalid input detected. Application stopping. " << endl;
//		return -1;
	}

	vector<vector<Point2f> > imagePoints;
	Mat cameraMatrix, distCoeffs;
	Size imageSize;
	int mode = s.inputType == Settings::IMAGE_LIST ? CAPTURING : DETECTION;
	clock_t prevTimestamp = 0;
	const Scalar RED(0, 0, 255), GREEN(0, 255, 0);
	const char ESC_KEY = 27;
	int counter = 1;

	//! [get_input]
	for (;;)
	{
		Mat view;
		bool blinkOutput = false;

		view = s.nextImage();

		//-----  If no more image, or got enough, then stop calibration and show result -------------
		if (mode == CAPTURING && imagePoints.size() >= (size_t)s.nrFrames)
		{
			if (runCalibrationAndSave(s, imageSize, cameraMatrix, distCoeffs, imagePoints))
				mode = CALIBRATED;
			else
				mode = DETECTION;
		}
		if (view.empty())          // If there are no more images stop the loop
		{
			// if calibration threshold was not reached yet, calibrate now
			if (mode != CALIBRATED && !imagePoints.empty())
				runCalibrationAndSave(s, imageSize, cameraMatrix, distCoeffs, imagePoints);
			break;
		}
		//! [get_input]

		imageSize = view.size();  // Format input image.
		if (s.flipVertical)    flip(view, view, 0);

		//! [find_pattern]
		vector<Point2f> pointBuf;

		bool found;
		switch (s.calibrationPattern) // Find feature points on the input format
		{
		case Settings::CHESSBOARD:
			found = findChessboardCorners(view, s.boardSize, pointBuf,
				CALIB_CB_ADAPTIVE_THRESH | CALIB_CB_FAST_CHECK | CALIB_CB_NORMALIZE_IMAGE);
			break;
		case Settings::CIRCLES_GRID:
			found = findCirclesGrid(view, s.boardSize, pointBuf);
			break;
		case Settings::ASYMMETRIC_CIRCLES_GRID:
			found = findCirclesGrid(view, s.boardSize, pointBuf, CALIB_CB_ASYMMETRIC_GRID);
			break;
		default:
			found = false;
			break;
		}
		//! [find_pattern]
		//! [pattern_found]
		if (found)                // If done with success,
		{
			// improve the found corners' coordinate accuracy for chessboard
			if (s.calibrationPattern == Settings::CHESSBOARD)
			{
				Mat viewGray;
				cvtColor(view, viewGray, COLOR_BGR2GRAY);
				cornerSubPix(viewGray, pointBuf, Size(11, 11),
					Size(-1, -1), TermCriteria(TermCriteria::EPS + TermCriteria::COUNT, 30, 0.1));
			}

			if (mode == CAPTURING &&  // For camera only take new samples after delay time
				(!s.inputCapture.isOpened() || clock() - prevTimestamp > s.delay*1e-3*CLOCKS_PER_SEC))
			{
				imagePoints.push_back(pointBuf);
				prevTimestamp = clock();
				blinkOutput = s.inputCapture.isOpened();
			}

			// Draw the corners.
			drawChessboardCorners(view, s.boardSize, Mat(pointBuf), found);
		}
		//! [pattern_found]
		//----------------------------- Output Text ------------------------------------------------
		//! [output_text]
		string msg = (mode == CAPTURING) ? "100/100" :
			mode == CALIBRATED ? "Calibrated" : "Press 'g' to start";
		int baseLine = 0;
		Size textSize = getTextSize(msg, 1, 1, 1, &baseLine);
		Point textOrigin(view.cols - 2 * textSize.width - 10, view.rows - 2 * baseLine - 10);

		if (mode == CAPTURING)
		{
			if (s.showUndistorsed)
				msg = format("%d/%d Undist", (int)imagePoints.size(), s.nrFrames);
			else
				msg = format("%d/%d", (int)imagePoints.size(), s.nrFrames);
		}

		putText(view, msg, textOrigin, 1, 1, mode == CALIBRATED ? GREEN : RED);

		if (blinkOutput)
			bitwise_not(view, view);
		//! [output_text]
		//------------------------- Video capture  output  undistorted ------------------------------
		//! [output_undistorted]
		if (mode == CALIBRATED && s.showUndistorsed)
		{
			Mat temp = view.clone();
			undistort(temp, view, cameraMatrix, distCoeffs);
		}
		//! [output_undistorted]
		//------------------------------ Show image and check for input commands -------------------
		//! [await_input]
		
		namedWindow("Image View" + to_string(counter), WINDOW_NORMAL);
		resizeWindow("Image View" + to_string(counter), 640, 480);
		imshow("Image View" + to_string(counter), view);
		char key = (char)waitKey(s.inputCapture.isOpened() ? 50 : s.delay);

		cout << "Image " << to_string(counter) << " Completed" << endl;
		counter++;

		if (key == ESC_KEY)
			break;

		if (key == 'u' && mode == CALIBRATED)
			s.showUndistorsed = !s.showUndistorsed;

		if (s.inputCapture.isOpened() && key == 'g')
		{
			mode = CAPTURING;
			imagePoints.clear();
		}
		//! [await_input]
	}

	// -----------------------Show the undistorted image for the image list ------------------------
	//! [show_results]
	if (s.inputType == Settings::IMAGE_LIST && s.showUndistorsed)
	{
		Mat view, rview, map1, map2;
		initUndistortRectifyMap(cameraMatrix, distCoeffs, Mat(),
			getOptimalNewCameraMatrix(cameraMatrix, distCoeffs, imageSize, 1, imageSize, 0),
			imageSize, CV_16SC2, map1, map2);

		mainMap1 = map1;
		mainMap2 = map2;

		for (size_t i = 0; i < s.imageList.size(); i++)
		{
			view = imread(s.imageList[i], 1);
			if (view.empty())
				continue;
			remap(view, rview, map1, map2, INTER_LINEAR);
			imshow("Image View", rview);
			char c = (char)waitKey();
			if (c == ESC_KEY || c == 'q' || c == 'Q')
				break;
		}
	}
	//! [show_results]

//	return 0;

}

void Panel::CalibrateCameraNoOutput(string sFilePath)
{
	help();

	cout << "NO OUTPUT" << endl;

	//! [file_read]
	Settings s;
	const string inputSettingsFile = sFilePath;
	FileStorage fs(inputSettingsFile, FileStorage::READ); // Read the settings
	if (!fs.isOpened())
	{
		cout << "Could not open the configuration file: \"" << inputSettingsFile << "\"" << endl;
		//		return -1;
	}
	fs["Settings"] >> s;
	fs.release();                                         // close Settings file
	//! [file_read]

	//FileStorage fout("settings.yml", FileStorage::WRITE); // write config as YAML
	//fout << "Settings" << s;

	if (!s.goodInput)
	{
		cout << "Invalid input detected. Application stopping. " << endl;
		//		return -1;
	}

	vector<vector<Point2f> > imagePoints;
	Mat cameraMatrix, distCoeffs;
	Size imageSize;
	int mode = s.inputType == Settings::IMAGE_LIST ? CAPTURING : DETECTION;
	clock_t prevTimestamp = 0;
	const Scalar RED(0, 0, 255), GREEN(0, 255, 0);
	const char ESC_KEY = 27;
	int counter = 1;

	//! [get_input]
	for (;;)
	{
		Mat view;
		bool blinkOutput = false;

		view = s.nextImage();

		//-----  If no more image, or got enough, then stop calibration and show result -------------
		if (mode == CAPTURING && imagePoints.size() >= (size_t)s.nrFrames)
		{
			if (runCalibrationAndSave(s, imageSize, cameraMatrix, distCoeffs, imagePoints))
				mode = CALIBRATED;
			else
				mode = DETECTION;
		}
		if (view.empty())          // If there are no more images stop the loop
		{
			// if calibration threshold was not reached yet, calibrate now
			if (mode != CALIBRATED && !imagePoints.empty())
				runCalibrationAndSave(s, imageSize, cameraMatrix, distCoeffs, imagePoints);
			break;
		}
		//! [get_input]

		imageSize = view.size();  // Format input image.
		if (s.flipVertical)    flip(view, view, 0);

		//! [find_pattern]
		vector<Point2f> pointBuf;

		bool found;
		switch (s.calibrationPattern) // Find feature points on the input format
		{
		case Settings::CHESSBOARD:
			found = findChessboardCorners(view, s.boardSize, pointBuf,
				CALIB_CB_ADAPTIVE_THRESH | CALIB_CB_FAST_CHECK | CALIB_CB_NORMALIZE_IMAGE);
			break;
		case Settings::CIRCLES_GRID:
			found = findCirclesGrid(view, s.boardSize, pointBuf);
			break;
		case Settings::ASYMMETRIC_CIRCLES_GRID:
			found = findCirclesGrid(view, s.boardSize, pointBuf, CALIB_CB_ASYMMETRIC_GRID);
			break;
		default:
			found = false;
			break;
		}
		//! [find_pattern]
		//! [pattern_found]
		if (found)                // If done with success,
		{
			// improve the found corners' coordinate accuracy for chessboard
			if (s.calibrationPattern == Settings::CHESSBOARD)
			{
				Mat viewGray;
				cvtColor(view, viewGray, COLOR_BGR2GRAY);
				cornerSubPix(viewGray, pointBuf, Size(11, 11),
					Size(-1, -1), TermCriteria(TermCriteria::EPS + TermCriteria::COUNT, 30, 0.1));
			}

			if (mode == CAPTURING &&  // For camera only take new samples after delay time
				(!s.inputCapture.isOpened() || clock() - prevTimestamp > s.delay*1e-3*CLOCKS_PER_SEC))
			{
				imagePoints.push_back(pointBuf);
				prevTimestamp = clock();
				blinkOutput = s.inputCapture.isOpened();
			}

			// Draw the corners.
			drawChessboardCorners(view, s.boardSize, Mat(pointBuf), found);
		}
		//! [pattern_found]
		//----------------------------- Output Text ------------------------------------------------
		//! [output_text]
		string msg = (mode == CAPTURING) ? "100/100" :
			mode == CALIBRATED ? "Calibrated" : "Press 'g' to start";
		int baseLine = 0;
		Size textSize = getTextSize(msg, 1, 1, 1, &baseLine);
		Point textOrigin(view.cols - 2 * textSize.width - 10, view.rows - 2 * baseLine - 10);

		if (mode == CAPTURING)
		{
			if (s.showUndistorsed)
				msg = format("%d/%d Undist", (int)imagePoints.size(), s.nrFrames);
			else
				msg = format("%d/%d", (int)imagePoints.size(), s.nrFrames);
		}

		putText(view, msg, textOrigin, 1, 1, mode == CALIBRATED ? GREEN : RED);

		if (blinkOutput)
			bitwise_not(view, view);
		//! [output_text]
		//------------------------- Video capture  output  undistorted ------------------------------
		//! [output_undistorted]
		if (mode == CALIBRATED && s.showUndistorsed)
		{
			Mat temp = view.clone();
			undistort(temp, view, cameraMatrix, distCoeffs);
		}
		//! [output_undistorted]
		
/*		//------------------------------ Show image and check for input commands -------------------
		//! [await_input]
		imshow("Image View", view);
		char key = (char)waitKey(s.inputCapture.isOpened() ? 50 : s.delay);

		if (key == ESC_KEY)
			break;

		if (key == 'u' && mode == CALIBRATED)
			s.showUndistorsed = !s.showUndistorsed;

		if (s.inputCapture.isOpened() && key == 'g')
		{
			mode = CAPTURING;
			imagePoints.clear();
		}
		//! [await_input]
*/
		cout << "Image " << to_string(counter) << " Completed" << endl;
		counter++;
	}

/*	// -----------------------Show the undistorted image for the image list ------------------------
	//! [show_results]
	if (s.inputType == Settings::IMAGE_LIST && s.showUndistorsed)
	{
		Mat view, rview, map1, map2;
		
		initUndistortRectifyMap(cameraMatrix, distCoeffs, Mat(),
			getOptimalNewCameraMatrix(cameraMatrix, distCoeffs, imageSize, 1, imageSize, 0),
			imageSize, CV_16SC2, map1, map2);

		for (size_t i = 0; i < s.imageList.size(); i++)
		{
			view = imread(s.imageList[i], 1);
			if (view.empty())
				continue;
			remap(view, rview, map1, map2, INTER_LINEAR);
			imshow("Image View", rview);
			char c = (char)waitKey();
			if (c == ESC_KEY || c == 'q' || c == 'Q')
				break;
		}
*/
	// -----------------------Calculate Maps ------------------------

	Mat view, rview, map1, map2;

	initUndistortRectifyMap(cameraMatrix, distCoeffs, Mat(),
		getOptimalNewCameraMatrix(cameraMatrix, distCoeffs, imageSize, 1, imageSize, 0),
		imageSize, CV_16SC2, map1, map2);

	mainMap1 = map1;
	mainMap2 = map2;

	//	return 0;

}

void Panel::CalibrateCameraFisheyeNoOutput(string sFilePath)
{
	help();

	cout << "NO OUTPUT" << endl;

	//! [file_read]
	Settings s;
	const string inputSettingsFile = sFilePath;
	FileStorage fs(inputSettingsFile, FileStorage::READ); // Read the settings
	if (!fs.isOpened())
	{
		cout << "Could not open the configuration file: \"" << inputSettingsFile << "\"" << endl;
		//		return -1;
	}
	fs["Settings"] >> s;
	fs.release();                                         // close Settings file
	//! [file_read]

	//FileStorage fout("settings.yml", FileStorage::WRITE); // write config as YAML
	//fout << "Settings" << s;

	if (!s.goodInput)
	{
		cout << "Invalid input detected. Application stopping. " << endl;
		//		return -1;
	}

	vector<vector<Point2f> > imagePoints;
	Mat cameraMatrix, distCoeffs;
	Size imageSize;
	int mode = s.inputType == Settings::IMAGE_LIST ? CAPTURING : DETECTION;
	clock_t prevTimestamp = 0;
	const Scalar RED(0, 0, 255), GREEN(0, 255, 0);
	const char ESC_KEY = 27;

	//! [get_input]
	for (;;)
	{
		Mat view;
		bool blinkOutput = false;

		view = s.nextImage();

		//-----  If no more image, or got enough, then stop calibration and show result -------------
		if (mode == CAPTURING && imagePoints.size() >= (size_t)s.nrFrames)
		{
			if (runCalibrationFisheyeAndSave(s, imageSize, cameraMatrix, distCoeffs, imagePoints))
				mode = CALIBRATED;
			else
				mode = DETECTION;
		}
		if (view.empty())          // If there are no more images stop the loop
		{
			// if calibration threshold was not reached yet, calibrate now
			if (mode != CALIBRATED && !imagePoints.empty())
				runCalibrationFisheyeAndSave(s, imageSize, cameraMatrix, distCoeffs, imagePoints);
			break;
		}
		//! [get_input]

		imageSize = view.size();  // Format input image.
		if (s.flipVertical)    flip(view, view, 0);

		//! [find_pattern]
		vector<Point2f> pointBuf;

		bool found;
		switch (s.calibrationPattern) // Find feature points on the input format
		{
		case Settings::CHESSBOARD:
			found = findChessboardCorners(view, s.boardSize, pointBuf,
				CALIB_CB_ADAPTIVE_THRESH | CALIB_CB_FAST_CHECK | CALIB_CB_NORMALIZE_IMAGE);
			break;
		case Settings::CIRCLES_GRID:
			found = findCirclesGrid(view, s.boardSize, pointBuf);
			break;
		case Settings::ASYMMETRIC_CIRCLES_GRID:
			found = findCirclesGrid(view, s.boardSize, pointBuf, CALIB_CB_ASYMMETRIC_GRID);
			break;
		default:
			found = false;
			break;
		}
		//! [find_pattern]
		//! [pattern_found]
		if (found)                // If done with success,
		{
			// improve the found corners' coordinate accuracy for chessboard
			if (s.calibrationPattern == Settings::CHESSBOARD)
			{
				Mat viewGray;
				cvtColor(view, viewGray, COLOR_BGR2GRAY);
				cornerSubPix(viewGray, pointBuf, Size(11, 11),
					Size(-1, -1), TermCriteria(TermCriteria::EPS + TermCriteria::COUNT, 30, 0.1));
			}

			if (mode == CAPTURING &&  // For camera only take new samples after delay time
				(!s.inputCapture.isOpened() || clock() - prevTimestamp > s.delay*1e-3*CLOCKS_PER_SEC))
			{
				imagePoints.push_back(pointBuf);
				prevTimestamp = clock();
				blinkOutput = s.inputCapture.isOpened();
			}

			// Draw the corners.
			drawChessboardCorners(view, s.boardSize, Mat(pointBuf), found);
		}
		//! [pattern_found]
		//----------------------------- Output Text ------------------------------------------------
		//! [output_text]
		string msg = (mode == CAPTURING) ? "100/100" :
			mode == CALIBRATED ? "Calibrated" : "Press 'g' to start";
		int baseLine = 0;
		Size textSize = getTextSize(msg, 1, 1, 1, &baseLine);
		Point textOrigin(view.cols - 2 * textSize.width - 10, view.rows - 2 * baseLine - 10);

		if (mode == CAPTURING)
		{
			if (s.showUndistorsed)
				msg = format("%d/%d Undist", (int)imagePoints.size(), s.nrFrames);
			else
				msg = format("%d/%d", (int)imagePoints.size(), s.nrFrames);
		}

		putText(view, msg, textOrigin, 1, 1, mode == CALIBRATED ? GREEN : RED);

		if (blinkOutput)
			bitwise_not(view, view);
		//! [output_text]
		//------------------------- Video capture  output  undistorted ------------------------------
		//! [output_undistorted]
		if (mode == CALIBRATED && s.showUndistorsed)
		{
			Mat temp = view.clone();
			fisheye::undistortImage(temp, view, cameraMatrix, distCoeffs);
		}
		//! [output_undistorted]

		/*		//------------------------------ Show image and check for input commands -------------------
		//! [await_input]
		imshow("Image View", view);
		char key = (char)waitKey(s.inputCapture.isOpened() ? 50 : s.delay);

		if (key == ESC_KEY)
		break;

		if (key == 'u' && mode == CALIBRATED)
		s.showUndistorsed = !s.showUndistorsed;

		if (s.inputCapture.isOpened() && key == 'g')
		{
		mode = CAPTURING;
		imagePoints.clear();
		}
		//! [await_input]
		*/
	}

	/*	// -----------------------Show the undistorted image for the image list ------------------------
	//! [show_results]
	if (s.inputType == Settings::IMAGE_LIST && s.showUndistorsed)
	{
	Mat view, rview, map1, map2;

	initUndistortRectifyMap(cameraMatrix, distCoeffs, Mat(),
	getOptimalNewCameraMatrix(cameraMatrix, distCoeffs, imageSize, 1, imageSize, 0),
	imageSize, CV_16SC2, map1, map2);

	for (size_t i = 0; i < s.imageList.size(); i++)
	{
	view = imread(s.imageList[i], 1);
	if (view.empty())
	continue;
	remap(view, rview, map1, map2, INTER_LINEAR);
	imshow("Image View", rview);
	char c = (char)waitKey();
	if (c == ESC_KEY || c == 'q' || c == 'Q')
	break;
	}
	*/
	// -----------------------Calculate Maps ------------------------

	Mat R, P, view, rview, map1, map2;

	fisheye::estimateNewCameraMatrixForUndistortRectify(cameraMatrix, distCoeffs, imageSize, Mat(), P);

	R.convertTo(R, CV_64F);

	fisheye::initUndistortRectifyMap(cameraMatrix, distCoeffs, R, P,
		imageSize, CV_16SC2, map1, map2);

	mainMap1 = map1;
	mainMap2 = map2;


	//	return 0;

}

