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
	m_lastGrasta = 0;

	M_WIDTH = 1745.0;
	M_HEIGHT = 981.0;
	M_WIDTH_1280 = 1280.0;
	M_HEIGHT_720 = 720.0;
	m_msdThreshold = 10000;
	m_loadTime = 3000;
	m_IsDebug_Key = false;
	m_IsDebug_Path = false;
	m_IsDebug_Fighting = false;
	m_IsDebug_Grinding = false;
	m_IsDebug_Fishing = false;
	m_IsDebug_Grasta = false;
	m_IsDebug_LOM = false;
	m_IsDebug_Platform = false;

	m_IsPrint = false;

	ocrNumbers = "1234567890,";
	ocrLetters = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ.'()";

	m_Locs_baits = { {827, 274}, {882, 397}, {870, 514}, {851, 630}, {854, 750}, {884, 790} };
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

	m_baitNames = { "Fishing Dango", "Worm", "Unexpected Worm", "Shopaholic Clam", "Spree Snail", "Dressed Crab", "Tear Crab", "Foamy Worm", "Bubbly Worm", "Snitch Sardine", "Blabber Sardine", "Crab Cake", "Premium Crab Cake" };

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
		cout << m_debugMsg;
}

string CAEBot::GetdbgMsg()
{
	string str(m_debugMsg);

	return str;
}

Bot_Mode CAEBot::GetMode()
{
	return m_botMode;
}

void CAEBot::SetMode(Bot_Mode botMode)
{
	m_botMode = botMode;
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
	bitbltPic(cv::Rect(x, y, cols, rows)).copyTo(partialPic);
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
	int result_cols = bitbltPic.cols - tmp.cols + 1;
	int result_rows = bitbltPic.rows - tmp.rows + 1;
	Mat result;
	result.create(result_rows, result_cols, CV_32FC1);

	matchTemplate(bitbltPic, tmp, result, TM_SQDIFF_NORMED);

	Point minLoc, maxLoc;
	double minVal, maxVal;

	minMaxLoc(result, &minVal, &maxVal, &minLoc, &maxLoc, Mat());

	return make_pair(minLoc.x, minLoc.y);
}

pair<int, int> CAEBot::findExclamationIcon(Excl_Type whichExcl)
{
	pair<int, int> exclIcon;

	switch (whichExcl) {
	case excl_Rift:
		exclIcon = findIcon(exclRiftIcon);
		exclIcon.first += (int)round(50 * m_widthPct);
		exclIcon.second += (int)round(70 * m_heightPct);
		break;
	case excl_Sepcter:
		exclIcon = findIcon(exclSepcterIcon);
		exclIcon.first += (int)round(70 * m_widthPct);
		exclIcon.second += (int)round(95 * m_heightPct);
		break;
	case excl_Grasta:
		exclIcon = findIcon(exclGrastaIcon);
		exclIcon.first += (int)round(50 * m_widthPct);
		exclIcon.second += (int)round(70 * m_heightPct);
		break;
	case excl_Chamber:
		exclIcon = findIcon(exclChamberIcon);
		exclIcon.first += (int)round(30 * m_widthPct);
		exclIcon.second += (int)round(50 * m_heightPct);
		break;
	case excl_ChamberPlasma:
		exclIcon = findIcon(exclChamberPlasmaIcon);
		exclIcon.first += (int)round(30 * m_widthPct);
		exclIcon.second += (int)round(50 * m_heightPct);
		break;
	case excl_KMS:
		exclIcon = findIcon(exclKMSIcon);
		exclIcon.first += (int)round(17 * m_widthPct);
		exclIcon.second += (int)round(40 * m_heightPct);
		break;
	default:
		exclIcon = findIcon(exclDefaultIcon);
		exclIcon.first += (int)round(11 * m_widthPct);
		exclIcon.second += (int)round(25 * m_heightPct);
		break;
	}

	return exclIcon;
}

void CAEBot::findAndClickSwampFishIcon()
{
	pair<int, int> fishIconLoc = findIcon(swampFishIcon);
	fishIconLoc.first += (int) round(36 * m_widthPct);
	fishIconLoc.second += (int) round(19 * m_heightPct);
	leftClick(fishIconLoc.first, fishIconLoc.second, 2000, false);
}

void CAEBot::findAndClickFishIcon()
{
	pair<int, int> fishIconLoc = findIcon(fishIcon);
	fishIconLoc.first += (int) round(30 * m_widthPct);
	fishIconLoc.second += (int) round(15 * m_heightPct);
	leftClick(fishIconLoc.first, fishIconLoc.second, 2000, false);
}

pair<int, int> CAEBot::findDoorIcon(Door_Type whichDoor)
{
	pair<int, int> doorIconLoc;

	switch (whichDoor) {
	case door_Kunlun:
		doorIconLoc = findIcon(doorKunlunIcon);
		doorIconLoc.first += (int)round(40 * m_widthPct);
		doorIconLoc.second += (int)round(40 * m_heightPct);
		break;
	case door_LOM:
		doorIconLoc = findIcon(doorLOMIcon);
		doorIconLoc.first += (int)round(50 * m_widthPct);
		doorIconLoc.second += (int)round(50 * m_heightPct);
		break;
	case door_Default:
	default:
		doorIconLoc = findIcon(doorDefaultIcon);
		doorIconLoc.first += (int)round(60 * m_widthPct);
		doorIconLoc.second += (int)round(60 * m_heightPct);
		break;
	}

	return doorIconLoc;
}

pair<int, int> CAEBot::findStairIcon(Stair_Type whichStair)
{
	pair<int, int> stairIconLoc;

	switch (whichStair) {
	case stair_Down:
		stairIconLoc = findIcon(stairDownIcon);
		stairIconLoc.first += (int)round(60 * m_widthPct);
		stairIconLoc.second += (int)round(60 * m_heightPct);
		break;
	case stair_LOM:
		stairIconLoc = findIcon(stairLOMIcon);
		stairIconLoc.first += (int)round(40 * m_widthPct);
		stairIconLoc.second += (int)round(40 * m_heightPct);
		break;
	case stair_Default:
	default:
		stairIconLoc = findIcon(stairDefaultIcon);
		stairIconLoc.first += (int)round(60 * m_widthPct);
		stairIconLoc.second += (int)round(60 * m_heightPct);
		break;
	}

	return stairIconLoc;
}

