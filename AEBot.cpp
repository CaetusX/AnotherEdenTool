#include <Windows.h>
#include <vector>
#include <string>
#include <iostream>
#include <random>
#include <utility>
#include <algorithm>
#include <fstream>
#include <math.h>
#include <time.h>
#include <ctime>
#include <chrono>

#include <opencv2/opencv.hpp>
#include <opencv2/text/ocr.hpp>

#include "AEBot.h"

using namespace cv;
using namespace std;
using namespace cv::text;

HWND m_window;

std::random_device dev;
std::mt19937 rng;
std::uniform_int_distribution<std::mt19937::result_type> slideSleepRand;
std::uniform_int_distribution<std::mt19937::result_type> slideRand;
std::uniform_int_distribution<std::mt19937::result_type> slideDistanceRand;
std::uniform_int_distribution<std::mt19937::result_type> boolRand;
std::uniform_int_distribution<std::mt19937::result_type> slideLClick;
std::uniform_int_distribution<std::mt19937::result_type> longSleepRand;

void ltrimString(string& str)
{
	str.erase(str.begin(), std::find_if(str.begin(), str.end(), [](int ch) {
		return !std::isspace(ch);
		}));
};

void rtrimString(string& str)
{
	str.erase(std::find_if(str.rbegin(), str.rend(), [](int ch) {
		return !std::isspace(ch);
		}).base(), str.end());
};

BOOL CALLBACK EnumWindowsProc(_In_ HWND hwnd, _In_ LPARAM lParam)
{
	DWORD dSize = MAX_PATH;
	TCHAR buffer[MAX_PATH] = { 0 };
	DWORD dwProcId = 0;

	GetWindowThreadProcessId(hwnd, &dwProcId);

	HANDLE hProc = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, dwProcId);
	if (hProc)
	{
		QueryFullProcessImageName(hProc, 0, buffer, &dSize);
		CloseHandle(hProc);

		string windowTitle(buffer);

		string finalWindow = windowTitle.substr(windowTitle.rfind("\\") + 1, string::npos);

		if (finalWindow.compare((*((pair<string*, string*>*)(lParam))->first)) == 0)
		{
			GetWindowText(hwnd, buffer, MAX_PATH);
			windowTitle = buffer;
			if (windowTitle.compare((*((pair<string*, string*>*)(lParam))->second)) == 0)
			{
				m_window = hwnd;
				return false;
			}
		}
	}

	return true;
}

BOOL CALLBACK EnumChildWindowsProc(_In_ HWND hwnd, _In_ LPARAM lParam)
{
	TCHAR buffer[MAX_PATH] = { 0 };
	GetWindowText(hwnd, buffer, MAX_PATH);
	string windowTitle = (string)buffer;

	if (windowTitle.compare((*((string*)(lParam)))) == 0)
	{
		m_window = hwnd;
		return false;
	}

	return true;
}

CAEBot::CAEBot(HWND pParent)
{
	M_WIDTH = 1745.0;
	M_HEIGHT = 981.0;
	M_WIDTH_1280 = 1280.0;
	M_HEIGHT_720 = 720.0;
	M_ABOVE_MENU = 800.0;

	m_msdThreshold = 10000;
	m_loadTime = 3000;
	m_IsDebug_Key = false;
	m_IsDebug_Path = false;
	m_IsDebug_Fighting = false;
	m_IsDebug_Grinding = false;
	m_IsDebug_Fishing = false;
	m_IsDebug_Grasta = false;
	m_IsDebug_LOM = false;
	m_IsDebug_Setting = false;
	m_IsDebug_Platform = false;

	m_IsPrint = false;

	ocrNumbers = "1234567890,";
	ocrLetters = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ.'()";

	m_Locs_Acteul = { {900, 506}, {675, 377}, {1131, 392}, {657, 643}, {1130, 648}, };
	m_Locs_Baruoki = { {842, 508}, {632, 409}, {1047, 402}, {630, 634}, {1046, 623}, };
	m_Locs_DragonPalace = { {870, 483}, {654, 402}, {1055, 399}, {673, 580}, {1069, 596}, };
	m_Locs_Elzion = { {909, 517}, {691, 397}, {1154, 429}, {673, 643}, {1166, 659}, };
	m_Locs_DimensionRift = { {887, 481}, {604, 470}, {1112, 471} };
	m_Locs_KiraBeach = { {936, 533}, {697, 389}, {1180, 404}, {705, 686}, {1173, 687} };
	m_Locs_RucyanaSands = { {862, 494}, {650, 398}, {1080, 403}, {632, 603}, {1103, 622} };
	m_Locs_Vasu = { {879, 492}, {626, 395}, {1133, 399}, {612, 598}, {1166, 614} };
	m_Locs_Igoma = { {880, 510}, {600, 420}, {1135, 425}, {630, 610}, {1120, 620} };
	m_Locs_Moonlight = { {920, 435}, {635, 280}, {1215, 300}, {625, 580}, {1215, 575} };
	m_Locs_AncientBattlefield = { {890, 475}, {645, 385}, {1115, 385}, {660, 570}, {1135, 575} };
	m_Locs_ZolPlains = { {850, 515}, {560, 410}, {1115, 400}, {545, 640}, {885, 675} };
	m_Locs_Default = { {850, 490}, {700, 430}, {1000, 430}, {710, 570}, {880, 570} };

	m_Skill_Normal = 0;
	m_Skill_Exchange = 5;
	m_Skill_Exchange_A = 5;
	m_Skill_Exchange_B = 6;
	m_Skill_AF = 7;
	m_Skill_for_AF = 3;
	m_CharacterFrontline = 4;

	m_currentMonsterVec = NULL;
	m_hasHorror = false;
	m_resValue = status_NoError;
	m_botMode = initialMode;

	m_Action_Interval = 3000;
	m_Fast_Action_Interval = 200;
	m_Walk_Distance_Ratio = 1.0;
	m_Smart_DownUp_Interval = 200;
	m_Smart_DownUp_Threshold = 700;
}

CAEBot::~CAEBot()
{
}

void CAEBot::SetStatus(Status_Code statusCode)
{
	m_resValue = statusCode;
}

Status_Code CAEBot::GetStatus()
{
	return 	m_resValue;
}

void CAEBot::dbgMsg(int debugLevel) 
{
	if (debugLevel)
	{
		cout << "[" << timeString() << "] " << m_debugMsg;
	}
}

void CAEBot::outputMsg()
{
	string str(m_debugMsg);
	m_outputMsg = str;
}

char* CAEBot::timeString()
{
	struct tm timeinfo;
	time_t currenttime;

	currenttime = time(NULL);
	localtime_s(&timeinfo, &currenttime);
	strftime(m_timeString, sizeof(m_timeString), "%Y-%m-%d_%H-%M-%S", &timeinfo);

	return m_timeString;
}

string CAEBot::GetOutputMsg()
{
	return m_outputMsg;
}

Bot_Mode CAEBot::GetMode()
{
	return m_botMode;
}

void CAEBot::SetMode(Bot_Mode botMode)
{
	m_botMode = botMode;
}

int CAEBot::GetEmulatorNumber()
{
	return m_emulatorNumber;
}

void CAEBot::SetEmulator(int emulatorIndex)
{
	m_emulatorIndex = emulatorIndex;
}

string CAEBot::GetEmulatorName(int emulatorIndex)
{
	return m_emulatorList[emulatorIndex].name;
}

void CAEBot::bitBltWholeScreen()
{
	BitBlt(hDest, 0, 0, m_width, m_height, hdc, 0, 0, SRCCOPY);
}

void CAEBot::copyPartialPic(Mat& partialPic, int cols, int rows, int x, int y)
{
	x = (int) round(x * m_widthPct);
	y = (int) round(y * m_heightPct);
	cols = (int) round(cols * m_widthPct);
	rows = (int) round(rows * m_heightPct);
	m_BitbltPic(cv::Rect(x, y, cols, rows)).copyTo(partialPic);
}

string CAEBot::runOCR(Mat& pic)
{
	vector<Rect>   boxes;
	vector<string> words;
	vector<float>  confidences;
	string output;
	Mat newPic;
	cvtColor(pic, newPic, COLOR_BGRA2BGR);
	ocr->run(newPic, output, &boxes, &words, &confidences, OCR_LEVEL_TEXTLINE);
	if (words.empty())
		return "";
	return words[0].substr(0, words[0].size() - 1);
}

string CAEBot::getText(Mat& pic)
{
	ocr->setWhiteList(ocrLetters);
	return runOCR(pic);
}

int CAEBot::getNumber(Mat& pic)
{
	ocr->setWhiteList(ocrNumbers);
	string strNum = runOCR(pic);

	//Remove any commas
	int pos = (int) strNum.find(',');
	while (pos != string::npos)
	{
		strNum.erase(pos, 1);
		pos = (int) strNum.find(',');
	}

	return stoi(strNum);
}

string CAEBot::ocrPictureText(int columns, int rows, int x, int y)
{
	bitBltWholeScreen();
	cv::Mat partialPic;
	copyPartialPic(partialPic, columns, rows, x, y);
	return getText(partialPic);
}

int CAEBot::ocrPictureNumber(int columns, int rows, int x, int y)
{
	bitBltWholeScreen();
	cv::Mat partialPic;
	copyPartialPic(partialPic, columns, rows, x, y);
	return getNumber(partialPic);
}

pair<int, int> CAEBot::findIcon(Mat& tmp)
{
	bitBltWholeScreen();
	int result_cols = m_BitbltPic.cols - tmp.cols + 1;
	int result_rows = m_BitbltPic.rows - tmp.rows + 1;
	Mat result;
	result.create(result_rows, result_cols, CV_32FC1);

	matchTemplate(m_BitbltPic, tmp, result, TM_SQDIFF_NORMED);

	Point minLoc, maxLoc;
	double minVal, maxVal;

	minMaxLoc(result, &minVal, &maxVal, &minLoc, &maxLoc, Mat());

	return make_pair(minLoc.x, minLoc.y);
}

pair<int, int> CAEBot::findIconInRegion(Mat& tmp, int cols, int rows, int x, int y)
{
	Mat imagePicCrop;
	bitBltWholeScreen();
	copyPartialPic(imagePicCrop, cols, rows, x, y);

	int result_cols = imagePicCrop.cols - tmp.cols + 1;
	int result_rows = imagePicCrop.rows - tmp.rows + 1;
	Mat result;
	result.create(result_rows, result_cols, CV_32FC1);

	matchTemplate(imagePicCrop, tmp, result, TM_SQDIFF_NORMED);

	Point minLoc, maxLoc;
	double minVal, maxVal;

	minMaxLoc(result, &minVal, &maxVal, &minLoc, &maxLoc, Mat());

	double target_x = minLoc.x + x * m_widthPct;
	double target_y = minLoc.y + y * m_heightPct;

	return make_pair((int) target_x, (int) target_y);
}

bool CAEBot::compareImage(string imageID, bool toSearch)
{
	Mat imagePicCrop, target_image;
	double MSD1;

	int target_x = 0;
	int target_y = 0;
	int target_w = m_width;
	int target_h = m_height;

	for (auto k = 0; k < m_DynamicImage.size(); k++)
	{
		if (imageID.compare(m_DynamicImage[k].id) == 0)
		{
			target_image = m_DynamicImage[k].image;
			target_x = m_DynamicImage[k].coordx;
			target_y = m_DynamicImage[k].coordy;
			target_w = m_DynamicImage[k].width;
			target_h = m_DynamicImage[k].height;
			break;
		}
	}

	bitBltWholeScreen();
	if (toSearch)
	{
		pair <int, int> iconlocation = findIcon(target_image);

		snprintf(m_debugMsg, 1024, "compareImage %d %d\n", iconlocation.first, iconlocation.second);
		dbgMsg(m_IsDebug_Platform);

		copyPartialPic(imagePicCrop, target_w, target_h, (int) (iconlocation.first / m_widthPct), (int) (iconlocation.second / m_heightPct));
	}
	else 
	{
		copyPartialPic(imagePicCrop, target_w, target_h, target_x, target_y); //
	}

	MSD1 = cv::norm(target_image, imagePicCrop);
	MSD1 = (MSD1 * MSD1 / target_image.total());
	
	snprintf(m_debugMsg, 1024, "compareImage %s %f (%d)\n", imageID.c_str(), MSD1, m_msdThreshold);
	dbgMsg(m_IsDebug_Platform);

	return (MSD1 <= m_msdThreshold); // something wrong with trap set up window
}

pair<bool, pair<int, int>> CAEBot::findClickInRegion(string findString, int cols, int rows, int x, int y)
{
	int target_w, target_h, target_x, target_y;
	Mat target_image, imagePicCrop;
	pair<bool, pair<int, int>> returnicon;

	returnicon.first = false;

	for (auto k = 0; k < m_DynamicImage.size(); k++)
	{
		if (findString.compare(m_DynamicImage[k].id) == 0)
		{
			target_image = m_DynamicImage[k].image;
			target_x = m_DynamicImage[k].coordx;
			target_y = m_DynamicImage[k].coordy;
			target_w = m_DynamicImage[k].width;
			target_h = m_DynamicImage[k].height;
			returnicon.first = true;
			break;
		}
	}

	if (!returnicon.first)
	{
		returnicon.second = { m_width + 1, m_height + 1 };
		return returnicon;
	}

	pair<int, int> iconLoc;
	if (cols == 0 || rows == 0)
		iconLoc = findIcon(target_image);
	else
		iconLoc = findIconInRegion(target_image, cols, rows, x, y);

	returnicon.second.first = iconLoc.first + (int)round(target_w * m_widthPct / 2);
	returnicon.second.second = iconLoc.second + (int)round(target_h * m_heightPct / 2);

	copyPartialPic(imagePicCrop, target_w, target_h, (int) (iconLoc.first / m_widthPct), (int) (iconLoc.second / m_heightPct)); //

	double MSD1 = cv::norm(target_image, imagePicCrop);
	MSD1 = (MSD1 * MSD1 / target_image.total());

	returnicon.first = (MSD1 < m_msdThreshold);

	snprintf(m_debugMsg, 1024, "findClickInRegion %x %d %d\n", returnicon.first, returnicon.second.first, returnicon.second.second);
	dbgMsg(m_IsDebug_Platform);

	return returnicon;
}

void CAEBot::leftClick(int x, int y, int sTime, bool changeLoc)
{
	// will change coordination system to the current one, except for those findClickInRegion
	if (changeLoc)
	{
		x = (int)round(x * m_widthPct);
		y = (int)round(y * m_heightPct);
	}

	snprintf(m_debugMsg, 1024, "LeftClick %d %d\n", x, y);
	dbgMsg(m_IsDebug_Platform);

	SendMessage(m_window, WM_MOUSEACTIVATE, 0, MAKELPARAM(HTCLIENT, WM_LBUTTONDOWN));
	SendMessage(m_window, WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(x, y));
	Sleep(10);
	SendMessage(m_window, WM_LBUTTONUP, 0, MAKELPARAM(x, y));

	Sleep(sTime);
}

// will always change coordination system to the current one
void CAEBot::leftClick(pair<int, int>& coord, int sTime)
{
	leftClick(coord.first, coord.second, sTime);
}

void CAEBot::drag(Direction_Info botDirection, int slideDistance, int xStart, int yStart, int sleepTime, int scrollRatio)
{
	snprintf(m_debugMsg, 1024, "drag %d %d %d %d\n", botDirection, slideDistance, xStart, yStart);
	dbgMsg(m_IsDebug_Platform);

	xStart = (int) round(xStart * m_widthPct);
	yStart = (int) round(yStart * m_heightPct);
	int delta = 0;
	int deviation = 3;

	switch (botDirection)
	{
	case RIGHT:
		deviation = (int) round(deviation * m_heightPct);
		slideDistance = (int) round(slideDistance * m_widthPct);
		xStart = (boolRand(rng) ? xStart + slideLClick(rng) : xStart - slideLClick(rng));
		yStart = (boolRand(rng) ? yStart + slideLClick(rng) : yStart - slideLClick(rng));
		SendMessage(m_window, WM_LBUTTONDOWN, 0, MAKELPARAM(xStart, yStart));
		Sleep(1);

		for (int i = 0; i < slideDistance / scrollRatio; ++i)
		{
			if (boolRand(rng) && delta < deviation)
				delta += boolRand(rng);
			else if (delta > -deviation)
				delta -= boolRand(rng);

			SendMessage(m_window, WM_MOUSEMOVE, MK_LBUTTON, MAKELPARAM(xStart - i * scrollRatio, yStart + delta));
			Sleep(1);
		}

		SendMessage(m_window, WM_LBUTTONUP, 0, MAKELPARAM(xStart - (slideDistance - 1), yStart + delta));
		break;
	case LEFT:
		deviation = (int) round(deviation * m_heightPct);
		slideDistance = (int) round(slideDistance * m_widthPct);
		xStart = (boolRand(rng) ? xStart + slideLClick(rng) : xStart - slideLClick(rng));
		yStart = (boolRand(rng) ? yStart + slideLClick(rng) : yStart - slideLClick(rng));
		SendMessage(m_window, WM_LBUTTONDOWN, 0, MAKELPARAM(xStart, yStart));
		Sleep(1);

		for (int i = 0; i < slideDistance / scrollRatio; ++i)
		{
			if (boolRand(rng) && delta < deviation)
				delta += boolRand(rng);
			else if (delta > -deviation)
				delta -= boolRand(rng);

			SendMessage(m_window, WM_MOUSEMOVE, MK_LBUTTON, MAKELPARAM(xStart + i * scrollRatio, yStart + delta));
			Sleep(1);
		}

		SendMessage(m_window, WM_LBUTTONUP, 0, MAKELPARAM(xStart + (slideDistance - 1), yStart + delta));
		break;
	case UP:
		deviation = (int) round(deviation * m_widthPct);
		slideDistance = (int) round(slideDistance * m_heightPct);
		xStart = (boolRand(rng) ? xStart + slideLClick(rng) : xStart - slideLClick(rng));
		yStart = (boolRand(rng) ? yStart + slideLClick(rng) : yStart - slideLClick(rng));
		SendMessage(m_window, WM_LBUTTONDOWN, 0, MAKELPARAM(xStart, yStart));
		Sleep(1);

		for (int i = 0; i < slideDistance / scrollRatio; ++i)
		{
			if (boolRand(rng) && delta < deviation)
				delta += boolRand(rng);
			else if (delta > -deviation)
				delta -= boolRand(rng);

			SendMessage(m_window, WM_MOUSEMOVE, MK_LBUTTON, MAKELPARAM(xStart + delta, yStart + i * scrollRatio));
			Sleep(1);
		}

		SendMessage(m_window, WM_LBUTTONUP, 0, MAKELPARAM(xStart + delta, yStart + (slideDistance - 1)));
		break;
	case DOWN:
		deviation = (int) round(deviation * m_widthPct);
		slideDistance = (int) round(slideDistance * m_heightPct);
		xStart = (boolRand(rng) ? xStart + slideLClick(rng) : xStart - slideLClick(rng));
		yStart = (boolRand(rng) ? yStart + slideLClick(rng) : yStart - slideLClick(rng));
		SendMessage(m_window, WM_LBUTTONDOWN, 0, MAKELPARAM(xStart, yStart));
		Sleep(1);

		for (int i = 0; i < slideDistance / scrollRatio; ++i)
		{
			if (boolRand(rng) && delta < deviation)
				delta += boolRand(rng);
			else if (delta > -deviation)
				delta -= boolRand(rng);

			SendMessage(m_window, WM_MOUSEMOVE, MK_LBUTTON, MAKELPARAM(xStart + delta, yStart - i * scrollRatio));
			Sleep(1);
		}

		SendMessage(m_window, WM_LBUTTONUP, 0, MAKELPARAM(xStart + delta, yStart - (slideDistance - 1)));
		break;
	}

	Sleep(sleepTime);
}