Bait_Type CAEBot::hashBait(string str) {
	for (int i = bait_Fishing_Dango; i <= bait_Premium_Crab_Cake; i++)
	{
		if (str.compare(m_baitNames[i]) == 0) return (Bait_Type) i;

	}
	
	return bait_Error_Max;
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
		dbgMsg(m_IsDebug_Path);

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

void CAEBot::parseBaitForArea(ifstream& file, bool maxBait, fishingSpot& area, set<int>& baitsNeeded, vector<pair<bool, int>>& baitsToPurchase)
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

		keyvalueInfo localKeyValue = parseKeyValue(str, string("="));
		Bait_Type baitType = hashBait(localKeyValue.key);
		int baitNumber = stoi(localKeyValue.value);

		if (baitType != bait_Error_Max)
		{
			if (maxBait)
			{
				baitsToPurchase[baitType].second = baitNumber;

				snprintf(m_debugMsg, 1024, "baitsToPurchase %x %d\n", baitType, baitNumber);
				dbgMsg(m_IsDebug_Fishing);
			}
			else 
			{
				if (baitNumber > 0)
				{
					area.baitsToUse.push_back(baitType);
					baitsNeeded.insert(baitType);
				}
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

void CAEBot::leftClick(int x, int y, int sTime, bool changeLoc)
{
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

void CAEBot::leftClick(pair<int, int>& coord, int sTime)
{
	leftClick(coord.first, coord.second, sTime);
}

void CAEBot::drag(Direction_Info botDirection, int slideDistance, int xStart, int yStart)
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

		for (int i = 0; i < slideDistance; ++i)
		{
			if (boolRand(rng) && delta < deviation)
				delta += boolRand(rng);
			else if (delta > -deviation)
				delta -= boolRand(rng);

			SendMessage(m_window, WM_MOUSEMOVE, MK_LBUTTON, MAKELPARAM(xStart - i, yStart + delta));
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

		for (int i = 0; i < slideDistance; ++i)
		{
			if (boolRand(rng) && delta < deviation)
				delta += boolRand(rng);
			else if (delta > -deviation)
				delta -= boolRand(rng);

			SendMessage(m_window, WM_MOUSEMOVE, MK_LBUTTON, MAKELPARAM(xStart + i, yStart + delta));
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

		for (int i = 0; i < slideDistance; ++i)
		{
			if (boolRand(rng) && delta < deviation)
				delta += boolRand(rng);
			else if (delta > -deviation)
				delta -= boolRand(rng);

			SendMessage(m_window, WM_MOUSEMOVE, MK_LBUTTON, MAKELPARAM(xStart + delta, yStart + i));
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

		for (int i = 0; i < slideDistance; ++i)
		{
			if (boolRand(rng) && delta < deviation)
				delta += boolRand(rng);
			else if (delta > -deviation)
				delta -= boolRand(rng);

			SendMessage(m_window, WM_MOUSEMOVE, MK_LBUTTON, MAKELPARAM(xStart + delta, yStart - i));
			Sleep(1);
		}

		SendMessage(m_window, WM_LBUTTONUP, 0, MAKELPARAM(xStart + delta, yStart - (slideDistance - 1)));
		break;
	}

	Sleep(1000);
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

void CAEBot::Walk(Direction_Info botDirection, int time, int sleepTime)
{
	int delta = 0;
	int slideDistance;
	int deviation = 3;
	int xStart = (boolRand(rng) ? (m_xCenter + slideLClick(rng)) : (m_xCenter - slideLClick(rng)));
	int yStart = (boolRand(rng) ? (m_yCenter + slideLClick(rng)) : (m_yCenter - slideLClick(rng)));

	snprintf(m_debugMsg, 1024, "Walk %d %d %d\n", botDirection, time, sleepTime);
	dbgMsg(m_IsDebug_Platform);

	//Start walking left
	SendMessage(m_window, WM_MOUSEACTIVATE, 0, MAKELPARAM(HTCLIENT, WM_LBUTTONDOWN));
	SendMessage(m_window, WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(xStart, yStart));
	Sleep(10);

	switch (botDirection)
	{
	case LEFT:
		deviation = (int) round(deviation * m_heightPct);
		slideDistance = (boolRand(rng) ? slideDistanceRand(rng) : (-1) * slideDistanceRand(rng)) + 300;
		slideDistance = (int) round(slideDistance * m_widthPct);
		for (int i = 0; i < slideDistance; ++i)
		{
			if (boolRand(rng) && delta < 3)
				delta += boolRand(rng);
			else if (delta > -3)
				delta -= boolRand(rng);

			SendMessage(m_window, WM_MOUSEMOVE, MK_LBUTTON, MAKELPARAM(xStart - i, yStart + delta));
			Sleep(1);
		}

		Sleep(time);
		SendMessage(m_window, WM_LBUTTONUP, 0, MAKELPARAM(xStart - (slideDistance - 1), yStart + delta));
		break;

	case RIGHT:
		deviation = (int) round(deviation * m_heightPct);
		slideDistance = (boolRand(rng) ? slideDistanceRand(rng) : (-1) * slideDistanceRand(rng)) + 300;
		slideDistance = (int) round(slideDistance * m_widthPct);
		for (int i = 0; i < slideDistance; ++i)
		{
			if (boolRand(rng) && delta < 3)
				delta += boolRand(rng);
			else if (delta > -3)
				delta -= boolRand(rng);

			SendMessage(m_window, WM_MOUSEMOVE, MK_LBUTTON, MAKELPARAM(xStart + i, yStart + delta));
			Sleep(1);
		}

		Sleep(time);
		SendMessage(m_window, WM_LBUTTONUP, 0, MAKELPARAM(xStart + (slideDistance - 1), yStart + delta));
		break;

	case DOWN:
		deviation = (int) round(deviation * m_widthPct);
		slideDistance = (boolRand(rng) ? slideDistanceRand(rng) : (-1) * slideDistanceRand(rng)) + 300;
		slideDistance = (int) round(slideDistance * m_heightPct);
		for (int i = 0; i < slideDistance; ++i)
		{
			if (boolRand(rng) && delta < 3)
				delta += boolRand(rng);
			else if (delta > -3)
				delta -= boolRand(rng);

			SendMessage(m_window, WM_MOUSEMOVE, MK_LBUTTON, MAKELPARAM(xStart + delta, yStart + i));
			Sleep(1);
		}

		Sleep(time);
		SendMessage(m_window, WM_LBUTTONUP, 0, MAKELPARAM(xStart + delta, yStart + (slideDistance - 1)));
		break;

	case UP:
		deviation = (int) round(deviation * m_widthPct);
		slideDistance = (boolRand(rng) ? slideDistanceRand(rng) : (-1) * slideDistanceRand(rng)) + 300;
		slideDistance = (int) round(slideDistance * m_heightPct);
		for (int i = 0; i < slideDistance; ++i)
		{
			if (boolRand(rng) && delta < 3)
				delta += boolRand(rng);
			else if (delta > -3)
				delta -= boolRand(rng);

			SendMessage(m_window, WM_MOUSEMOVE, MK_LBUTTON, MAKELPARAM(xStart + delta, yStart - i));
			Sleep(1);
		}

		Sleep(time);
		SendMessage(m_window, WM_LBUTTONUP, 0, MAKELPARAM(xStart + delta, yStart - (slideDistance - 1)));
		break;

	}

	Sleep(sleepTime);
}

void CAEBot::clickAttack(int time)
{
	leftClick(m_Button_Attack, time);
}

bool CAEBot::inBattle()
{
	Mat partialPic;
	bitBltWholeScreen();
	copyPartialPic(partialPic, 106, 39, 77, 37);
	bool inBattleResult = (getText(partialPic).compare("Status") == 0);

	snprintf(m_debugMsg, 1024, "in battle %x\n", inBattleResult);
	dbgMsg(m_IsDebug_Fighting);

	return inBattleResult;
}

bool CAEBot::endBattle()
{
	Mat partialPic1, partialPic2;
	bitBltWholeScreen();

	copyPartialPic(partialPic1, 150, 70, 20, 40);
	bool endBattleResult1 = (getText(partialPic1).compare("Items") == 0);

	copyPartialPic(partialPic2, 400, 70, 660, 80);
	bool endBattleResult2 = (getText(partialPic2).find("Got") != string::npos);

	snprintf(m_debugMsg, 1024, "end battle %x %x\n", endBattleResult1, endBattleResult2);
	dbgMsg(m_IsDebug_Fighting);

	return (endBattleResult1 || endBattleResult2);
}

Status_Code CAEBot::fightUntilEnd()
{
	Sleep(1000);

	if (!inBattle()) // not a fight
	{
		m_resValue = status_NotFight;
		return m_resValue;
	}

	return engageMobFightNow();
}

Status_Code CAEBot::WalkUntilBattle(Direction_Info botdirection)
{
	struct tm timeinfo;
	char buf[80], lastbuf[80];
	time_t rawtime, lastrawtime;

	lastrawtime = time(NULL);
	localtime_s(&timeinfo, &lastrawtime);
	strftime(lastbuf, sizeof(lastbuf), "%Y-%m-%d_%H-%M-%S", &timeinfo);

	while (!inBattle())
	{
		rawtime = time(NULL);
		localtime_s(&timeinfo, &rawtime);
		strftime(buf, sizeof(buf), "%Y-%m-%d_%H-%M-%S", &timeinfo);

		auto timegap = difftime(rawtime, lastrawtime);

		if (timegap > m_Fight_Timeout) // return if timeout
		{
			if (m_IsPrint) captureScreenNow("WalkUntilBattle");
			m_resValue = status_Timeout;
			return m_resValue;
		}

		switch (botdirection)
		{
		case LEFT:
			Walk(LEFT, m_Fight_GrindingStep);
			break;
		case RIGHT:
			Walk(RIGHT, m_Fight_GrindingStep);
			break;
		case LR:
		default:
			Walk(LEFT, m_Fight_GrindingStep);
			Walk(RIGHT, m_Fight_GrindingStep);
			break;
		}
	}

	return fightUntilEnd();
}

Status_Code CAEBot::engageMobFightNow(int horrorThreshold)
{
	struct tm timeinfo;
	char buf[80], lastbuf[80];
	time_t rawtime, lastrawtime;

	int num_loop = (int)m_skillsMobSet.size();
	int iRun = 0;
	vector<int> lastSkillsRow = { 0, 0, 0, 0 };
	bool isThisPSlime = false;

	if (m_IsPrint) captureScreenNow("Mob");

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

			rawtime = time(NULL);
			localtime_s(&timeinfo, &rawtime);
			strftime(buf, sizeof(lastbuf), "%Y-%m-%d_%H-%M-%S", &timeinfo);

			snprintf(m_debugMsg, 1024, "Plantium Slime encountered at %s - %s\n", m_currentLocation.c_str(), buf);
			dbgMsg(m_IsDebug_LOM);
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

				leftClick(m_Button_Characters[j].xyPosition, m_Fight_ActionInterval);
				if (iSkill == m_Skill_Exchange_A || iSkill == m_Skill_Exchange_B)
				{
					leftClick(m_Button_Skills[m_Skill_Exchange].xyPosition, m_Fight_ActionInterval);
					leftClick(m_Button_Characters[iSkill - m_Skill_Exchange + m_CharacterFrontline].xyPosition, m_Fight_ActionInterval);
				}
				else
					leftClick(m_Button_Skills[iSkill].xyPosition, m_Fight_ActionInterval);

				// Click normal skill in case the skill is blocked
				leftClick(m_Button_Skills[0].xyPosition, m_Fight_ActionInterval);
				// Click front buttom in case someone is defeated
				leftClick(m_Button_Skills[4].xyPosition, m_Fight_ActionInterval);
				// Click somewhere else in case someone is disabled
				leftClick(m_Button_Yes, m_Fight_ActionInterval);
			}
		}

		clickAttack(200);

		lastrawtime = time(NULL);
		localtime_s(&timeinfo, &lastrawtime);
		strftime(lastbuf, sizeof(lastbuf), "%Y-%m-%d_%H-%M-%S", &timeinfo);

		while (!inBattle())
		{
			rawtime = time(NULL);
			localtime_s(&timeinfo, &rawtime);
			strftime(buf, sizeof(buf), "%Y-%m-%d_%H-%M-%S", &timeinfo);

			auto timegap = difftime(rawtime, lastrawtime);

			if (timegap > m_Fight_Timeout) // return if timeout
			{
				break;
			}

			if (endBattle())
			{
				break;
			}
			Sleep(200);
		}

		iRun++;
	} while (inBattle());

	clickAttack(200); //Get past the results screen
	clickAttack(200); //Get past the results screen
	clickAttack(200); //Get past the results screen

	if (m_botMode == grindingLOMSlimeMode)
	{
		clickAttack(1000); //Get past the results screen
	}
	m_resValue = status_NoError;
	return m_resValue;
}

Status_Code CAEBot::engageHorrorFightNow(bool restoreHPMP)
{
	cv::Mat partialPic;
	string textToCheck;

	struct tm timeinfo;
	char buf[80], lastbuf[80];
	time_t rawtime, lastrawtime;

	Sleep(1000);

	if (!inBattle()) // not horror
	{
		leftClick(m_xCenter, m_yCenter, 3000);
		m_resValue = status_NotFight;
		return m_resValue;
	}

	while (!inBattle())
	{
		Walk(LEFT, 5000, 200);
		Walk(RIGHT, 5000, 200);
	}

	int num_loop = (int)m_skillsHorrorSet.size();

	rawtime = time(NULL);
	localtime_s(&timeinfo, &rawtime);
	strftime(buf, sizeof(buf), "%Y-%m-%d_%H-%M-%S", &timeinfo);

	Mat imagePicCrop;

	if (m_IsPrint) captureScreenNow("Horror");

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
				m_resValue = status_FightFail;
				return m_resValue;
			}
		}

		// check whether AF bar is full
		copyPartialPic(imagePicCrop, 730, 50, 880, 30); // AF bar 880, 30, 730, 50.  AF bar 860, 10, 770, 90
		double MSD1;
		MSD1 = cv::norm(afBarFullPic, imagePicCrop);
		MSD1 = MSD1 * MSD1 / afBarFullPic.total();

		if (MSD1 < m_Fight_AFFullThreshold || isAFon)
		{
			rawtime = time(NULL);
			localtime_s(&timeinfo, &rawtime);
			strftime(buf, sizeof(buf), "%Y-%m-%d_%H-%M-%S", &timeinfo);

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

			rawtime = time(NULL);
			localtime_s(&timeinfo, &rawtime);
			strftime(buf, sizeof(buf), "%Y-%m-%d_%H-%M-%S", &timeinfo);

			if (m_IsPrint) captureScreenNow("AFDone");

			iRun = 0;
			isAFon = false;
		}
		else 
		{
			iRun = iRun % num_loop;

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

				leftClick(m_Button_Characters[j].xyPosition, 200);
				if (iSkill == m_Skill_Exchange_A || iSkill == m_Skill_Exchange_B)
				{
					leftClick(m_Button_Skills[m_Skill_Exchange].xyPosition, 200);
					leftClick(m_Button_Characters[iSkill - m_Skill_Exchange + m_CharacterFrontline].xyPosition, 200);
				}
				else
					leftClick(m_Button_Skills[iSkill].xyPosition, 200);

				// Click normal skill in case the skill is blocked
				leftClick(m_Button_Skills[0].xyPosition, 200);
				// Click front buttom in case someone is defeated
				leftClick(m_Button_Skills[4].xyPosition, 200);
				// Click somewhere else in case someone is disabled
				leftClick(m_Button_Yes, 200);
			}

			if (isAFon) 
			{
				continue;
			}

			clickAttack(500); // Fight Horror
			iRun++;
		}

		lastrawtime = time(NULL);
		localtime_s(&timeinfo, &lastrawtime);
		strftime(lastbuf, sizeof(lastbuf), "%Y-%m-%d_%H-%M-%S", &timeinfo);

		while (!inBattle())
		{
			rawtime = time(NULL);
			localtime_s(&timeinfo, &rawtime);
			strftime(buf, sizeof(buf), "%Y-%m-%d_%H-%M-%S", &timeinfo);

			auto timegap = difftime(rawtime, lastrawtime);

			if (timegap > m_Fight_Timeout) // return if timeout
			{
				break;
			}

			if (endBattle())
			{
				break;
			}

			Sleep(500);
		}

	} while (inBattle());

	rawtime = time(NULL);
	localtime_s(&timeinfo, &rawtime);
	strftime(buf, sizeof(buf), "%Y-%m-%d_%H-%M-%S", &timeinfo);

	if (m_IsPrint) captureScreenNow("Result");

	Sleep(4000);

	clickAttack(); //Get past the results screen
	clickAttack(); //Get past the fish results screen

	Sleep(4000);

	if (restoreHPMP)
	{
		// restore HP & MP
		leftClick(m_Button_Menu);
		leftClick(m_Button_Food);
		leftClick(m_Button_Yes, 4000);
		leftClick(m_Button_Yes);
		leftClick(m_Button_Yes);
		leftClick(m_Button_Yes);
	}

	m_resValue = status_NoError;
	return m_resValue;
}