void CAEBot::dragMap(Direction_Info botDirection, int slideDistance)
{
	switch (botDirection)
	{
	case RIGHT:
		drag(RIGHT, slideDistance, 1600, 300);
		break;
	case LEFT:
		drag(LEFT, slideDistance, 185, 300);
		break;
	case UP:
		drag(UP, slideDistance, 850, 350);
		break;
	case DOWN:
		drag(DOWN, slideDistance, 850, 850);
		break;
	}
}

void CAEBot::walk(Direction_Info botDirection, int time, int sleepTime)
{
	int xStart = m_xCenter;
	int yStart = m_yCenter;
	int slideDistance = 100;

	if (time < slideDistance)
		time = slideDistance;

	snprintf(m_debugMsg, 1024, "walk %d %d %d\n", botDirection, time, sleepTime);
	dbgMsg(m_IsDebug_Platform);

	//Start walking left
	SendMessage(m_window, WM_MOUSEACTIVATE, 0, MAKELPARAM(HTCLIENT, WM_LBUTTONDOWN));
	SendMessage(m_window, WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(xStart, yStart));
	Sleep(10);

	switch (botDirection)
	{
	case LEFT:
		for (int i = 0; i < slideDistance; ++i)
		{
			SendMessage(m_window, WM_MOUSEMOVE, MK_LBUTTON, MAKELPARAM(xStart - i, yStart));
			Sleep(1);
		}

		Sleep(time- slideDistance);
		SendMessage(m_window, WM_LBUTTONUP, 0, MAKELPARAM(xStart - (slideDistance - 1), yStart));
		break;

	case RIGHT:
		for (int i = 0; i < slideDistance; ++i)
		{
			SendMessage(m_window, WM_MOUSEMOVE, MK_LBUTTON, MAKELPARAM(xStart + i, yStart));
			Sleep(1);
		}

		Sleep(time- slideDistance);
		SendMessage(m_window, WM_LBUTTONUP, 0, MAKELPARAM(xStart + (slideDistance - 1), yStart));
		break;

	case DOWN:
		SendMessage(m_window, WM_MOUSEMOVE, MK_LBUTTON, MAKELPARAM(xStart, yStart + slideDistance));
		Sleep(slideDistance);

		Sleep(time- slideDistance);
		SendMessage(m_window, WM_LBUTTONUP, 0, MAKELPARAM(xStart, yStart + (slideDistance - 1)));
		break;

	case UP:
		SendMessage(m_window, WM_MOUSEMOVE, MK_LBUTTON, MAKELPARAM(xStart, yStart - slideDistance));
		Sleep(slideDistance);

		Sleep(time - slideDistance);
		SendMessage(m_window, WM_LBUTTONUP, 0, MAKELPARAM(xStart, yStart - (slideDistance - 1)));
		break;

	}

	Sleep(sleepTime);
}

bool CAEBot::inBattle()
{
	Mat partialPic;

	/*
	bitBltWholeScreen();
	copyPartialPic(partialPic, 106, 39, 77, 37);
	bool inBattleResult = (getText(partialPic).compare("Status") == 0);
	*/

	bool inBattleStatus = compareImage("Battle Status");
	bool inBattleAttack = compareImage("Attack Dark") || compareImage("Attack Light");

	if (inBattleStatus && inBattleAttack)
	{
		snprintf(m_debugMsg, 1024, "in battle %x %x\n", inBattleStatus, inBattleAttack);
		dbgMsg(m_IsDebug_Fighting);
	}
	else 
	{
		snprintf(m_debugMsg, 1024, "not in battle %x %x\n", inBattleStatus, inBattleAttack);
		dbgMsg(m_IsDebug_Fighting);
	}

	return inBattleStatus && inBattleAttack;
}

bool CAEBot::endBattle()
{
	Mat partialPic1, partialPic2;
	bitBltWholeScreen();

	copyPartialPic(partialPic1, 150, 70, 20, 40);
	bool endBattleResult1 = (getText(partialPic1).compare("Items") == 0);

	copyPartialPic(partialPic2, 400, 70, 660, 80);
	bool endBattleResult2 = (getText(partialPic2).find("Got") != string::npos);

	if (endBattleResult1 || endBattleResult2)
	{
		snprintf(m_debugMsg, 1024, "end battle %x %x\n", endBattleResult1, endBattleResult2);
		dbgMsg(m_IsDebug_Fighting);
	}

	return (endBattleResult1 || endBattleResult2);
}

Status_Code CAEBot::smartWorldMap(pair<int, int>& coord)
{
	time_t currenttime, startingtime;
	startingtime = time(NULL);

	leftClick(coord, 500);

	while (!compareImage("World Map"))
	{
		if (m_resValue == status_Stop)
			return m_resValue;

		currenttime = time(NULL);
		auto timegap = difftime(currenttime, startingtime);
		if (timegap > m_Time_Out) // return if timeout
		{
			snprintf(m_debugMsg, 1024, "smartWorldMap timeout %d\n", (int)timegap);
			dbgMsg(m_IsDebug_Platform);
			return status_Timeout;
		}
		else
		{
			Sleep(500);
		}
	}

	Sleep(1000);
	return status_NoError;
}

Status_Code CAEBot::smartMiniMap(pair<int, int>& coord)
{
	time_t currenttime, startingtime;
	startingtime = time(NULL);

	leftClick(coord, 500);

	while (!compareImage("Mini Map"))
	{
		if (m_resValue == status_Stop)
			return m_resValue;

		currenttime = time(NULL);
		auto timegap = difftime(currenttime, startingtime);
		if (timegap > m_Time_Out) // return if timeout
		{
			snprintf(m_debugMsg, 1024, "smartMiniMap timeout %d\n", (int)timegap);
			dbgMsg(m_IsDebug_Platform);
			return status_Timeout;
		}
		else
		{
			Sleep(500);
		}
	}

	Sleep(1000);
	return status_NoError;
}

Status_Code CAEBot::smartLoadMap(pair<int, int>& coord)
{
	time_t currenttime, startingtime;
	startingtime = time(NULL);

	leftClick(coord, 500);

	while (!compareImage("YES", true))
	{
		if (m_resValue == status_Stop)
			return m_resValue;

		currenttime = time(NULL);
		auto timegap = difftime(currenttime, startingtime);
		if (timegap > m_Time_Out) // return if timeout
		{
			snprintf(m_debugMsg, 1024, "smartLoadMap timeout %d\n", (int)timegap);
			dbgMsg(m_IsDebug_Platform);
			return status_Timeout;
		}
		else
		{
			Sleep(500);
		}
	}
	leftClick(m_Button_Yes, 500);

	return sleepLoadTime();
}

Status_Code CAEBot::smartDownUp(Direction_Info updownDirection, Direction_Info leftrightDirection)
{
	time_t currenttime, startingtime;
	startingtime = time(NULL);

	walk(updownDirection, 100, m_Smart_DownUp_Interval);

	while(compareImage("Menu"))
	{
		if (m_resValue == status_Stop)
			return m_resValue;

		snprintf(m_debugMsg, 1024, "smartDownUp %x %x\n", updownDirection, leftrightDirection);
		dbgMsg(m_IsDebug_Platform);

		currenttime = time(NULL);
		auto timegap = difftime(currenttime, startingtime);
		if (timegap > m_Time_Out) // return if timeout
		{
			snprintf(m_debugMsg, 1024, "smartDownUp timeout %d\n", (int)timegap);
			dbgMsg(m_IsDebug_Platform);
			return status_Timeout;
		}
		else
		{
			walk(leftrightDirection, 100, m_Smart_DownUp_Interval);

			walk(updownDirection, 100, m_Smart_DownUp_Interval);
		}
	}

	snprintf(m_debugMsg, 1024, "smartDownUp %x %x successful!\n", updownDirection, leftrightDirection);
	dbgMsg(m_IsDebug_Platform);

	Sleep(m_Action_Interval);
	return status_NoError;
}

Status_Code CAEBot::sleepLoadTime()
{
	Sleep(m_loadTime);

	time_t currenttime, startingtime;
	startingtime = time(NULL);

	while (!compareImage("Menu"))
	{
		if (m_resValue == status_Stop)
			return m_resValue;

		currenttime = time(NULL);
		auto timegap = difftime(currenttime, startingtime);
		if (timegap > m_Time_Out) // return if timeout
		{
			snprintf(m_debugMsg, 1024, "sleepLoadTime timeout %d\n", (int)timegap);
			dbgMsg(m_IsDebug_Path);
			return status_Timeout;
		}
		else 
			Sleep(500);
	}

	return status_NoError;
}

Status_Code CAEBot::fightUntilEnd()
{
	Sleep(1000);

	if (!inBattle()) // not a fight
	{
		return status_NotFight;
	}

	return engageMobFightNow();
}

Status_Code CAEBot::walkUntilBattle(Direction_Info botdirection)
{
	time_t currenttime, startingtime;

	startingtime = time(NULL);

	while (!inBattle())
	{
		if (m_resValue == status_Stop)
			return m_resValue;

		currenttime = time(NULL);
		auto timegap = difftime(currenttime, startingtime);
		if (timegap > m_Time_Out) // return if timeout
		{
			if (m_IsPrint && m_IsDebug_Fighting) captureScreenNow("walkUntilBattle_timeout");

			snprintf(m_debugMsg, 1024, "walkUntilBattle timeout %d\n", (int) timegap);
			dbgMsg(m_IsDebug_Fighting);

			return status_Timeout;
		}

		switch (botdirection)
		{
		case LEFT:
			walk(LEFT, m_Fight_GrindingStep);
			break;
		case RIGHT:
			walk(RIGHT, m_Fight_GrindingStep);
			break;
		case LR:
			walk(LEFT, m_Fight_GrindingStep);
			walk(RIGHT, m_Fight_GrindingStep);
		case NOWHERE:
		default:
			walk(LEFT, 300, 100);
			walk(RIGHT, 300, 100);
			break;
		}
	}

	return fightUntilEnd();
}

Status_Code CAEBot::engageMobFightNow(int horrorThreshold)
{
	time_t currenttime, lastfighttime;

	int num_loop = (int)m_skillsMobSet.size();
	int iRun = 0;
	vector<int> lastSkillsRow = { 0, 0, 0, 0 };
	bool isThisPSlime = false;

	if (m_IsPrint && m_IsDebug_Fighting) captureScreenNow("Mob");

	if (!inBattle()) // no fight
		return status_NotFight;

	snprintf(m_debugMsg, 1024, "Start a battle...\n");
	dbgMsg(m_IsDebug_Fighting);

	if (m_Fight_GrindingCount)
	{
		m_currentGrindingCounter++;
	}

	if (m_botMode == grindingLOMSlimeMode)
	{
		//check 578, 403, 150x150

		double MSD;
		//Check for battle
		//For 3 seconds, read the screen and compare it to the current monster pics. If a cl1ose enough similarity is found, assume its a regular battle and proceed to auto attack
		//If not, assume its a horror and exit
		Mat partialHorrorPic;
		auto horrorStartTime = chrono::high_resolution_clock::now();
		int lowestMSD = 999999999;
		while (std::chrono::duration_cast<std::chrono::seconds>(chrono::high_resolution_clock::now() - horrorStartTime).count() < 3)
		{
			bitBltWholeScreen();
			copyPartialPic(partialHorrorPic, 150, 150, 578, 403);

			for (auto i = 0; i < m_MonsterVec_LOMPSlime.size(); i++)
			{
				MSD = cv::norm(m_MonsterVec_LOMPSlime[i], partialHorrorPic);
				MSD = MSD * MSD / m_MonsterVec_LOMPSlime[i].total();

				if (MSD < lowestMSD)
				{
					lowestMSD = (int)MSD;
				}
				Sleep(10);
			}
		}

		if (lowestMSD < horrorThreshold) //It should be a plantium slime
		{
			isThisPSlime = true;

			snprintf(m_debugMsg, 1024, "Plantium Slime encountered at [%s]\n", m_currentLocation.c_str());
			dbgMsg(m_IsDebug_LOM);
			outputMsg();
		}
	}

	do
	{
		if (iRun == num_loop && m_Fight_GrindingCount)
		{
			//only non endless grinding needs to count grinding runs, to estimate the MP usage
			m_currentGrindingCounter++;
		}

		//loop through the skills set
		iRun = iRun % num_loop;

		for (auto j = 0; j < m_CharacterFrontline; j++)
		{
			int iSkill = m_skillsMobSet[iRun][j];
			if (iSkill > m_Skill_AF)
				iSkill = 0;

			if (iSkill == m_Skill_AF)
			{
				break;
			}

			if (iSkill != m_Skill_Normal || iSkill != lastSkillsRow[j])
			{
				lastSkillsRow[j] = iSkill;

				leftClick(m_Button_Characters[j].xyPosition, m_Fast_Action_Interval);
				if (iSkill == m_Skill_Exchange_A || iSkill == m_Skill_Exchange_B)
				{
					leftClick(m_Button_Skills[m_Skill_Exchange].xyPosition, m_Fast_Action_Interval);
					leftClick(m_Button_Characters[iSkill - m_Skill_Exchange + m_CharacterFrontline].xyPosition, m_Fast_Action_Interval);
				}
				else
					leftClick(m_Button_Skills[iSkill].xyPosition, m_Fast_Action_Interval);

				// Click normal skill in case the skill is blocked
				leftClick(m_Button_Skills[0].xyPosition, m_Fast_Action_Interval);
				// Click front buttom in case someone is defeated
				//leftClick(m_Button_Skills[4].xyPosition, m_Fast_Action_Interval);
				// Click somewhere else in case someone is disabled
				leftClick(m_Button_PassThrough, m_Fast_Action_Interval);
			}
		}

		snprintf(m_debugMsg, 1024, "[%d] Ready to attack with %x %x %x %x\n", m_currentGrindingCounter, lastSkillsRow[0], lastSkillsRow[1], lastSkillsRow[2], lastSkillsRow[3]);
		dbgMsg(m_IsDebug_Fighting);

		leftClick(m_Button_Attack, 500);

		snprintf(m_debugMsg, 1024, "After attack\n");
		dbgMsg(m_IsDebug_Fighting);

		lastfighttime = time(NULL);
		while (!endBattle())
		{
			currenttime = time(NULL);
			auto timegap = difftime(currenttime, lastfighttime);
			if (timegap > m_Time_Out) // return if timeout
			{
				break;
			}

			if (inBattle())
			{
				break;
			}

			Sleep(200);
		}

		iRun++;
	} while (inBattle());

	snprintf(m_debugMsg, 1024, "A battle ends\n");
	dbgMsg(m_IsDebug_Fighting);

	for (auto i = 0; i < 6; i++)
	{
		leftClick(m_Button_PassThrough, m_Fast_Action_Interval);
	}

	Sleep(m_Action_Interval);

	snprintf(m_debugMsg, 1024, "End of a battle\n");
	dbgMsg(m_IsDebug_Fighting);

	return status_NoError;
}

Status_Code CAEBot::engageHorrorFightNow(bool restoreHPMP)
{
	cv::Mat partialPic;
	string textToCheck;

	time_t currenttime, lastfighttime;

	Sleep(1000);

	if (!inBattle()) // not horror
	{
		leftClick(m_Button_PassThrough, m_Fast_Action_Interval);
		return status_NotFight;
	}

	while (!inBattle())
	{
		if (m_resValue == status_Stop)
			return m_resValue;

		walk(LEFT, 5000, 200);
		walk(RIGHT, 5000, 200);
	}

	int num_loop = (int)m_skillsHorrorSet.size();

	Mat imagePicCrop;

	if (m_IsPrint && m_IsDebug_Fighting) captureScreenNow("Horror");

	int iRun = 0;
	bool isAFon = false;
	do
	{
		bitBltWholeScreen();

		// check whether the fight failed
		copyPartialPic(partialPic, 770, 90, 490, 420);
		textToCheck = getText(partialPic);
		if (textToCheck.find("Stones") != string::npos) // battle failed
		{
			leftClick(690, 740); // Click Quit

			bitBltWholeScreen();
			copyPartialPic(partialPic, 1000, 160, 380, 360);
			textToCheck = getText(partialPic);
			if (textToCheck.find("location") != string::npos)
			{
				leftClick(1080, 600); // Click Quit
				Sleep(7000);
				return status_FightFail;
			}
		}

		// check whether AF bar is full
		copyPartialPic(imagePicCrop, 730, 50, 880, 30); // AF bar 880, 30, 730, 50.  AF bar 860, 10, 770, 90
		double MSD1;
		MSD1 = cv::norm(afBarFullPic, imagePicCrop);
		MSD1 = MSD1 * MSD1 / afBarFullPic.total();

		if (MSD1 < m_Fight_AFFullThreshold || isAFon)
		{
			snprintf(m_debugMsg, 1024, "AF\n");
			dbgMsg(m_IsDebug_Fighting);

			leftClick(m_Button_AF, m_Fight_AFInterval);

			double MSD2 = 10000;
			do
			{
				leftClick(m_Button_Skills[m_Skill_for_AF].xyPosition, m_Fight_AFInterval);

				bitBltWholeScreen();
				copyPartialPic(imagePicCrop, 730, 50, 880, 30); // AF bar 880, 30, 730, 50.  AF bar 860, 10, 770, 90
				MSD2 = cv::norm(afBarEmptyPic, imagePicCrop);
				MSD2 = MSD2 * MSD2 / afBarEmptyPic.total();
			} while (MSD2 > 400);

			snprintf(m_debugMsg, 1024, "AF Done\n");
			dbgMsg(m_IsDebug_Fighting);

			if (m_IsPrint && m_IsDebug_Fighting) captureScreenNow("AFDone");

			iRun = 0;
			isAFon = false;
		}
		else 
		{
			iRun = iRun % num_loop;

			snprintf(m_debugMsg, 1024, "Attack with %d %d %d %d\n", m_skillsHorrorSet[iRun][0], m_skillsHorrorSet[iRun][1], m_skillsHorrorSet[iRun][2], m_skillsHorrorSet[iRun][3]);
			dbgMsg(m_IsDebug_Fighting);

			for (auto j = 0; j < m_CharacterFrontline; j++)
			{
				int iSkill = m_skillsHorrorSet[iRun][j];
				if (iSkill > m_Skill_AF)
					iSkill = 0;

				if (iSkill == m_Skill_AF)
				{
					isAFon = true;
					break;
				}

				leftClick(m_Button_Characters[j].xyPosition, 500);
				if (iSkill == m_Skill_Exchange_A || iSkill == m_Skill_Exchange_B)
				{
					leftClick(m_Button_Skills[m_Skill_Exchange].xyPosition, 500);
					leftClick(m_Button_Characters[iSkill - m_Skill_Exchange + m_CharacterFrontline].xyPosition, 500);
				}
				else
					leftClick(m_Button_Skills[iSkill].xyPosition, 500);

				// Click normal skill in case the skill is blocked
				leftClick(m_Button_Skills[0].xyPosition, 500);
				// Click front buttom in case someone is defeated
				//leftClick(m_Button_Skills[4].xyPosition, 500;
				// Click somewhere else in case someone is disabled
				leftClick(m_Button_PassThrough, m_Fast_Action_Interval);
			}

			if (isAFon) 
			{
				continue;
			}

			leftClick(m_Button_Attack, 500); // Fight Horror
			iRun++;
		}

		lastfighttime = time(NULL);

		while (!endBattle())
		{
			currenttime = time(NULL);
			auto timegap = difftime(currenttime, lastfighttime);
			if (timegap > m_Time_Out) // return if timeout
			{
				break;
			}

			if (inBattle())
			{
				break;
			}

			Sleep(500);
		}

	} while (inBattle());

	if (m_IsPrint && m_IsDebug_Fighting) captureScreenNow("Result");

	Sleep(m_Action_Interval);
	
	for (auto i = 0; i < 3; i++)
	{
		leftClick(m_Button_PassThrough, m_Fast_Action_Interval);
	}

	Sleep(m_Action_Interval);

	for (auto i = 0; i < 3; i++)
	{
		leftClick(m_Button_PassThrough, m_Fast_Action_Interval);
	}

	leftClick(m_Button_Yes);

	if (restoreHPMP)
	{
		// restore HP & MP
		leftClick(m_Button_Menu);
		leftClick(m_Button_Food);
		leftClick(m_Button_Yes, m_Action_Interval);
		leftClick(m_Button_Yes);
		leftClick(m_Button_Yes);
		leftClick(m_Button_Yes);
	}

	return status_NoError;
}

void CAEBot::fish(vector<pair<int, int>>& sections, int m_msdThreshold, int horrorThreshold)
{
	//Strategy is to quadrisect the lake and toss into each of the four sections and center
	int catchIndex = 0, horrorIndex = 0, monsterIndex = 0;

	time_t currenttime, lastCatch;

	lastCatch = time(NULL);

	snprintf(m_debugMsg, 1024, "Starting fishing at [%s]\n", m_currentLocation.c_str());
	dbgMsg(m_IsDebug_Fishing);
	outputMsg();

	Sleep(1000);

	bitBltWholeScreen();
	Mat lakeImg = m_BitbltPic.clone();

	for (int i = 0; i < m_currentBaitsToUse->size(); ++i)
	{
		if (m_baitList[(*m_currentBaitsToUse)[i]].hasBait == false) //If we have 0 of current bait, go to next bait
			continue;

		changeBait((*m_currentBaitsToUse)[i]);

		//shuffle(sections.begin() + 1, sections.end(), default_random_engine((unsigned int) std::chrono::system_clock::now().time_since_epoch().count()));

		bool increaseSection = false;
		int lastSection = 0;
		for (int j = 0; j < sections.size(); )
		{
			bitBltWholeScreen();
			cv::Mat partialPic;
			copyPartialPic(partialPic, 286, 55, 1070, 893);  // 995 - 785 = 210, 695 - 655 = 40, 785, 655, 
			string textChangeBait = getText(partialPic);
			if (textChangeBait.find("Change") == string::npos) //No Change Bait
			{
				if (m_IsPrint && m_IsDebug_Fishing) captureScreenNow("NOTFISHING");

				leftClick(m_Button_X);
				return;
			}

			//Cast line
			leftClick(sections[j].first, sections[j].second);

			string status = ocrPictureText(1242, 71, 257, 80);
			if (status.find("bait") != string::npos) //Out of bait
			{
				m_baitList[(*m_currentBaitsToUse)[i]].hasBait = false;

				do //Look for a bait that we actually have
				{
					++i;
				} while (i < m_currentBaitsToUse->size() && m_baitList[(*m_currentBaitsToUse)[i]].hasBait == false);

				if (i >= m_currentBaitsToUse->size())
					return;

				changeBait((*m_currentBaitsToUse)[i]);
				leftClick(sections[j].first, sections[j].second);
			}
			else if (status.find("any fish") != string::npos || status.find("box") != string::npos) //Pool is empty or cooler is full
			{
				snprintf(m_debugMsg, 1024, "Leaving [%s] (%d/%d)\n", m_currentLocation.c_str(), horrorIndex, catchIndex);
				dbgMsg(m_IsDebug_Fishing);
				outputMsg();

				return;
			}

			double MSD;
			auto startTime = chrono::high_resolution_clock::now();
			bool noFishCheck = true;

			while (1) //This constantly reads the screen looking for either a certain status, or whether a zoom in has occurred
			{
				//ocrPictureText() is really cpu intensive, so avoid doing it unless necessary
				if (std::chrono::duration_cast<std::chrono::milliseconds>(chrono::high_resolution_clock::now() - startTime).count() >= 11000 && noFishCheck)
				{
					noFishCheck = false;
					status = ocrPictureText(1242, 71, 257, 80);

					if (status.find("no fish") != string::npos)
					{
						increaseSection = true;
						leftClick(sections[j].first, sections[j].second);
						break;
					}
				}

				bitBltWholeScreen();
				MSD = cv::norm(lakeImg, m_BitbltPic);
				MSD = MSD * MSD / lakeImg.total();

				if (MSD > m_msdThreshold) //If the current screen is sufficiently different (high mean square difference) from the normal lake image, then a zoom in has occurred
				{
					Sleep(100); //Emulate human reaction time
					leftClick(sections[j].first, sections[j].second);

					catchIndex++;

					//Click through success or failure
					Sleep(m_Action_Interval);
					leftClick(sections[j].first, sections[j].second);
					Sleep(m_Action_Interval);

					bitBltWholeScreen();
					MSD = cv::norm(lakeImg, m_BitbltPic);
					MSD = MSD * MSD / lakeImg.total();
					if (MSD > m_msdThreshold) //Double, gotta click past it
					{
						leftClick(sections[j].first, sections[j].second);
						Sleep(m_Action_Interval);

						bitBltWholeScreen();
						MSD = cv::norm(lakeImg, m_BitbltPic);
						MSD = MSD * MSD / lakeImg.total();
						if (MSD > m_msdThreshold) //Triple
						{
							leftClick(sections[j].first, sections[j].second);
							Sleep(m_Action_Interval);
						}
					}

					//Check for battle
					bitBltWholeScreen();
					MSD = cv::norm(lakeImg, m_BitbltPic);
					MSD = MSD * MSD / lakeImg.total();
					if (MSD > m_msdThreshold) //Should have returned to normal lake image; if not, its a battle
					{
						Sleep(5000); //Give ample time for battle to fully load

						if (inBattle()) // double check whether it is a battle
						{
							if (m_hasHorror) //Its possibly a horror or lakelord, so need to check to make sure before trying to auto it down
							{
								bool isThisHorror = false;
								if (m_currentMonsterVec != NULL)
								{
									//For 3 seconds, read the screen and compare it to the current monster pics. If a cl1ose enough similarity is found, assume its a regular battle and proceed to auto attack
									//If not, assume its a horror and exit
									Mat partialHorrorPic;
									auto horrorStartTime = chrono::high_resolution_clock::now();
									int lowestMSD = 999999999;
									while (std::chrono::duration_cast<std::chrono::seconds>(chrono::high_resolution_clock::now() - horrorStartTime).count() < 3)
									{
										bitBltWholeScreen();
										copyPartialPic(partialHorrorPic, 140, 140, 420, 380);
										for (int i = 0; i < m_currentMonsterVec->size(); ++i)
										{
											MSD = cv::norm((*m_currentMonsterVec)[i], partialHorrorPic);
											MSD = MSD * MSD / (*m_currentMonsterVec)[i].total();

											if (MSD < lowestMSD)
											{
												lowestMSD = (int)MSD;
												monsterIndex = i;
											}
										}
										Sleep(10);
									}

									if (lowestMSD > horrorThreshold) //If its not a monster, it must be a horror
										isThisHorror = true;
								}
								else
								{
									isThisHorror = true;
								}

								if (isThisHorror)
								{
									snprintf(m_debugMsg, 1024, "Horror or Lake Lord detected at [%s] %d\n", m_currentLocation.c_str(), horrorIndex);
									dbgMsg(m_IsDebug_Fishing);
									outputMsg();

									if (m_Fight_HorrorCount)
									{
										m_resValue = engageHorrorFightNow();
										horrorIndex++;
										
										if (m_resValue != status_NoError) // failed
										{
											snprintf(m_debugMsg, 1024, "Fight Horror or Lake Lord failed!\n");
											dbgMsg(m_IsDebug_Fishing);
											return;
										}
										if (horrorIndex >= m_Fight_HorrorCount) // limit the number of horrors / lake lords to fight
										{
											snprintf(m_debugMsg, 1024, "Leaving [%s] (%d/%d) catched - Horror count reached\n", m_currentLocation.c_str(), horrorIndex, catchIndex);
											dbgMsg(m_IsDebug_Fishing);
											outputMsg();
											return;
										}
									}
									else
										exit(0);
								}
								else
								{
									snprintf(m_debugMsg, 1024, "Fighting at [%s] monster %x\n", m_currentLocation.c_str(), monsterIndex);
									dbgMsg(m_IsDebug_Fishing);
								}
							}

							fightUntilEnd();

							Sleep(5000);

							snprintf(m_debugMsg, 1024, "Fighting ends at [%s] monster %x\n", m_currentLocation.c_str(), monsterIndex);
							dbgMsg(m_IsDebug_Fishing);
							leftClick(m_Button_PassThrough); //Click past fish results screen

							Sleep(5000);

							fishIconClickFunction();

							Sleep(1000);
						} 
						else // not a battle, probably a fish
						{
							leftClick(sections[j].first, sections[j].second);
							Sleep(m_Action_Interval);
						}
					}

					lastCatch = time(NULL);
					lastSection = j;
					break;

				}
				else
				{
					// if wait too long, let's move to other area.
					currenttime = time(NULL);
					auto timegap = difftime(currenttime, lastCatch);
					if (timegap > m_Time_Out * 5)
					{
						snprintf(m_debugMsg, 1024, "Leaving [%s] (%d/%d) catched - Idling\n", m_currentLocation.c_str(), horrorIndex, catchIndex);
						dbgMsg(m_IsDebug_Fishing);
						outputMsg();
						return;
					}
				}

				Sleep(12); //Try to stay around 60 fps read time
			}

			if (increaseSection)
			{
				++j;
				j %= sections.size();
				if (j == lastSection)
					break;
			}

			increaseSection = false;
		}
	}

	snprintf(m_debugMsg, 1024, "Leaving [%s] (%d/%d) catched - All baits tried\n", m_currentLocation.c_str(), horrorIndex, catchIndex);
	dbgMsg(m_IsDebug_Fishing);
	outputMsg();
}

void CAEBot::changeBait(Bait_Type type)
{
	int slotNum = 0;
	if (type > 0)
	{
		//Calculate slot that bait is in
		for (int i = 0; i < type; ++i)
		{
			if (m_baitList[i].hasBait == true)
				++slotNum;
		}
	}

	leftClick(1225, 925);

	if (slotNum > 4)
	{
		drag(DOWN, 74, m_xCenter, m_yCenter);
		for (int i = 5; i < slotNum; ++i)
		{
			drag(DOWN, 100, m_xCenter, m_yCenter);
		}

		leftClick(m_Button_Baits[5].xyPosition.first, m_Button_Baits[5].xyPosition.second);
	}
	else
		leftClick(m_Button_Baits[slotNum].xyPosition.first, m_Button_Baits[slotNum].xyPosition.second);
};

Status_Code CAEBot::goToTargetLocation(vector<pathInfo> pathInfoList)
{
	snprintf(m_debugMsg, 1024, "----> [%s]\n", m_currentLocation.c_str());
	dbgMsg(m_IsDebug_Path);

	for (auto j = 0; j < pathInfoList.size(); j++)
	{
		if (m_resValue == status_Stop)
			return m_resValue;

		string curType = pathInfoList[j].type;
		string curValue1 = pathInfoList[j].value1;
		string curValue2 = pathInfoList[j].value2;

		snprintf(m_debugMsg, 1024, "%s %s %s\n", curType.c_str(), curValue1.c_str(), curValue2.c_str());
		dbgMsg(m_IsDebug_Path);

		m_resValue = status_NoError;
		if (curType.compare("Click") == 0)
		{
			if (!curValue2.empty())
			{
				int x = stoi(curValue1);
				int y = stoi(curValue2);

				x = (int)round(x * M_WIDTH / M_WIDTH_1280);
				y = (int)round(y * M_HEIGHT / M_HEIGHT_720);

				leftClick(x, y);
			}
			else
			{
				if (curValue1.compare("YesButton") == 0)
				{
					leftClick(m_Button_Yes);
				}
				else if (curValue1.compare("MapButton") == 0)
				{
					m_resValue = smartWorldMap(m_Button_Map);
				}
				else if (curValue1.compare("Antiquity") == 0)
				{
					leftClick(m_Button_Antiquity);
				}
				else if (curValue1.compare("Present") == 0)
				{
					leftClick(m_Button_Present);
				}
				else if (curValue1.compare("Future") == 0)
				{
					leftClick(m_Button_Future);
				}
				else if (curValue1.compare("EndOfTimeLoc") == 0)
				{
					leftClick(m_Button_EndOfTimeLoc);
				}
				else if (curValue1.compare("PassThrough") == 0)
				{
					leftClick(m_Button_PassThrough);
				}
				else
				{
					bool isfound = false;

					for (auto i = 0; i < m_Button_MapButtons.size(); i++)
					{
						if (curValue1.compare(m_Button_MapButtons[i].buttonName) == 0)
						{
							isfound = true;

							leftClick(m_Button_MapButtons[i].xyPosition);
							break;
						}
					}

					if (!isfound)
					{
						snprintf(m_debugMsg, 1024, "!!! %s not found\n", curValue1.c_str());
						dbgMsg(m_IsDebug_Path);
					}
				}
			}
		}
		else if (curType.compare("Sleep") == 0) {

			if (curValue1.compare("LoadTime") == 0)
			{
				m_resValue = sleepLoadTime();
			}
			else
			{
				Sleep(stoi(curValue1));
			}
		}
		else if (curType.compare("Walk") == 0) {

			if (curValue1.compare("LEFT") == 0)
			{
				walk(LEFT, int(stoi(curValue2) * m_Walk_Distance_Ratio));
			}
			else if (curValue1.compare("RIGHT") == 0)
			{
				walk(RIGHT, int(stoi(curValue2) * m_Walk_Distance_Ratio));
			}
			else if (curValue1.compare("DOWN") == 0)
			{
				walk(DOWN, int(stoi(curValue2) * m_Walk_Distance_Ratio));
			}
			else if (curValue1.compare("UP") == 0)
			{
				walk(UP, int(stoi(curValue2) * m_Walk_Distance_Ratio));
			}
			else if (curValue1.compare("UntilBattle") == 0)
			{
				if (curValue2.empty())
				{
					m_resValue = walkUntilBattle(LR);
				}
				else if (curValue2.compare("LEFT") == 0)
				{
					m_resValue = walkUntilBattle(LEFT);
				}
				else if (curValue2.compare("RIGHT") == 0)
				{
					m_resValue = walkUntilBattle(RIGHT);
				}
				else if (curValue2.compare("NOWHERE") == 0)
				{
					m_resValue = walkUntilBattle(NOWHERE);
				}
			}

		}
		else if (curType.compare("Load") == 0) 
		{
			if (curValue1.compare("WorldMap") == 0)
			{
				m_resValue = smartWorldMap(m_Button_Map);
			}
			else if (curValue1.compare("MINI") == 0)
			{
				bool isfound = false;

				for (auto i = 0; i < m_Button_MapButtons.size(); i++)
				{
					if (curValue2.compare(m_Button_MapButtons[i].buttonName) == 0)
					{
						isfound = true;
						m_resValue = smartMiniMap(m_Button_MapButtons[i].xyPosition);
						break;
					}
				}

				if (!isfound)
				{
					snprintf(m_debugMsg, 1024, "!!! %s not found\n", curValue2.c_str());
					dbgMsg(m_IsDebug_Path);
				}
			}
			else 
			{
				bool isfound = false;

				for (auto i = 0; i < m_Button_MapButtons.size(); i++)
				{
					if (curValue1.compare(m_Button_MapButtons[i].buttonName) == 0)
					{
						isfound = true;
						m_resValue = smartLoadMap(m_Button_MapButtons[i].xyPosition);

						break;
					}
				}

				if (!isfound)
				{
					snprintf(m_debugMsg, 1024, "!!! %s not found\n", curValue1.c_str());
					dbgMsg(m_IsDebug_Path);
				}
			}
		
		}
		else if (curType.compare("Drag") == 0) {
			if (curValue1.compare("LEFT") == 0)
			{
				dragMap(LEFT, stoi(curValue2));
			}
			else if (curValue1.compare("RIGHT") == 0)
			{
				dragMap(RIGHT, stoi(curValue2));
			}
			else if (curValue1.compare("DOWN") == 0)
			{
				dragMap(DOWN, stoi(curValue2));
			}
			else if (curValue1.compare("UP") == 0)
			{
				dragMap(UP, stoi(curValue2));
			}
		}
		else if (curType.compare("Find") == 0) {
			pair<bool, pair <int, int>> findclickres = findClickInRegion(curValue1, (int)M_WIDTH, (int)M_ABOVE_MENU, 0, 0);
			leftClick(findclickres.second.first, findclickres.second.second, m_Action_Interval, false);
		}
		else if (curType.compare("Fight") == 0) {
			if (curValue1.compare("UntilEnd") == 0)
			{
				fightUntilEnd();
			}
		}
		else if (curType.compare("Task") == 0) {
			if (curValue1.compare("HarpoonFishing") == 0)
			{
				m_resValue = harpoonFunction();
			}
			else if (curValue1.compare("HorrorFishing") == 0)
			{
				m_resValue = harpoonHorror();
			}
			else if (curValue1.compare("TrapFishing") == 0)
			{
				m_resValue = harpoonTrapFunction(curValue2);
			}
		}
		else if (curType.compare("Smart") == 0) {
			Direction_Info updown, leftright;
			if (curValue1.compare("DOWN") == 0)
			{
				updown = DOWN;
			}
			else if (curValue1.compare("UP") == 0)
			{
				updown = UP;
			}
		
			if (curValue2.compare("LEFT") == 0)
			{
				leftright = LEFT;
			}
			else if (curValue2.compare("RIGHT") == 0)
			{
				leftright = RIGHT;
			}

			smartDownUp(updown, leftright);
		}
		else if (curType.compare("Capture") == 0) {
			if (curValue1.compare("Screen") == 0)
			{
				captureScreenNow();
			}
		}

		if (m_resValue == status_Timeout)
			return m_resValue;
	}

	return status_NoError;
}