void CAEBot::fish(vector<pair<int, int>>& sections, int m_msdThreshold, int horrorThreshold)
{
	//Strategy is to quadrisect the lake and toss into each of the four sections and center
	int catchIndex = 0, horrorIndex = 0, monsterIndex = 0;
	struct tm timeinfo;
	char buf[80];
	time_t rawtime, lastCatch;

	rawtime = time(NULL);
	localtime_s(&timeinfo, &rawtime);
	strftime(buf, sizeof(buf), "%Y-%m-%d_%H-%M-%S", &timeinfo);
	lastCatch = rawtime;

	snprintf(m_debugMsg, 1024, "Starting fishing at %s - %s\n", m_currentLocation.c_str(), buf);
	dbgMsg(m_IsDebug_Fishing);

	Sleep(1000);

	bitBltWholeScreen();
	Mat lakeImg = bitbltPic.clone();

	for (int i = 0; i < m_currentBaitsToUse->size(); ++i)
	{
		if (m_baitList[(*m_currentBaitsToUse)[i]].first == false) //If we have 0 of current bait, go to next bait
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
				if (m_IsPrint) captureScreenNow("NOTFISHING");

				rawtime = time(NULL);
				localtime_s(&timeinfo, &rawtime);
				strftime(buf, sizeof(buf), "%Y-%m-%d_%H-%M-%S", &timeinfo);

				leftClick(m_Button_X);
				return;
			}

			//Cast line
			leftClick(sections[j].first, sections[j].second);

			string status = ocrPictureText(1242, 71, 257, 80);
			if (status.find("bait") != string::npos) //Out of bait
			{
				m_baitList[(*m_currentBaitsToUse)[i]].first = false;

				do //Look for a bait that we actually have
				{
					++i;
				} while (i < m_currentBaitsToUse->size() && m_baitList[(*m_currentBaitsToUse)[i]].first == false);

				if (i >= m_currentBaitsToUse->size())
					return;

				changeBait((*m_currentBaitsToUse)[i]);
				leftClick(sections[j].first, sections[j].second);
			}
			else if (status.find("any fish") != string::npos || status.find("box") != string::npos) //Pool is empty or cooler is full
			{
				rawtime = time(NULL);
				localtime_s(&timeinfo, &rawtime);
				strftime(buf, sizeof(buf), "%Y-%m-%d_%H-%M-%S", &timeinfo);

				snprintf(m_debugMsg, 1024, "Leaving %s (%d/%d) - %s\n", m_currentLocation.c_str(), horrorIndex, catchIndex, buf);
				dbgMsg(m_IsDebug_Fishing);

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
				MSD = cv::norm(lakeImg, bitbltPic);
				MSD = MSD * MSD / lakeImg.total();

				if (MSD > m_msdThreshold) //If the current screen is sufficiently different (high mean square difference) from the normal lake image, then a zoom in has occurred
				{
					Sleep(100); //Emulate human reaction time
					leftClick(sections[j].first, sections[j].second);

					catchIndex++;

					//Click through success or failure
					Sleep(4000);
					leftClick(sections[j].first, sections[j].second);
					Sleep(2000);

					bitBltWholeScreen();
					MSD = cv::norm(lakeImg, bitbltPic);
					MSD = MSD * MSD / lakeImg.total();
					if (MSD > m_msdThreshold) //Double, gotta click past it
					{
						leftClick(sections[j].first, sections[j].second);
						Sleep(2000);

						bitBltWholeScreen();
						MSD = cv::norm(lakeImg, bitbltPic);
						MSD = MSD * MSD / lakeImg.total();
						if (MSD > m_msdThreshold) //Triple
						{
							leftClick(sections[j].first, sections[j].second);
							Sleep(2000);
						}
					}

					//Check for battle
					bitBltWholeScreen();
					MSD = cv::norm(lakeImg, bitbltPic);
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
									rawtime = time(NULL);
									localtime_s(&timeinfo, &rawtime);
									strftime(buf, sizeof(buf), "%Y-%m-%d_%H-%M-%S", &timeinfo);

									snprintf(m_debugMsg, 1024, "Horror or Lake Lord detected at %s %d - %s\n", m_currentLocation.c_str(), horrorIndex, buf);
									dbgMsg(m_IsDebug_Fishing);

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
											snprintf(m_debugMsg, 1024, "Leaving %s (%d/%d) catched - Horror count reached %s\n", m_currentLocation.c_str(), horrorIndex, catchIndex, buf);
											dbgMsg(m_IsDebug_Fishing);
											return;
										}
									}
									else
										exit(0);
								}
								else
								{
									snprintf(m_debugMsg, 1024, "Fighting at %s monster %x\n", m_currentLocation.c_str(), monsterIndex);
									dbgMsg(m_IsDebug_Fishing);
								}
							}

							fightUntilEnd();
							clickAttack(); //Click past fish results screen
							Sleep(5000);

							fishIconClickFunction();

							Sleep(1000);
						} 
						else // not a battle, probably a fish
						{
							leftClick(sections[j].first, sections[j].second);
							Sleep(2000);
						}
					}

					rawtime = time(NULL);
					localtime_s(&timeinfo, &rawtime);
					lastCatch = rawtime;
					lastSection = j;
					break;

				}
				else
				{
					// if wait too long, let's move to other area.
					rawtime = time(NULL);
					localtime_s(&timeinfo, &rawtime);
					strftime(buf, sizeof(buf), "%Y-%m-%d_%H-%M-%S", &timeinfo);

					auto timegap = difftime(rawtime, lastCatch);

					if (timegap > 300)
					{
						snprintf(m_debugMsg, 1024, "Leaving %s (%d/%d) catched - Idling %s\n", m_currentLocation.c_str(), horrorIndex, catchIndex, buf);
						dbgMsg(m_IsDebug_Fishing);

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

	rawtime = time(NULL);
	localtime_s(&timeinfo, &rawtime);
	strftime(buf, sizeof(buf), "%Y-%m-%d_%H-%M-%S", &timeinfo);

	snprintf(m_debugMsg, 1024, "Leaving %s (%d/%d) catched - All baits tried %s\n", m_currentLocation.c_str(), horrorIndex, catchIndex, buf);
	dbgMsg(m_IsDebug_Fishing);
}

void CAEBot::changeBait(Bait_Type type)
{
	int slotNum = 0;
	if (type > 0)
	{
		//Calculate slot that bait is in
		for (int i = 0; i < type; ++i)
		{
			if (m_baitList[i].first == true)
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

		leftClick(m_Locs_baits[5].first, m_Locs_baits[5].second);
	}
	else
		leftClick(m_Locs_baits[slotNum].first, m_Locs_baits[slotNum].second);
};

Status_Code CAEBot::goToTargetLocation(vector<pathInfo> pathInfoList)
{
	struct tm timeinfo;
	char buf[80];
	time_t rawtime;

	rawtime = time(NULL);
	localtime_s(&timeinfo, &rawtime);
	strftime(buf, sizeof(buf), "%Y-%m-%d_%H-%M-%S", &timeinfo);

	snprintf(m_debugMsg, 1024, "----> %s %s\n", m_currentLocation.c_str(), buf);
	dbgMsg(m_IsDebug_Path);

	for (auto j = 0; j < pathInfoList.size(); j++)
	{
		string curType = pathInfoList[j].type;
		string curValue1 = pathInfoList[j].value1;
		string curValue2 = pathInfoList[j].value2;

		snprintf(m_debugMsg, 1024, "%s %s %s\n", curType.c_str(), curValue1.c_str(), curValue2.c_str());
		dbgMsg(m_IsDebug_Path);

		if (curType.compare("Click") == 0)
		{
			if (!curValue2.empty())
			{
				int x = stoi(curValue1);
				int y = stoi(curValue2);

				x = (int)round(x * M_WIDTH / M_WIDTH_1280);
				y = (int) round(y * M_HEIGHT / M_HEIGHT_720);

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
					leftClick(m_Button_Map, 3500);
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
				Sleep(m_loadTime);
			}
			else
			{
				Sleep(stoi(curValue1));
			}
		}
		else if (curType.compare("Walk") == 0) {

			if (curValue1.compare("LEFT") == 0)
			{
				Walk(LEFT, stoi(curValue2));
			}
			else if (curValue1.compare("RIGHT") == 0)
			{
				Walk(RIGHT, stoi(curValue2));
			}
			else if (curValue1.compare("DOWN") == 0)
			{
				Walk(DOWN, stoi(curValue2));
			}
			else if (curValue1.compare("UP") == 0)
			{
				Walk(UP, stoi(curValue2));
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
			if (curValue1.compare("Exclamation") == 0)
			{
				pair<int, int> pnt = findExclamationIcon();
				leftClick(pnt.first, pnt.second, 2000, false);
			}
			if (curValue1.compare("ExclamationRift") == 0)
			{
				pair<int, int> pnt = findExclamationIcon(excl_Rift);
				leftClick(pnt.first, pnt.second, 2000, false);
			}
			if (curValue1.compare("ExclamationSpecter") == 0)
			{
				pair<int, int> pnt = findExclamationIcon(excl_Sepcter);
				leftClick(pnt.first, pnt.second, 2000, false);
			}
			if (curValue1.compare("ExclamationGrasta") == 0)
			{
				pair<int, int> pnt = findExclamationIcon(excl_Grasta);
				leftClick(pnt.first, pnt.second, 2000, false);
			}
			if (curValue1.compare("ExclamationChamber") == 0)
			{
				pair<int, int> pnt = findExclamationIcon(excl_Chamber);
				leftClick(pnt.first, pnt.second, 2000, false);
			}
			if (curValue1.compare("ExclamationChamberPlasma") == 0)
			{
				pair<int, int> pnt = findExclamationIcon(excl_ChamberPlasma);
				leftClick(pnt.first, pnt.second, 2000, false);
			}
			if (curValue1.compare("ExclamationKMS") == 0)
			{
				pair<int, int> pnt = findExclamationIcon(excl_KMS);
				leftClick(pnt.first, pnt.second, 2000, false);
			}
			else if (curValue1.compare("Door") == 0)
			{
				pair<int, int> pnt = findDoorIcon(door_Default);
				leftClick(pnt.first, pnt.second, 2000, false);
			}
			else if (curValue1.compare("DoorKunlun") == 0)
			{
				pair<int, int> pnt = findDoorIcon(door_Kunlun);
				leftClick(pnt.first, pnt.second, 2000, false);
			}
			else if (curValue1.compare("DoorLOM") == 0)
			{
				pair<int, int> pnt = findDoorIcon(door_LOM);
				leftClick(pnt.first, pnt.second, 2000, false);
			}
			else if (curValue1.compare("Stair") == 0)
			{
				pair<int, int> pnt = findStairIcon(stair_Default);
				leftClick(pnt.first, pnt.second, 2000, false);
			}
			else if (curValue1.compare("StairDown") == 0)
			{
				pair<int, int> pnt = findStairIcon(stair_Down);
				leftClick(pnt.first, pnt.second, 2000, false);
			}
			else if (curValue1.compare("StairLOM") == 0)
			{
				pair<int, int> pnt = findStairIcon(stair_LOM);
				leftClick(pnt.first, pnt.second, 2000, false);
			}
			else if (curValue1.compare("SwampFishIcon") == 0)
			{
				findAndClickSwampFishIcon();
			}
			else if (curValue1.compare("FishIcon") == 0)
			{
				findAndClickFishIcon();
			}
			else // dynamic image
			{
				for (auto k = 0; k < m_DynamicImage.size(); k++)
				{
					if (curValue1.compare(m_DynamicImage[k].name) == 0)
					{
						pair<int, int> iconLoc = findIcon(m_DynamicImage[k].image);
						iconLoc.first += (int)round(m_DynamicImage[k].width * m_widthPct);
						iconLoc.second += (int)round(m_DynamicImage[k].height * m_heightPct);
						leftClick(iconLoc.first, iconLoc.second, 2000, false);
						break;
					}
				}
			}
		}
		else if (curType.compare("Fight") == 0) {
			if (curValue1.compare("UntilEnd") == 0)
			{
				fightUntilEnd();
			}
			else if (curValue1.compare("UntilBattle") == 0)
			{
				if (curValue2.empty())
					m_resValue = WalkUntilBattle();
				else if (curValue2.compare("LEFT") == 0)
					m_resValue = WalkUntilBattle(LEFT);
				else if (curValue2.compare("RIGHT") == 0)
					m_resValue = WalkUntilBattle(RIGHT);
				else
					m_resValue = WalkUntilBattle();

				if (m_resValue == status_Timeout)
					return m_resValue;
			}
		}
		else if (curType.compare("Capture") == 0) {
			if (curValue1.compare("Screen") == 0)
			{
				captureScreenNow();
			}
		}
	}

	m_resValue = status_NoError;
	return m_resValue;
}

void CAEBot::goToFishingLocation()
{
	for (int i = 0; i < m_LocationList.size(); i++)
	{
		if (m_LocationList[i].locationName.compare(m_currentLocation) == 0)
		{
			goToTargetLocation(m_LocationList[i].pathStepsList);
		}
	}
}

void CAEBot::goToSpacetimeRift(bool heal)
{
	snprintf(m_debugMsg, 1024, "Spacetime Rift [%x]\n", heal);
	dbgMsg(m_IsDebug_Path);

	leftClick(m_Button_Map, 3500);
	leftClick(m_Button_EndOfTimeLoc);
	leftClick(m_Button_SpacetimeRift);
	leftClick(m_Button_SpacetimeRift);
	leftClick(m_Button_Yes);
	Sleep(m_loadTime);

	if (heal)
	{
		leftClick(m_Button_Tree);
		leftClick(m_Button_Tree);
		leftClick(m_Button_Yes, 4000);
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

	pair<int, int> pnt = findExclamationIcon();
	leftClick(pnt.first, pnt.second, 2000, false);

	for (int i = 0; i < 30; ++i) //click through text
		leftClick(380, 355, 100);

	leftClick(878, 888);

	//Buy baits if needed
	const static vector<pair<int, int>> baitsLocs = { {1200, 136}, {1200, 292}, {1200, 448}, {1200, 604}, {1200, 760}, {1200, 916} };
	const static vector<int> baitCosts = { 500, 1000, 3000, 1, 2, 3, 4, 6, 10, 30, 4000, 6000 };
	string currText;

	//Only 5 bait slots are fully on screen at a time, with the 6th partially visible. For every bait after 6, scroll down until new bait occupies bottom slot and then attempt to buy
	for (int i = 0; i < m_baitList.size(); i++)
	{
		if (i > 4)
		{
			if (i == 5)
				drag(DOWN, 60, 1200, 300);
			else
				drag(DOWN, 135, 1200, 300);
			leftClick(baitsLocs[5].first, baitsLocs[5].second);
		}
		else
			leftClick(baitsLocs[i].first, baitsLocs[i].second);

		//If the next one we've done is the same as the last, we've reached the end of the list
		string txt = ocrPictureText(577, 51, 100, 102);
		if (currText.compare(txt) == 0)
			break;
		currText = txt;

		while (txt.compare(m_baitNames[i]) != 0 && i < m_baitList.size()) //You don't get the baits in order, so some may be missing depending on your progression (blabber sardines)
			++i;

		//Get amount currently held
		int held = ocrPictureNumber(141, 30, 500, 165);
		int numToBuy = m_baitList[i].second - held;

		snprintf(m_debugMsg, 1024, "Buy %s %d\n", txt.c_str(), m_baitList[i].second);
		dbgMsg(m_IsDebug_Fishing);

		if (numToBuy > 0)
		{
			if (i > 2 && i < 10) //Fish point bait. Here, since fish points aren't infinite like gold, only buy as much as you can up to the max set in config
			{
				//Get amount of fish points owned
				int fishPoints = ocrPictureNumber(193, 28, 1346, 10);
				//Get cost of current selected bait

				int fpCost = baitCosts[i];
				if (fpCost < fishPoints) //Can we even afford one?
				{
					if (numToBuy * fpCost >= fishPoints) //If we don't have enough fish points to buy the max set, then buy as many as we can
						numToBuy = (int)floor(fishPoints / fpCost);
				}
			}

			int thisTimeToBuy;
			snprintf(m_debugMsg, 1024, "Bait [%d]: %d holding %d to buy %d. buying...\n", i, m_baitList[i].second, held, numToBuy);
			dbgMsg(m_IsDebug_Fishing);


			for (; numToBuy > 0; numToBuy -= thisTimeToBuy)
			{
				if (numToBuy > 99)
					thisTimeToBuy = 99;
				else 
					thisTimeToBuy = numToBuy;

				if (thisTimeToBuy > 0)
				{
					for (int j = 0; j < thisTimeToBuy - 1; ++j)
						leftClick(555, 665, 500);

					leftClick(350, 850);
					leftClick(1070, 650);
					leftClick(880, 575);

					held = ocrPictureNumber(141, 30, 500, 165);
				}
			}
		}

		m_baitList[i].first = held > 0;
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
		findAndClickFishIcon();
	}
	else if (m_currentLocation.compare("Man-Eating Swamp") == 0)
	{
		findAndClickSwampFishIcon();
	}
	else
	{
		leftClick(m_currentFishIconLoc.first, m_currentFishIconLoc.second, 2000);
	}
}

void CAEBot::stateFishing()
{
	while (1)
	{
		if (m_resValue == status_Stop)
			return;

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

void CAEBot::stateSilverHitBell(Bot_Mode silverHitBellstate)
{
	Mat bellPicCrop;
	int MSD1;
	int lowMSD = (int)99999999999;

	loadSettingConfig();

	while (1)
	{
		if (m_resValue == status_Stop)
			return;

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
			if (silverHitBellstate == silverHitBell30Mode && i > 30)
				break;
			else if (silverHitBellstate == silverHitBell999Mode && i > 999)
				return;

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

void CAEBot::stateJumpRopeRatle()
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
			return;

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
}

void CAEBot::stateJumpRopeBaruoki()
{
	Mat jmpRopePicCrop;
	int MSD1;
	int lowMSD = (int) 99999999999;

	loadSettingConfig();

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
			return;

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
}

void CAEBot::stateSeparateGrasta()
{
	loadSettingConfig();

	Sleep(1000);

	pair<int, int> pnt = findExclamationIcon(excl_Grasta);
	leftClick(pnt.first, pnt.second, 2000, false);

	leftClick(880, 530);

	if (m_grastaType == grasta_Life)
		leftClick(535, 290);
	else if (m_grastaType == grasta_Support)
		leftClick(535, 460);
	else if (m_grastaType == grasta_Special)
		leftClick(535, 630);

	//Buy baits if needed
	const static vector<pair<int, int>> ItemLocs = { {1220, 160}, {1215, 325}, {1215, 465}, {1215, 625}, {1215, 800}, {1225, 925} };

	snprintf(m_debugMsg, 1024, "To separate grasta from [%d]\n", m_lastGrasta);
	dbgMsg(m_IsDebug_Grasta);

	int i = 0;
	//Only 5  slots are fully on screen at a time, with the 6th partially visible. For every one after 6, scroll down until new one occupies bottom slot and then attempt to check

	int k=0;

	while (1)
	{
		if (m_resValue == status_Stop)
			return;

		if (k < 5)
		{
			leftClick(ItemLocs[k].first, ItemLocs[k].second, 200);
		}
		else
		{
			if (k < m_lastGrasta)
			{
				drag(DOWN, m_SectionHeight * (m_lastGrasta - 5) + 60, 1225, 900);
				k = m_lastGrasta;
			}
			else
			{
				if (k == 5)
					drag(DOWN, 60, 1225, 900);
				else 
					drag(DOWN, 135, 1225, 900);
			}
			leftClick(ItemLocs[5].first, ItemLocs[5].second, 200);
		}

		string txt;
		//If the next one we've done is the same as the last, we've reached the end of the list
		if (m_grastaType == grasta_Attack)
			txt = ocrPictureText(577, 51, 100, 102);
		else if (m_grastaType == grasta_Life)
			txt = ocrPictureText(577, 51, 100, 102);

		string attr_1 = ocrPictureText(110, 30, 390, 170);
		int value_1 = ocrPictureNumber(110, 30, 565, 170);
		string attr_2 = ocrPictureText(110, 30, 390, 225);
		int value_2 = ocrPictureNumber(110, 30, 565, 225);

		//found grasta to separate
		snprintf(m_debugMsg, 1024, "Grasta [%d] %s: %s %d %s %d\n", k, txt.c_str(), attr_1.c_str(), value_1, attr_2.c_str(), value_2);
		dbgMsg(m_IsDebug_Grasta);

		for (auto j = 0; j < m_grastaNames.size(); j++)
		{
			if (txt.find(m_grastaNames[j]) != string::npos)
			{
				m_lastGrasta = k;
				snprintf(m_debugMsg, 1024, "To separate [%d]\n", m_lastGrasta);
				dbgMsg(m_IsDebug_Grasta);

				leftClick(350, 870, 500);
				leftClick(1080, 600, 500);
				leftClick(880, 735, 500);
				Sleep(1000);
				k = 0;

				break;
			}
		}
		k++;
	}

	leftClick(m_Button_X);
}

Status_Code CAEBot::grindingRun(bool endlessGrinding)
{
	struct tm timeinfo;
	char buf[80];
	char buflastfight[80];
	time_t rawtime, lastFight;

	rawtime = time(NULL);
	localtime_s(&timeinfo, &rawtime);
	strftime(buf, sizeof(buf), "%Y-%m-%d_%H-%M-%S", &timeinfo);

	snprintf(m_debugMsg, 1024, "Start grinding %s [%x] %s\n", m_currentLocation.c_str(), endlessGrinding, buf);
	dbgMsg(m_IsDebug_Grinding);

	lastFight = rawtime;
	strftime(buflastfight, sizeof(buflastfight), "%Y-%m-%d_%H-%M-%S", &timeinfo);

	int currentDirection = LEFT;

	while (m_currentGrindingCounter < m_Fight_GrindingCount || endlessGrinding)
	{
		if (m_resValue == status_Stop)
			return m_resValue;

		while (!inBattle())
		{
			rawtime = time(NULL);
			localtime_s(&timeinfo, &rawtime);
			strftime(buf, sizeof(buf), "%Y-%m-%d_%H-%M-%S", &timeinfo);

			auto timegap = difftime(rawtime, lastFight);

			if (timegap > m_Fight_Timeout) // return if timeout
			{
				snprintf(m_debugMsg, 1024, "Leaving %s - Idling %s - %s\n", m_currentLocation.c_str(), buflastfight, buf);
				dbgMsg(m_IsDebug_Grinding);

				if (m_IsPrint) captureScreenNow("Idling");

				m_resValue = status_Timeout;
				return m_resValue;
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
				Walk(LEFT, m_Fight_GrindingStep, 10);
				break;
			case RIGHT:
			default:
				Walk(RIGHT, m_Fight_GrindingStep, 10);
			}
		}

		engageMobFightNow();

		lastFight = time(NULL);
		localtime_s(&timeinfo, &lastFight);
		strftime(buflastfight, sizeof(buflastfight), "%Y-%m-%d_%H-%M-%S", &timeinfo);
	}

	rawtime = time(NULL);
	localtime_s(&timeinfo, &rawtime);
	strftime(buf, sizeof(buf), "%Y-%m-%d_%H-%M-%S", &timeinfo);

	snprintf(m_debugMsg, 1024, "Leaving %s (%d) - Counter Number reached - %s\n", m_currentLocation.c_str(), m_currentGrindingCounter, buf);
	dbgMsg(m_IsDebug_Grinding);
	m_resValue = status_NoError;
	return m_resValue;
}

void CAEBot::stateGrindingTravel()
{
	while (1)
	{
		if (m_resValue == status_Stop)
			return;

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
						m_currentGrindingCounter = 0;
						goToSpacetimeRift();
						goToTargetLocation(m_LocationList[j].pathStepsList); //load X 1280 Y 720 coordination 
						grindingRun();
					}
				}
			}
		}

		//Wait to go back for refresh
		Sleep(3000);
	}
}

void CAEBot::stateGrindingLOMSlime()
{
	while (1)
	{
		if (m_resValue == status_Stop)
			return;

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
							goToTargetLocation(m_LocationList[resetindex].pathStepsList); // reset
						}
					}
					break;
				}
			}
		}

		//Wait to go back for refresh
		Sleep(3000);
	}
}