void CAEBot::goToFishingLocation()
{
	for (int i = 0; i < m_LocationList.size(); i++)
	{
		if (m_LocationList[i].locationName.compare(m_currentLocation) == 0)
		{
			snprintf(m_debugMsg, 1024, "Starting fishing at [%s]\n", m_currentLocation.c_str());
			dbgMsg(m_IsDebug_Fishing);
			outputMsg();

			m_resValue = goToTargetLocation(m_LocationList[i].pathStepsList);
			// no need to care about time out here
		}
	}
}

void CAEBot::goToSpacetimeRift(bool heal)
{
	snprintf(m_debugMsg, 1024, "Space-time Rift [%x]\n", heal);
	dbgMsg(m_IsDebug_Path);

	smartWorldMap(m_Button_Map);
	leftClick(m_Button_EndOfTimeLoc);
	leftClick(m_Button_SpacetimeRift);
	leftClick(m_Button_SpacetimeRift);
	leftClick(m_Button_Yes);
	Sleep(m_loadTime);

	if (heal)
	{
		leftClick(m_Button_Tree);
		leftClick(m_Button_Tree);
		leftClick(m_Button_Yes, m_Action_Interval);
		leftClick(m_Button_Yes);
		leftClick(m_Button_Yes);
		leftClick(m_Button_Yes);
	}
}

void CAEBot::goToFishVendor()
{
	goToSpacetimeRift(false);
	m_currentLocation = "Fish Vendor";
	goToFishingLocation();
	buyBaitsFromVendor();
}

void CAEBot::goToHarpoonVendor()
{
	goToSpacetimeRift();
	m_currentLocation = "Harpoon Vendor";
	goToFishingLocation();

	for (auto i = 480; i < 1540; i = i + 30) //click through text
		leftClick(i, 290, 100);

	buyBaitsFromVendor();
}

void CAEBot::buyBaitsFromVendor()
{
	string currText = "";

	for (auto i = 0; i < 20; i++) //click through text
		leftClick(m_Button_PassThrough, 100);

	leftClick(930, 850);

	//Only 5 bait slots are fully on screen at a time, with the 6th partially visible. For every bait after 6, scroll down until new bait occupies bottom slot and then attempt to buy
	for (auto i = 0, j = 0; i < m_baitList.size(); i++)
	{
		if (i > 4)
		{
			if (i == 5)
				drag(DOWN, 60, 1200, 300);
			else
				drag(DOWN, 135, 1200, 300);
			leftClick(m_Button_Baits[5].xyPosition.first, m_Button_Baits[5].xyPosition.second);
		}
		else
			leftClick(m_Button_Baits[i].xyPosition.first, m_Button_Baits[i].xyPosition.second);

		//If the next one we've done is the same as the last, we've reached the end of the list
		string txt = ocrPictureText(577, 51, 100, 102);

		snprintf(m_debugMsg, 1024, "Checking Bait %s(%s) %x %d\n", txt.c_str(), m_baitList[j].baitName.c_str(), m_baitList[j].hasBait, m_baitList[j].baitNumber);
		dbgMsg(m_IsDebug_Fishing);

		if (currText.compare(txt) == 0)
			break;
		currText = txt;

		while (txt.compare(m_baitList[j].baitName) != 0 && j < m_baitList.size()) //You don't get the baits in order, so some may be missing depending on your progression (blabber sardines)
		{
			snprintf(m_debugMsg, 1024, "Checking (%s) %x %d\n", m_baitList[j].baitName.c_str(), m_baitList[j].hasBait, m_baitList[j].baitNumber);
			dbgMsg(m_IsDebug_Fishing);

			j++;
		}

		//Get amount currently held
		int held = ocrPictureNumber(141, 30, 500, 165);
		int numToBuy = m_baitList[j].baitNumber - held;

		snprintf(m_debugMsg, 1024, "Buy Bait %s %d\n", txt.c_str(), m_baitList[j].baitNumber);
		dbgMsg(m_IsDebug_Fishing);

		if (numToBuy > 0)
		{
			//Get amount of fish points owned
			int fishPoints = ocrPictureNumber(193, 28, 1346, 10);
			//Get cost of current selected bait
			double fpCost = m_baitList[j].baitPrice;
			if (fpCost < fishPoints) //Can we even afford one?
			{
				if (numToBuy * fpCost >= fishPoints) //If we don't have enough fish points to buy the max set, then buy as many as we can
					numToBuy = (int)floor(fishPoints / fpCost);
			}

			int thisTimeToBuy;
			snprintf(m_debugMsg, 1024, "Bait [%d]: %d holding %d to buy %d. buying...\n", i, m_baitList[j].baitNumber, held, numToBuy);
			dbgMsg(m_IsDebug_Fishing);

			for (; numToBuy > 0; numToBuy -= thisTimeToBuy)
			{
				if (numToBuy > 99)
					thisTimeToBuy = 99;
				else
					thisTimeToBuy = numToBuy;

				if (thisTimeToBuy > 0)
				{
					for (int k = 0; k < thisTimeToBuy - 1; k++)
						leftClick(555, 665, 500);

					leftClick(350, 850);
					leftClick(1070, 650);
					leftClick(880, 575);

					held = ocrPictureNumber(141, 30, 500, 165);
				}
			}
		}

		m_baitList[j].hasBait = held > 0;
	}

	leftClick(m_Button_X);
}

void CAEBot::fishFunction()
{
	goToSpacetimeRift();
	goToFishingLocation();

	if (m_currentLocation.compare("Kira Beach") == 0)
	{
		//no monster
		fish(m_Locs_KiraBeach);
		leftClick(m_Button_Leave);
	}
	else if (m_currentLocation.compare("Baruoki") == 0)
	{
		m_hasHorror = true;
		m_currentMonsterVec = &m_MonsterVec_Baruoki;

		m_currentFishIconLoc = make_pair(711, 366);

		fish(m_Locs_Baruoki);
		leftClick(m_Button_Leave);

		m_hasHorror = false;
		m_currentMonsterVec = NULL;
	}
	else if (m_currentLocation.compare("Naaru Uplands") == 0)
	{
		//has monster, no horror
		m_currentFishIconLoc = make_pair(775, 450);

		fish(m_Locs_Baruoki);
		leftClick(m_Button_Leave);
	}
	else if (m_currentLocation.compare("Acteul") == 0)
	{
		m_hasHorror = true;
		m_currentMonsterVec = &m_MonsterVec_Acteul;

		m_currentFishIconLoc = make_pair(1029, 367);

		fish(m_Locs_Acteul);
		leftClick(m_Button_Leave);

		m_hasHorror = false;
		m_currentMonsterVec = NULL;
	}
	else if (m_currentLocation.compare("Elzion Airport") == 0)
	{
		m_hasHorror = true;
		m_currentMonsterVec = &m_MonsterVec_Elzion;

		m_currentFishIconLoc = make_pair(1004, 448);

		fish(m_Locs_Elzion);
		leftClick(m_Button_Leave);

		m_hasHorror = false;
		m_currentMonsterVec = NULL;
	}
	else if (m_currentLocation.compare("Zol Plains") == 0)
	{
		//no monster
		//m_currentFishIconLoc = make_pair(935, 455);

		fish(m_Locs_ZolPlains);
		leftClick(m_Button_Leave);
	}
	else if (m_currentLocation.compare("Lake Tillian") == 0)
	{
		//has monster, no horror
		m_currentFishIconLoc = make_pair(741, 448);

		fish(m_Locs_Acteul);
		leftClick(m_Button_Leave);
	}
	else if (m_currentLocation.compare("Vasu Mountains") == 0)
	{
		m_hasHorror = true;
		m_currentMonsterVec = &m_MonsterVec_Vasu;

		m_currentFishIconLoc = make_pair(727, 442);

		fish(m_Locs_Vasu);
		leftClick(m_Button_Leave);

		m_hasHorror = false;
		m_currentMonsterVec = NULL;
	}
	else if (m_currentLocation.compare("Karek Swampland") == 0)
	{
		//has monster, no horror
		m_currentFishIconLoc = make_pair(623, 448);

		fish(m_Locs_Acteul);
		leftClick(m_Button_Leave);
	}
	else if (m_currentLocation.compare("Moonlight Forest") == 0)
	{
		//no monster
		m_currentFishIconLoc = make_pair(435, 360);

		fish(m_Locs_Moonlight);
		leftClick(m_Button_Leave);
	}
	else if (m_currentLocation.compare("Ancient Battlefield") == 0)
	{
		//no monster
		m_currentFishIconLoc = make_pair(1300, 455);

		fish(m_Locs_AncientBattlefield);
		leftClick(m_Button_Leave);
	}
	else if (m_currentLocation.compare("Snake Neck Igoma") == 0)
	{
		//no monster
		m_currentFishIconLoc = make_pair(685, 450);

		fish(m_Locs_Igoma);
		leftClick(m_Button_Leave);
	}
	else if (m_currentLocation.compare("Rinde") == 0)
	{
		//has monster, no horror
		m_currentFishIconLoc = make_pair(970, 368);

		fish(m_Locs_KiraBeach);
		leftClick(m_Button_Leave);
	}
	else if (m_currentLocation.compare("Serena Coast") == 0)
	{
		m_hasHorror = true;
		m_currentMonsterVec = &m_MonsterVec_Serena;

		m_currentFishIconLoc = make_pair(796, 448);

		fish(m_Locs_Acteul);
		leftClick(m_Button_Leave);

		m_hasHorror = false;
		m_currentMonsterVec = NULL;
	}
	else if (m_currentLocation.compare("Rucyana Sands") == 0)
	{
		m_hasHorror = true;
		m_currentMonsterVec = &m_MonsterVec_Rucyana;

		m_currentFishIconLoc = make_pair(744, 411);

		fish(m_Locs_RucyanaSands);
		leftClick(m_Button_Leave);

		m_hasHorror = false;
		m_currentMonsterVec = NULL;
	}
	else if (m_currentLocation.compare("Last Island") == 0)
	{
		m_hasHorror = true;
		m_currentMonsterVec = &m_MonsterVec_LastIsland;

		fish(m_Locs_KiraBeach);
		leftClick(m_Button_Leave);

		m_hasHorror = false;
		m_currentMonsterVec = NULL;
	}
	else if (m_currentLocation.compare("Nilva") == 0)
	{
		//has monster, no horror
		m_currentFishIconLoc = make_pair(1068, 400);

		fish(m_Locs_Elzion);
		leftClick(m_Button_Leave);
	}
	else if (m_currentLocation.compare("Man-Eating Swamp") == 0)
	{
		//has monster, no horror
		fish(m_Locs_Acteul, 5500);
		leftClick(m_Button_Leave);
	}
	else if (m_currentLocation.compare("Charol Plains") == 0)
	{
		//has monster, no horror
		m_currentFishIconLoc = make_pair(840, 456);

		fish(m_Locs_Baruoki);
		leftClick(m_Button_Leave);
	}
	else if (m_currentLocation.compare("Dimension Rift") == 0)
	{
		m_hasHorror = true;
		m_currentMonsterVec = &m_MonsterVec_DimensionRift;

		m_currentFishIconLoc = make_pair(408, 345);

		fish(m_Locs_DimensionRift, 10000, 2500);
		leftClick(m_Button_Leave);

		m_hasHorror = false;
		m_currentMonsterVec = NULL;
	}
	else if (m_currentLocation.compare("Dragon Palace - Outer Wall Plum") == 0)
	{
		//no monster
		//m_currentFishIconLoc = make_pair(450, 400);

		fish(m_Locs_DragonPalace);
		leftClick(m_Button_Leave);
	}
	else if (m_currentLocation.compare("Dragon Palace Past - Outer Wall Plum") == 0)
	{
		//no monster
		//m_currentFishIconLoc = make_pair(450, 400);

		fish(m_Locs_DragonPalace);
		leftClick(m_Button_Leave);
	}
	else if (m_currentLocation.compare("Dragon Palace - Outer Wall Bamboo") == 0)
	{
		//no monster
		//m_currentFishIconLoc = make_pair(1240, 410);

		fish(m_Locs_DragonPalace);
		leftClick(m_Button_Leave);
	}
	else if (m_currentLocation.compare("Dragon Palace Past - Outer Wall Bamboo") == 0)
	{
		//no monster
		//m_currentFishIconLoc = make_pair(1240, 410);

		fish(m_Locs_DragonPalace);
		leftClick(m_Button_Leave);
	}
	else if (m_currentLocation.compare("Dragon Palace - Inner Wall") == 0)
	{
		//no monster
		//m_currentFishIconLoc = make_pair(751, 407);

		fish(m_Locs_DragonPalace);
		leftClick(m_Button_Leave);
	}
	else if (m_currentLocation.compare("Dragon Palace Past - Inner Wall") == 0)
	{
		//no monster
		//m_currentFishIconLoc = make_pair(603, 408);

		fish(m_Locs_DragonPalace);
		leftClick(m_Button_Leave);
	}
	else if (m_currentLocation.compare("Dragon Palace - Outer Wall Pine") == 0)
	{
		//no monster
		//m_currentFishIconLoc = make_pair(630, 408);

		fish(m_Locs_DragonPalace);
		leftClick(m_Button_Leave);
	}
	else if (m_currentLocation.compare("Dragon Palace Past - Outer Wall Pine") == 0)
	{
		//no monster
		//m_currentFishIconLoc = make_pair(630, 405);

		fish(m_Locs_DragonPalace);
		leftClick(m_Button_Leave);
	}

}

void CAEBot::fishIconClickFunction()
{
	//In Man Eating Swamp, any battle shifts your position, so the fish icon location must be found again
	if (m_currentLocation.compare("Kira Beach") == 0 || m_currentLocation.compare("Last Island") == 0)
	{
		pair<bool, pair<int, int>> findclickres = findClickInRegion("FishIcon", (int)M_WIDTH, (int)M_ABOVE_MENU, 0, 0);
		leftClick(findclickres.second.first, findclickres.second.second, m_Action_Interval, false);
	}
	else if (m_currentLocation.compare("Man-Eating Swamp") == 0)
	{
		pair<bool, pair<int, int>> findclickres = findClickInRegion("SwampFishIcon", (int)M_WIDTH, (int)M_ABOVE_MENU, 0, 0);
		leftClick(findclickres.second.first, findclickres.second.second, m_Action_Interval, false);
	}
	else
	{
		leftClick(m_currentFishIconLoc.first, m_currentFishIconLoc.second, m_Action_Interval);
	}
}

Status_Code CAEBot::harpoonFunction()
{
	string txt;
	auto counter = 0;
	time_t currenttime, startingtime;

	bitBltWholeScreen();

	bool resultShown = false;
	pair<int, int> harpoonfish;

	pair<bool, pair <int, int>> findclickres;
	
	findclickres = findClickInRegion("HarpoonFish", (int)M_WIDTH, 200, 0, 240);
	harpoonfish = findclickres.second;

	leftClick(harpoonfish.first, harpoonfish.second, m_Action_Interval, false);
	Sleep(m_Harpoon_Interval); // wait for the result to load

	while (!compareImage("HarpoonResult"))
	{
		// five attemps, failed
		if (counter >= 4)
		{
			snprintf(m_debugMsg, 1024, "Failed %d attemps...\n", counter + 1);
			dbgMsg(m_IsDebug_Fishing);

			if (m_IsPrint && m_IsDebug_Fishing) captureScreenNow("harpoonFunction");

			return status_NoHarpoonCatch;
		}

		// it is possible the fish escapes
		txt = ocrPictureText(1100, 80, 330, 75);
		// try to search the harpoon
		if (txt.find("harpoon") != string::npos || txt.find("the") != string::npos)
		{
			snprintf(m_debugMsg, 1024, "No catch \"%s\"....\n", txt.c_str());
			dbgMsg(m_IsDebug_Fishing);

			leftClick(m_Button_PassThrough, m_Action_Interval);
			return status_HarpoonCatchFail;
		}

		findclickres = findClickInRegion("HarpoonFish", (int)M_WIDTH, 200, 0, 240);
		harpoonfish = findclickres.second;

		//snprintf(m_debugMsg, 1024, "Harpoon fish attempt [%d] %d %d....\n", counter + 1, harpoonfish.first, harpoonfish.second);
		//dbgMsg(m_IsDebug_Fishing);

		leftClick(harpoonfish.first, harpoonfish.second, m_Action_Interval, false);
		Sleep(m_Harpoon_Interval); // wait for the result to load
		counter++;
	}

	counter = 0;
	startingtime = time(NULL);

	while (compareImage("HarpoonResult"))
	{
		counter++;

		if (counter >= 3)
		{
			leftClick(m_Button_PassThrough, m_Action_Interval);
			break;
		}

		leftClick(m_Button_PassThrough, m_Action_Interval);
		Sleep(m_Harpoon_Interval); // wait for the result to load

		currenttime = time(NULL);
		auto timegap = difftime(currenttime, startingtime);

		if (timegap > m_Time_Out) // return if timeout
		{
			snprintf(m_debugMsg, 1024, "Time out. Stop harpoon at [%s]\n", m_currentLocation.c_str());
			dbgMsg(m_IsDebug_Fishing);
			
			// time out, something wrong, let's go
			return status_Timeout;
		}
	}

	snprintf(m_debugMsg, 1024, "Harpoon fish %d caught\n", counter);
	dbgMsg(m_IsDebug_Fishing);

	for (auto i = 0; i < 3; i++)
	{
		leftClick(m_Button_PassThrough, m_Fast_Action_Interval);
	}

	return status_NoError;
}

Status_Code CAEBot::harpoonHorror()
{

	pair<int, int> harpoonhorror;
	pair<bool, pair <int, int>> findclickres;

	findclickres = findClickInRegion("HarpoonHorror", 1545, 500, 100, 50);
	harpoonhorror = findclickres.second;

	if (!(harpoonhorror.first >= 1390 * m_widthPct && harpoonhorror.second <= 235 * m_heightPct))
	{
		leftClick(harpoonhorror.first, harpoonhorror.second, m_Action_Interval, false);

		Sleep(5000); // wait for the battle to start

		if (inBattle())
		{
			snprintf(m_debugMsg, 1024, "Harpoon Horror encountered at [%s]\n", m_currentLocation.c_str());
			dbgMsg(m_IsDebug_Fishing);
			outputMsg();

			m_resValue = engageHorrorFightNow(false);
			leftClick(m_Button_PassThrough, m_Action_Interval);
			return m_resValue;
		}
		else
			return status_NotFight;
	}
	else
	{ 
		return status_NotFight;
	}
}

Status_Code CAEBot::harpoonMassShooting()
{
	for (auto x = m_Harpoon_Xmin * m_widthPct; x <= m_Harpoon_Xmax * m_widthPct; x = x + m_Harpoon_Xinc * m_widthPct)
	{
		for (auto y = m_Harpoon_Ymin * m_heightPct; y <= m_Harpoon_Ymax * m_heightPct; y = y + m_Harpoon_Yinc * m_heightPct)
		{
			if (m_resValue == status_Stop)
				return m_resValue;

			pair<int, int> harpoonhit;
			harpoonhit.first = (int) x;
			harpoonhit.second = (int) y;
			leftClick(harpoonhit, m_Fast_Action_Interval);
		}
	}

	return status_NoError;
}

Status_Code CAEBot::harpoonTrapFunction(string trapRef)
{
	Mat preHarpoonLImg, preHarpoonRImg, postHarpoonLImg, postHarpoonRImg;;
	pair<int, int> findclicktrap, findclicktraplarge, findclickbait1, findclickbait2, findclickbait3, findclickbait4;
	time_t currenttime, startingtime;
	
	pair<bool, pair <int, int>> findclickres;
	bool findtraplarge;

	findclickres = findClickInRegion("HarpoonTrap", (int)M_WIDTH, (int)(440 * m_heightPct), 0, (int)(160 * m_heightPct));
	findclicktrap = findclickres.second;

	leftClick(findclicktrap.first, findclicktrap.second, m_Action_Interval, false);

	findclickres = findClickInRegion("HarpoonTrapLarge", (int)M_WIDTH, (int)(440 * m_heightPct), 0, (int)(160 * m_heightPct));
	findtraplarge = findclickres.first;
	findclicktraplarge = findclickres.second;

	if (findtraplarge)
	{
		leftClick(findclicktraplarge.first, findclicktraplarge.second, m_Action_Interval, false);

		return harpoonSetTrap(trapRef);
	}
	else 
	{
		string txt = ocrPictureText(900, 65, 415, 48);
		// Less than xx minute(s) before the
		// The alertness is rising. There's
		if (txt.find("before") != string::npos || txt.find("than") != string::npos || txt.find("minute") != string::npos || txt.find("The") != string::npos)
		{
			snprintf(m_debugMsg, 1024, "Trap [%s] \"%s\"\n", trapRef.c_str(), txt.c_str());
			dbgMsg(m_IsDebug_Fishing);
			outputMsg();

			leftClick(m_Button_PassThrough, m_Action_Interval);
			for (auto i = 0; i < 3; i++)
			{
				leftClick(m_Button_PassThrough, m_Fast_Action_Interval);
			}
			return status_HarpoonTrapWaiting; // still waiting for trap
		}
		else
		{
			startingtime = time(NULL);
			// You've caught a total of xx fish
			txt = ocrPictureText(900, 127, 415, 48);
			while (txt.find("total") == string::npos && txt.find("caught") == string::npos && txt.find("fish") == string::npos)
			{
				if (m_resValue == status_Stop)
					return m_resValue;
			
				currenttime = time(NULL);
				auto timegap = difftime(currenttime, startingtime);

				if (timegap > m_Time_Out) // return if timeout
				{
					snprintf(m_debugMsg, 1024, "Time out for Trap [%s]\n", trapRef.c_str());
					dbgMsg(m_IsDebug_Fishing);

					if (m_IsPrint && m_IsDebug_Fishing) captureScreenNow("HarpoonTrapFunction timeout result");

					leftClick(m_Button_PassThrough, m_Action_Interval);
					for (auto i = 0; i < 3; i++)
					{
						leftClick(m_Button_PassThrough, m_Fast_Action_Interval);
					}
					return status_Timeout; // time out, something wrong, let's go
				}
				else 
				{
					Sleep(500);
				}

				txt = ocrPictureText(900, 127, 415, 48);
			}

			snprintf(m_debugMsg, 1024, "Harvest! Trap [%s] \"%s\"\n", trapRef.c_str(), txt.c_str());
			dbgMsg(m_IsDebug_Fishing);
			outputMsg();

			leftClick(m_Button_PassThrough, m_Action_Interval);

			for (auto i = 0; i < 3; i++)
			{
				leftClick(m_Button_PassThrough, m_Fast_Action_Interval);
			}

			leftClick(findclicktrap.first, findclicktrap.second, m_Action_Interval, false);

			leftClick(findclicktrap.first, findclicktrap.second, m_Action_Interval, false);

			return harpoonSetTrap(trapRef);
		}
	}
}

/*

	Mat preHarpoonLImg, preHarpoonRImg, postHarpoonLImg, postHarpoonRImg;;
	double MSD1, MSD2;
	pair<int, int> findclicktrap, findclicktraplarge, findclickbait1, findclickbait2, findclickbait3, findclickbait4;
	time_t currenttime, startingtime;

	findclicktrap = findClickInRegion("HarpoonTrap", (int) M_WIDTH, (int) (440 * m_heightPct), 0, (int) (160 * m_heightPct));

	bitBltWholeScreen();
	copyPartialPic(preHarpoonLImg, 200, 210, (int) (findclicktrap.first / m_widthPct - 250), 190);
	copyPartialPic(preHarpoonRImg, 200, 210, (int) (findclicktrap.first / m_widthPct + 50), 190);

	leftClick(findclicktrap.first, findclicktrap.second, m_Action_Interval, false);

	bitBltWholeScreen();
	copyPartialPic(postHarpoonLImg, 200, 210, (int)(findclicktrap.first / m_widthPct - 250), 190);
	copyPartialPic(postHarpoonRImg, 200, 210, (int)(findclicktrap.first / m_widthPct + 50), 190);

	MSD1 = cv::norm(preHarpoonLImg, postHarpoonLImg);
	MSD1 = (MSD1 * MSD1 / preHarpoonLImg.total());
	MSD2 = cv::norm(preHarpoonRImg, postHarpoonRImg);
	MSD2 = (MSD2 * MSD2 / preHarpoonRImg.total());

	//snprintf(m_debugMsg, 1024, "MSD1 %f MSD2 %f\n", MSD1, MSD2);
	//dbgMsg(m_IsDebug_Fishing);

	if (MSD1 > m_Harpoon_Threshold && MSD2 > m_Harpoon_Threshold)
	{
		findclicktraplarge = findClickInRegion("HarpoonTrapLarge", (int)M_WIDTH, (int)(440 * m_heightPct), 0, (int)(160 * m_heightPct));
		findclickbait1 = findClickInRegion("HarpoonBait1", (int)M_WIDTH, (int)(440 * m_heightPct), 0, (int)(160 * m_heightPct));
		findclickbait2 = findClickInRegion("HarpoonBait2", (int)M_WIDTH, (int)(440 * m_heightPct), 0, (int)(160 * m_heightPct));
		findclickbait3 = findClickInRegion("HarpoonBait3", (int)M_WIDTH, (int)(440 * m_heightPct), 0, (int)(160 * m_heightPct));
		findclickbait4 = findClickInRegion("HarpoonBait4", (int)M_WIDTH, (int)(440 * m_heightPct), 0, (int)(160 * m_heightPct));

		if ((findclicktraplarge.first > findclicktrap.first - 100 * m_widthPct && findclicktraplarge.first < findclicktrap.first + 100 * m_widthPct &&
			findclicktraplarge.second > findclicktrap.second - 100 * m_heightPct && findclicktraplarge.second < findclicktrap.second + 100 * m_heightPct)
			&& ((findclickbait1.first > findclicktrap.first - 220 * m_widthPct && findclickbait1.first < findclicktrap.first + 220 * m_widthPct &&
				findclickbait1.second > findclicktrap.second - 140 * m_heightPct && findclickbait1.second < findclicktrap.second + 20 * m_heightPct)
				|| (findclickbait2.first > findclicktrap.first - 220 * m_widthPct && findclickbait2.first < findclicktrap.first + 220 * m_widthPct &&
					findclickbait2.second > findclicktrap.second - 140 * m_heightPct && findclickbait2.second < findclicktrap.second + 20 * m_heightPct)
				|| (findclickbait3.first > findclicktrap.first - 220 * m_widthPct && findclickbait3.first < findclicktrap.first + 220 * m_widthPct &&
					findclickbait3.second > findclicktrap.second - 140 * m_heightPct && findclickbait3.second < findclicktrap.second + 20 * m_heightPct)
				|| (findclickbait4.first > findclicktrap.first - 220 * m_widthPct && findclickbait4.first < findclicktrap.first + 220 * m_widthPct &&
					findclickbait4.second > findclicktrap.second - 140 * m_heightPct && findclickbait4.second < findclicktrap.second + 20 * m_heightPct)))
		{
			leftClick(findclicktraplarge.first, findclicktraplarge.second, m_Action_Interval, false);

			return harpoonSetTrap(trapRef);
		}
		else
		{
			snprintf(m_debugMsg, 1024, "Something wrong. Abort trap [%s]\n", trapRef.c_str());
			dbgMsg(m_IsDebug_Fishing);

			if (m_IsPrint && m_IsDebug_Fishing) captureScreenNow("HarpoonTrapFunction abort");

			for (auto i = 0; i < 3; i++)
			{
				leftClick(m_Button_PassThrough, m_Fast_Action_Interval);
			}
			return status_HarpoonTrapFail; // something wrong, let's go
		}
	}
	else
	{
		string txt = ocrPictureText(900, 65, 415, 48);
		// Less than xx minute(s) before the
		// The alertness is rising. There's
		if (txt.find("before") != string::npos || txt.find("than") != string::npos || txt.find("minute") != string::npos || txt.find("The") != string::npos)
		{
			snprintf(m_debugMsg, 1024, "Trap [%s] \"%s\"\n", trapRef.c_str(), txt.c_str());
			dbgMsg(m_IsDebug_Fishing);

			for (auto i = 0; i < 3; i++)
			{
				leftClick(m_Button_PassThrough, m_Fast_Action_Interval);
			}
			return status_HarpoonTrapWaiting; // still waiting for trap
		}
		else
		{
			startingtime = time(NULL);
			// You've caught a total of xx fish
			txt = ocrPictureText(900, 127, 415, 48);
			while (txt.find("total") == string::npos && txt.find("caught") == string::npos && txt.find("fish") == string::npos)
			{
				if (m_resValue == status_Stop)
					return m_resValue;

				currenttime = time(NULL);
				auto timegap = difftime(currenttime, startingtime);

				if (timegap > m_Time_Out) // return if timeout
				{
					snprintf(m_debugMsg, 1024, "Time out for Trap [%s]\n", trapRef.c_str());
					dbgMsg(m_IsDebug_Fishing);

					if (m_IsPrint && m_IsDebug_Fishing) captureScreenNow("HarpoonTrapFunction timeout result");

					for (auto i = 0; i < 3; i++)
					{
						leftClick(m_Button_PassThrough, m_Fast_Action_Interval);
					}
					return status_Timeout; // time out, something wrong, let's go
				}
				else
				{
					Sleep(500);
				}

				txt = ocrPictureText(900, 127, 415, 48);
			}

			snprintf(m_debugMsg, 1024, "Harvest! Trap [%s] \"%s\"\n", trapRef.c_str(), txt.c_str());
			dbgMsg(m_IsDebug_Fishing);
			outputMsg();

			leftClick(m_Button_PassThrough, m_Action_Interval);

			for (auto i = 0; i < 3; i++)
			{
				leftClick(m_Button_PassThrough, m_Fast_Action_Interval);
			}

			leftClick(findclicktrap.first, findclicktrap.second, m_Action_Interval, false);

			leftClick(findclicktrap.first, findclicktrap.second, m_Action_Interval, false);

			return harpoonSetTrap(trapRef);
		}
	}
*/

Status_Code CAEBot::harpoonSetTrap(string trapRef)
{
	string txt;

	bool imageresult = compareImage("HarpoonSetupTrap");
	if (!imageresult) // something wrong with trap set up window
	{
		snprintf(m_debugMsg, 1024, "Trap [%s] set up window is wrong\n", trapRef.c_str());
		dbgMsg(m_IsDebug_Fishing);

		if (m_IsPrint && m_IsDebug_Fishing) captureScreenNow("HarpoonSetupTrap");
		leftClick(m_Button_PassThrough, m_Fast_Action_Interval);
		return status_HarpoonTrapFail;
	}

	for (auto i = 0; i < 5; i++)
	{
		// choose the top most trap
		leftClick(1200, 620 - i * 120, 1000);
	}

	// click Select
	leftClick(680, 340);

	txt = ocrPictureText(220, 40, 760, 160);
	if (txt.find("a") == string::npos && txt.find("e") == string::npos)
	{
		leftClick(m_Button_X);
		return status_HarpoonTrapFail;
	}

	for (auto i = 0; i < 4; i++)
	{
		// choose the bottom most bait, 792, 636, 480, 324
		leftClick(780, 792 - i * 156, 1000);
	}

	//click to set up the trap
	leftClick(580, 880);

	snprintf(m_debugMsg, 1024, "Trap [%s] set up done\n", trapRef.c_str());
	dbgMsg(m_IsDebug_Fishing);
	outputMsg();

	leftClick(m_Button_PassThrough, m_Action_Interval);
	for (auto i = 0; i < 3; i++)
	{
		leftClick(m_Button_PassThrough, m_Fast_Action_Interval);
	}
	return status_NoError;
}

Status_Code CAEBot::stateFishing()
{
	snprintf(m_debugMsg, 1024, "Start fishing >>>>>>>>>\n");
	dbgMsg(m_IsDebug_Fishing);

	while (1)
	{
		if (m_resValue == status_Stop)
			return m_resValue;

		loadFishingConfig();
		loadPathConfig();
		loadSettingConfig();

		goToFishVendor();
		for (int i = 0; i < m_fishingSpots.size(); ++i)
		{
			m_currentBaitsToUse = &(m_fishingSpots[i].baitsToUse);
			m_currentLocation = m_fishingSpots[i].locationName;
			fishFunction();
		}
	}
}

Status_Code CAEBot::stateHarpoonFishing()
{
	snprintf(m_debugMsg, 1024, "Start harpoon fishing>>>>>>>>>\n");
	dbgMsg(m_IsDebug_Fishing);

	while (1)
	{
		for (auto j = 0; j < m_Harpoon_Loop; j++)
		{
			loadFishingConfig();
			loadPathConfig();
			loadSettingConfig();

			if (m_resValue == status_Stop)
				return m_resValue;

			if (!m_Harpoon_SkipVendor && j == 0)
				goToHarpoonVendor();

			for (int i = 0; i < m_harpoonSpots.size(); ++i)
			{
				m_currentLocation = m_harpoonSpots[i].first;
				goToFishingLocation();
			}
		}
	}
}

Status_Code CAEBot::stateSilverHitBell(Bot_Mode silverHitBellstate)
{
	Mat bellPicCrop;
	int MSD1;
	int lowMSD = (int)99999999999;

	while (1)
	{
		if (m_resValue == status_Stop)
			return m_resValue;

		int i = 0;

		SendMessage(m_window, WM_MOUSEACTIVATE, 0, MAKELPARAM(HTCLIENT, WM_LBUTTONDOWN));
		SendMessage(m_window, WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(m_xCenter, m_yCenter));
		Sleep(1);
		SendMessage(m_window, WM_LBUTTONUP, 0, MAKELPARAM(m_xCenter, m_yCenter));
		Sleep(3000);

		SendMessage(m_window, WM_MOUSEACTIVATE, 0, MAKELPARAM(HTCLIENT, WM_LBUTTONDOWN));
		SendMessage(m_window, WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(m_xCenter, m_yCenter));
		Sleep(1);
		SendMessage(m_window, WM_LBUTTONUP, 0, MAKELPARAM(m_xCenter, m_yCenter));

		Sleep(1200);

		SendMessage(m_window, WM_MOUSEACTIVATE, 0, MAKELPARAM(HTCLIENT, WM_LBUTTONDOWN));
		SendMessage(m_window, WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(m_xCenter, m_yCenter));
		Sleep(1);
		SendMessage(m_window, WM_LBUTTONUP, 0, MAKELPARAM(m_xCenter, m_yCenter));
		i++;

		Sleep(1350);

		SendMessage(m_window, WM_MOUSEACTIVATE, 0, MAKELPARAM(HTCLIENT, WM_LBUTTONDOWN));
		SendMessage(m_window, WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(m_xCenter, m_yCenter));
		Sleep(1);
		SendMessage(m_window, WM_LBUTTONUP, 0, MAKELPARAM(m_xCenter, m_yCenter));
		i++;

		for (; i < 4;) {
			Sleep(1350);

			SendMessage(m_window, WM_MOUSEACTIVATE, 0, MAKELPARAM(HTCLIENT, WM_LBUTTONDOWN));
			SendMessage(m_window, WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(m_xCenter, m_yCenter));
			Sleep(1);
			SendMessage(m_window, WM_LBUTTONUP, 0, MAKELPARAM(m_xCenter, m_yCenter));
			i++;
		}

		int j = 750;
		Sleep(j);
		while (1)
		{
			if (m_resValue == status_Stop)
				return m_resValue;

			if (silverHitBellstate == silverHitBell30Mode && i > 30)
				break;
			else if (silverHitBellstate == silverHitBell999Mode && i > 999)
				return status_NoError;

			if (j > 2000)
				break;

			Sleep(1);
			bitBltWholeScreen();

			copyPartialPic(bellPicCrop, 160, 80, 1095, 323);
			MSD1 = (int)cv::norm(hitBellPic, bellPicCrop);
			MSD1 = MSD1 * MSD1 / (int)hitBellPic.total();

			if (MSD1 < m_msdThreshold)
			{
				int tosleep = 1;
				Sleep(tosleep);
				SendMessage(m_window, WM_MOUSEACTIVATE, 0, MAKELPARAM(HTCLIENT, WM_LBUTTONDOWN));
				SendMessage(m_window, WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(m_xCenter, m_yCenter));
				Sleep(1);
				SendMessage(m_window, WM_LBUTTONUP, 0, MAKELPARAM(m_xCenter, m_yCenter));
				j = 800;
				Sleep(j);
				i++;
			}
			else
			{
				j = j + 25;
				Sleep(25);
			}
		}

		leftClick(m_xCenter, m_yCenter); // miss the bell
		Sleep(10000);

		leftClick(m_xCenter, m_yCenter);
		Sleep(4000);
		leftClick(m_xCenter, m_yCenter);
		leftClick(m_xCenter, m_yCenter);
		Sleep(2000);

		// Click Icon
		leftClick(613, 286, 5000);
		// Click Msg
		leftClick(m_xCenter, m_yCenter);
		// Click Yes
		leftClick(m_Button_Yes, 5000);
		// After Click Yes 1
		leftClick(m_xCenter, m_yCenter, 5000);
		// After Click Yes 2
		leftClick(m_xCenter, m_yCenter, 5000);
	}

}

Status_Code CAEBot::stateJumpRopeRatle()
{
	Mat jmpRopePicCrop;
	int MSD1;
	int lowMSD = (int) 99999999999;

	bool check = false;

	SendMessage(m_window, WM_MOUSEACTIVATE, 0, MAKELPARAM(HTCLIENT, WM_LBUTTONDOWN));
	SendMessage(m_window, WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(m_xCenter, m_yCenter));
	Sleep(1);
	SendMessage(m_window, WM_LBUTTONUP, 0, MAKELPARAM(m_xCenter, m_yCenter));

	Sleep(1000);

	int q = 0;
	while (1)
	{
		if (m_resValue == status_Stop)
			return m_resValue;

		Sleep(1);
		bitBltWholeScreen();

		if (check) {
			copyPartialPic(jmpRopePicCrop, 21, 109, 710, 575);
			MSD1 = (int) cv::norm(jmpRopePic2, jmpRopePicCrop);
			MSD1 = MSD1 * MSD1 / (int) jmpRopePic2.total();

			if (MSD1 > 1000)
			{
				check = false;

				SendMessage(m_window, WM_MOUSEACTIVATE, 0, MAKELPARAM(HTCLIENT, WM_LBUTTONDOWN));
				SendMessage(m_window, WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(m_xCenter, m_yCenter));
				Sleep(1);
				SendMessage(m_window, WM_LBUTTONUP, 0, MAKELPARAM(m_xCenter, m_yCenter));
				if (q++ == 10001)
					break;
			}
		}
		else
		{
			copyPartialPic(jmpRopePicCrop, 25, 79, 745, 305);
			MSD1 = (int) cv::norm(jmpRopePic1, jmpRopePicCrop);
			MSD1 = MSD1 * MSD1 / (int) jmpRopePic1.total();

			if (MSD1 > 1000)
			{
				check = true;
				Sleep(200);
			}
		}
	}

	return status_NoError;
}