void CAEBot::stateGrinding()
{
	while (1)
	{
		if (m_resValue == status_Stop)
			return;

		loadPathConfig();
		loadSettingConfig();

		grindingRun(true);
	}
}

void CAEBot::captureScreenNow(const char* nameSuffix)
{
	Mat imagePicCrop;
	char filename[MAX_PATH];
	struct tm timeinfo;
	char buf[80];
	time_t rawtime;

	rawtime = time(NULL);
	localtime_s(&timeinfo, &rawtime);
	strftime(buf, sizeof(buf), "%Y-%m-%d_%H-%M-%S", &timeinfo);
	bitBltWholeScreen();
	copyPartialPic(imagePicCrop, (int)M_WIDTH, (int)M_HEIGHT, 0, 0);

	snprintf(filename, MAX_PATH, "%s\\%s_%s.jpg", m_CurrentPath, buf, nameSuffix);

	snprintf(m_debugMsg, 1024, "%s\n", filename);
	dbgMsg(m_IsDebug_Key);

	imwrite(filename, imagePicCrop);
}

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
			m_IsDebug_Platform = debugvalue & 0x80;
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
		else if (key.compare("Run Timeout") == 0)
		{
			m_Fight_Timeout = stoi(value);
		}
		else if (key.compare("Run Count") == 0)
		{
			m_Fight_GrindingCount = stoi(value);
		}
		else if (key.compare("Action Interval") == 0)
		{
			m_Fight_ActionInterval = stoi(value);
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
			m_lastGrasta = stoi(value);
		}
		else if (key.compare("Section Height") == 0)
		{
			m_SectionHeight = stoi(value);
		}
		else if (key.compare("Grasta Type") == 0)
		{
			m_grastaType = stoi(value);
		}
		else if (key.compare("Grasta to Seperate") == 0)
		{
			m_grastaNames = parseGrastaNames(file);
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
		else if (key.compare("CharactersButtons") == 0)
		{
			m_Button_Characters = parseButtons(file);
		}
		else if (key.compare("SkillsButtons") == 0)
		{
			m_Button_Skills = parseButtons(file);
		}
		else if (key.compare("Dynamic Image") == 0)
		{
			parseDynamicImage(file);
		}
	}


	snprintf(m_debugMsg, 1024, "SkillsHorrorSet:::\n");
	dbgMsg(m_IsDebug_Fighting);

	for (auto j = 0; j < m_skillsHorrorSet.size(); j++)
	{
		snprintf(m_debugMsg, 1024, "Turn [%d]: %d %d %d %d\n", j, m_skillsHorrorSet[j][0], m_skillsHorrorSet[j][1], m_skillsHorrorSet[j][2], m_skillsHorrorSet[j][3]);
		dbgMsg(m_IsDebug_Fighting);
	}

	snprintf(m_debugMsg, 1024, "SkillsMobSet:::\n");
	dbgMsg(m_IsDebug_Fighting);

	for (auto j = 0; j < m_skillsMobSet.size(); j++)
	{
		snprintf(m_debugMsg, 1024, "Turn [%d]: %d %d %d %d\n", j, m_skillsMobSet[j][0], m_skillsMobSet[j][1], m_skillsMobSet[j][2], m_skillsMobSet[j][3]);
		dbgMsg(m_IsDebug_Fighting);
	}

	snprintf(m_debugMsg, 1024, "Grinding:::\n");
	dbgMsg(m_IsDebug_Grinding);

	for (auto j = 0; j < m_grindingSpots.size(); j++)
	{
		snprintf(m_debugMsg, 1024, "---> %s %d\n", (m_grindingSpots[j].first).c_str(), m_grindingSpots[j].second);
		dbgMsg(m_IsDebug_Grinding);
	}
}