Status_Code CAEBot::stateJumpRopeBaruoki()
{
	Mat jmpRopePicCrop;
	int MSD1;
	int lowMSD = (int) 99999999999;

	bool check = true;

	SendMessage(m_window, WM_MOUSEACTIVATE, 0, MAKELPARAM(HTCLIENT, WM_LBUTTONDOWN));
	SendMessage(m_window, WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(m_xCenter, m_yCenter));
	Sleep(1);
	SendMessage(m_window, WM_LBUTTONUP, 0, MAKELPARAM(m_xCenter, m_yCenter));

	Sleep(1000);

	int q = 0;
	while (1)
	{
		if (m_resValue == status_Stop)
			return m_resValue;

		Sleep(1);
		bitBltWholeScreen();

		if (check) {
			copyPartialPic(jmpRopePicCrop, 12, 108, 761, 585);
			MSD1 = (int) cv::norm(jmpRopePic4, jmpRopePicCrop);
			MSD1 = MSD1 * MSD1 / (int) jmpRopePic4.total();

			if (MSD1 > 1000)
			{
				check = false;

				SendMessage(m_window, WM_MOUSEACTIVATE, 0, MAKELPARAM(HTCLIENT, WM_LBUTTONDOWN));
				SendMessage(m_window, WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(m_xCenter, m_yCenter));
				Sleep(1);
				SendMessage(m_window, WM_LBUTTONUP, 0, MAKELPARAM(m_xCenter, m_yCenter));
				if (q++ == 10001)
					break;
			}
		}
		else
		{
			copyPartialPic(jmpRopePicCrop, 44, 20, 958, 324);
			MSD1 = (int) cv::norm(jmpRopePic3, jmpRopePicCrop);
			MSD1 = MSD1 * MSD1 / (int) jmpRopePic3.total();

			if (MSD1 > 1000)
			{
				check = true;
				Sleep(200);
			}
		}
	}

	return status_NoError;
}

Status_Code CAEBot::stateSeparateGrasta()
{
	snprintf(m_debugMsg, 1024, "To separate grasta >>>>>>>>>\n");
	dbgMsg(m_IsDebug_Grasta);

	int i = 0;
	//Only 4 slots are fully on screen at a time, with the 5th partially visible. 
	//if last grasta is 0, then scroll one by one
	//otherwise fast scroll to reach to the target grasta

	int k=0;
	int grastaseparated = 0;
	while (1)
	{
		if (m_resValue == status_Stop)
			return m_resValue;

		if (k < m_Grasta_Target)
		{
			snprintf(m_debugMsg, 1024, "Go to grasta [%d]\n", m_Grasta_Target);
			dbgMsg(m_IsDebug_Grasta);

			for (; k < m_Grasta_Target; k++)
			{
				snprintf(m_debugMsg, 1024, ".");
				dbgMsg(m_IsDebug_Grasta);
				drag(DOWN, m_Grasta_ScrollHeight, 1225, 900, 1, m_Grasta_ScrollRatio);
			}

			snprintf(m_debugMsg, 1024, "located\n");
			dbgMsg(m_IsDebug_Grasta);
			leftClick(m_Grasta_Button[0].xyPosition.first, m_Grasta_Button[0].xyPosition.second, 100);
		}
		else
		{
			if (k == 0)
			{
			}
			else
			{
				drag(DOWN, m_Grasta_SectionHeight, 1225, 900, 1);
			}
			leftClick(m_Grasta_Button[0].xyPosition.first, m_Grasta_Button[0].xyPosition.second, 100);
		}

		string txt;
		txt = ocrPictureText(556, 62, 95, 74);
		string attr_1 = ocrPictureText(162, 36, 391, 185);
		int value_1 = ocrPictureNumber(162, 36, 577, 185);
		string attr_2 = ocrPictureText(162, 36, 391, 241);
		int value_2 = ocrPictureNumber(162, 36, 577, 241);

		//found grasta to separate
		snprintf(m_debugMsg, 1024, "Grasta [%d] %s: %s %d %s %d\n", k, txt.c_str(), attr_1.c_str(), value_1, attr_2.c_str(), value_2);
		dbgMsg(m_IsDebug_Grasta);

		for (auto j = 0; j < m_Grasta_Names.size(); j++)
		{
			if (txt.find(m_Grasta_Names[j]) != string::npos)
			{
				snprintf(m_debugMsg, 1024, "To separate grasta[%d] %s: %s %d %s %d\n", k, txt.c_str(), attr_1.c_str(), value_1, attr_2.c_str(), value_2);
				dbgMsg(m_IsDebug_Grasta);

				leftClick(350, 870, 500);
				leftClick(1080, 600, 500);

				string txtOK;
				txtOK = ocrPictureText(154, 54, 800, 720);
				while (txtOK.find("OK") == string::npos)
				{
					snprintf(m_debugMsg, 1024, ".");
					dbgMsg(m_IsDebug_Grasta);
					Sleep(500);
					txtOK = ocrPictureText(154, 54, 800, 720);
				}

				leftClick(880, 750, 500);
				while (!compareImage("Cat Hokora"))
				{
					snprintf(m_debugMsg, 1024, ".");
					dbgMsg(m_IsDebug_Grasta);
					Sleep(500);
				}

				grastaseparated++;

				snprintf(m_debugMsg, 1024, "\n%s: %s %d %s %d separated. total %d separated\n", txt.c_str(), attr_1.c_str(), value_1, attr_2.c_str(), value_2, grastaseparated);
				dbgMsg(m_IsDebug_Grasta);
				outputMsg();

				k = 0;
				break;
			}
		}

		if (m_Grasta_NumberCap && grastaseparated >= m_Grasta_NumberCap)
		{
			snprintf(m_debugMsg, 1024, "Separate cap reached [%d/%d]\n", grastaseparated, m_Grasta_NumberCap);
			dbgMsg(m_IsDebug_Grasta);
			outputMsg();

			k = 0;
			grastaseparated = 0;
			leftClick(m_Button_X);


			pair<bool, pair<int, int>> findclickres = findClickInRegion("ExclamationGrasta", (int)M_WIDTH, (int)M_ABOVE_MENU, 0, 0);
			leftClick(findclickres.second.first, findclickres.second.second, m_Action_Interval, false);

			leftClick(m_Grasta_Button[Grasta_Action_Separate].xyPosition);

			while (!compareImage("Cat Hokora"))
			{
				snprintf(m_debugMsg, 1024, ".");
				dbgMsg(m_IsDebug_Grasta);
				Sleep(500);
			}

			for (auto i = 0; i < m_Grasta_Button.size(); i++)
			{
				if (m_Grasta_TypeSelection.compare(m_Grasta_Button[i].buttonName) == 0)
				{
					leftClick(m_Grasta_Button[i].xyPosition);
					break;
				}
			}
			leftClick(m_Grasta_Button[Grasta_Type_All].xyPosition);
			for (auto i = 0; i < m_Grasta_Button.size(); i++)
			{
				if (m_Grasta_FilterSelection.compare(m_Grasta_Button[i].buttonName) == 0)
				{
					leftClick(m_Grasta_Button[i].xyPosition);
					break;
				}
			}

			while (!compareImage("Cat Hokora"))
			{
				snprintf(m_debugMsg, 1024, ".");
				dbgMsg(m_IsDebug_Grasta);
				Sleep(500);
			}
		}
		else
		{
			k++;
		}
	}

	leftClick(m_Button_X);
}

Status_Code CAEBot::grindingRun(bool endlessGrinding, int forcetimeout)
{
	time_t currenttime, lastFight, startingtime;

	if (endlessGrinding)
		snprintf(m_debugMsg, 1024, "Start grinding [%s] endless\n", m_currentLocation.c_str());
	else
		snprintf(m_debugMsg, 1024, "Start grinding [%s]\n", m_currentLocation.c_str());

		dbgMsg(m_IsDebug_Grinding);
	outputMsg();

	currenttime = time(NULL);
	lastFight = currenttime;
	startingtime = currenttime;

	int currentDirection = LEFT;

	while ((m_currentGrindingCounter < m_Fight_GrindingCount || endlessGrinding) && !m_Fight_GrindingSkipRunning)
	{
		if (m_resValue == status_Stop)
			return m_resValue;

		while (!inBattle())
		{
			if (m_resValue == status_Stop)
				return m_resValue;

			currenttime = time(NULL);
			auto timegap = difftime(currenttime, startingtime);
			if (forcetimeout && forcetimeout < timegap)
			{
				snprintf(m_debugMsg, 1024, "Stop grinding\n");
				dbgMsg(m_IsDebug_Grinding);
				outputMsg();

				return status_NoError;
			}
				
			timegap = difftime(currenttime, lastFight);
			if (timegap > m_Time_Out) // return if timeout
			{
				snprintf(m_debugMsg, 1024, "Leaving [%s] - Idling %f\n", m_currentLocation.c_str(), timegap);
				dbgMsg(m_IsDebug_Grinding);
				outputMsg();

				if (m_IsPrint && m_IsDebug_Grinding) captureScreenNow("Idling");

				return status_Timeout;
			}

			if (m_Fight_GrindingDirection == LR)
			{
				currentDirection = (currentDirection + 1) % 2;
			}
			else
			{
				currentDirection = m_Fight_GrindingDirection;
			}

			switch (currentDirection) {
			case LEFT:
				walk(LEFT, m_Fight_GrindingStep, 10);
				break;
			case RIGHT:
			default:
				walk(RIGHT, m_Fight_GrindingStep, 10);
			}
		}

		engageMobFightNow();

		lastFight = time(NULL);
	}

	snprintf(m_debugMsg, 1024, "Leaving [%s] (%d) - Counter Number reached\n", m_currentLocation.c_str(), m_currentGrindingCounter);
	dbgMsg(m_IsDebug_Grinding);
	outputMsg();

	leftClick(m_Button_PassThrough);

	return status_NoError;
}

Status_Code CAEBot::stateTravelGrinding()
{
	snprintf(m_debugMsg, 1024, "Start travel grinding >>>>>>>>>\n");
	dbgMsg(m_IsDebug_Grinding);

	while (1)
	{
		if (m_resValue == status_Stop)
			return m_resValue;

		loadPathConfig();
		loadSettingConfig();

		for (auto i = 0; i < m_grindingSpots.size(); i++)
		{
			m_currentLocation = m_grindingSpots[i].first;

			for (auto j = 0; j < m_LocationList.size(); j++)
			{
				// if this location is the right one to grind
				if (m_LocationList[j].locationName.compare(m_currentLocation) == 0)
				{
					for (auto k = 0; k < m_grindingSpots[i].second; k++)
					{
						if (m_resValue == status_Stop)
							return m_resValue;

						m_currentGrindingCounter = 0;

						snprintf(m_debugMsg, 1024, "Preparing to grind [%s]\n", m_currentLocation.c_str());
						dbgMsg(m_IsDebug_Grinding);
						outputMsg();

						if (! m_Fight_GrindingSkipSpaceTimeRift)
							goToSpacetimeRift();

						m_resValue = goToTargetLocation(m_LocationList[j].pathStepsList); //load X 1280 Y 720 coordination 
						// if no time out
						if (!m_LocationList[j].skipRunning && m_resValue == status_NoError)
						{
							m_resValue = grindingRun(false);
							// no need to care about the time out here
						}
					}
				}
			}
		}

		//Wait to go back for refresh
		Sleep(m_Action_Interval);
	}
}


Status_Code CAEBot::stateStationGrinding()
{
	int numloop = 0;

	while (1)
	{
		if (m_resValue == status_Stop)
			return m_resValue;

		loadPathConfig();
		loadSettingConfig();

		snprintf(m_debugMsg, 1024, "Preparing to stay here grinding [%d]\n", numloop);
		dbgMsg(m_IsDebug_Grinding);
		outputMsg();

		m_resValue = goToTargetLocation(m_stationGrindingSpot.pathStepsList);
		// no need to care about the time out here
		numloop++;
	}
}

Status_Code CAEBot::stateLOMSlimeGrinding()
{
	snprintf(m_debugMsg, 1024, "Start LOM Slime grinding >>>>>>>>>\n");
	dbgMsg(m_IsDebug_Grinding);

	while (1)
	{
		if (m_resValue == status_Stop)
			return m_resValue;

		loadPathConfig();
		loadSettingConfig();

		m_grindingSpots.clear(); // re config the grinding spots

		if (!m_Fight_LOMHeal.empty())
			m_grindingSpots.push_back(make_pair(m_Fight_LOMHeal, 1));

		if (!m_Fight_LOMSlimeA.empty())
			m_grindingSpots.push_back(make_pair(m_Fight_LOMSlimeA, 1));

		if (!m_Fight_LOMHeal.empty())
			m_grindingSpots.push_back(make_pair(m_Fight_LOMHeal, 1));

		if (!m_Fight_LOMSlimeB.empty())
			m_grindingSpots.push_back(make_pair(m_Fight_LOMSlimeB, 1));

		if (!m_Fight_LOMSlimeRun.empty())
			m_grindingSpots.push_back(make_pair(m_Fight_LOMSlimeRun, m_Fight_LOMTurn));

		int resetindex = -1;
		if (!m_Fight_LOMReset.empty())
		{
			for (auto j = 0; j < m_LocationList.size(); j++)
			{
				// if this location is the right one to grind
				if (m_LocationList[j].locationName.compare(m_Fight_LOMReset) == 0)
				{
					resetindex = j;
					break;
				}
			}
		}

		snprintf(m_debugMsg, 1024, "Grinding LOM Slime:::\n");
		dbgMsg(m_IsDebug_LOM);
		for (auto i = 0; i < m_grindingSpots.size(); i++)
		{
			snprintf(m_debugMsg, 1024, "---> %s %d\n", (m_grindingSpots[i].first).c_str(), m_grindingSpots[i].second);
			dbgMsg(m_IsDebug_LOM);
		}

		for (auto i = 0; i < m_grindingSpots.size(); i++)
		{
			m_currentLocation = m_grindingSpots[i].first;
			for (auto j = 0; j < m_LocationList.size(); j++)
			{
				// if this location is the right one to grind
				if (m_LocationList[j].locationName.compare(m_grindingSpots[i].first) == 0)
				{

					for (auto k = 0; k < m_grindingSpots[i].second; k++)
					{
						m_resValue = goToTargetLocation(m_LocationList[j].pathStepsList); //load X 1280 Y 720 coordination 

						if (m_resValue == status_Timeout)
						{
							m_resValue = goToTargetLocation(m_LocationList[resetindex].pathStepsList); // reset
						}
					}
					break;
				}
			}
		}

		//Wait to go back for refresh
		Sleep(m_Action_Interval);
	}
}

Status_Code CAEBot::stateEndlessGrinding()
{
	snprintf(m_debugMsg, 1024, "Start endless grinding >>>>>>>>>\n");
	dbgMsg(m_IsDebug_Grinding);

	while (1)
	{
		if (m_resValue == status_Stop)
			return m_resValue;

		loadPathConfig();
		loadSettingConfig();

		m_currentLocation = "locally";
		m_resValue = grindingRun(true);
	}
}

Status_Code CAEBot::captureScreenNow(const char* nameSuffix)
{
	Mat imagePicCrop;
	char filename[MAX_PATH];

	bitBltWholeScreen();
	copyPartialPic(imagePicCrop, (int)M_WIDTH, (int)M_HEIGHT, 0, 0);

	snprintf(filename, MAX_PATH, "%s\\%s_%s.jpg", m_CurrentPath, timeString(), nameSuffix);

	snprintf(m_debugMsg, 1024, "%s\n", filename);
	dbgMsg(m_IsDebug_Key);
	outputMsg();

	imwrite(filename, imagePicCrop);

	return status_NoError;
}

Status_Code CAEBot::captureImageNow(Mat imagePicCrop, const char* nameSuffix)
{
	char filename[MAX_PATH];

	snprintf(filename, MAX_PATH, "%s\\%s_%s.jpg", m_CurrentPath, timeString(), nameSuffix);

	snprintf(m_debugMsg, 1024, "%s\n", filename);
	dbgMsg(m_IsDebug_Key);
	outputMsg();

	imwrite(filename, imagePicCrop);

	return status_NoError;
}

keyvalueInfo CAEBot::parseKeyValue(string str, string parser, bool order)
{
	keyvalueInfo returnkeyvalueInfo;

	int loc = (int)str.find(parser);
	if (order)
	{
		returnkeyvalueInfo.key = str.substr(0, loc);
		rtrimString(returnkeyvalueInfo.key);
		returnkeyvalueInfo.value = str.substr(loc + 1);
		ltrimString(returnkeyvalueInfo.value);
	}
	else
	{
		returnkeyvalueInfo.key = str.substr(loc + 1);
		ltrimString(returnkeyvalueInfo.key);
		returnkeyvalueInfo.value = str.substr(0, loc);
		rtrimString(returnkeyvalueInfo.value);
	}

	return returnkeyvalueInfo;
}

pair<int, int> CAEBot::parseXYinfo(string str)
{
	int loc = (int)str.find(",");

	int x = stoi(str.substr(0, loc));
	int y = stoi(str.substr(loc + 1));

	pair<int, int> xyPosition;
	xyPosition.first = (int)round(x * M_WIDTH / M_WIDTH_1280);
	xyPosition.second = (int)round(y * M_HEIGHT / M_HEIGHT_720);

	return xyPosition;
}

pathInfo CAEBot::parsePathStep(string str)
{
	pathInfo pathStep;

	int loc = (int)str.find("=");
	pathStep.type = str.substr(0, loc);
	string val = str.substr(loc + 1);

	size_t pos = 0;
	pos = val.find(",");
	if (pos != std::string::npos)
	{
		pathStep.value1 = val.substr(0, pos);
		pathStep.value2 = val.substr(pos + 1);
	}
	else
	{
		pathStep.value1 = val;
		pathStep.value2.clear();
	}

	return pathStep;
};