void CAEBot::loadFishingConfig()
{
	//Read in config file
	ifstream file("config_fishing.ini");

	m_fishingSpots.clear();
	m_baitList.clear();
	set<int> baitsNeeded;
	string str;

	//Set up baits
	for (int i = bait_Fishing_Dango; i < bait_Premium_Crab_Cake + 1; i++)
	{
		m_baitList.push_back(make_pair(false, 0));
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
			fishingSpot tempFishingSpot;
			set<int> tempBaits;
			parseBaitForArea(file, true, tempFishingSpot, tempBaits, m_baitList);
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
			if ((
				fishingLocation.compare("Ancient Battlefield") == 0 || fishingLocation.compare("Baruoki") == 0 || fishingLocation.compare("Acteul") == 0 ||
				fishingLocation.compare("Charol Plains") == 0 || fishingLocation.compare("Elzion Airport") == 0 || fishingLocation.compare("Dimension Rift") == 0 ||
				fishingLocation.compare("Dragon Palace - Inner Wall") == 0 || fishingLocation.compare("Dragon Palace Past - Inner Wall") == 0 ||
				fishingLocation.compare("Dragon Palace - Outer Wall Bamboo") == 0 || fishingLocation.compare("Dragon Palace Past - Outer Wall Bamboo") == 0 ||
				fishingLocation.compare("Dragon Palace - Outer Wall Pine") == 0 || fishingLocation.compare("Dragon Palace Past - Outer Wall Pine") == 0 ||
				fishingLocation.compare("Dragon Palace - Outer Wall Plum") == 0 || fishingLocation.compare("Dragon Palace Past - Outer Wall Plum") == 0 ||
				fishingLocation.compare("Karek Swampland") == 0 || fishingLocation.compare("Kira Beach") == 0 || fishingLocation.compare("Lake Tillian") == 0 ||
				fishingLocation.compare("Last Island") == 0 || fishingLocation.compare("Man-Eating Swamp") == 0 || fishingLocation.compare("Moonlight Forest") == 0 ||
				fishingLocation.compare("Naaru Uplands") == 0 || fishingLocation.compare("Nilva") == 0 || fishingLocation.compare("Rinde") == 0 ||
				fishingLocation.compare("Rucyana Sands") == 0 || fishingLocation.compare("Serena Coast") == 0 || fishingLocation.compare("Snake Neck Igoma") == 0 ||
				fishingLocation.compare("Vasu Mountains") == 0 || fishingLocation.compare("Zol Plains") == 0)
				&& fishingOrder > 0)
			{
				m_fishingSpots.push_back(newFishingSpot);
				vector<pair<bool, int>> tempBaitList;
				parseBaitForArea(file, false, m_fishingSpots.back(), baitsNeeded, tempBaitList);
			}
		}
	}

	snprintf(m_debugMsg, 1024, "Fishing:::\n");
	dbgMsg(m_IsDebug_Fishing);
	for (auto j = 0; j < m_fishingSpots.size(); j++)
	{
		snprintf(m_debugMsg, 1024, "---> %s order %d\n", (m_fishingSpots[j].locationName).c_str(), m_fishingSpots[j].orderNumber);
		dbgMsg(m_IsDebug_Fishing);
	}

	std::sort(m_fishingSpots.begin(), m_fishingSpots.end(), [](fishingSpot& lhs, fishingSpot& rhs) { return lhs.orderNumber < rhs.orderNumber; });

	//Check to see if any spots actually use the bait we've selected to buy. If not, don't buy any for this run
	for (int i = 0; i < m_baitList.size(); ++i)
	{
		if (baitsNeeded.find(i) == baitsNeeded.end())
			m_baitList[i].second = 0;
	}

	for (int i = 0; i < m_baitList.size(); ++i)
	{
		snprintf(m_debugMsg, 1024, "Bait to buy %x number %d\n", m_baitList[i].first, m_baitList[i].second);
		dbgMsg(m_IsDebug_Fishing);
	}
}