void CAEBot::parseDynamicImage(ifstream& file)
{
	string str, key, value;
	keyvalueInfo localKeyValue;
	while (1)
	{
		std::getline(file, str);
		ltrimString(str);
		if (str[0] == '-' || str[0] == '\n') //skip comments and blank lines
			continue;
		if (str.compare("~END") == 0)
			break;

		imageInfo dynamicimage;

		localKeyValue = parseKeyValue(str, string(":"));
		key = localKeyValue.key;
		value = localKeyValue.value;

		if (key.compare("ID") == 0)
			dynamicimage.id = value;
		else
			continue;

		while (1)
		{
			std::getline(file, str);
			ltrimString(str);
			if (str[0] == '-' || str[0] == '\n') //skip comments and blank lines
				continue;
			if (str.compare("~IMAGE") == 0)
			{
				m_DynamicImage.push_back(dynamicimage);
				break;
			}

			localKeyValue = parseKeyValue(str, string(":"));
			key = localKeyValue.key;
			value = localKeyValue.value;

			if (key.compare("Name") == 0)
				dynamicimage.name = value;
			else if (key.compare("Width") == 0)
				dynamicimage.width = stoi(value);
			else if (key.compare("Height") == 0)
				dynamicimage.height = stoi(value);
			else if (key.compare("LocationX") == 0)
				dynamicimage.coordx = stoi(value);
			else if (key.compare("LocationY") == 0)
				dynamicimage.coordy = stoi(value);
		}
	}
};

vector<buttonInfo> CAEBot::parseButtons(ifstream& file)
{
	vector<buttonInfo> buttonlist;
	buttonInfo nameButton;
	string str;
	while (1)
	{
		std::getline(file, str);
		ltrimString(str);
		if (str[0] == '-' || str[0] == '\n') //skip comments and blank lines
			continue;
		if (str.compare("~END") == 0)
			break;

		keyvalueInfo localKeyValue = parseKeyValue(str, string("="));
		nameButton.buttonName = localKeyValue.key;
		nameButton.xyPosition = parseXYinfo(localKeyValue.value);

		snprintf(m_debugMsg, 1024, "Button [%s]: %d %d\n", (localKeyValue.key).c_str(), nameButton.xyPosition.first, nameButton.xyPosition.second);
		dbgMsg(m_IsDebug_Setting);

		buttonlist.push_back(nameButton);
	}

	return buttonlist;
}

vector<vector<int>> CAEBot::parseSkillsSet(ifstream& file)
{
	vector<vector<int>> skillsSet;
	string str;
	while (1)
	{
		std::getline(file, str);
		ltrimString(str);
		if (str[0] == '-' || str[0] == '\n') //skip comments and blank lines
			continue;
		if (str.compare("~END") == 0)
			break;

		vector<int> skillsRow = vector<int>(4);
		std::string token;
		size_t pos = 0;
		size_t index = 0;
		while (index < m_CharacterFrontline) {
			pos = str.find(",");
			if (pos != std::string::npos)
				token = str.substr(0, pos);
			else
				token = str;

			skillsRow[index] = stoi(token);
			str.erase(0, pos + 1);
			index++;
		}
		skillsSet.push_back(skillsRow);
	}

	return skillsSet;
};

vector<pair<string, int>> CAEBot::parseGrindingSpotsList(ifstream& file)
{
	vector<pair<string, int>> grindingSpots;
	string str;

	while (1)
	{
		std::getline(file, str);
		ltrimString(str);
		if (str[0] == '-' || str[0] == '\n') //skip comments and blank lines
			continue;
		if (str.compare("~END") == 0)
			break;

		keyvalueInfo localKeyValue = parseKeyValue(str, string("="));
		if (stoi(localKeyValue.value))
		{
			pair<string, int> grindingspot = { localKeyValue.key, stoi(localKeyValue.value) };
			grindingSpots.push_back(grindingspot);
		}
	}

	return grindingSpots;
};

vector<string> CAEBot::parseGrastaNames(ifstream& file)
{
	vector<string> grastanamelist;
	string str;
	while (1)
	{
		std::getline(file, str);
		ltrimString(str);
		if (str[0] == '-' || str[0] == '\n') //skip comments and blank lines
			continue;
		if (str.compare("~END") == 0)
			break;

		keyvalueInfo localKeyValue = parseKeyValue(str, string("="));

		if (stoi(localKeyValue.value))
		{
			snprintf(m_debugMsg, 1024, "To separate %s\n", (localKeyValue.key).c_str());
			dbgMsg(m_IsDebug_Grasta);

			grastanamelist.push_back(localKeyValue.key);
		}
	}

	return grastanamelist;
};

void CAEBot::parseBaitForArea(ifstream& file, bool constructBait, fishingSpot& area, set<Bait_Type>& baitsNeeded, vector<baitInfo>& baitsToPurchase)
{
	string str;
	Bait_Type i = bait_Fishing_Dango;
	while (1)
	{
		std::getline(file, str);
		ltrimString(str);
		if (str[0] == '-' || str[0] == '\n') //skip comments and blank lines
			continue;
		if (str.compare("~END") == 0)
			break;

		keyvalueInfo localKeyValue = parseKeyValue(str, string("="));
		int baitNumber = stoi(localKeyValue.value);
		string baitName;
		double baitPrice;

		if (constructBait)
		{
			localKeyValue = parseKeyValue(str, string(":"));
			baitPrice = stod(localKeyValue.value);
		}

		baitName = localKeyValue.key;

		if (constructBait)
		{
			baitsToPurchase[i].baitName = baitName;
			baitsToPurchase[i].baitPrice = baitPrice;
			baitsToPurchase[i].baitNumber = baitNumber;
			snprintf(m_debugMsg, 1024, "[%d]%s %.1f %d\n", (int) i, baitName.c_str(), baitPrice, baitNumber);
			dbgMsg(m_IsDebug_Setting);
			i = (Bait_Type) (i + 1);
		}
		else
		{
			if (baitNumber > 0)
			{
				area.baitsToUse.push_back(i);
				baitsNeeded.insert(i);
			}
		}
	}
};

void CAEBot::parsePathList(ifstream& file, locationInfo& location)
{
	string str;
	while (1)
	{
		std::getline(file, str);
		ltrimString(str);
		if (str[0] == '-' || str[0] == '\n') //skip comments and blank lines
			continue;
		if (str.compare("~END") == 0)
			break;

		pathInfo pathStep = parsePathStep(str);
		location.pathStepsList.push_back(pathStep);
	}
};

void CAEBot::parseEmulator(ifstream& file)
{
	string str, key, value;
	keyvalueInfo localKeyValue;

	m_emulatorNumber = 0;

	while (1)
	{
		std::getline(file, str);
		ltrimString(str);
		if (str[0] == '-' || str[0] == '\n') //skip comments and blank lines
			continue;
		if (str.compare("~END") == 0)
			break;

		emulatorInfo emulatorinfo;

		localKeyValue = parseKeyValue(str, string(":"));
		key = localKeyValue.key;
		value = localKeyValue.value;

		if (key.compare("Emulator Name") == 0)
			emulatorinfo.name = value;
		else
			continue;

		while (1)
		{
			std::getline(file, str);
			ltrimString(str);
			if (str[0] == '-' || str[0] == '\n') //skip comments and blank lines
				continue;
			if (str.compare("~EMULATOR") == 0)
			{
				m_emulatorList.push_back(emulatorinfo);
				m_emulatorNumber++;
				break;
			}

			localKeyValue = parseKeyValue(str, string(":"));
			key = localKeyValue.key;
			value = localKeyValue.value;

			if (key.compare("Window Name") == 0)
				emulatorinfo.windowName = value;
			else if (key.compare("Executable Name") == 0)
				emulatorinfo.exeName = value;
			else if (key.compare("InnerWindow Name") == 0)
				emulatorinfo.innerWindowName = value;
		}
	}
};

void CAEBot::loadSettingConfig()
{
	//Read in config file
	ifstream file("config_setting.ini");

	string str, key, value;
	keyvalueInfo localKeyValue;

	while (std::getline(file, str))
	{
		ltrimString(str);
		if (str[0] == '-' || str[0] == '\n') //skip comments and blank lines
			continue;

		localKeyValue = parseKeyValue(str, string("="));
		key = localKeyValue.key;
		value = localKeyValue.value;

		if (key.compare("Load Time") == 0)
		{
			m_loadTime = stoi(value);
		}
		else if (key.compare("Action Interval") == 0)
		{
			m_Action_Interval = stoi(value);
		}
		else if (key.compare("Fast Interval") == 0)
		{
			m_Fast_Action_Interval = stoi(value);
		}
		else if (key.compare("Smart DownUp Interval") == 0)
		{
			m_Smart_DownUp_Interval = stoi(value);
		}
		else if (key.compare("Smart DownUp Threshold") == 0)
		{
			m_Smart_DownUp_Threshold = stoi(value);
		}
		else if (key.compare("Walk Distance Ratio") == 0)
		{
			m_Walk_Distance_Ratio = stod(value);
		}
		else if (key.compare("MSD Threshold") == 0)
		{
			m_msdThreshold = stoi(value);
		}
		else if (key.compare("Print Image") == 0)
		{
			m_IsPrint = stoi(value);
		}
		else if (key.compare("Debug") == 0)
		{
			int debugvalue = stol(value, NULL, 2);

			m_IsDebug_Key = debugvalue & 0x1;
			m_IsDebug_Path = debugvalue & 0x2;
			m_IsDebug_Fighting = debugvalue & 0x4;
			m_IsDebug_Grinding = debugvalue & 0x8;
			m_IsDebug_Fishing = debugvalue & 0x10;
			m_IsDebug_Grasta = debugvalue & 0x20;
			m_IsDebug_LOM = debugvalue & 0x40;
			m_IsDebug_Setting = debugvalue & 0x80;
			m_IsDebug_Platform = debugvalue & 0x100;
		}

		/********************/
		/* Grinding section */
		/********************/
		else if (key.compare("AF Interval") == 0)
		{
			m_Fight_AFInterval = stoi(value);
		}
		else if (key.compare("AF Full Threshold") == 0)
		{
			m_Fight_AFFullThreshold = stoi(value);
		}
		else if (key.compare("Skip SpaceTime Rift") == 0)
		{
			m_Fight_GrindingSkipSpaceTimeRift = stoi(value);
		}
		else if (key.compare("Skip Running") == 0)
		{
			m_Fight_GrindingSkipRunning = stoi(value);
		}
		else if (key.compare("Direction") == 0)
		{
			if (value.compare("LEFT") == 0)
				m_Fight_GrindingDirection = LEFT;
			else if (value.compare("RIGHT") == 0)
				m_Fight_GrindingDirection = RIGHT;
			else if (value.compare("LR") == 0)
				m_Fight_GrindingDirection = LR;
		}
		else if (key.compare("Run Steps") == 0)
		{
			m_Fight_GrindingStep = stoi(value);
		}
		else if (key.compare("Time Out") == 0)
		{
			m_Time_Out = stoi(value);
		}
		else if (key.compare("Run Count") == 0)
		{
			m_Fight_GrindingCount = stoi(value);
		}
		else if (key.compare("Horror Count") == 0)
		{
			m_Fight_HorrorCount = stoi(value);
		}
		else if (key.compare("SkillsHorrorSet") == 0)
		{
			m_skillsHorrorSet = parseSkillsSet(file);
		}
		else if (key.compare("SkillsMobSet") == 0)
		{
			m_skillsMobSet = parseSkillsSet(file);
		}
		else if (key.compare("GrindingSpots") == 0)
		{
			m_grindingSpots = parseGrindingSpotsList(file);
		}

		/***************************/
		/* harpoon fishing section */
		/***************************/

		else if (key.compare("Harpoon Loop") == 0)
		{
			m_Harpoon_Loop = stoi(value);
		}
		else if (key.compare("Harpoon X incremental") == 0)
		{
			m_Harpoon_Xinc = (int)round(stoi(value) * M_WIDTH / M_WIDTH_1280);
		}
		else if (key.compare("Harpoon Y incremental") == 0)
		{
			m_Harpoon_Yinc = (int)round(stoi(value) * M_WIDTH / M_WIDTH_1280);
		}
		else if (key.compare("Harpoon X min") == 0)
		{
			m_Harpoon_Xmin = (int)round(stoi(value) * M_WIDTH / M_WIDTH_1280);
		}
		else if (key.compare("Harpoon X max") == 0)
		{
			m_Harpoon_Xmax = (int)round(stoi(value) * M_WIDTH / M_WIDTH_1280);
		}
		else if (key.compare("Harpoon Y min") == 0)
		{
			m_Harpoon_Ymin = (int)round(stoi(value) * M_HEIGHT / M_HEIGHT_720);
		}
		else if (key.compare("Harpoon Y max") == 0)
		{
			m_Harpoon_Ymax = (int)round(stoi(value) * M_HEIGHT / M_HEIGHT_720);
		}
		else if (key.compare("Harpoon Skip Vendor") == 0)
		{
			m_Harpoon_SkipVendor = stoi(value);
		}
		else if (key.compare("Harpoon Interval") == 0)
		{
			m_Harpoon_Interval = stoi(value);
		}
		else if (key.compare("Harpoon Threshold") == 0)
		{
			m_Harpoon_Threshold = stoi(value);
		}

		/************************/
		/* LOM grinding section */
		/************************/

		else if (key.compare("LOM Heal") == 0)
		{
			m_Fight_LOMHeal = value;
		}
		else if (key.compare("LOM Slime A") == 0)
		{
			m_Fight_LOMSlimeA = value;
		}
		else if (key.compare("LOM Slime B") == 0)
		{
			m_Fight_LOMSlimeB = value;
		}
		else if (key.compare("LOM Slime Run") == 0)
		{
			m_Fight_LOMSlimeRun = value;
		}
		else if (key.compare("LOM turn") == 0)
		{
			m_Fight_LOMTurn = stoi(value);
		}
		else if (key.compare("LOM Slime Reset") == 0)
		{
			m_Fight_LOMReset = value;
		}

		/******************/
		/* Grasta section */
		/******************/

		else if (key.compare("Last Grasta") == 0)
		{
			m_Grasta_Target = stoi(value);
		}
		else if (key.compare("Section Height") == 0)
		{
			m_Grasta_SectionHeight = stoi(value);
		}
		else if (key.compare("Scroll Height") == 0)
		{
			m_Grasta_ScrollHeight = stoi(value);
		}
		else if (key.compare("Scroll Ratio") == 0)
		{
			m_Grasta_ScrollRatio = stoi(value);
		}
		else if (key.compare("Number Cap") == 0)
		{
			m_Grasta_NumberCap = stoi(value);
		}
		else if (key.compare("Grasta Type Selection") == 0)
		{
			m_Grasta_TypeSelection = value;
		}
		else if (key.compare("Grasta Filter Selection") == 0)
		{
			m_Grasta_FilterSelection = value;
		}
		else if (key.compare("Grasta to Seperate") == 0)
		{
			m_Grasta_Names = parseGrastaNames(file);
		}
		else if (key.compare("Grasta Buttons") == 0)
		{
			m_Grasta_Button = parseButtons(file);
		}

		/***************/
		/* Map section */
		/***************/
		else if (key.compare("MapsButtons") == 0)
		{
			m_Button_MapButtons = parseButtons(file);
		}

		/******************/
		/* Common section */
		/******************/
		else if (key.compare("YesButton") == 0)
		{
			m_Button_Yes = parseXYinfo(value);
		}
		else if (key.compare("LeaveButton") == 0)
		{
			m_Button_Leave = parseXYinfo(value);
		}
		else if (key.compare("XButton") == 0)
		{
			m_Button_X = parseXYinfo(value);
		}
		else if (key.compare("MenuButton") == 0)
		{
			m_Button_Menu = parseXYinfo(value);
		}
		else if (key.compare("FoodButton") == 0)
		{
			m_Button_Food = parseXYinfo(value);
		}
		else if (key.compare("AttackButton") == 0)
		{
			m_Button_Attack = parseXYinfo(value);
		}
		else if (key.compare("AFButton") == 0)
		{
			m_Button_AF = parseXYinfo(value);
		}
		else if (key.compare("MapButton") == 0)
		{
			m_Button_Map = parseXYinfo(value);
		}
		else if (key.compare("Antiquity") == 0)
		{
			m_Button_Antiquity = parseXYinfo(value);
		}
		else if (key.compare("Present") == 0)
		{
			m_Button_Present = parseXYinfo(value);
		}
		else if (key.compare("Future") == 0)
		{
			m_Button_Future = parseXYinfo(value);
		}
		else if (key.compare("EndOfTimeLoc") == 0)
		{
			m_Button_EndOfTimeLoc = parseXYinfo(value);
		}
		else if (key.compare("SpacetimeRift") == 0)
		{
			m_Button_SpacetimeRift = parseXYinfo(value);
		}
		else if (key.compare("Tree") == 0)
		{
			m_Button_Tree = parseXYinfo(value);
		}
		else if (key.compare("PassThrough") == 0)
		{
			m_Button_PassThrough = parseXYinfo(value);
		}
		else if (key.compare("CharactersButtons") == 0)
		{
			m_Button_Characters = parseButtons(file);
		}
		else if (key.compare("SkillsButtons") == 0)
		{
			m_Button_Skills = parseButtons(file);
		}
		else if (key.compare("BaitsButtons") == 0)
		{
			m_Button_Baits = parseButtons(file);
		}
	}

	snprintf(m_debugMsg, 1024, "SkillsHorrorSet:::\n");
	dbgMsg(m_IsDebug_Setting);

	for (auto j = 0; j < m_skillsHorrorSet.size(); j++)
	{
		snprintf(m_debugMsg, 1024, "Turn [%d]: %d %d %d %d\n", j, m_skillsHorrorSet[j][0], m_skillsHorrorSet[j][1], m_skillsHorrorSet[j][2], m_skillsHorrorSet[j][3]);
		dbgMsg(m_IsDebug_Setting);
	}

	snprintf(m_debugMsg, 1024, "SkillsMobSet:::\n");
	dbgMsg(m_IsDebug_Setting);

	for (auto j = 0; j < m_skillsMobSet.size(); j++)
	{
		snprintf(m_debugMsg, 1024, "Turn [%d]: %d %d %d %d\n", j, m_skillsMobSet[j][0], m_skillsMobSet[j][1], m_skillsMobSet[j][2], m_skillsMobSet[j][3]);
		dbgMsg(m_IsDebug_Setting);
	}

	snprintf(m_debugMsg, 1024, "Grinding:::\n");
	dbgMsg(m_IsDebug_Setting);

	for (auto j = 0; j < m_grindingSpots.size(); j++)
	{
		snprintf(m_debugMsg, 1024, "---> %s %d\n", (m_grindingSpots[j].first).c_str(), m_grindingSpots[j].second);
		dbgMsg(m_IsDebug_Setting);
	}

	snprintf(m_debugMsg, 1024, "<<<< Setting profile loaded! >>>>\n");
	dbgMsg(m_IsDebug_Key);
}

void CAEBot::loadFishingConfig()
{
	//Read in config file
	ifstream file("config_fishing.ini");

	m_fishingSpots.clear();
	m_baitList.clear();
	set<Bait_Type> baitsNeeded;
	string str;

	//Set up baits
	for (int i = 0; i < bait_Error_Max; i++)
	{
		baitInfo currBait;
		currBait.baitName = "";
		currBait.baitPrice = 0;
		currBait.baitNumber = 0;
		currBait.hasBait = false;

		m_baitList.push_back(currBait);
	}

	while (std::getline(file, str))
	{
		ltrimString(str);
		if (str[0] == '-' || str[0] == '\n') //skip comments and blank lines
			continue;

		keyvalueInfo localKeyValue = parseKeyValue(str, string(":"));

		/******************/
		/* Baits section */
		/******************/
		if (localKeyValue.key.compare("Baits") == 0)
		{
			fishingSpot nullSpot;
			set<Bait_Type> tempBaits;
			parseBaitForArea(file, true, nullSpot, tempBaits, m_baitList);
		}
		/****************************/
		/* Fishing Location section */
		/****************************/

		else if (localKeyValue.key.compare("Location") == 0)
		{
			string fishingLocation = localKeyValue.value;
			int fishingOrder;

			std::getline(file, str);
			ltrimString(str);
			localKeyValue = parseKeyValue(str, string("="));

			if (localKeyValue.key.compare("Order") == 0)
			{
				fishingOrder = stoi(localKeyValue.value);
			}

			fishingSpot newFishingSpot;
			newFishingSpot.orderNumber = fishingOrder;
			newFishingSpot.locationName = fishingLocation;

			m_fishingSpots.push_back(newFishingSpot);
			vector<baitInfo> tempBaitList;
			parseBaitForArea(file, false, m_fishingSpots.back(), baitsNeeded, tempBaitList);
		}

		/************************************/
		/* Harpoon Fishing Location section */
		/************************************/
		else if (localKeyValue.key.compare("HarpoonLocation") == 0)
		{
			m_harpoonSpots = parseGrindingSpotsList(file);
		}

	}

	snprintf(m_debugMsg, 1024, "Fishing:::\n");
	dbgMsg(m_IsDebug_Setting);
	for (auto j = 0; j < m_fishingSpots.size(); j++)
	{
		snprintf(m_debugMsg, 1024, "---> Fishing %s order %d\n", (m_fishingSpots[j].locationName).c_str(), m_fishingSpots[j].orderNumber);
		dbgMsg(m_IsDebug_Setting);
	}
	for (auto j = 0; j < m_harpoonSpots.size(); j++)
	{
		snprintf(m_debugMsg, 1024, "---> Harpoon %s order %d\n", (m_harpoonSpots[j].first).c_str(), m_harpoonSpots[j].second);
		dbgMsg(m_IsDebug_Setting);
	}

	std::sort(m_fishingSpots.begin(), m_fishingSpots.end(), [](fishingSpot& lhs, fishingSpot& rhs) { return lhs.orderNumber < rhs.orderNumber; });

	std::sort(m_harpoonSpots.begin(), m_harpoonSpots.end(), [](pair<string, int> lhs, pair<string, int> rhs) { return lhs.second < rhs.second; });

	//Check to see if any spots actually use the bait we've selected to buy. If not, don't buy any for this run
	for (int i = 0; i <= bait_Premium_Crab_Cake; i++)
	{
		if (baitsNeeded.find((Bait_Type)i) == baitsNeeded.end())
			m_baitList[i].baitNumber = 0;
	}

	for (int i = 0; i < m_baitList.size(); i++)
	{
		snprintf(m_debugMsg, 1024, "Bait \"%s\" to buy %x number %d\n", m_baitList[i].baitName.c_str(), m_baitList[i].hasBait, m_baitList[i].baitNumber);
		dbgMsg(m_IsDebug_Setting);
	}

	snprintf(m_debugMsg, 1024, "<<<< Fishing profile loaded! >>>>\n");
	dbgMsg(m_IsDebug_Key);
}

void CAEBot::loadPathConfig()
{
	//Read in config file
	ifstream file("config_path.ini");

	m_LocationList.clear();
	m_stationGrindingSpot.pathStepsList.clear();
	string str;

	while (std::getline(file, str))
	{
		ltrimString(str);
		if (str[0] == '-' || str[0] == '\n') //skip comments and blank lines
			continue;

		keyvalueInfo localKeyValue = parseKeyValue(str, string(":"));

		if (localKeyValue.key.compare("Location") == 0)
		{
			locationInfo locInfo;
			locInfo.locationName = localKeyValue.value;

			if (localKeyValue.value.compare("Default") == 0)
			{
				parsePathList(file, m_stationGrindingSpot);
			}
			else
			{
				if (localKeyValue.value.find("Sparkles") != std::string::npos)
					locInfo.skipRunning = true;
				else
					locInfo.skipRunning = false;

				m_LocationList.push_back(locInfo);
				parsePathList(file, m_LocationList.back());
			}
		}
	}

	snprintf(m_debugMsg, 1024, "<<<< Path profile loaded! >>>>\n");
	dbgMsg(m_IsDebug_Key);
}

void CAEBot::loadConfig()
{
	//Read in config file
	ifstream file("config.ini");

	GetCurrentDirectory(MAX_PATH, m_CurrentPath);

	string key, value, str;

	while (std::getline(file, str))
	{
		ltrimString(str);
		if (str[0] == '-' || str[0] == '\n') //skip comments and blank lines
			continue;

		keyvalueInfo localKeyValue = parseKeyValue(str, string("="));
		key = localKeyValue.key;
		value = localKeyValue.value;

		if (key.compare("Emulator List") == 0)
		{
			m_emulatorIndex = stoi(value);
			parseEmulator(file);
		}

		/*****************/
		/* Mode section */
		/*****************/
		else if (key.compare("Baruoki Jump Rope") == 0)
		{
			if (stoi(value) == 1)
				m_botMode = baruokiJumpRopeMode;
		}
		else if (key.compare("Ratle Jump Rope") == 0)
		{
			if (stoi(value) == 1)
				m_botMode = ratleJumpRope;
		}
		else if (key.compare("Hit Bell") == 0)
		{
			if (stoi(value) == 1)
				m_botMode = silverHitBell30Mode;
			else if (stoi(value) == 2)
				m_botMode = silverHitBell999Mode;
		}
		else if (key.compare("Seperate Grasta") == 0)
		{
			if (stoi(value) == 1)
				m_botMode = seperateGrastaMode;
		}
		else if (key.compare("Engage Fight") == 0)
		{
			if (stoi(value) == 1)
				m_botMode = engageFightMode;
		}
		else if (key.compare("Grinding") == 0)
		{
			if (stoi(value) == 1)
				m_botMode = grindingMode;
			else if (stoi(value) == 2)
				m_botMode = grindingTravelMode;
			else if (stoi(value) == 3)
				m_botMode = grindingStationMode;
			else if (stoi(value) == 4)
				m_botMode = grindingLOMSlimeMode;
		}
		else if (key.compare("Fishing") == 0)
		{
			if (stoi(value) == 1)
				m_botMode = fishingMode;
			else if (stoi(value) == 2)
				m_botMode = harpoonFishingMode;
		}
		else if (key.compare("Capture Screen") == 0)
		{
			if (stoi(value) == 1)
				m_botMode = captureScreenMode;
		}
		else if (key.compare("Dynamic Image") == 0)
		{
			parseDynamicImage(file);
		}
	}

	/*
	snprintf(m_debugMsg, 1024, "Total %d Emulators and choose [%d]::\n", m_emulatorNumber, m_emulatorIndex);
	dbgMsg(true);

	for (auto j = 0; j < m_emulatorNumber; j++)
	{
		snprintf(m_debugMsg, 1024, "[%d] %s %s %s %s\n", j, m_emulatorList[j].name.c_str(), m_emulatorList[j].windowName.c_str(), m_emulatorList[j].exeName.c_str(), m_emulatorList[j].innerWindowName.c_str());
		dbgMsg(true);
	}
	*/
	snprintf(m_debugMsg, 1024, "<<<< Configuration loaded! >>>>\n");
	dbgMsg(m_IsDebug_Key);
}

void CAEBot::init()
{
	loadConfig();
	loadSettingConfig();
	loadPathConfig();
	loadFishingConfig();

	snprintf(m_debugMsg, 1024, "<<<< Initiated! >>>>\n");
	dbgMsg(m_IsDebug_Key);
}

Status_Code CAEBot::setup()
{
	bool emuStatus;
	string emulator, windowName, innerWindowName;
	emulator = m_emulatorList[m_emulatorIndex].exeName;
	windowName = m_emulatorList[m_emulatorIndex].windowName;
	innerWindowName = m_emulatorList[m_emulatorIndex].innerWindowName;

	pair<string*, string*> enumInput = make_pair(&(emulator), &(windowName));
	emuStatus = EnumWindows(EnumWindowsProc, LPARAM(&enumInput));
	if (emuStatus)
		return status_WrongEmulator;
	emuStatus = EnumChildWindows(m_window, EnumChildWindowsProc, LPARAM(&(innerWindowName)));
	if (emuStatus)
		return status_WrongEmulator;

	ocr = OCRTesseract::create(NULL, NULL, NULL, OEM_TESSERACT_ONLY, PSM_SINGLE_LINE);

	RECT rect;
	GetWindowRect(m_window, &rect);
	m_height = rect.bottom - rect.top;
	m_width = rect.right - rect.left;

	m_xCenter = m_width / 2;
	m_yCenter = m_height / 2;
	m_heightPct = (double)(m_height / M_HEIGHT);
	m_widthPct = (double)(m_width / M_WIDTH);

	afBarEmptyPic = imread("images\\AEBarEmpty.png", IMREAD_UNCHANGED);
	afBarFullPic = imread("images\\AEBarFull.png", IMREAD_UNCHANGED);
	hitBellPic = imread("images\\Bell.png", IMREAD_UNCHANGED);
	jmpRopePic1 = imread("images\\jmpRopePic1.png", IMREAD_UNCHANGED);
	jmpRopePic2 = imread("images\\jmpRopePic2.png", IMREAD_UNCHANGED);
	jmpRopePic3 = imread("images\\jmpRopePic3.png", IMREAD_UNCHANGED);
	jmpRopePic4 = imread("images\\jmpRopePic4.png", IMREAD_UNCHANGED);

	for (auto i = 0; i < m_DynamicImage.size(); i++)
	{
		string dynamicimagename = string("images\\") + m_DynamicImage[i].name;
		m_DynamicImage[i].image = imread(dynamicimagename, IMREAD_UNCHANGED);
	}

	m_MonsterVec_Baruoki.push_back(imread("images\\monster_baruoki.png", IMREAD_UNCHANGED));
	m_MonsterVec_Acteul.push_back(imread("images\\monster_acteul.png", IMREAD_UNCHANGED));
	m_MonsterVec_Vasu.push_back(imread("images\\monster_vasu.png", IMREAD_UNCHANGED));
	m_MonsterVec_Serena.push_back(imread("images\\monster_serena.png", IMREAD_UNCHANGED));
	m_MonsterVec_Rucyana.push_back(imread("images\\monster_rucyana.png", IMREAD_UNCHANGED));
	m_MonsterVec_Elzion.push_back(imread("images\\monster_elzion.png", IMREAD_UNCHANGED));
	m_MonsterVec_LastIsland.push_back(imread("images\\monster_lastisland.png", IMREAD_UNCHANGED));
	m_MonsterVec_DimensionRift.push_back(imread("images\\monster_dimensionrift1.png", IMREAD_UNCHANGED));
	m_MonsterVec_DimensionRift.push_back(imread("images\\monster_dimensionrift2.png", IMREAD_UNCHANGED));
	m_MonsterVec_DimensionRift.push_back(imread("images\\monster_dimensionrift3.png", IMREAD_UNCHANGED));
	m_MonsterVec_LOMPSlime.push_back(imread("images\\monster_lompslime1.png", IMREAD_UNCHANGED));
	m_MonsterVec_LOMPSlime.push_back(imread("images\\monster_lompslime2.png", IMREAD_UNCHANGED));

	if (m_heightPct != 1.0 || m_widthPct != 1.0)
	{
		resize(afBarEmptyPic, afBarEmptyPic, Size(), m_widthPct, m_heightPct, m_heightPct >= 1.0 ? INTER_CUBIC : INTER_AREA);

		resize(afBarFullPic, afBarFullPic, Size(), m_widthPct, m_heightPct, m_heightPct >= 1.0 ? INTER_CUBIC : INTER_AREA);
		resize(hitBellPic, hitBellPic, Size(), m_widthPct, m_heightPct, m_heightPct >= 1.0 ? INTER_CUBIC : INTER_AREA);
		resize(jmpRopePic1, jmpRopePic1, Size(), m_widthPct, m_heightPct, m_heightPct >= 1.0 ? INTER_CUBIC : INTER_AREA);
		resize(jmpRopePic2, jmpRopePic2, Size(), m_widthPct, m_heightPct, m_heightPct >= 1.0 ? INTER_CUBIC : INTER_AREA);
		resize(jmpRopePic3, jmpRopePic3, Size(), m_widthPct, m_heightPct, m_heightPct >= 1.0 ? INTER_CUBIC : INTER_AREA);
		resize(jmpRopePic4, jmpRopePic4, Size(), m_widthPct, m_heightPct, m_heightPct >= 1.0 ? INTER_CUBIC : INTER_AREA);

		resize(m_MonsterVec_Baruoki[0], m_MonsterVec_Baruoki[0], Size(), m_widthPct, m_heightPct, m_heightPct >= 1.0 ? INTER_CUBIC : INTER_AREA);
		resize(m_MonsterVec_Acteul[0], m_MonsterVec_Acteul[0], Size(), m_widthPct, m_heightPct, m_heightPct >= 1.0 ? INTER_CUBIC : INTER_AREA);
		resize(m_MonsterVec_Vasu[0], m_MonsterVec_Vasu[0], Size(), m_widthPct, m_heightPct, m_heightPct >= 1.0 ? INTER_CUBIC : INTER_AREA);
		resize(m_MonsterVec_Serena[0], m_MonsterVec_Serena[0], Size(), m_widthPct, m_heightPct, m_heightPct >= 1.0 ? INTER_CUBIC : INTER_AREA);
		resize(m_MonsterVec_Rucyana[0], m_MonsterVec_Rucyana[0], Size(), m_widthPct, m_heightPct, m_heightPct >= 1.0 ? INTER_CUBIC : INTER_AREA);
		resize(m_MonsterVec_Elzion[0], m_MonsterVec_Elzion[0], Size(), m_widthPct, m_heightPct, m_heightPct >= 1.0 ? INTER_CUBIC : INTER_AREA);
		resize(m_MonsterVec_LastIsland[0], m_MonsterVec_LastIsland[0], Size(), m_widthPct, m_heightPct, m_heightPct >= 1.0 ? INTER_CUBIC : INTER_AREA);
		resize(m_MonsterVec_DimensionRift[0], m_MonsterVec_DimensionRift[0], Size(), m_widthPct, m_heightPct, m_heightPct >= 1.0 ? INTER_CUBIC : INTER_AREA);
		resize(m_MonsterVec_DimensionRift[1], m_MonsterVec_DimensionRift[1], Size(), m_widthPct, m_heightPct, m_heightPct >= 1.0 ? INTER_CUBIC : INTER_AREA);
		resize(m_MonsterVec_DimensionRift[2], m_MonsterVec_DimensionRift[2], Size(), m_widthPct, m_heightPct, m_heightPct >= 1.0 ? INTER_CUBIC : INTER_AREA);

		resize(m_MonsterVec_LOMPSlime[0], m_MonsterVec_LOMPSlime[0], Size(), m_widthPct, m_heightPct, m_heightPct >= 1.0 ? INTER_CUBIC : INTER_AREA);
		resize(m_MonsterVec_LOMPSlime[1], m_MonsterVec_LOMPSlime[1], Size(), m_widthPct, m_heightPct, m_heightPct >= 1.0 ? INTER_CUBIC : INTER_AREA);

		for (auto i = 0; i < m_DynamicImage.size(); i++)
		{
			resize(m_DynamicImage[i].image, m_DynamicImage[i].image, Size(), m_widthPct, m_heightPct, m_heightPct >= 1.0 ? INTER_CUBIC : INTER_AREA);
		}
	}

	rng = std::mt19937(dev());
	slideSleepRand = std::uniform_int_distribution<std::mt19937::result_type>(0, 1);
	slideLClick = std::uniform_int_distribution<std::mt19937::result_type>(0, (int) round(m_widthPct * 30));
	slideRand = std::uniform_int_distribution<std::mt19937::result_type>(0, 2);
	boolRand = std::uniform_int_distribution<std::mt19937::result_type>(0, 1);
	slideDistanceRand = std::uniform_int_distribution<std::mt19937::result_type>(0, 100);
	longSleepRand = std::uniform_int_distribution<std::mt19937::result_type>(0, 1000);

	hdc = GetWindowDC(m_window);
	hDest = CreateCompatibleDC(hdc);

	void* ptrBitmapPixels;

	BITMAPINFO bi;
	ZeroMemory(&bi, sizeof(BITMAPINFO));
	bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bi.bmiHeader.biWidth = m_width;
	bi.bmiHeader.biHeight = -m_height;
	bi.bmiHeader.biPlanes = 1;
	bi.bmiHeader.biBitCount = 32;
	hbDesktop = CreateDIBSection(hdc, &bi, DIB_RGB_COLORS, &ptrBitmapPixels, NULL, 0);

	SelectObject(hDest, hbDesktop);

	m_BitbltPic = Mat(m_height, m_width, CV_8UC4, ptrBitmapPixels, 0);

	snprintf(m_debugMsg, 1024, "<<<< Setup completed! >>>>\n");
	dbgMsg(m_IsDebug_Key);
	return status_NoError;
}

Status_Code CAEBot::run()
{
	// reload config files
	loadFishingConfig();
	loadPathConfig();
	loadSettingConfig();

	m_resValue = status_NoError;
	m_debugMsg[0] = NULL;
	m_outputMsg.clear();

	switch (m_botMode) {
	case baruokiJumpRopeMode:
		m_resValue = stateJumpRopeBaruoki();
		break;
	case ratleJumpRope:
		m_resValue = stateJumpRopeRatle();
		break;
	case silverHitBell30Mode:
	case silverHitBell999Mode:
		m_resValue = stateSilverHitBell(GetMode());
		break;
	case seperateGrastaMode:
		m_resValue = stateSeparateGrasta();
		break;
	case engageFightMode:
		m_resValue = engageHorrorFightNow();
		break;
	case grindingMode:
		m_resValue = stateEndlessGrinding();
		break;
	case grindingTravelMode:
		m_resValue = stateTravelGrinding();
		break;
	case grindingStationMode:
		m_resValue = stateStationGrinding();
		break;
	case grindingLOMSlimeMode:
		m_resValue = stateLOMSlimeGrinding();
		break;
	case fishingMode:
		m_resValue = stateFishing();
		break;
	case harpoonFishingMode:
		m_resValue = stateHarpoonFishing();
		break;
	case captureScreenMode:
	default:
		m_resValue = captureScreenNow();
		break;
	}

	return m_resValue;
}

int main(int argc, char* argv[])
{
	CAEBot* m_bot = new CAEBot();

	m_bot->init();

	if (argc >= 2)
	{
		int emuindex = stoi(argv[1]);
		if (m_bot->GetEmulatorNumber() > emuindex && emuindex >= 0)
		{
			m_bot->SetEmulator(emuindex);
		}
	}

	Status_Code emuStatus = m_bot->setup();
	if (emuStatus != status_NoError)
		return 0;

	m_bot->run();
}