void CAEBot::loadPathConfig()
{
	//Read in config file
	ifstream file("config_path.ini");

	m_LocationList.clear();
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
			m_LocationList.push_back(locInfo);
			parsePathList(file, m_LocationList.back());
		}
	}
}

void CAEBot::setup()
{
	//Read in config file
	ifstream file("config.ini");

	string emulator, windowName, innerWindowName;

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

		if (key.compare("Window Name") == 0)
		{
			windowName = value;
		}
		if (key.compare("Emulator Name") == 0)
		{
			emulator = value;
		}
		if (key.compare("InnerWindow Name") == 0)
		{
			innerWindowName = value;
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
				m_botMode = grindingLOMSlimeMode;
		}
		else if (key.compare("Fishing") == 0)
		{
			if (stoi(value) == 1)
				m_botMode = fishingMode;
		}
		else if (key.compare("Capture Screen") == 0)
		{
			if (stoi(value) == 1)
				m_botMode = captureScreenMode;
		}
	}

	ocr = OCRTesseract::create(NULL, NULL, NULL, OEM_TESSERACT_ONLY, PSM_SINGLE_LINE);

	pair<string*, string*> enumInput = make_pair(&emulator, &windowName);
	EnumWindows(EnumWindowsProc, LPARAM(&enumInput));
	EnumChildWindows(m_window, EnumChildWindowsProc, LPARAM(&innerWindowName));

	RECT rect;
	GetWindowRect(m_window, &rect);
	m_height = rect.bottom - rect.top;
	m_width = rect.right - rect.left;

	m_xCenter = m_width / 2;
	m_yCenter = m_height / 2;
	m_heightPct = (double)(m_height / M_HEIGHT);
	m_widthPct = (double)(m_width / M_WIDTH);

	fishIcon = imread("images\\fish.png", IMREAD_UNCHANGED);
	exclDefaultIcon = imread("images\\exclamation.png", IMREAD_UNCHANGED);
	exclRiftIcon = imread("images\\exclamationRift.png", IMREAD_UNCHANGED);
	exclSepcterIcon = imread("images\\exclamationSepcter.png", IMREAD_UNCHANGED);
	exclGrastaIcon = imread("images\\exclamationGrasta.png", IMREAD_UNCHANGED);
	exclChamberIcon = imread("images\\exclamationChamber.png", IMREAD_UNCHANGED);
	exclChamberPlasmaIcon = imread("images\\exclamationChamberPlasma.png", IMREAD_UNCHANGED);
	exclKMSIcon = imread("images\\exclamationKMS.png", IMREAD_UNCHANGED);
	swampFishIcon = imread("images\\swampFish.png", IMREAD_UNCHANGED);
	doorDefaultIcon = imread("images\\doorDefault.png", IMREAD_UNCHANGED);
	doorKunlunIcon = imread("images\\doorKunlun.png", IMREAD_UNCHANGED);
	doorLOMIcon = imread("images\\doorLOM.png", IMREAD_UNCHANGED);
	stairDefaultIcon = imread("images\\stairDefault.png", IMREAD_UNCHANGED);
	stairDownIcon = imread("images\\stairDown.png", IMREAD_UNCHANGED);
	stairLOMIcon = imread("images\\stairLOM.png", IMREAD_UNCHANGED);

	afBarEmptyPic = imread("images\\AEBarEmpty.png", IMREAD_UNCHANGED);
	afBarFullPic = imread("images\\AEBarFull.png", IMREAD_UNCHANGED);
	hitBellPic = imread("images\\Bell.png", IMREAD_UNCHANGED);
	jmpRopePic1 = imread("images\\jmpRopePic1.png", IMREAD_UNCHANGED);
	jmpRopePic2 = imread("images\\jmpRopePic2.png", IMREAD_UNCHANGED);
	jmpRopePic3 = imread("images\\jmpRopePic3.png", IMREAD_UNCHANGED);
	jmpRopePic4 = imread("images\\jmpRopePic4.png", IMREAD_UNCHANGED);

	for (auto i = 0; i < m_DynamicImage.size(); i++)
	{
		m_DynamicImage[i].name = string("images\\") + m_DynamicImage[i].name;
		m_DynamicImage[i].image = imread(m_DynamicImage[i].name, IMREAD_UNCHANGED);
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
		resize(fishIcon, fishIcon, Size(), m_widthPct, m_heightPct, m_heightPct >= 1.0 ? INTER_CUBIC : INTER_AREA);
		resize(exclDefaultIcon, exclDefaultIcon, Size(), m_widthPct, m_heightPct, m_heightPct >= 1.0 ? INTER_CUBIC : INTER_AREA);
		resize(exclRiftIcon, exclRiftIcon, Size(), m_widthPct, m_heightPct, m_heightPct >= 1.0 ? INTER_CUBIC : INTER_AREA);
		resize(exclSepcterIcon, exclSepcterIcon, Size(), m_widthPct, m_heightPct, m_heightPct >= 1.0 ? INTER_CUBIC : INTER_AREA);
		resize(exclGrastaIcon, exclGrastaIcon, Size(), m_widthPct, m_heightPct, m_heightPct >= 1.0 ? INTER_CUBIC : INTER_AREA);
		resize(exclChamberIcon, exclChamberIcon, Size(), m_widthPct, m_heightPct, m_heightPct >= 1.0 ? INTER_CUBIC : INTER_AREA);
		resize(exclChamberPlasmaIcon, exclChamberPlasmaIcon, Size(), m_widthPct, m_heightPct, m_heightPct >= 1.0 ? INTER_CUBIC : INTER_AREA);
		resize(exclKMSIcon, exclKMSIcon, Size(), m_widthPct, m_heightPct, m_heightPct >= 1.0 ? INTER_CUBIC : INTER_AREA);
		resize(swampFishIcon, swampFishIcon, Size(), m_widthPct, m_heightPct, m_heightPct >= 1.0 ? INTER_CUBIC : INTER_AREA);
		resize(doorDefaultIcon, doorDefaultIcon, Size(), m_widthPct, m_heightPct, m_heightPct >= 1.0 ? INTER_CUBIC : INTER_AREA);
		resize(doorKunlunIcon, doorKunlunIcon, Size(), m_widthPct, m_heightPct, m_heightPct >= 1.0 ? INTER_CUBIC : INTER_AREA);
		resize(doorLOMIcon, doorLOMIcon, Size(), m_widthPct, m_heightPct, m_heightPct >= 1.0 ? INTER_CUBIC : INTER_AREA);
		resize(stairDefaultIcon, stairDefaultIcon, Size(), m_widthPct, m_heightPct, m_heightPct >= 1.0 ? INTER_CUBIC : INTER_AREA);
		resize(stairDownIcon, stairDownIcon, Size(), m_widthPct, m_heightPct, m_heightPct >= 1.0 ? INTER_CUBIC : INTER_AREA);
		resize(stairLOMIcon, stairLOMIcon, Size(), m_widthPct, m_heightPct, m_heightPct >= 1.0 ? INTER_CUBIC : INTER_AREA);

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
			resize(m_DynamicImage[i].image, m_DynamicImage[i].image, Size(), m_widthPct, m_heightPct, m_heightPct >= 1.0 ? INTER_CUBIC : INTER_AREA);
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

	bitbltPic = Mat(m_height, m_width, CV_8UC4, ptrBitmapPixels, 0);

	loadSettingConfig();
	loadPathConfig();
	loadFishingConfig();
}

void CAEBot::run()
{
	switch (m_botMode) {
	case baruokiJumpRopeMode:
		stateJumpRopeBaruoki();
		break;
	case ratleJumpRope:
		stateJumpRopeRatle();
		break;
	case silverHitBell30Mode:
	case silverHitBell999Mode:
		stateSilverHitBell(GetMode());
		break;
	case seperateGrastaMode:
		stateSeparateGrasta();
		break;
	case engageFightMode:
		engageHorrorFightNow();
		break;
	case grindingMode:
		stateGrinding();
		break;
	case grindingTravelMode:
		stateGrindingTravel();
		break;
	case grindingLOMSlimeMode:
		stateGrindingLOMSlime();
		break;
	case fishingMode:
		stateFishing();
		break;
	case captureScreenMode:
	default:
		captureScreenNow();
		break;
	}
}

int main()
{
	CAEBot* m_bot = new CAEBot();

	m_bot->setup();

	m_bot->run();
}
