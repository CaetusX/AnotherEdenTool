#define _CRT_SECURE_NO_WARNINGS

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
#include <thread>

#include <opencv2/opencv.hpp>
#include <opencv2/text/ocr.hpp>

#include "AEBot.h"

using namespace cv;
using namespace std;
using namespace cv::text;

HWND m_window;

std::random_device dev;
std::mt19937 rng;
std::uniform_int_distribution<std::mt19937::result_type> boolRand;

string ocrNumericSet = "1234567890,";
string ocrAlphabeticSet = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ.'()-";
string ocrAlphanumericSet = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890,.'()-";

CAEBot* m_sharedThreadAEBot;
bool m_sharedThreadAEmutex;

void TimerforDailyChronoStone(CAEBot* aebot, int countdownsec, int waitingsec) {
	if (! m_sharedThreadAEmutex)
	{
		return;
	}
	else 
	{
		m_sharedThreadAEmutex = false;
	}

	if (m_sharedThreadAEBot == NULL)
		return;

	if (m_sharedThreadAEBot != aebot)
		return;

	std::this_thread::sleep_for(std::chrono::seconds(countdownsec)); //sleep for given seconds

	//do something...
	Bot_Mode lastmode;
	lastmode = aebot->GetMode();

	//stop everything for the daily chrone stone
	aebot->SetStatus(status_DailyChroneStone);

	//sleep for 60 seconds so that the previous action fully stops
	std::this_thread::sleep_for(std::chrono::seconds(waitingsec));

	Status_Code returnstatus;
	returnstatus = aebot->dailyChroneStone();

	if (returnstatus == status_NoError)
	{
		m_sharedThreadAEmutex = true;

		aebot->SetMode(lastmode);
		aebot->run();
	}
	else
		aebot->SetStatus(status_Stop);
}

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

	m_Image_Threshold = 10000;
	m_Load_Time = 3000;

	m_Debug_Type_Setting = false;
	m_Debug_Type_Platform = false;
	m_Debug_Type_Path = false;
	m_Debug_Type_Fighting = false;
	m_Debug_Type_Grinding = false;
	m_Debug_Type_Fishing = false;
	m_Debug_Type_Grasta = false;
	m_Debug_Type_LOM = false;

	m_IsPrint = false;

	m_Fishing_Locs_Acteul = { {900, 506}, {675, 377}, {1131, 392}, {657, 643}, {1130, 648}, };
	m_Fishing_Locs_Baruoki = { {842, 508}, {632, 409}, {1047, 402}, {630, 634}, {1046, 623}, };
	m_Fishing_Locs_DragonPalace = { {870, 483}, {654, 402}, {1055, 399}, {673, 580}, {1069, 596}, };
	m_Fishing_Locs_Elzion = { {909, 517}, {691, 397}, {1154, 429}, {673, 643}, {1166, 659}, };
	m_Fishing_Locs_DimensionRift = { {887, 481}, {604, 470}, {1112, 471} };
	m_Fishing_Locs_KiraBeach = { {936, 533}, {697, 389}, {1180, 404}, {705, 686}, {1173, 687} };
	m_Fishing_Locs_RucyanaSands = { {862, 494}, {650, 398}, {1080, 403}, {632, 603}, {1103, 622} };
	m_Fishing_Locs_Vasu = { {879, 492}, {626, 395}, {1133, 399}, {612, 598}, {1166, 614} };
	m_Fishing_Locs_Igoma = { {880, 510}, {600, 420}, {1135, 425}, {630, 610}, {1120, 620} };
	m_Fishing_Locs_Moonlight = { {920, 435}, {635, 280}, {1215, 300}, {625, 580}, {1215, 575} };
	m_Fishing_Locs_AncientBattlefield = { {890, 475}, {645, 385}, {1115, 385}, {660, 570}, {1135, 575} };
	m_Fishing_Locs_ZolPlains = { {850, 515}, {560, 410}, {1115, 400}, {545, 640}, {885, 675} };
	m_Fishing_Locs_Default = { {850, 490}, {700, 430}, {1000, 430}, {710, 570}, {880, 570} };

	m_Skill_Normal = 0;
	m_Skill_Exchange = 5;
	m_Skill_Exchange_A = 5;
	m_Skill_Exchange_B = 6;
	m_Skill_AF = 7;
	m_Skill_for_AF = 3;
	m_CharacterFrontline = 4;

	m_resValue = status_NoError;

	m_Time_Out = 60;

	m_Action_Interval = 3000;
	m_Fast_Action_Interval = 200;
	m_Slow_Action_Interval = 5000;
	m_Walk_Distance_Ratio = 1.0;
	m_DCS_Waiting = 120;
	m_DCS_Ad_Loading = 60;
	m_DCS_Ad_Showing = 90;

	m_Smart_DownUp_Interval = 200;
	m_Smart_DownUp_Threshold = 700;

	m_SummaryInfo.botmode = initialMode;
	m_SummaryInfo.loopNumber = 0;
	m_SummaryInfo.locationNumber = 0;
	m_SummaryInfo.runFishCaught = 0;
	m_SummaryInfo.totalFishCaught = 0;
	m_SummaryInfo.runMobFought = 0;
	m_SummaryInfo.totalMobFought = 0;
	m_SummaryInfo.runHorrorFought = 0;
	m_SummaryInfo.totalHorrorFought = 0;
	m_SummaryInfo.currentLocation = "Local";

	m_sharedThreadAEmutex = true;
}

CAEBot::~CAEBot()
{
	m_sharedThreadAEBot = NULL;
}

void CAEBot::SetStatus(Status_Code statusCode)
{
	snprintf(m_debugMsg, 1024, "Set Status ---> %x !!!", statusCode);
	dbgMsg(m_Debug_Type_Platform, debug_Key);

	m_resValue = statusCode;
}

Status_Code CAEBot::GetStatus()
{
	return 	m_resValue;
}

void CAEBot::dbgMsg(int debugGroup, Debug_Level debugLevel)
{
	if (debugGroup && (debugLevel <= m_Debug_Level))
	{
		char debugMsg[1024];
		snprintf(debugMsg, 1024, "[%s](%x) %s\r\n", timeString(), m_resValue, m_debugMsg);
		m_outputMsg.append(debugMsg);
		cout << debugMsg;
	}
}

char* CAEBot::timeString(bool toSave)
{
	struct tm timeinfo;
	time_t currenttime;

	currenttime = time(NULL);
	localtime_s(&timeinfo, &currenttime);
	if (toSave)
		strftime(m_timeString, sizeof(m_timeString), "%Y-%m-%d_%H-%M-%S", &timeinfo);
	else
		strftime(m_timeString, sizeof(m_timeString), "%Y-%m-%d %H:%M:%S", &timeinfo);

	return m_timeString;
}

void CAEBot::updateStatus(Status_Code statusCode)
{
	m_resValue = Status_Code((int)m_resValue | (int)statusCode);
}

bool CAEBot::checkStatus(Status_Code statuscode)
{
	Status_Code resultcode = Status_Code ((int)m_resValue & (int)statuscode);
	return (resultcode == statuscode);
}

void CAEBot::clearStatus()
{
	// clear status at the beginning of each location
	m_resValue = status_NoError;
}

Debug_Level CAEBot::GetDebugLevel()
{
	return m_Debug_Level;
}

void CAEBot::SetDebugLevel(Debug_Level debuglevel)
{
	m_Debug_Level = debuglevel;
}

string CAEBot::GetOutputMsg()
{
	string outputmsg = m_outputMsg;
	m_outputMsg.clear();
	return outputmsg;
}

string CAEBot::GetSummaryMsg()
{
	string outputmsg;
	char summarymsg[1024];

	outputmsg.clear();

	switch (m_SummaryInfo.botmode)
	{
	case grindingEndlessMode:
		snprintf(summarymsg, 1024, "%s\r\nEndless Grinding at [%s]\r\n", timeString(), m_SummaryInfo.currentLocation.c_str());
		break;
	case grindingTravelMode:
		snprintf(summarymsg, 1024, "%s\r\nTravel Grinding at [%s]\r\n", timeString(), m_SummaryInfo.currentLocation.c_str());
		break;
	case grindingStationMode:
		snprintf(summarymsg, 1024, "%s\r\nStation Grinding at [%s]\r\n", timeString(), m_SummaryInfo.currentLocation.c_str());
		break;
	case grindingLOMSlimeMode:
		snprintf(summarymsg, 1024, "%s\r\nLOM Grinding at [%s]\r\n", timeString(), m_SummaryInfo.currentLocation.c_str());
		break;
	case fishingAnglerMode:
		snprintf(summarymsg, 1024, "%s\r\nAngler Fishing at [%s]\r\n", timeString(), m_SummaryInfo.currentLocation.c_str());
		break;
	case fishingHarpoonMode:
		snprintf(summarymsg, 1024, "%s\r\nHarpoon Fishing at [%s]\r\n", timeString(), m_SummaryInfo.currentLocation.c_str());
		break;
	case ratleJumpRope:
		snprintf(summarymsg, 1024, "%s\r\nJump rope at [Ratle]\r\n", timeString());
		break;
	case baruokiJumpRopeMode:
		snprintf(summarymsg, 1024, "%s\r\nJump rope at [Baruoki]\r\n", timeString());
		break;
	case silverHitBell30Mode:
	case silverHitBell999Mode:
		snprintf(summarymsg, 1024, "%s\r\nHit bell at [Izana]\r\n", timeString());
		break;
	case separateGrastaMode:
		snprintf(summarymsg, 1024, "%s\r\nSeparate grasta\r\n", timeString());
		break;
	case captureScreenMode:
		snprintf(summarymsg, 1024, "%s\r\n%s\r\n", timeString(), m_debugMsg);
		break;
	default:
		snprintf(summarymsg, 1024, "%s\r\n", timeString());
		break;
	}

	outputmsg.append(summarymsg);

	switch (m_SummaryInfo.botmode)
	{
	case grindingEndlessMode:
	case grindingTravelMode:
	case grindingStationMode:
	case grindingLOMSlimeMode:
	case fishingAnglerMode:
	case fishingHarpoonMode:
		struct tm timeinfo;
		time_t currenttime;
		char timeString[80];
		localtime_s(&timeinfo, &(m_SummaryInfo.startingtime));
		strftime(timeString, sizeof(timeString), "%Y-%m-%d %H:%M:%S", &timeinfo);

		currenttime = time(NULL);
		int timegap;
		timegap = (int)difftime(currenttime, m_SummaryInfo.startingtime);

		snprintf(summarymsg, 1024, "Total %d loops %d locations running [%dH %dM %dS] from %s\r\nFish caught:\t%d at the current location;\t Total\t%d\r\nMob fought:\t%d at the current location;\t Total\t%d\r\nHorror fought:\t%d at the current location;\t Total\t%d\r\n", 
			m_SummaryInfo.loopNumber, m_SummaryInfo.locationNumber, timegap / 3600, (timegap / 60) % 60, timegap % 60, timeString,
			m_SummaryInfo.runFishCaught, m_SummaryInfo.totalFishCaught,
			m_SummaryInfo.runMobFought, m_SummaryInfo.totalMobFought,
			m_SummaryInfo.runHorrorFought, m_SummaryInfo.totalHorrorFought);
		outputmsg.append(summarymsg);

		if (m_SummaryInfo.isStopTimer)
		{
			tm utc_field = *std::gmtime(&(m_SummaryInfo.startingtime));
			utc_field.tm_isdst = -1;

			// JST is GMT +9
			int hh = 0;
			int mm = 0;
			int ss = 0;

			if (m_SummaryInfo.stopTimer.tm_sec >= utc_field.tm_sec)
				ss = m_SummaryInfo.stopTimer.tm_sec - utc_field.tm_sec;
			else
			{
				ss = m_SummaryInfo.stopTimer.tm_sec - utc_field.tm_sec + 60;
				mm -= 1;
			}

			if (m_SummaryInfo.stopTimer.tm_min + mm >= utc_field.tm_min)
				mm = m_SummaryInfo.stopTimer.tm_min + mm - utc_field.tm_min;
			else
			{
				mm = m_SummaryInfo.stopTimer.tm_min + mm - utc_field.tm_min + 60;
				hh -= 1;
			}

			hh = (m_SummaryInfo.stopTimer.tm_hour + hh - (utc_field.tm_hour + 9) + 48) % 24;

			snprintf(summarymsg, 1024, "Stop in %dH %dM %dS at [JST %02d:%02d:%02d]", hh, mm, ss, m_SummaryInfo.stopTimer.tm_hour, m_SummaryInfo.stopTimer.tm_min, m_SummaryInfo.stopTimer.tm_sec);
			outputmsg.append(summarymsg);
		}
		break;
	default:
		if (m_resValue == status_DailyChroneStone)
		{
			snprintf(summarymsg, 1024, "Waiting to process daily chrone stone. Please don't move!");
			outputmsg.append(summarymsg);
		}
		break;
	}

	return outputmsg;
}

Bot_Mode CAEBot::GetMode()
{
	return m_SummaryInfo.botmode;
}

void CAEBot::SetMode(Bot_Mode botMode)
{
	snprintf(m_debugMsg, 1024, "Set Mode ---> %x !!!", botMode);
	dbgMsg(m_Debug_Type_Platform, debug_Key);

	m_SummaryInfo.botmode = botMode;
}

int CAEBot::GetEmulatorNumber()
{
	return m_EmulatorNumber;
}

void CAEBot::SetEmulator(int emulatorIndex)
{
	m_EmulatorIndex = emulatorIndex;
}

string CAEBot::GetEmulatorName(int emulatorIndex)
{
	return m_EmulatorList[emulatorIndex].name;
}

void CAEBot::bitBltWholeScreen()
{
	BitBlt(m_hDest, 0, 0, m_width, m_height, m_hdc, 0, 0, SRCCOPY);
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
	m_ocr->run(newPic, output, &boxes, &words, &confidences, OCR_LEVEL_TEXTLINE);
	if (words.empty())
		return "";
	return words[0].substr(0, words[0].size() - 1);
}

string CAEBot::getText(Mat& pic)
{
	m_ocr->setWhiteList(ocrAlphabeticSet);
	return runOCR(pic);
}

string CAEBot::getTextNumber(Mat& pic)
{
	m_ocr->setWhiteList(ocrAlphanumericSet);
	return runOCR(pic);
}

int CAEBot::getNumber(Mat& pic)
{
	m_ocr->setWhiteList(ocrNumericSet);
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

string CAEBot::ocrPicture(OCR_Type ocrtype, int columns, int rows, int x, int y)
{
	bitBltWholeScreen();
	cv::Mat partialPic;
	copyPartialPic(partialPic, columns, rows, x, y);

	switch (ocrtype)
	{
		case ocr_Alphabetic:
			return getText(partialPic);
		case ocr_Numeric:
			return to_string(getNumber(partialPic));
		case ocr_AlphaNumeric:
		default:
			return getTextNumber(partialPic);
	}
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

imageInfo CAEBot::retrieveImage(string imageID)
{
	imageInfo returnimageinfo;
	for (auto k = 0; k < m_DynamicImage.size(); k++)
	{
		if (imageID.compare(m_DynamicImage[k].id) == 0)
		{
			returnimageinfo = m_DynamicImage[k];
		}
	}

	return returnimageinfo;
}

bool CAEBot::compareImage(string imageID)
{
	Mat imagePicCrop, target_image;
	double lastMSD = m_Image_Threshold;

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

			bitBltWholeScreen();
			if (target_x == 0 && target_y == 0)
			{
				pair <int, int> iconlocation = findIcon(target_image);

				snprintf(m_debugMsg, 1024, "compareImage at %d %d", iconlocation.first, iconlocation.second);
				dbgMsg(m_Debug_Type_Platform, debug_Granular);

				copyPartialPic(imagePicCrop, target_w, target_h, (int)(iconlocation.first / m_widthPct), (int)(iconlocation.second / m_heightPct));
			}
			else
			{
				copyPartialPic(imagePicCrop, target_w, target_h, target_x, target_y); //
			}

			double MSD1 = cv::norm(target_image, imagePicCrop);
			MSD1 = (MSD1 * MSD1 / target_image.total());

			snprintf(m_debugMsg, 1024, "compareImage %s %f (%d)", imageID.c_str(), MSD1, m_Image_Threshold);
			dbgMsg(m_Debug_Type_Platform, debug_Granular);

			if (MSD1 < lastMSD)
				lastMSD = MSD1;
		}
	}

	return (lastMSD < m_Image_Threshold);
}

pair<bool, pair<int, int>> CAEBot::findClick(string imageID, int cols, int rows, int x, int y)
{
	int target_w, target_h, target_x, target_y;
	Mat target_image, imagePicCrop;
	pair<bool, pair<int, int>> returnicon;
	double lastMSD = (int) 99999999999;
		
	returnicon.first = false;
	returnicon.second.first = m_width;
	returnicon.second.second = m_height;

	for (auto k = 0; k < m_DynamicImage.size(); k++)
	{
		if (imageID.compare(m_DynamicImage[k].id) == 0)
		{
			target_image = m_DynamicImage[k].image;
			target_x = m_DynamicImage[k].coordx;
			target_y = m_DynamicImage[k].coordy;
			target_w = m_DynamicImage[k].width;
			target_h = m_DynamicImage[k].height;

			pair<int, int> iconLoc;
			if (cols == 0 || rows == 0)
				iconLoc = findIcon(target_image);
			else
				iconLoc = findIconInRegion(target_image, cols, rows, x, y);

			copyPartialPic(imagePicCrop, target_w, target_h, (int)(iconLoc.first / m_widthPct), (int)(iconLoc.second / m_heightPct)); //

			double MSD1 = cv::norm(target_image, imagePicCrop);
			MSD1 = (MSD1 * MSD1 / target_image.total());

			snprintf(m_debugMsg, 1024, "findclick %s [%d %d] %f (%d)", imageID.c_str(), iconLoc.first, iconLoc.second, MSD1, m_Image_Threshold);
			dbgMsg(m_Debug_Type_Platform, debug_Brief);

			if (MSD1 < lastMSD)
			{
				lastMSD = MSD1;
				returnicon.first = MSD1 < m_Image_Threshold;
				returnicon.second.first = iconLoc.first + (int)round(target_w * m_widthPct / 2);
				returnicon.second.second = iconLoc.second + (int)round(target_h * m_heightPct / 2);
			}
		}
	}

	return returnicon;
}

void CAEBot::leftClick(int x, int y, bool changeLoc)
{
	leftClick(x, y, m_Action_Interval, changeLoc);
}

void CAEBot::leftClick(int x, int y, int sTime, bool changeLoc)
{
	// will change coordination system to the current one, except for those findClick
	if (changeLoc)
	{
		x = (int)round(x * m_widthPct);
		y = (int)round(y * m_heightPct);
	}

	snprintf(m_debugMsg, 1024, "LeftClick %d %d", x, y);
	dbgMsg(m_Debug_Type_Platform, debug_Detail);

	SendMessage(m_window, WM_MOUSEACTIVATE, 0, MAKELPARAM(HTCLIENT, WM_LBUTTONDOWN));
	SendMessage(m_window, WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(x, y));
	Sleep(10);
	SendMessage(m_window, WM_LBUTTONUP, 0, MAKELPARAM(x, y));

	Sleep(sTime);
}

void CAEBot::leftClick(pair<int, int>& coord)
{
	leftClick(coord.first, coord.second, m_Action_Interval);
}

// will always change coordination system to the current one
void CAEBot::leftClick(pair<int, int>& coord, int sTime)
{
	leftClick(coord.first, coord.second, sTime);
}

void CAEBot::drag(Direction_Info botDirection, int slideDistance, int xStart, int yStart, int sleepTime, int scrollRatio)
{
	snprintf(m_debugMsg, 1024, "drag %d %d %d %d", botDirection, slideDistance, xStart, yStart);
	dbgMsg(m_Debug_Type_Platform, debug_Detail);

	xStart = (int)round(xStart * m_widthPct);
	yStart = (int)round(yStart * m_heightPct);

	switch (botDirection)
	{
	case RIGHT:
		slideDistance = (int)round(slideDistance * m_widthPct);
		SendMessage(m_window, WM_LBUTTONDOWN, 0, MAKELPARAM(xStart, yStart));
		Sleep(1);

		for (int i = 0; i < slideDistance / scrollRatio; i++)
		{
			SendMessage(m_window, WM_MOUSEMOVE, MK_LBUTTON, MAKELPARAM(xStart - i * scrollRatio, yStart));
			Sleep(1);
		}

		SendMessage(m_window, WM_LBUTTONUP, 0, MAKELPARAM(xStart - (slideDistance - 1), yStart));
		break;
	case LEFT:
		slideDistance = (int)round(slideDistance * m_widthPct);
		SendMessage(m_window, WM_LBUTTONDOWN, 0, MAKELPARAM(xStart, yStart));
		Sleep(1);

		for (int i = 0; i < slideDistance / scrollRatio; i++)
		{
			SendMessage(m_window, WM_MOUSEMOVE, MK_LBUTTON, MAKELPARAM(xStart + i * scrollRatio, yStart));
			Sleep(1);
		}

		SendMessage(m_window, WM_LBUTTONUP, 0, MAKELPARAM(xStart + (slideDistance - 1), yStart));
		break;
	case UP:
		slideDistance = (int)round(slideDistance * m_heightPct);
		SendMessage(m_window, WM_LBUTTONDOWN, 0, MAKELPARAM(xStart, yStart));
		Sleep(1);

		for (int i = 0; i < slideDistance / scrollRatio; i++)
		{
			SendMessage(m_window, WM_MOUSEMOVE, MK_LBUTTON, MAKELPARAM(xStart, yStart + i * scrollRatio));
			Sleep(1);
		}

		SendMessage(m_window, WM_LBUTTONUP, 0, MAKELPARAM(xStart, yStart + (slideDistance - 1)));
		break;
	case DOWN:
		slideDistance = (int)round(slideDistance * m_heightPct);
		SendMessage(m_window, WM_LBUTTONDOWN, 0, MAKELPARAM(xStart, yStart));
		Sleep(1);

		for (int i = 0; i < slideDistance / scrollRatio; i++)
		{
			SendMessage(m_window, WM_MOUSEMOVE, MK_LBUTTON, MAKELPARAM(xStart, yStart - i * scrollRatio));
			Sleep(1);
		}

		SendMessage(m_window, WM_LBUTTONUP, 0, MAKELPARAM(xStart, yStart - (slideDistance - 1)));
		break;
	}

	Sleep(sleepTime);
}

void CAEBot::dragMap(Direction_Info botDirection, int slideDistance)
{
	switch (botDirection)
	{
	case RIGHT:
		drag(RIGHT, slideDistance, 1600, 491);
		break;
	case LEFT:
		drag(LEFT, slideDistance, 145, 490);
		break;
	case UP:
		drag(UP, slideDistance, 872, 150);
		break;
	case DOWN:
		drag(DOWN, slideDistance, 873, 831);
		break;
	}
}

void CAEBot::walk(Direction_Info botDirection, int time)
{
	walk(botDirection, time, m_Action_Interval);
}

void CAEBot::walk(Direction_Info botDirection, int time, int sleepTime)
{
	int xStart = m_xCenter;
	int yStart = m_yCenter;
	int slideDistance = 100;

	if (time < slideDistance)
		time = slideDistance;

	snprintf(m_debugMsg, 1024, "walk %d %d %d", botDirection, time, sleepTime);
	dbgMsg(m_Debug_Type_Platform, debug_Detail);

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
	/*
	Mat partialPic;
	bitBltWholeScreen();
	copyPartialPic(partialPic, 106, 39, 77, 37);
	bool inBattleResult = (getText(partialPic).compare("Status") == 0);
	*/

	bool inBattleStatus = compareImage("Battle Status");
	bool inBattleAttack = compareImage("Attack");

	if (inBattleStatus && inBattleAttack)
	{
		snprintf(m_debugMsg, 1024, "in battle %x %x", inBattleStatus, inBattleAttack);
		dbgMsg(m_Debug_Type_Fighting, debug_Granular);
	}
	else 
	{
		snprintf(m_debugMsg, 1024, "not in battle %x %x", inBattleStatus, inBattleAttack);
		dbgMsg(m_Debug_Type_Fighting, debug_Granular);
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
		snprintf(m_debugMsg, 1024, "end battle %x %x", endBattleResult1, endBattleResult2);
		dbgMsg(m_Debug_Type_Fighting, debug_Granular);
	}
	else
	{
		snprintf(m_debugMsg, 1024, "not end battle %x %x", endBattleResult1, endBattleResult2);
		dbgMsg(m_Debug_Type_Fighting, debug_Granular);
	}

	return (endBattleResult1 || endBattleResult2);
}

Status_Code CAEBot::smartWorldMap(pair<int, int>& coord)
{
	time_t currenttime, startingtime;
	startingtime = time(NULL);

	leftClick(coord, 500);

	while (!compareImage("TopTitleBar") && !compareImage("Map Title Bar"))
	{
		if (checkStatus(status_MajorError))
			return m_resValue;

		currenttime = time(NULL);
		auto timegap = difftime(currenttime, startingtime);
		if (timegap > m_Time_Out) // return if timeout
		{
			snprintf(m_debugMsg, 1024, "smartWorldMap timeout %d", (int)timegap);
			dbgMsg(m_Debug_Type_Platform, debug_Alert);
			return status_Timeout;
		}
		else
		{
			Sleep(500);
		}
	}

	Sleep(m_Slow_Action_Interval);
	return status_NoError;
}

Status_Code CAEBot::smartMiniMap(pair<int, int>& coord)
{
	time_t currenttime, startingtime;
	startingtime = time(NULL);

	leftClick(coord, 500);

	while (!compareImage("Mini Map X") && !compareImage("Map Title Bar"))
	{
		if (checkStatus(status_MajorError))
			return m_resValue;

		currenttime = time(NULL);
		auto timegap = difftime(currenttime, startingtime);
		if (timegap > m_Time_Out) // return if timeout
		{
			snprintf(m_debugMsg, 1024, "smartMiniMap timeout %d", (int)timegap);
			dbgMsg(m_Debug_Type_Platform, debug_Alert);
			return status_Timeout;
		}
		else
		{
			Sleep(500);
		}
	}

	Sleep(m_Slow_Action_Interval);
	return status_NoError;
}

Status_Code CAEBot::smartLoadMap(pair<int, int>& coord)
{
	time_t currenttime, startingtime;
	startingtime = time(NULL);

	leftClick(coord, 500);

	while (!compareImage("Confirm"))
	{
		if (checkStatus(status_MajorError))
			return m_resValue;

		currenttime = time(NULL);
		auto timegap = difftime(currenttime, startingtime);
		if (timegap > m_Time_Out) // return if timeout
		{
			snprintf(m_debugMsg, 1024, "smartLoadMap timeout %d", (int)timegap);
			dbgMsg(m_Debug_Type_Platform, debug_Alert);
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
		if (checkStatus(status_MajorError))
			return m_resValue;

		snprintf(m_debugMsg, 1024, "smartDownUp %x %x", updownDirection, leftrightDirection);
		dbgMsg(m_Debug_Type_Platform, debug_Detail);

		currenttime = time(NULL);
		auto timegap = difftime(currenttime, startingtime);
		if (timegap > m_Time_Out) // return if timeout
		{
			snprintf(m_debugMsg, 1024, "smartDownUp timeout %d", (int)timegap);
			dbgMsg(m_Debug_Type_Platform, debug_Alert);
			return status_Timeout;
		}
		else
		{
			walk(leftrightDirection, 100, m_Smart_DownUp_Interval);

			walk(updownDirection, 100, m_Smart_DownUp_Interval);
		}
	}

	snprintf(m_debugMsg, 1024, "smartDownUp %x %x successful!", updownDirection, leftrightDirection);
	dbgMsg(m_Debug_Type_Platform, debug_Brief);

	Sleep(m_Action_Interval);
	return status_NoError;
}

Status_Code CAEBot::sleepLoadTime()
{
	Sleep(m_Load_Time);

	time_t currenttime, startingtime;
	startingtime = time(NULL);

	while (!compareImage("Menu"))
	{
		if (checkStatus(status_MajorError))
			return m_resValue;

		currenttime = time(NULL);
		auto timegap = difftime(currenttime, startingtime);
		if (timegap > m_Time_Out) // return if timeout
		{
			snprintf(m_debugMsg, 1024, "sleepLoadTime timeout %d", (int)timegap);
			dbgMsg(m_Debug_Type_Path, debug_Alert);
			return status_Timeout;
		}
		else 
			Sleep(500);
	}

	Sleep(m_Slow_Action_Interval);
	return status_NoError;
}

Status_Code CAEBot::fightUntilEnd()
{
	Sleep(m_Slow_Action_Interval);

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
		if (checkStatus(status_MajorError) || checkStatus(status_MediumError))
		{
			Sleep(3000);
			return m_resValue;
		}

		currenttime = time(NULL);
		auto timegap = difftime(currenttime, startingtime);
		if (timegap > m_Time_Out) // return if timeout
		{
			snprintf(m_debugMsg, 1024, "walkUntilBattle timeout");
			dbgMsg(m_Debug_Type_Fighting, debug_Alert);

			if (m_IsPrint && m_Debug_Type_Fighting) captureScreenNow("walkUntilBattle_timeout");

			return status_Timeout;
		}

		switch (botdirection)
		{
		case LEFT:
			walk(LEFT, m_Grinding_Step);
			break;
		case RIGHT:
			walk(RIGHT, m_Grinding_Step);
			break;
		case LR:
			walk(LEFT, m_Grinding_Step);
			walk(RIGHT, m_Grinding_Step);
		case NOWHERE:
		default:
			walk(LEFT, 300, 100);
			walk(RIGHT, 300, 100);
			break;
		}
	}

	return engageMobFightNow();
}

Status_Code CAEBot::engageMobFightNow()
{
	time_t currenttime, lastfighttime;

	int num_loop = (int)m_Skill_MobSet.size();
	int iRun = 0;
	vector<int> lastSkillsRow = { 0, 0, 0, 0 };

	if (m_IsPrint && m_Debug_Type_Fighting) captureScreenNow("Mob");

	snprintf(m_debugMsg, 1024, "Start a battle...");
	dbgMsg(m_Debug_Type_Fighting, debug_Detail);

	m_CurrentGrindingCounter++;

	do
	{
		if (iRun == num_loop)
		{
			//only non endless grinding needs to count grinding runs, to estimate the MP usage
			m_CurrentGrindingCounter++;
		}

		//loop through the skills set
		iRun = iRun % num_loop;

		for (auto j = 0; j < m_CharacterFrontline; j++)
		{
			int iSkill = m_Skill_MobSet[iRun][j];
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

		snprintf(m_debugMsg, 1024, "[%d] Ready to attack with %x %x %x %x", m_CurrentGrindingCounter, lastSkillsRow[0], lastSkillsRow[1], lastSkillsRow[2], lastSkillsRow[3]);
		dbgMsg(m_Debug_Type_Fighting, debug_Detail);

		leftClick(m_Button_Attack, 500);

		snprintf(m_debugMsg, 1024, "After attack");
		dbgMsg(m_Debug_Type_Fighting, debug_Detail);

		lastfighttime = time(NULL);
		while (!endBattle())
		{
			currenttime = time(NULL);
			auto timegap = difftime(currenttime, lastfighttime);
			if (timegap > m_Time_Out) // return if timeout
			{
				snprintf(m_debugMsg, 1024, "Fight Horror time out");
				dbgMsg(m_Debug_Type_Fighting, debug_Alert);

				if (m_IsPrint && m_Debug_Type_Fighting) captureScreenNow("FightMobTimeout");

				return status_Timeout;
			}

			if (inBattle())
			{
				break;
			}

			Sleep(200);
		}

		iRun++;
	} while (inBattle());

	m_SummaryInfo.runMobFought = m_SummaryInfo.runMobFought + 1;
	m_SummaryInfo.totalMobFought = m_SummaryInfo.totalMobFought + 1;

	snprintf(m_debugMsg, 1024, "A battle ends");
	dbgMsg(m_Debug_Type_Fighting, debug_Detail);

	for (auto i = 0; i < 3; i++)
	{
		leftClick(m_Button_PassThrough, m_Fast_Action_Interval);
	}

	Sleep(m_Slow_Action_Interval);

	snprintf(m_debugMsg, 1024, "End of a battle");
	dbgMsg(m_Debug_Type_Fighting, debug_Detail);

	return status_NoError;
}

Status_Code CAEBot::engageHorrorFightNow(bool restoreHPMP)
{
	cv::Mat partialPic;
	string textToCheck;

	time_t currenttime, lastfighttime;

	Sleep(m_Action_Interval);

	if (!inBattle()) // not horror
	{
		leftClick(m_Button_PassThrough, m_Fast_Action_Interval);
		return status_NotFight;
	}

	snprintf(m_debugMsg, 1024, "Fighting Horror starts");
	dbgMsg(m_Debug_Type_Fighting, debug_Detail);

	int num_loop = (int)m_Skill_HorrorSet.size();

	Mat imagePicCrop;

	if (m_IsPrint && m_Debug_Type_Fighting) captureScreenNow("Horror");

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
		imageInfo afBarFullPic = retrieveImage("AFBarFull");
		copyPartialPic(imagePicCrop, afBarFullPic.width, afBarFullPic.height, afBarFullPic.coordx, afBarFullPic.coordy); // AF bar 880, 30, 730, 50.  AF bar 860, 10, 770, 90

		double MSD1;
		MSD1 = cv::norm(afBarFullPic.image, imagePicCrop);
		MSD1 = MSD1 * MSD1 / afBarFullPic.image.total();

		if (MSD1 < m_Grinding_AFFullThreshold || isAFon)
		{
			snprintf(m_debugMsg, 1024, "AF");
			dbgMsg(m_Debug_Type_Fighting, debug_Detail);

			leftClick(m_Button_AF, m_Grinding_AFInterval);

			double MSD2 = 10000;
			do
			{
				leftClick(m_Button_Skills[m_Skill_for_AF].xyPosition, m_Grinding_AFInterval);

				bitBltWholeScreen();
				imageInfo afBarEmptyPic = retrieveImage("AFBarEmpty");
				copyPartialPic(imagePicCrop, afBarEmptyPic.width, afBarEmptyPic.height, afBarEmptyPic.coordx, afBarEmptyPic.coordy); // AF bar 880, 30, 730, 50.  AF bar 860, 10, 770, 90

				MSD2 = cv::norm(afBarEmptyPic.image, imagePicCrop);
				MSD2 = MSD2 * MSD2 / afBarEmptyPic.image.total();
			} while (MSD2 > 400);

			snprintf(m_debugMsg, 1024, "AF Done");
			dbgMsg(m_Debug_Type_Fighting, debug_Detail);

			if (m_IsPrint && m_Debug_Type_Fighting) captureScreenNow("AFDone");

			iRun = 0;
			isAFon = false;
		}
		else 
		{
			iRun = iRun % num_loop;

			snprintf(m_debugMsg, 1024, "Attack with %d %d %d %d", m_Skill_HorrorSet[iRun][0], m_Skill_HorrorSet[iRun][1], m_Skill_HorrorSet[iRun][2], m_Skill_HorrorSet[iRun][3]);
			dbgMsg(m_Debug_Type_Fighting, debug_Detail);

			for (auto j = 0; j < m_CharacterFrontline; j++)
			{
				int iSkill = m_Skill_HorrorSet[iRun][j];
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


		snprintf(m_debugMsg, 1024, "After attack");
		dbgMsg(m_Debug_Type_Fighting, debug_Detail);

		lastfighttime = time(NULL);
		while (!endBattle())
		{
			currenttime = time(NULL);
			auto timegap = difftime(currenttime, lastfighttime);
			if (timegap > m_Time_Out) // return if timeout
			{
				snprintf(m_debugMsg, 1024, "Fight Horror time out");
				dbgMsg(m_Debug_Type_Fighting, debug_Alert);

				if (m_IsPrint && m_Debug_Type_Fighting) captureScreenNow("FightHorrorTimeout");

				return status_Timeout;
			}

			if (inBattle())
			{
				break;
			}

			Sleep(500);
		}

	} while (inBattle());

	if (m_IsPrint && m_Debug_Type_Fighting) captureScreenNow("Result");

	m_SummaryInfo.runHorrorFought = m_SummaryInfo.runHorrorFought + 1;
	m_SummaryInfo.totalHorrorFought = m_SummaryInfo.totalHorrorFought + 1;

	snprintf(m_debugMsg, 1024, "Fighting Horror ends");
	dbgMsg(m_Debug_Type_Fighting, debug_Detail);

	Sleep(m_Action_Interval);
	
	for (auto i = 0; i < 3; i++)
	{
		leftClick(m_Button_PassThrough, m_Fast_Action_Interval);
	}

	Sleep(m_Slow_Action_Interval);

	leftClick(m_Button_Yes);

	for (auto i = 0; i < 3; i++)
	{
		leftClick(m_Button_PassThrough, m_Fast_Action_Interval);
	}

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

Status_Code CAEBot::fish(vector<pair<int, int>>& sections)
{
	//Strategy is to quadrisect the lake and toss into each of the four sections and center
	int catchIndex = 0, horrorIndex = 0, monsterIndex = 0;

	time_t currenttime, lastCatch;

	lastCatch = time(NULL);

	snprintf(m_debugMsg, 1024, "Starting fishing at [%s]", m_SummaryInfo.currentLocation.c_str());
	dbgMsg(m_Debug_Type_Fishing, debug_Key);

	Sleep(m_Action_Interval);

	bitBltWholeScreen();
	Mat lakeImg = m_BitbltPic.clone();

	for (int i = 0; i < m_CurrentBaitsToUse->size(); ++i)
	{
		if (m_baitList[(*m_CurrentBaitsToUse)[i]].hasBait == false) //If we have 0 of current bait, go to next bait
			continue;

		changeBait((*m_CurrentBaitsToUse)[i]);

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
				if (m_IsPrint && m_Debug_Type_Fishing) captureScreenNow("NOTFISHING");

				leftClick(m_Button_X);
				return status_NoFishing;
			}

			//Cast line
			leftClick(sections[j].first, sections[j].second);

			string status = ocrPicture(ocr_Alphabetic, 1242, 71, 257, 80);
			if (status.find("bait") != string::npos) //Out of bait
			{
				m_baitList[(*m_CurrentBaitsToUse)[i]].hasBait = false;

				do //Look for a bait that we actually have
				{
					++i;
				} while (i < m_CurrentBaitsToUse->size() && m_baitList[(*m_CurrentBaitsToUse)[i]].hasBait == false);

				if (i >= m_CurrentBaitsToUse->size())
					return status_FishingNoBait;

				changeBait((*m_CurrentBaitsToUse)[i]);
				leftClick(sections[j].first, sections[j].second);
			}
			else if (status.find("any fish") != string::npos || status.find("box") != string::npos) //Pool is empty or cooler is full
			{
				snprintf(m_debugMsg, 1024, "Leaving [%s] (%d/%d)", m_SummaryInfo.currentLocation.c_str(), horrorIndex, catchIndex);
				dbgMsg(m_Debug_Type_Fishing, debug_Key);

				return status_NoFishing;
			}

			double MSD;
			auto startTime = chrono::high_resolution_clock::now();
			bool noFishCheck = true;

			while (1) //This constantly reads the screen looking for either a certain status, or whether a zoom in has occurred
			{
				//ocrPicture() is really cpu intensive, so avoid doing it unless necessary
				if (std::chrono::duration_cast<std::chrono::milliseconds>(chrono::high_resolution_clock::now() - startTime).count() >= 11000 && noFishCheck)
				{
					noFishCheck = false;
					status = ocrPicture(ocr_Alphabetic, 1242, 71, 257, 80);

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

				if (MSD > m_Image_Threshold) //If the current screen is sufficiently different (high mean square difference) from the normal lake image, then a zoom in has occurred
				{
					Sleep(100); //Emulate human reaction time
					leftClick(sections[j].first, sections[j].second);

					catchIndex++;
					m_SummaryInfo.runFishCaught = m_SummaryInfo.runFishCaught + 1;
					m_SummaryInfo.totalFishCaught = m_SummaryInfo.totalFishCaught + 1;

					//Click through success or failure
					Sleep(m_Action_Interval);
					leftClick(sections[j].first, sections[j].second);
					Sleep(m_Action_Interval);

					bitBltWholeScreen();
					MSD = cv::norm(lakeImg, m_BitbltPic);
					MSD = MSD * MSD / lakeImg.total();
					if (MSD > m_Image_Threshold) //Double, gotta click past it
					{
						leftClick(sections[j].first, sections[j].second);

						m_SummaryInfo.runFishCaught = m_SummaryInfo.runFishCaught + 1;
						m_SummaryInfo.totalFishCaught = m_SummaryInfo.totalFishCaught + 1;

						Sleep(m_Action_Interval);

						bitBltWholeScreen();
						MSD = cv::norm(lakeImg, m_BitbltPic);
						MSD = MSD * MSD / lakeImg.total();
						if (MSD > m_Image_Threshold) //Triple
						{
							leftClick(sections[j].first, sections[j].second);

							m_SummaryInfo.runFishCaught = m_SummaryInfo.runFishCaught + 1;
							m_SummaryInfo.totalFishCaught = m_SummaryInfo.totalFishCaught + 1;

							Sleep(m_Action_Interval);
						}
					}

					//Check for battle
					bitBltWholeScreen();
					MSD = cv::norm(lakeImg, m_BitbltPic);
					MSD = MSD * MSD / lakeImg.total();
					if (MSD > m_Image_Threshold) //Should have returned to normal lake image; if not, its a battle
					{
						Sleep(5000); //Give ample time for battle to fully load

						if (inBattle()) // double check whether it is a battle
						{
							if (m_Fishing_HasHorror) //Its possibly a horror or lakelord, so need to check to make sure before trying to auto it down
							{
								bool isThisHorror = false;
								bool isThisMob = false;
								if (m_Fishing_HasMob)
								{
									//For 3 seconds, read the screen and compare it to the current monster pics. If a cl1ose enough similarity is found, assume its a regular battle and proceed to auto attack
									//If not, assume its a horror and exit
									auto horrorStartTime = chrono::high_resolution_clock::now();
									while (std::chrono::duration_cast<std::chrono::seconds>(chrono::high_resolution_clock::now() - horrorStartTime).count() < 3)
									{
										if (compareImage("FishingMob"))
										{
											isThisMob = true;
											break;
										}
										Sleep(10);
									}

									if (!isThisMob) //If its not a monster, it must be a horror
										isThisHorror = true;
								}
								else
								{
									isThisHorror = true;
								}

								if (isThisHorror)
								{
									snprintf(m_debugMsg, 1024, "Horror or Lake Lord detected at [%s] %d", m_SummaryInfo.currentLocation.c_str(), horrorIndex);
									dbgMsg(m_Debug_Type_Fishing, debug_Brief);

									if (m_Fishing_HorrorCount)
									{
										m_resValue = engageHorrorFightNow();
										horrorIndex++;
										
										if (m_resValue != status_NoError) // failed
										{
											snprintf(m_debugMsg, 1024, "Fight Horror or Lake Lord failed!");
											dbgMsg(m_Debug_Type_Fishing, debug_Key);
											return status_FightFail;
										}
										if (horrorIndex >= m_Fishing_HorrorCount) // limit the number of horrors / lake lords to fight
										{
											snprintf(m_debugMsg, 1024, "Leaving [%s] (%d/%d) catched - Horror count reached", m_SummaryInfo.currentLocation.c_str(), horrorIndex, catchIndex);
											dbgMsg(m_Debug_Type_Fishing, debug_Key);
											return status_FishingHorrorMax;
										}

										snprintf(m_debugMsg, 1024, "Horror or Lake Lord defeated at [%s] %d", m_SummaryInfo.currentLocation.c_str(), horrorIndex);
										dbgMsg(m_Debug_Type_Fishing, debug_Key);
									}
									else
										return status_BreakRun;
								}
								else
								{
									snprintf(m_debugMsg, 1024, "Fighting at [%s] monster %x", m_SummaryInfo.currentLocation.c_str(), monsterIndex);
									dbgMsg(m_Debug_Type_Fishing, debug_Detail);
								}
							}

							fightUntilEnd();

							Sleep(5000);

							snprintf(m_debugMsg, 1024, "Fighting ends at [%s] monster %x", m_SummaryInfo.currentLocation.c_str(), monsterIndex);
							dbgMsg(m_Debug_Type_Fishing, debug_Detail);
							leftClick(m_Button_PassThrough); //Click past fish results screen

							Sleep(5000);

							fishIconClickFunction();

							Sleep(m_Action_Interval);
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
						snprintf(m_debugMsg, 1024, "Leaving [%s] (%d/%d) catched - Idling", m_SummaryInfo.currentLocation.c_str(), horrorIndex, catchIndex);
						dbgMsg(m_Debug_Type_Fishing, debug_Key);
						return status_Timeout;
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

	snprintf(m_debugMsg, 1024, "Leaving [%s] (%d/%d) catched - All baits tried", m_SummaryInfo.currentLocation.c_str(), horrorIndex, catchIndex);
	dbgMsg(m_Debug_Type_Fishing, debug_Key);

	return status_NoError;
}

Status_Code CAEBot::changeBait(Bait_Type type)
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
		drag(DOWN, 74, 872, 491);
		for (int i = 5; i < slotNum; ++i)
		{
			drag(DOWN, 100, 872, 491);
		}

		leftClick(m_Button_Baits[5].xyPosition.first, m_Button_Baits[5].xyPosition.second);
	}
	else
		leftClick(m_Button_Baits[slotNum].xyPosition.first, m_Button_Baits[slotNum].xyPosition.second);
	
	return status_NoError;
};

Status_Code CAEBot::goToTargetLocation(string targetlocation)
{
	bool foundtarget = false;
	locationInfo currentlocation;
	Status_Code localstatus;

	for (auto i = 0; i < m_LocationList.size(); i++)
	{
		if (m_LocationList[i].locationName.compare(targetlocation) == 0)
		{
			snprintf(m_debugMsg, 1024, "Moving to [%s]", targetlocation.c_str());
			dbgMsg(m_Debug_Type_Path, debug_Brief);

			currentlocation = m_LocationList[i];
			foundtarget = true;
			break;
			// no need to care about time out here
		}
	}

	if (!foundtarget)
	{
		updateStatus(status_NoPathFound);
		return m_resValue;
	}

	for (auto j = 0; j < currentlocation.pathStepsList.size(); j++)
	{
		if (checkStatus(status_MajorError) || checkStatus(status_MediumError) || checkStatus(status_MinorError))
			return m_resValue;

		string curType = currentlocation.pathStepsList[j].type;
		string curValue1 = currentlocation.pathStepsList[j].value1;
		string curValue2 = currentlocation.pathStepsList[j].value2;

		snprintf(m_debugMsg, 1024, "%s %s %s", curType.c_str(), curValue1.c_str(), curValue2.c_str());
		dbgMsg(m_Debug_Type_Path, debug_Detail);

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
					localstatus = smartWorldMap(m_Button_Map);
					updateStatus(localstatus);
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
						snprintf(m_debugMsg, 1024, "!!! %s not found", curValue1.c_str());
						dbgMsg(m_Debug_Type_Path, debug_Detail);
					}
				}
			}
		}
		else if (curType.compare("Sleep") == 0) {

			if (curValue1.compare("LoadTime") == 0)
			{
				localstatus = sleepLoadTime();
				updateStatus(localstatus);
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
					localstatus = walkUntilBattle(LR);
					updateStatus(localstatus);
				}
				else if (curValue2.compare("LEFT") == 0)
				{
					localstatus = walkUntilBattle(LEFT);
					updateStatus(localstatus);
				}
				else if (curValue2.compare("RIGHT") == 0)
				{
					localstatus = walkUntilBattle(RIGHT);
					updateStatus(localstatus);
				}
				else if (curValue2.compare("NOWHERE") == 0)
				{
					localstatus = walkUntilBattle(NOWHERE);
					updateStatus(localstatus);
				}
			}

		}
		else if (curType.compare("Load") == 0) 
		{
			if (curValue1.compare("WorldMap") == 0)
			{
				if (curValue2.empty())
				{
					localstatus = smartWorldMap(m_Button_Map);
					updateStatus(localstatus);
				}
				else
				{
					bool isfound = false;

					for (auto i = 0; i < m_Button_MapButtons.size(); i++)
					{
						if (curValue2.compare(m_Button_MapButtons[i].buttonName) == 0)
						{
							isfound = true;
							localstatus = smartWorldMap(m_Button_MapButtons[i].xyPosition);
							updateStatus(localstatus);
							break;
						}
					}

					if (!isfound)
					{
						snprintf(m_debugMsg, 1024, "WorldMap %s not found!", curValue2.c_str());
						dbgMsg(m_Debug_Type_Path, debug_Detail);
					}
				}
			}
			else if (curValue1.compare("MiniMap") == 0)
			{
				bool isfound = false;

				for (auto i = 0; i < m_Button_MapButtons.size(); i++)
				{
					if (curValue2.compare(m_Button_MapButtons[i].buttonName) == 0)
					{
						isfound = true;
						localstatus = smartMiniMap(m_Button_MapButtons[i].xyPosition);
						updateStatus(localstatus);
						break;
					}
				}

				if (!isfound)
				{
					snprintf(m_debugMsg, 1024, "MiniMap %s not found!", curValue2.c_str());
					dbgMsg(m_Debug_Type_Path, debug_Detail);
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
						localstatus = smartLoadMap(m_Button_MapButtons[i].xyPosition);
						updateStatus(localstatus);
						break;
					}
				}

				if (!isfound)
				{
					snprintf(m_debugMsg, 1024, "LoadMap %s not found!", curValue1.c_str());
					dbgMsg(m_Debug_Type_Path, debug_Detail);
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
			pair<bool, pair <int, int>> findclickres = findClick(curValue1, (int)M_WIDTH, (int)M_ABOVE_MENU, 0, 0);
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
				localstatus = harpoonFunction();
				updateStatus(localstatus);
			}
			else if (curValue1.compare("HorrorFishing") == 0)
			{
				localstatus = harpoonHorror();
				updateStatus(localstatus);
			}
			else if (curValue1.compare("TrapFishing") == 0)
			{
				localstatus = harpoonTrapFunction(curValue2);
				updateStatus(localstatus);
			}
			else if (curValue1.compare("PlatiumSlime") == 0)
			{
				localstatus = lomPlatiumSlime();
				updateStatus(localstatus);
			}
			else if (curValue1.compare("Grinding") == 0)
			{
				localstatus = grindingRun(false);
				updateStatus(localstatus);
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

	}

	return status_NoError;
}

Status_Code CAEBot::goToFishingLocation(string targetlocation)
{
	time_t currenttime, startingtime;
	string currText, txt1, txt2;
	pair<bool, pair <int, int>> findclickres;

	snprintf(m_debugMsg, 1024, "Move to pond %s at [%s]", targetlocation.c_str(), m_SummaryInfo.currentLocation.c_str());
	dbgMsg(m_Debug_Type_Fishing, debug_Brief);

	goToTargetLocation("Pond Teleport");

	startingtime = time(NULL);
	while (!compareImage("PondTeleport"))
	{
		currenttime = time(NULL);
		auto timegap = difftime(currenttime, startingtime);

		if (timegap > m_Time_Out) // return if timeout
		{
			snprintf(m_debugMsg, 1024, "Time out pond teleport at [%s]", m_SummaryInfo.currentLocation.c_str());
			dbgMsg(m_Debug_Type_Fishing, debug_Alert);

			// time out, something wrong, let's go
			return status_Timeout;
		}

		Sleep(500);
	}

	findclickres = findClick("PondTeleport", 0, 0, 0, 0);
	leftClick(findclickres.second.first, findclickres.second.second, m_Action_Interval, false);

	startingtime = time(NULL);
	while (!compareImage("TopTitleBar"))
	{
		currenttime = time(NULL);
		auto timegap = difftime(currenttime, startingtime);

		if (timegap > m_Time_Out) // return if timeout
		{
			snprintf(m_debugMsg, 1024, "Time out fishing spot at [%s]", m_SummaryInfo.currentLocation.c_str());
			dbgMsg(m_Debug_Type_Fishing, debug_Alert);

			// time out, something wrong, let's go
			return status_Timeout;
		}

		Sleep(500);
	}

	for (auto i = 0; i < m_Fishing_Spots.size(); i++)
	{
		if (i > 4)
		{
			if (i == 5)
				drag(DOWN, 100, 1200, 300);
			else
				drag(DOWN, 140, 1200, 300);
			leftClick(m_Button_Baits[5].xyPosition.first, m_Button_Baits[5].xyPosition.second);
		}
		else
			leftClick(m_Button_Baits[i].xyPosition.first, m_Button_Baits[i].xyPosition.second);

		//If the next one we've done is the same as the last, we've reached the end of the list
		txt1 = ocrPicture(ocr_Alphabetic, 680, 58, 60, 80);

		snprintf(m_debugMsg, 1024, "Pond Name [%s]", txt1.c_str());
		dbgMsg(m_Debug_Type_Fishing, debug_Detail);

		if (i <= 4)
			txt2 = ocrPicture(ocr_Alphabetic, 560, 60, 840, 105 + i * 164);
		else
		{
			findclickres = findClick("AreaFishing", (int)M_WIDTH, 280, 0, 670);
			// why -20? AreaFishing is 96 x 96, there is a gap of 28 from the top of AreaFishing to the top of the words
			txt2 = ocrPicture(ocr_Alphabetic, 560, 60, 840, (int) (findclickres.second.second / m_heightPct - 30) );
		}

		if (targetlocation.compare(txt1) == 0)
		{
			leftClick(395, 880);
			sleepLoadTime();
			return status_NoError;
		}
	}

	leftClick(m_Button_X);
	leftClick(1560, 920);
	sleepLoadTime();
	return status_NoPathFound;
}

void CAEBot::goToSpacetimeRift(bool heal)
{
	snprintf(m_debugMsg, 1024, "Space-time Rift [%x]", heal);
	dbgMsg(m_Debug_Type_Path, debug_Detail);

	smartWorldMap(m_Button_Map);
	smartWorldMap(m_Button_EndOfTimeLoc);
	smartLoadMap(m_Button_SpacetimeRift);

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
	m_SummaryInfo.currentLocation = "Fish Vendor";
	goToTargetLocation(m_SummaryInfo.currentLocation);

	for (auto i = 1200; i > 600; i = i - 30) //click through text
		leftClick(i, 310, 100);

	buyBaitsFromVendor();
}

void CAEBot::goToHarpoonVendor(bool fromotherplace)
{
	if (fromotherplace)
	{
		goToSpacetimeRift();

		m_SummaryInfo.currentLocation = "Harpoon Vendor From Other Place";
		goToTargetLocation(m_SummaryInfo.currentLocation);
	}
	else
	{
		m_SummaryInfo.currentLocation = "Harpoon Vendor";
		goToTargetLocation(m_SummaryInfo.currentLocation);
	}

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
	for (auto i = 0, j = 0; i < m_baitList.size() && j < m_baitList.size(); i++)
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
		string txt = ocrPicture(ocr_Alphabetic, 577, 51, 100, 102);

		snprintf(m_debugMsg, 1024, "Checking Bait %s(%s) %x %d", txt.c_str(), m_baitList[j].baitName.c_str(), m_baitList[j].hasBait, m_baitList[j].baitNumber);
		dbgMsg(m_Debug_Type_Fishing, debug_Detail);

		if (currText.compare(txt) == 0)
			break;
		currText = txt;

		bool baitfound = false;
		while (j < m_baitList.size())
		{
			//You don't get the baits in order, so some may be missing depending on your progression (blabber sardines)
			if (txt.compare(m_baitList[j].baitName) != 0) {
				j++;
			}
			else
			{
				baitfound = true;
				break;
			}
		}

		if (baitfound)
		{
			//Get amount currently held
			int held = stoi(ocrPicture(ocr_Numeric, 141, 30, 500, 165));
			int numToBuy = m_baitList[j].baitNumber - held;

			snprintf(m_debugMsg, 1024, "Buy Bait %s %d", txt.c_str(), m_baitList[j].baitNumber);
			dbgMsg(m_Debug_Type_Fishing, debug_Detail);

			if (numToBuy > 0)
			{
				//Get amount of fish points owned
				int fishPoints = stoi(ocrPicture(ocr_Numeric, 193, 28, 1346, 10));
				//Get cost of current selected bait
				double fpCost = m_baitList[j].baitPrice;
				if (fpCost < fishPoints) //Can we even afford one?
				{
					if (numToBuy * fpCost >= fishPoints) //If we don't have enough fish points to buy the max set, then buy as many as we can
						numToBuy = (int)floor(fishPoints / fpCost);
				}

				int thisTimeToBuy;
				snprintf(m_debugMsg, 1024, "Bait [%d]: %d holding %d to buy %d. buying...", i, m_baitList[j].baitNumber, held, numToBuy);
				dbgMsg(m_Debug_Type_Fishing, debug_Detail);

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

						held = stoi(ocrPicture(ocr_Numeric, 141, 30, 500, 165));
					}
				}
			}

			m_baitList[j].hasBait = held > 0;
		}
	}

	leftClick(m_Button_X);
}

Status_Code CAEBot::fishFunction()
{
	Status_Code localstatus;
	if (m_SummaryInfo.currentLocation.compare("Kira Beach") == 0)
	{
		//no monster
		localstatus = fish(m_Fishing_Locs_KiraBeach);
		updateStatus(localstatus);
		leftClick(m_Button_Leave);
	}
	else if (m_SummaryInfo.currentLocation.compare("Baruoki") == 0)
	{
		m_Fishing_HasHorror = true;
		m_Fishing_HasMob = true;

		m_CurrentFishIconLoc = make_pair(711, 366);

		localstatus = fish(m_Fishing_Locs_Baruoki);
		updateStatus(localstatus);
		leftClick(m_Button_Leave);

		m_Fishing_HasHorror = false;
		m_Fishing_HasMob = false;
	}
	else if (m_SummaryInfo.currentLocation.compare("Naaru Uplands") == 0)
	{
		//has monster, no horror
		m_CurrentFishIconLoc = make_pair(775, 450);

		localstatus = fish(m_Fishing_Locs_Baruoki);
		updateStatus(localstatus);
		leftClick(m_Button_Leave);
	}
	else if (m_SummaryInfo.currentLocation.compare("Acteul") == 0)
	{
		m_Fishing_HasHorror = true;
		m_Fishing_HasMob = true;

		m_CurrentFishIconLoc = make_pair(1029, 367);

		localstatus = fish(m_Fishing_Locs_Acteul);
		updateStatus(localstatus);
		leftClick(m_Button_Leave);

		m_Fishing_HasHorror = false;
		m_Fishing_HasMob = false;
	}
	else if (m_SummaryInfo.currentLocation.compare("Elzion Airport") == 0)
	{
		m_Fishing_HasHorror = true;
		m_Fishing_HasMob = true;

		m_CurrentFishIconLoc = make_pair(1004, 448);

		localstatus = fish(m_Fishing_Locs_Elzion);
		updateStatus(localstatus);
		leftClick(m_Button_Leave);

		m_Fishing_HasHorror = false;
		m_Fishing_HasMob = false;
	}
	else if (m_SummaryInfo.currentLocation.compare("Zol Plains") == 0)
	{
		//no monster
		//m_CurrentFishIconLoc = make_pair(935, 455);

		localstatus = fish(m_Fishing_Locs_ZolPlains);
		updateStatus(localstatus);
		leftClick(m_Button_Leave);
	}
	else if (m_SummaryInfo.currentLocation.compare("Lake Tillian") == 0)
	{
		//has monster, no horror
		m_CurrentFishIconLoc = make_pair(741, 448);

		localstatus = fish(m_Fishing_Locs_Acteul);
		updateStatus(localstatus);
		leftClick(m_Button_Leave);
	}
	else if (m_SummaryInfo.currentLocation.compare("Vasu Mountains") == 0)
	{
		m_Fishing_HasHorror = true;
		m_Fishing_HasMob = true;

		m_CurrentFishIconLoc = make_pair(727, 442);

		localstatus = fish(m_Fishing_Locs_Vasu);
		updateStatus(localstatus);
		leftClick(m_Button_Leave);

		m_Fishing_HasHorror = false;
		m_Fishing_HasMob = false;
	}
	else if (m_SummaryInfo.currentLocation.compare("Karek Swampland") == 0)
	{
		//has monster, no horror
		m_CurrentFishIconLoc = make_pair(623, 448);

		localstatus = fish(m_Fishing_Locs_Acteul);
		updateStatus(localstatus);
		leftClick(m_Button_Leave);
	}
	else if (m_SummaryInfo.currentLocation.compare("Moonlight Forest") == 0)
	{
		//no monster
		m_CurrentFishIconLoc = make_pair(435, 360);

		localstatus = fish(m_Fishing_Locs_Moonlight);
		updateStatus(localstatus);
		leftClick(m_Button_Leave);
	}
	else if (m_SummaryInfo.currentLocation.compare("Ancient Battlefield") == 0)
	{
		//no monster
		m_CurrentFishIconLoc = make_pair(1300, 455);

		localstatus = fish(m_Fishing_Locs_AncientBattlefield);
		updateStatus(localstatus);
		leftClick(m_Button_Leave);
	}
	else if (m_SummaryInfo.currentLocation.compare("Snake Neck Igoma") == 0)
	{
		//no monster
		m_CurrentFishIconLoc = make_pair(685, 450);

		localstatus = fish(m_Fishing_Locs_Igoma);
		updateStatus(localstatus);
		leftClick(m_Button_Leave);
	}
	else if (m_SummaryInfo.currentLocation.compare("Rinde") == 0)
	{
		//has monster, no horror
		m_CurrentFishIconLoc = make_pair(970, 368);

		localstatus = fish(m_Fishing_Locs_KiraBeach);
		updateStatus(localstatus);
		leftClick(m_Button_Leave);
	}
	else if (m_SummaryInfo.currentLocation.compare("Serena Coast") == 0)
	{
		m_Fishing_HasHorror = true;
		m_Fishing_HasMob = true;

		m_CurrentFishIconLoc = make_pair(796, 448);

		localstatus = fish(m_Fishing_Locs_Acteul);
		updateStatus(localstatus);
		leftClick(m_Button_Leave);

		m_Fishing_HasHorror = false;
		m_Fishing_HasMob = false;
	}
	else if (m_SummaryInfo.currentLocation.compare("Rucyana Sands") == 0)
	{
		m_Fishing_HasHorror = true;
		m_Fishing_HasMob = true;

		m_CurrentFishIconLoc = make_pair(744, 411);

		localstatus = fish(m_Fishing_Locs_RucyanaSands);
		updateStatus(localstatus);
		leftClick(m_Button_Leave);

		m_Fishing_HasHorror = false;
		m_Fishing_HasMob = false;
	}
	else if (m_SummaryInfo.currentLocation.compare("Last Island") == 0)
	{
		m_Fishing_HasHorror = true;
		m_Fishing_HasMob = true;

		localstatus = fish(m_Fishing_Locs_KiraBeach);
		updateStatus(localstatus);
		leftClick(m_Button_Leave);

		m_Fishing_HasHorror = false;
		m_Fishing_HasMob = false;
	}
	else if (m_SummaryInfo.currentLocation.compare("Nilva") == 0)
	{
		//has monster, no horror
		m_CurrentFishIconLoc = make_pair(1068, 400);

		localstatus = fish(m_Fishing_Locs_Elzion);
		updateStatus(localstatus);
		leftClick(m_Button_Leave);
	}
	else if (m_SummaryInfo.currentLocation.compare("Man-Eating Swamp") == 0)
	{
		//has monster, no horror
		localstatus = fish(m_Fishing_Locs_Acteul);
		updateStatus(localstatus);
		leftClick(m_Button_Leave);
	}
	else if (m_SummaryInfo.currentLocation.compare("Charol Plains") == 0)
	{
		//has monster, no horror
		m_CurrentFishIconLoc = make_pair(840, 456);

		localstatus = fish(m_Fishing_Locs_Baruoki);
		updateStatus(localstatus);
		leftClick(m_Button_Leave);
	}
	else if (m_SummaryInfo.currentLocation.compare("Dimension Rift") == 0)
	{
		m_Fishing_HasHorror = true;
		m_Fishing_HasMob = true;

		m_CurrentFishIconLoc = make_pair(408, 345);

		localstatus = fish(m_Fishing_Locs_DimensionRift);
		updateStatus(localstatus);
		leftClick(m_Button_Leave);

		m_Fishing_HasHorror = false;
		m_Fishing_HasMob = false;
	}
	else if (m_SummaryInfo.currentLocation.compare("Dragon Palace - Outer Wall Plum") == 0)
	{
		//no monster
		//m_CurrentFishIconLoc = make_pair(450, 400);

		localstatus = fish(m_Fishing_Locs_DragonPalace);
		updateStatus(localstatus);
		leftClick(m_Button_Leave);
	}
	else if (m_SummaryInfo.currentLocation.compare("Dragon Palace Past - Outer Wall Plum") == 0)
	{
		//no monster
		//m_CurrentFishIconLoc = make_pair(450, 400);

		localstatus = fish(m_Fishing_Locs_DragonPalace);
		updateStatus(localstatus);
		leftClick(m_Button_Leave);
	}
	else if (m_SummaryInfo.currentLocation.compare("Dragon Palace - Outer Wall Bamboo") == 0)
	{
		//no monster
		//m_CurrentFishIconLoc = make_pair(1240, 410);

		localstatus = fish(m_Fishing_Locs_DragonPalace);
		updateStatus(localstatus);
		leftClick(m_Button_Leave);
	}
	else if (m_SummaryInfo.currentLocation.compare("Dragon Palace Past - Outer Wall Bamboo") == 0)
	{
		//no monster
		//m_CurrentFishIconLoc = make_pair(1240, 410);

		localstatus = fish(m_Fishing_Locs_DragonPalace);
		updateStatus(localstatus);
		leftClick(m_Button_Leave);
	}
	else if (m_SummaryInfo.currentLocation.compare("Dragon Palace - Inner Wall") == 0)
	{
		//no monster
		//m_CurrentFishIconLoc = make_pair(751, 407);

		localstatus = fish(m_Fishing_Locs_DragonPalace);
		updateStatus(localstatus);
		leftClick(m_Button_Leave);
	}
	else if (m_SummaryInfo.currentLocation.compare("Dragon Palace Past - Inner Wall") == 0)
	{
		//no monster
		//m_CurrentFishIconLoc = make_pair(603, 408);

		localstatus = fish(m_Fishing_Locs_DragonPalace);
		updateStatus(localstatus);
		leftClick(m_Button_Leave);
	}
	else if (m_SummaryInfo.currentLocation.compare("Dragon Palace - Outer Wall Pine") == 0)
	{
		//no monster
		//m_CurrentFishIconLoc = make_pair(630, 408);

		localstatus = fish(m_Fishing_Locs_DragonPalace);
		updateStatus(localstatus);
		leftClick(m_Button_Leave);
	}
	else if (m_SummaryInfo.currentLocation.compare("Dragon Palace Past - Outer Wall Pine") == 0)
	{
		//no monster
		//m_CurrentFishIconLoc = make_pair(630, 405);

		localstatus = fish(m_Fishing_Locs_DragonPalace);
		updateStatus(localstatus);
		leftClick(m_Button_Leave);
	}

	return m_resValue;
}

void CAEBot::fishIconClickFunction()
{
	//In Man Eating Swamp, any battle shifts your position, so the fish icon location must be found again
	if (m_SummaryInfo.currentLocation.compare("Kira Beach") == 0 || m_SummaryInfo.currentLocation.compare("Last Island") == 0)
	{
		pair<bool, pair<int, int>> findclickres = findClick("FishIcon", (int)M_WIDTH, (int)M_ABOVE_MENU, 0, 0);
		leftClick(findclickres.second.first, findclickres.second.second, m_Action_Interval, false);
	}
	else if (m_SummaryInfo.currentLocation.compare("Man-Eating Swamp") == 0)
	{
		pair<bool, pair<int, int>> findclickres = findClick("SwampFishIcon", (int)M_WIDTH, (int)M_ABOVE_MENU, 0, 0);
		leftClick(findclickres.second.first, findclickres.second.second, m_Action_Interval, false);
	}
	else
	{
		leftClick(m_CurrentFishIconLoc.first, m_CurrentFishIconLoc.second, m_Action_Interval);
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
	
	findclickres = findClick("HarpoonFish", (int)M_WIDTH, 200, 0, 240);
	harpoonfish = findclickres.second;

	if (m_Harpoon_MassShooting)
	{
		harpoonMassShooting();
	}
	else
	{
		leftClick(harpoonfish.first, harpoonfish.second, m_Action_Interval, false);
		Sleep(m_Harpoon_Interval); // wait for the result to load
	}

	while (!compareImage("FishingResult"))
	{
		// five attemps, failed
		if (counter >= 4)
		{
			snprintf(m_debugMsg, 1024, "Failed %d attemps...", counter + 1);
			dbgMsg(m_Debug_Type_Fishing, debug_Detail);

			if (m_IsPrint && m_Debug_Type_Fishing) captureScreenNow("harpoonFunction");

			return status_NoHarpoonCatch;
		}

		// it is possible the fish escapes
		txt = ocrPicture(ocr_Alphabetic, 1100, 80, 330, 75);
		// try to search the harpoon
		if (txt.find("harpoon") != string::npos || txt.find("the") != string::npos)
		{
			snprintf(m_debugMsg, 1024, "No catch \"%s\"....", txt.c_str());
			dbgMsg(m_Debug_Type_Fishing, debug_Detail);

			leftClick(m_Button_PassThrough, m_Action_Interval);
			return status_HarpoonCatchFail;
		}

		// it is possible the fish escapes
		txt = ocrPicture(ocr_Alphabetic, 1100, 80, 330, 75);
		// try to search the harpoon
		// "It nimbly dodged the harpoon!" or "It was too strong and the harpoon was flung away!"
		if (txt.find("harpoon") != string::npos || txt.find("the") != string::npos)
		{
			snprintf(m_debugMsg, 1024, "No catch \"%s\"....", txt.c_str());
			dbgMsg(m_Debug_Type_Fishing, debug_Detail);

			leftClick(m_Button_PassThrough, m_Action_Interval);
			return status_HarpoonCatchFail;
		}
		// "Your cooling box is full!"
		else if (txt.find("cooling") != string::npos || txt.find("box") != string::npos || txt.find("full") != string::npos)
		{

			return status_BreakRun;
		}

		findclickres = findClick("HarpoonFish", (int)M_WIDTH, 200, 0, 240);
		harpoonfish = findclickres.second;

		//snprintf(m_debugMsg, 1024, "Harpoon fish attempt [%d] %d %d....", counter + 1, harpoonfish.first, harpoonfish.second);
		//dbgMsg(m_Debug_Type_Fishing);

		leftClick(harpoonfish.first, harpoonfish.second, m_Action_Interval, false);
		Sleep(m_Harpoon_Interval); // wait for the result to load

		counter++;
	}

	counter = 0;
	startingtime = time(NULL);

	while (compareImage("FishingResult"))
	{
		counter++;

		m_SummaryInfo.runFishCaught = m_SummaryInfo.runFishCaught + 1;
		m_SummaryInfo.totalFishCaught = m_SummaryInfo.totalFishCaught + 1;

		if (counter >= 6)
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
			snprintf(m_debugMsg, 1024, "Time out. Stop harpoon at [%s]", m_SummaryInfo.currentLocation.c_str());
			dbgMsg(m_Debug_Type_Fishing, debug_Alert);
			
			// time out, something wrong, let's go
			return status_Timeout;
		}
	}

	snprintf(m_debugMsg, 1024, "Harpoon fish %d caught", counter);
	dbgMsg(m_Debug_Type_Fishing, debug_Brief);

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
	time_t currenttime, startingtime;
	bool foundHorror = false;

	if (m_Harpoon_MassShooting)
	{
		harpoonMassShooting();

		if (inBattle())
			foundHorror = true;
	}
	else
	{
		findclickres = findClick("HarpoonHorror", 1545, 500, 100, 50);
		harpoonhorror = findclickres.second;

		if (findclickres.first &&
			!(harpoonhorror.first >= 1390 * m_widthPct && harpoonhorror.second <= 235 * m_heightPct))
		{
			leftClick(harpoonhorror.first, harpoonhorror.second, m_Action_Interval, false);
			foundHorror = true;
		}
	}

	if (foundHorror)
	{
		leftClick(harpoonhorror.first, harpoonhorror.second, m_Action_Interval, false);

		startingtime = time(NULL);
		while (!inBattle())
		{
			currenttime = time(NULL);
			auto timegap = difftime(currenttime, startingtime);
			if (timegap > m_Time_Out) // return if timeout
			{
				snprintf(m_debugMsg, 1024, "Harpoon Horror time out at [%s]", m_SummaryInfo.currentLocation.c_str());
				dbgMsg(m_Debug_Type_Fishing, debug_Alert);

				// time out, something wrong, let's go
				return status_Timeout;
			}

			Sleep(500); // wait for the battle to start
		}

		snprintf(m_debugMsg, 1024, "Harpoon Horror encountered at [%s]", m_SummaryInfo.currentLocation.c_str());
		dbgMsg(m_Debug_Type_Fishing, debug_Brief);

		m_resValue = engageHorrorFightNow(false);
		if (m_resValue == status_NoError)
		{
			snprintf(m_debugMsg, 1024, "Harpoon Horror caught at [%s]", m_SummaryInfo.currentLocation.c_str());
			dbgMsg(m_Debug_Type_Fishing, debug_Key);
		}

		leftClick(m_Button_PassThrough, m_Action_Interval);
		return m_resValue;
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
			if (checkStatus(status_MajorError))
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

	if (m_Harpoon_MassShooting)
		return status_NoHarpoonTrap;

	findclickres = findClick("HarpoonTrap", (int)M_WIDTH, (int) 440, 0, (int) 160);
	findclicktrap = findclickres.second;

	leftClick(findclicktrap.first, findclicktrap.second, m_Action_Interval, false);

	findclickres = findClick("HarpoonTrapLarge", (int)M_WIDTH, (int) 440, 0, (int) 160);
	findtraplarge = findclickres.first;
	findclicktraplarge = findclickres.second;

	if (findtraplarge && 
		abs(findclicktrap.first - findclicktraplarge.first) < 100 && abs(findclicktrap.second - findclicktraplarge.second) < 100 )
	{
		leftClick(findclicktraplarge.first, findclicktraplarge.second, m_Action_Interval, false);

		return harpoonSetTrap(trapRef);
	}
	else 
	{
		string txt = ocrPicture(ocr_AlphaNumeric, 900, 65, 415, 48);
		// Less than xx minute(s) before the
		// The alertness is rising. There's
		if (txt.find("before") != string::npos || txt.find("than") != string::npos || txt.find("minute") != string::npos || txt.find("The") != string::npos)
		{
			snprintf(m_debugMsg, 1024, "Trap [%s] \"%s\"", trapRef.c_str(), txt.c_str());
			dbgMsg(m_Debug_Type_Fishing, debug_Brief);

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
			txt = ocrPicture(ocr_AlphaNumeric, 900, 127, 415, 48);
			while (txt.find("total") == string::npos && txt.find("caught") == string::npos && txt.find("fish") == string::npos)
			{
				if (checkStatus(status_MajorError))
					return m_resValue;
			
				currenttime = time(NULL);
				auto timegap = difftime(currenttime, startingtime);

				if (timegap > m_Time_Out) // return if timeout
				{
					snprintf(m_debugMsg, 1024, "Time out for Trap [%s]", trapRef.c_str());
					dbgMsg(m_Debug_Type_Fishing, debug_Alert);

					if (m_IsPrint && m_Debug_Type_Fishing) captureScreenNow("HarpoonTrapFunction timeout result");

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

				txt = ocrPicture(ocr_Alphabetic, 900, 127, 415, 48);
			}

			snprintf(m_debugMsg, 1024, "Harvest! Trap [%s] \"%s\"", trapRef.c_str(), txt.c_str());
			dbgMsg(m_Debug_Type_Fishing, debug_Key);

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

Status_Code CAEBot::harpoonSetTrap(string trapRef)
{
	string txt;

	bool imageresult = compareImage("TopTitleBar");
	if (!imageresult) // something wrong with trap set up window
	{
		snprintf(m_debugMsg, 1024, "Trap [%s] set up window is wrong", trapRef.c_str());
		dbgMsg(m_Debug_Type_Fishing, debug_Alert);

		if (m_IsPrint && m_Debug_Type_Fishing) captureScreenNow("HarpoonSetupTrap");
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

	txt = ocrPicture(ocr_Alphabetic, 220, 40, 760, 160);
	if (txt.find("a") == string::npos && txt.find("e") == string::npos)
	{
		leftClick(m_Button_X);
		return status_HarpoonTrapFail;
	}

	// choose the bottom most bait, 792, 636, 480, 324 if we want to buy and baits are still available
	vector<int> availablebaits;
	availablebaits.clear();
	for (int i = (int) m_baitList.size() - 1; i >= harpoonBait_Seaworm; i--)
	{
		snprintf(m_debugMsg, 1024, "Bait \"%s\" hasBait %x number %d", m_baitList[i].baitName.c_str(), m_baitList[i].hasBait, m_baitList[i].baitNumber);
		dbgMsg(m_Debug_Type_Fishing, debug_Detail);

		if (m_baitList[i].hasBait)
		{
			availablebaits.push_back(i);
		}
	}

	// just in case, click all baits above what we want to use
	for (int j = (int) availablebaits.size() ; j > 0 ; j--)
	{
		// choose the bottom most bait, 792, 636, 480, 324
		leftClick(780, 324 + (j - 1) * 156, 1000);
	}

	// choose the top most bait just in case
	leftClick(780, 324, 1000);

	//click to set up the trap
	leftClick(580, 880);

	snprintf(m_debugMsg, 1024, "Trap [%s] set up done", trapRef.c_str());
	dbgMsg(m_Debug_Type_Fishing, debug_Brief);

	leftClick(m_Button_PassThrough, m_Action_Interval);
	for (auto i = 0; i < 3; i++)
	{
		leftClick(m_Button_PassThrough, m_Fast_Action_Interval);
	}
	return status_NoError;
}

Status_Code CAEBot::lomPlatiumSlime()
{
	bool platiumslimeencountered = false;
	time_t currenttime, startingtime;
	currenttime = time(NULL);
	startingtime = currenttime;

	if (!compareImage("LOM Slime")) // not platium slime
	{
		if (checkStatus(status_MajorError))
			return m_resValue;

		currenttime = time(NULL);
		auto timegap = difftime(currenttime, startingtime);
		if (timegap > m_Time_Out) // return if timeout
		{
			if (m_IsPrint && m_Debug_Type_LOM) captureScreenNow("lomPlatiumSlime");

			snprintf(m_debugMsg, 1024, "lomPlatiumSlime timeout %d", (int)timegap);
			dbgMsg(m_Debug_Type_LOM, debug_Alert);

			return status_Timeout;
		}

		Sleep(500);
	}

	snprintf(m_debugMsg, 1024, "Platium Slime encountered at [%s]", m_SummaryInfo.currentLocation.c_str());
	dbgMsg(m_Debug_Type_LOM, debug_Brief);

	m_resValue = engageHorrorFightNow(false);
	if (m_resValue == status_NoError)
	{
		snprintf(m_debugMsg, 1024, "Platium Slime defeated at [%s]", m_SummaryInfo.currentLocation.c_str());
		dbgMsg(m_Debug_Type_LOM, debug_Key);

		leftClick(m_Button_PassThrough, m_Action_Interval);
		return m_resValue;
	}
	else
	{
		return status_NotFight;
	}
}

Status_Code CAEBot::stateFishing()
{
	Status_Code localstatus;
	snprintf(m_debugMsg, 1024, "Start fishing >>>>>>>>>");
	dbgMsg(m_Debug_Type_Fishing, debug_Key);

	while (1)
	{
		if (checkStatus(status_MajorError) || checkStatus(status_MediumError))
			return m_resValue;

		loadFishingConfig();
		loadPathConfig();
		loadSettingConfig();

		goToFishVendor();

		m_SummaryInfo.loopNumber = m_SummaryInfo.loopNumber + 1;

		for (int i = 0; i < m_Fishing_Spots.size(); ++i)
		{
			if (checkStatus(status_MajorError) || checkStatus(status_MediumError))
				return m_resValue;

			m_CurrentBaitsToUse = &(m_Fishing_Spots[i].baitsToUse);

			clearStatus();
			m_SummaryInfo.locationNumber = m_SummaryInfo.locationNumber + 1;
			m_SummaryInfo.runFishCaught = 0;
			m_SummaryInfo.runMobFought = 0;
			m_SummaryInfo.runHorrorFought = 0;
			m_SummaryInfo.currentLocation = m_Fishing_Spots[i].locationName;

			goToSpacetimeRift();

			if (m_Fishing_PondTeleport)
			{
				localstatus = goToFishingLocation(m_Fishing_Spots[i].pondName);
				updateStatus(localstatus);
			}
			else
			{
				localstatus = goToTargetLocation(m_SummaryInfo.currentLocation);
				updateStatus(localstatus);
			}

			if (checkStatus(status_MajorError) || checkStatus(status_MediumError))
				return m_resValue;

			localstatus = fishFunction();
			updateStatus(localstatus);
		}
	}
}

Status_Code CAEBot::stateHarpoonFishing()
{
	Status_Code localstatus;
	snprintf(m_debugMsg, 1024, "Start harpoon fishing>>>>>>>>>");
	dbgMsg(m_Debug_Type_Fishing, debug_Key);

	bool firstrun = true;

	while (1)
	{
		if (checkStatus(status_MajorError) || checkStatus(status_MediumError))
			return m_resValue;

		loadFishingConfig();
		loadPathConfig();
		loadSettingConfig();

		if (!m_Harpoon_SkipVendor)
			goToHarpoonVendor(firstrun);
		
		firstrun = false;

		for (auto j = 0; j < m_Harpoon_Loop; j++)
		{
			if (checkStatus(status_MajorError) || checkStatus(status_MediumError))
				return m_resValue;

			m_SummaryInfo.loopNumber = m_SummaryInfo.loopNumber + 1;

			for (int i = 0; i < m_Harpoon_Spots.size(); ++i)
			{
				if (checkStatus(status_MajorError) || checkStatus(status_MediumError))
					return m_resValue;

				clearStatus();
				m_SummaryInfo.locationNumber = m_SummaryInfo.locationNumber + 1;
				m_SummaryInfo.runFishCaught = 0;
				m_SummaryInfo.runMobFought = 0;
				m_SummaryInfo.runHorrorFought = 0;
				m_SummaryInfo.currentLocation = m_Harpoon_Spots[i].first;

				localstatus = goToTargetLocation(m_SummaryInfo.currentLocation);
				updateStatus(localstatus);
			}
		}
	}
}

Status_Code CAEBot::stateSilverHitBell(Bot_Mode silverHitBellstate)
{
	Mat bellPicCrop;
	int MSD1;
	int lowMSD = (int) 99999999999;

	while (1)
	{
		if (checkStatus(status_MajorError))
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
			if (checkStatus(status_MajorError))
				return m_resValue;

			if (silverHitBellstate == silverHitBell30Mode && i > 30)
				break;
			else if (silverHitBellstate == silverHitBell999Mode && i > 999)
				return status_NoError;

			if (j > 2000)
				break;

			Sleep(1);
			bitBltWholeScreen();

			imageInfo hitBellPic = retrieveImage("Bell");
			copyPartialPic(bellPicCrop, hitBellPic.width, hitBellPic.height, hitBellPic.coordx, hitBellPic.coordy);

			MSD1 = (int)cv::norm(hitBellPic.image, bellPicCrop);
			MSD1 = MSD1 * MSD1 / (int)hitBellPic.image.total();

			if (MSD1 < m_Image_Threshold)
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
		if (checkStatus(status_MajorError))
			return m_resValue;

		Sleep(1);
		bitBltWholeScreen();

		if (check) {

			imageInfo jmpRopePic2 = retrieveImage("jmpRopePic2");
			copyPartialPic(jmpRopePicCrop, jmpRopePic2.width, jmpRopePic2.height, jmpRopePic2.coordx, jmpRopePic2.coordy);

			MSD1 = (int) cv::norm(jmpRopePic2.image, jmpRopePicCrop);
			MSD1 = MSD1 * MSD1 / (int) jmpRopePic2.image.total();

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
			imageInfo jmpRopePic1 = retrieveImage("jmpRopePic1");
			copyPartialPic(jmpRopePicCrop, jmpRopePic1.width, jmpRopePic1.height, jmpRopePic1.coordx, jmpRopePic1.coordy);

			MSD1 = (int) cv::norm(jmpRopePic1.image, jmpRopePicCrop);
			MSD1 = MSD1 * MSD1 / (int) jmpRopePic1.image.total();

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
		if (checkStatus(status_MajorError))
			return m_resValue;

		Sleep(1);
		bitBltWholeScreen();

		if (check) {
			imageInfo jmpRopePic4 = retrieveImage("jmpRopePic4");
			copyPartialPic(jmpRopePicCrop, jmpRopePic4.width, jmpRopePic4.height, jmpRopePic4.coordx, jmpRopePic4.coordy);

			MSD1 = (int) cv::norm(jmpRopePic4.image, jmpRopePicCrop);
			MSD1 = MSD1 * MSD1 / (int) jmpRopePic4.image.total();

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
			imageInfo jmpRopePic3 = retrieveImage("jmpRopePic3");
			copyPartialPic(jmpRopePicCrop, jmpRopePic3.width, jmpRopePic3.height, jmpRopePic3.coordx, jmpRopePic3.coordy);

			MSD1 = (int) cv::norm(jmpRopePic3.image, jmpRopePicCrop);
			MSD1 = MSD1 * MSD1 / (int) jmpRopePic3.image.total();

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
	snprintf(m_debugMsg, 1024, "To separate grasta >>>>>>>>>");
	dbgMsg(m_Debug_Type_Grasta, debug_Key);

	int i = 0;
	//Only 4 slots are fully on screen at a time, with the 5th partially visible. 
	//if last grasta is 0, then scroll one by one
	//otherwise fast scroll to reach to the target grasta

	int k=0;
	int grastaseparated = 0;
	while (1)
	{
		if (checkStatus(status_MajorError))
			return m_resValue;

		if (k < m_Grasta_Target)
		{
			snprintf(m_debugMsg, 1024, "Go to grasta [%d]", m_Grasta_Target);
			dbgMsg(m_Debug_Type_Grasta, debug_Detail);

			for (; k < m_Grasta_Target; k++)
			{
				snprintf(m_debugMsg, 1024, ".");
				dbgMsg(m_Debug_Type_Grasta, debug_Detail);
				drag(DOWN, m_Grasta_ScrollHeight, 1225, 900, 1, m_Grasta_ScrollRatio);
			}

			snprintf(m_debugMsg, 1024, "located");
			dbgMsg(m_Debug_Type_Grasta, debug_Detail);
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
		txt = ocrPicture(ocr_Alphabetic, 556, 62, 95, 74);
		string attr_1 = ocrPicture(ocr_Alphabetic, 162, 36, 391, 185);
		int value_1 = stoi(ocrPicture(ocr_Numeric, 162, 36, 577, 185));
		string attr_2 = ocrPicture(ocr_Alphabetic, 162, 36, 391, 241);
		int value_2 = stoi(ocrPicture(ocr_Numeric, 162, 36, 577, 241));

		//found grasta to separate
		snprintf(m_debugMsg, 1024, "Grasta [%d] %s: %s %d %s %d", k, txt.c_str(), attr_1.c_str(), value_1, attr_2.c_str(), value_2);
		dbgMsg(m_Debug_Type_Grasta, debug_Detail);

		for (auto j = 0; j < m_Grasta_Names.size(); j++)
		{
			if (txt.find(m_Grasta_Names[j]) != string::npos)
			{
				snprintf(m_debugMsg, 1024, "To separate grasta[%d] %s: %s %d %s %d", k, txt.c_str(), attr_1.c_str(), value_1, attr_2.c_str(), value_2);
				dbgMsg(m_Debug_Type_Grasta, debug_Detail);

				leftClick(350, 870, 500);
				leftClick(1080, 600, 500);

				string txtOK;
				txtOK = ocrPicture(ocr_Alphabetic, 154, 54, 800, 720);
				while (txtOK.find("OK") == string::npos)
				{
					snprintf(m_debugMsg, 1024, ".");
					dbgMsg(m_Debug_Type_Grasta, debug_Detail);
					Sleep(500);
					txtOK = ocrPicture(ocr_Alphabetic, 154, 54, 800, 720);
				}

				leftClick(880, 750, 500);
				while (!compareImage("TopTitleBar"))
				{
					snprintf(m_debugMsg, 1024, ".");
					dbgMsg(m_Debug_Type_Grasta, debug_Detail);
					Sleep(500);
				}

				grastaseparated++;

				snprintf(m_debugMsg, 1024, "%s: %s %d %s %d separated. total %d separated", txt.c_str(), attr_1.c_str(), value_1, attr_2.c_str(), value_2, grastaseparated);
				dbgMsg(m_Debug_Type_Grasta, debug_Brief);

				k = 0;
				break;
			}
		}

		if (m_Grasta_NumberCap && grastaseparated >= m_Grasta_NumberCap)
		{
			snprintf(m_debugMsg, 1024, "Separate cap reached [%d/%d]", grastaseparated, m_Grasta_NumberCap);
			dbgMsg(m_Debug_Type_Grasta, debug_Brief);

			k = 0;
			grastaseparated = 0;
			leftClick(m_Button_X);


			pair<bool, pair<int, int>> findclickres = findClick("ExclamationGrasta", (int)M_WIDTH, (int)M_ABOVE_MENU, 0, 0);
			leftClick(findclickres.second.first, findclickres.second.second, m_Action_Interval, false);

			leftClick(m_Grasta_Button[Grasta_Action_Separate].xyPosition);

			while (!compareImage("TopTitleBar"))
			{
				snprintf(m_debugMsg, 1024, ".");
				dbgMsg(m_Debug_Type_Grasta, debug_Detail);
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

			while (!compareImage("TopTitleBar"))
			{
				snprintf(m_debugMsg, 1024, ".");
				dbgMsg(m_Debug_Type_Grasta, debug_Detail);
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
		snprintf(m_debugMsg, 1024, "Start grinding [%s] endless", m_SummaryInfo.currentLocation.c_str());
	else
		snprintf(m_debugMsg, 1024, "Start grinding [%s]", m_SummaryInfo.currentLocation.c_str());

	dbgMsg(m_Debug_Type_Grinding, debug_Key);

	currenttime = time(NULL);
	lastFight = currenttime;
	startingtime = currenttime;

	int currentDirection = LEFT;

	while (endlessGrinding || !m_Grinding_SkipRunning)
	{
		if (checkStatus(status_MajorError) || checkStatus(status_MediumError))
			return m_resValue;

		while (!inBattle())
		{
			if (checkStatus(status_MajorError) || checkStatus(status_MediumError))
				return m_resValue;

			currenttime = time(NULL);
			auto timegap = difftime(currenttime, startingtime);
			if (forcetimeout && forcetimeout < timegap)
			{
				snprintf(m_debugMsg, 1024, "Stop grinding");
				dbgMsg(m_Debug_Type_Grinding, debug_Key);

				return status_NoError;
			}
				
			timegap = difftime(currenttime, lastFight);
			if (timegap > m_Time_Out) // return if timeout
			{
				snprintf(m_debugMsg, 1024, "Leaving [%s] - Idling %f", m_SummaryInfo.currentLocation.c_str(), timegap);
				dbgMsg(m_Debug_Type_Grinding, debug_Key);

				if (m_IsPrint && m_Debug_Type_Grinding) captureScreenNow("Idling");

				return status_Timeout;
			}

			if (m_Grinding_Direction == LR)
			{
				currentDirection = (currentDirection + 1) % 2;
			}
			else
			{
				currentDirection = m_Grinding_Direction;
			}

			switch (currentDirection) {
			case LEFT:
				walk(LEFT, m_Grinding_Step, 10);
				break;
			case RIGHT:
			default:
				walk(RIGHT, m_Grinding_Step, 10);
			}
		}

		engageMobFightNow();

		lastFight = time(NULL);
	}

	snprintf(m_debugMsg, 1024, "Leaving [%s] (%d) - Counter Number reached", m_SummaryInfo.currentLocation.c_str(), m_CurrentGrindingCounter);
	dbgMsg(m_Debug_Type_Grinding, debug_Key);

	leftClick(m_Button_PassThrough);

	return status_NoError;
}

Status_Code CAEBot::stateTravelGrinding()
{
	Status_Code localstatus;
	snprintf(m_debugMsg, 1024, "Start travel grinding >>>>>>>>>");
	dbgMsg(m_Debug_Type_Grinding, debug_Key);

	while (1)
	{
		if (checkStatus(status_MajorError) || checkStatus(status_MediumError))
			return m_resValue;

		loadPathConfig();
		loadSettingConfig();

		m_SummaryInfo.loopNumber = m_SummaryInfo.loopNumber + 1;

		for (auto i = 0; i < m_Grinding_Spots.size(); i++)
		{
			m_SummaryInfo.currentLocation = m_Grinding_Spots[i].first;

			for (auto k = 0; k < m_Grinding_Spots[i].second; k++)
			{
				if (checkStatus(status_MajorError) || checkStatus(status_MediumError))
					return m_resValue;
				
				m_CurrentGrindingCounter = 0;

				snprintf(m_debugMsg, 1024, "Preparing to grind [%s]", m_SummaryInfo.currentLocation.c_str());
				dbgMsg(m_Debug_Type_Grinding, debug_Brief);

				if (! m_Grinding_SkipSpaceTimeRift)
					goToSpacetimeRift();

				clearStatus();
				m_SummaryInfo.locationNumber = m_SummaryInfo.locationNumber + 1;
				m_SummaryInfo.runFishCaught = 0;
				m_SummaryInfo.runMobFought = 0;
				m_SummaryInfo.runHorrorFought = 0;

				localstatus = goToTargetLocation(m_SummaryInfo.currentLocation); //load X 1280 Y 720 coordination 
				updateStatus(localstatus);
			}
		}

		//Wait to go back for refresh
		Sleep(m_Action_Interval);
	}
}


Status_Code CAEBot::stateStationGrinding()
{
	Status_Code localstatus;
	snprintf(m_debugMsg, 1024, "Start station grinding >>>>>>>>>");
	dbgMsg(m_Debug_Type_Grinding, debug_Key);

	while (1)
	{
		if (checkStatus(status_MajorError) || checkStatus(status_MediumError))
			return m_resValue;

		loadPathConfig();
		loadSettingConfig();

		snprintf(m_debugMsg, 1024, "Preparing to stay here grinding [%d]", m_SummaryInfo.loopNumber);
		dbgMsg(m_Debug_Type_Grinding, debug_Brief);

		m_SummaryInfo.loopNumber = m_SummaryInfo.loopNumber + 1;

		clearStatus();
		m_SummaryInfo.locationNumber = m_SummaryInfo.locationNumber + 1;
		m_SummaryInfo.runFishCaught = 0;
		m_SummaryInfo.runMobFought = 0;
		m_SummaryInfo.runHorrorFought = 0;

		m_SummaryInfo.currentLocation = "Local";

		localstatus = goToTargetLocation(m_SummaryInfo.currentLocation);
		updateStatus(localstatus);
	}
}

Status_Code CAEBot::stateLOMSlimeGrinding()
{
	Status_Code localstatus;
	snprintf(m_debugMsg, 1024, "Start LOM Slime grinding >>>>>>>>>");
	dbgMsg(m_Debug_Type_Grinding, debug_Key);

	while (1)
	{
		if (checkStatus(status_MajorError) || checkStatus(status_MediumError))
			return m_resValue;

		loadPathConfig();
		loadSettingConfig();

		m_Grinding_Spots.clear(); // re config the grinding spots

		if (!m_Grinding_LOMHeal.empty())
			m_Grinding_Spots.push_back(make_pair(m_Grinding_LOMHeal, 1));

		if (!m_Grinding_LOMSlimeA.empty())
			m_Grinding_Spots.push_back(make_pair(m_Grinding_LOMSlimeA, 1));

		if (!m_Grinding_LOMHeal.empty())
			m_Grinding_Spots.push_back(make_pair(m_Grinding_LOMHeal, 1));

		if (!m_Grinding_LOMSlimeB.empty())
			m_Grinding_Spots.push_back(make_pair(m_Grinding_LOMSlimeB, 1));

		if (!m_Grinding_LOMSlimeRun.empty())
			m_Grinding_Spots.push_back(make_pair(m_Grinding_LOMSlimeRun, m_Grinding_LOMTurn));

		snprintf(m_debugMsg, 1024, "Grinding LOM Slime:::");
		dbgMsg(m_Debug_Type_LOM, debug_Detail);
		for (auto i = 0; i < m_Grinding_Spots.size(); i++)
		{
			snprintf(m_debugMsg, 1024, "---> %s %d", (m_Grinding_Spots[i].first).c_str(), m_Grinding_Spots[i].second);
			dbgMsg(m_Debug_Type_LOM, debug_Detail);
		}

		m_SummaryInfo.loopNumber = m_SummaryInfo.loopNumber + 1;

		for (auto i = 0; i < m_Grinding_Spots.size(); i++)
		{
			m_SummaryInfo.currentLocation = m_Grinding_Spots[i].first;

			for (auto k = 0; k < m_Grinding_Spots[i].second; k++)
			{
				clearStatus();
				m_SummaryInfo.locationNumber = m_SummaryInfo.locationNumber + 1;
				m_SummaryInfo.runFishCaught = 0;
				m_SummaryInfo.runMobFought = 0;
				m_SummaryInfo.runHorrorFought = 0;

				localstatus = goToTargetLocation(m_SummaryInfo.currentLocation); //load X 1280 Y 720 coordination 
				updateStatus(localstatus);
				if (checkStatus(status_Timeout))
				{
					clearStatus();
					m_SummaryInfo.locationNumber = m_SummaryInfo.locationNumber + 1;
					m_SummaryInfo.runFishCaught = 0;
					m_SummaryInfo.runMobFought = 0;
					m_SummaryInfo.runHorrorFought = 0;
					m_SummaryInfo.currentLocation = m_Grinding_LOMReset;

					localstatus = goToTargetLocation(m_SummaryInfo.currentLocation); // reset
					updateStatus(localstatus);
				}
			}
		}

		//Wait to go back for refresh
		Sleep(m_Action_Interval);
	}
}

Status_Code CAEBot::stateEndlessGrinding()
{
	snprintf(m_debugMsg, 1024, "Start endless grinding >>>>>>>>>");
	dbgMsg(m_Debug_Type_Grinding, debug_Key);

	while (1)
	{
		if (checkStatus(status_MajorError) || checkStatus(status_MediumError))
			return m_resValue;

		loadPathConfig();
		loadSettingConfig();

		m_SummaryInfo.loopNumber = m_SummaryInfo.loopNumber + 1;

		clearStatus();
		m_SummaryInfo.locationNumber = m_SummaryInfo.locationNumber + 1;
		m_SummaryInfo.runFishCaught = 0;
		m_SummaryInfo.runMobFought = 0;
		m_SummaryInfo.runHorrorFought = 0;

		m_resValue = grindingRun(true);
	}
}

Status_Code CAEBot::dailyChroneStone()
{
	time_t currenttime, startingtime;
	pair<bool, pair <int, int>> findclickres;

	m_SummaryInfo.botmode = idleMode;

	snprintf(m_debugMsg, 1024, "Check Daily Chrone Stone >>>>>>>>>");
	dbgMsg(m_Debug_Type_Platform, debug_Key);

	startingtime = time(NULL);
	while (!compareImage("Watch Video"))
	{
		currenttime = time(NULL);
		auto timegap = difftime(currenttime, startingtime);
		if (timegap > m_DCS_Timeout) // return if timeout
		{
			snprintf(m_debugMsg, 1024, "Daily Chrone Stone timeout %d", (int)timegap);
			dbgMsg(m_Debug_Type_Platform, debug_Key);
			return status_Timeout;
		}
		else
		{
			Sleep(1000);
		}
	}

	//wait seconds for ads to fully load
	Sleep(m_DCS_Ad_Loading * 1000);

	snprintf(m_debugMsg, 1024, "Watch Video");
	dbgMsg(m_Debug_Type_Platform, debug_Key);

	findclickres = findClick("Watch Video", 0, 0, 0, 0);
	leftClick(findclickres.second.first, findclickres.second.second, m_Action_Interval, false);

	bool dailyAds = false;
	startingtime = time(NULL);
	while (!compareImage("Confirm"))
	{
		currenttime = time(NULL);
		auto timegap = difftime(currenttime, startingtime);
		if (timegap > m_DCS_Ad_Showing) // if ads showing
		{
			dailyAds = true;

			snprintf(m_debugMsg, 1024, "It should be a video Ad");
			dbgMsg(m_Debug_Type_Platform, debug_Key);
		}
		else
		{
			Sleep(1000);
		}
	}

	snprintf(m_debugMsg, 1024, "Confirm with Daily Chrone Stone %x", dailyAds);
	dbgMsg(m_Debug_Type_Platform, debug_Key);

	if (dailyAds)
	{
		leftClick(m_Button_AdsClose);
		Sleep(10000);
		leftClick(m_Button_AdsClose);
	}
	else
	{
		findclickres = findClick("Confirm", 0, 0, 0, 0);
		leftClick(findclickres.second.first, findclickres.second.second, m_Action_Interval, false);
	}

	snprintf(m_debugMsg, 1024, "to click confirm");
	dbgMsg(m_Debug_Type_Platform, debug_Key);

	startingtime = time(NULL);
	while (!compareImage("OK"))
	{
		currenttime = time(NULL);
		auto timegap = difftime(currenttime, startingtime);
		if (timegap > m_DCS_Ad_Showing) // if ads showing
		{
			snprintf(m_debugMsg, 1024, "Daily Chrone Stone ok timeout %d", (int)timegap);
			dbgMsg(m_Debug_Type_Platform, debug_Alert);
			return status_Timeout;
		}
		else
		{
			Sleep(1000);
		}
	}

	findclickres = findClick("OK", 0, 0, 0, 0);
	leftClick(findclickres.second.first, findclickres.second.second, m_Action_Interval, false);

	startingtime = time(NULL);
	while (!compareImage("Menu"))
	{
		currenttime = time(NULL);
		auto timegap = difftime(currenttime, startingtime);
		if (timegap > m_DCS_Ad_Showing) // return if timeout
		{
			snprintf(m_debugMsg, 1024, "Daily Chrone Stone result timeout %d", (int)timegap);
			dbgMsg(m_Debug_Type_Platform, debug_Alert);
			return status_Timeout;
		}
		else
		{
			Sleep(1000);
		}
	}

	snprintf(m_debugMsg, 1024, "Daily Chrone Stone completed");
	dbgMsg(m_Debug_Type_Platform, debug_Key);

	return status_NoError;
}

Status_Code CAEBot::captureScreenNow(const char* nameSuffix)
{
	Mat imagePicCrop;
	char filename[MAX_PATH];

	bitBltWholeScreen();
	copyPartialPic(imagePicCrop, (int)M_WIDTH, (int)M_HEIGHT, 0, 0);

	snprintf(filename, MAX_PATH, "%s\\%s_%s.jpg", m_CurrentPath, timeString(true), nameSuffix);

	snprintf(m_debugMsg, 1024, "%s", filename);
	dbgMsg(m_Debug_Type_Platform, debug_Key);

	imwrite(filename, imagePicCrop);

	return status_NoError;
}

Status_Code CAEBot::captureImageNow(Mat imagePicCrop, const char* nameSuffix)
{
	char filename[MAX_PATH];

	snprintf(filename, MAX_PATH, "%s\\%s_%s.jpg", m_CurrentPath, timeString(true), nameSuffix);

	snprintf(m_debugMsg, 1024, "%s", filename);
	dbgMsg(m_Debug_Type_Platform, debug_Brief);

	imwrite(filename, imagePicCrop);

	return status_NoError;
}

Status_Code CAEBot::idleNow()
{
	snprintf(m_debugMsg, 1024, "Idling...");
	dbgMsg(m_Debug_Type_Platform, debug_Key);

	while (1)
	{
		if ( (m_resValue == status_Stop) )
		{ 
			snprintf(m_debugMsg, 1024, "status %x mode %x", m_resValue, m_SummaryInfo.botmode);
			dbgMsg(m_Debug_Type_Platform, debug_Key);
			return m_resValue;
		}

		snprintf(m_debugMsg, 1024, "... %x", m_resValue);
		dbgMsg(m_Debug_Type_Platform, debug_Key);

		Sleep(1000);
	}
}

keyvalueInfo CAEBot::parseKeyValue(string str, string parser, bool reverseorder)
{
	keyvalueInfo returnkeyvalueInfo;

	int loc = (int)str.find(parser);
	if (reverseorder)
	{
		returnkeyvalueInfo.key = str.substr(loc + 1);
		ltrimString(returnkeyvalueInfo.key);
		returnkeyvalueInfo.value = str.substr(0, loc);
		rtrimString(returnkeyvalueInfo.value);
	}
	else
	{
		returnkeyvalueInfo.key = str.substr(0, loc);
		rtrimString(returnkeyvalueInfo.key);
		returnkeyvalueInfo.value = str.substr(loc + 1);
		ltrimString(returnkeyvalueInfo.value);
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

		snprintf(m_debugMsg, 1024, "Button [%s]: %d %d", (localKeyValue.key).c_str(), nameButton.xyPosition.first, nameButton.xyPosition.second);
		dbgMsg(m_Debug_Type_Setting, debug_Brief);

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
			snprintf(m_debugMsg, 1024, "To separate %s", (localKeyValue.key).c_str());
			dbgMsg(m_Debug_Type_Grasta, debug_Detail);

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
			snprintf(m_debugMsg, 1024, "[%d]%s %.1f %d", (int) i, baitName.c_str(), baitPrice, baitNumber);
			dbgMsg(m_Debug_Type_Setting, debug_Detail);
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

	m_EmulatorNumber = 0;

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
				m_EmulatorList.push_back(emulatorinfo);
				m_EmulatorNumber++;
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

		if (key.compare("Print Image") == 0)
		{
			m_IsPrint = stoi(value);
		}
		else if (key.compare("Stop Timer") == 0)
		{
			keyvalueInfo hmskeyvalue = parseKeyValue(value, string(":"));
			if (hmskeyvalue.key.compare("HH") == 0)
			{
				m_SummaryInfo.isStopTimer = false;
			}
			else 
			{
				m_SummaryInfo.isStopTimer = true;
				m_SummaryInfo.stopTimer.tm_hour = stoi(hmskeyvalue.key);

				hmskeyvalue = parseKeyValue(hmskeyvalue.value, string(":"));
				m_SummaryInfo.stopTimer.tm_min = stoi(hmskeyvalue.key);
				m_SummaryInfo.stopTimer.tm_sec = stoi(hmskeyvalue.value);
			}
		}
		else if (key.compare("Load Time") == 0)
		{
			m_Load_Time = stoi(value);
		}
		else if (key.compare("Action Interval") == 0)
		{
			m_Action_Interval = stoi(value);
		}
		else if (key.compare("Fast Interval") == 0)
		{
			m_Fast_Action_Interval = stoi(value);
		}
		else if (key.compare("Slow Interval") == 0)
		{
			m_Slow_Action_Interval = stoi(value);
		}
		else if (key.compare("Image Threshold") == 0)
		{
			m_Image_Threshold = stoi(value);
		}
		else if (key.compare("Time Out") == 0)
		{
			m_Time_Out = stoi(value);
		}
		else if (key.compare("Debug Type") == 0)
		{
			int debugvalue = stol(value, NULL, 2);

			m_Debug_Type_Setting = debugvalue & 0x1;
			m_Debug_Type_Platform = debugvalue & 0x2;
			m_Debug_Type_Path = debugvalue & 0x4;
			m_Debug_Type_Fighting = debugvalue & 0x8;
			m_Debug_Type_Grinding = debugvalue & 0x10;
			m_Debug_Type_Fishing = debugvalue & 0x20;
			m_Debug_Type_Grasta = debugvalue & 0x40;
			m_Debug_Type_LOM = debugvalue & 0x80;
		}
		else if (key.compare("Debug Level") == 0)
		{
			if (stoi(value) > debug_Detail)
				m_Debug_Level = debug_Detail;
			else
				m_Debug_Level = (Debug_Level)stoi(value);
		}

		/******************************/
		/* Daily Chrone Stone section */
		/******************************/
		else if (key.compare("DCS Waiting Time") == 0)
		{
			m_DCS_Waiting = stoi(value);
		}
		else if (key.compare("DCS Time Out") == 0)
		{
			m_DCS_Timeout = stoi(value);
		}
		else if (key.compare("DCS Ad Loading Time") == 0)
		{
			m_DCS_Ad_Loading = stoi(value);
		}
		else if (key.compare("DCS Ad Showing Time") == 0)
		{
			m_DCS_Ad_Showing = stoi(value);
		}


		/********************/
		/* Grinding section */
		/********************/
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
		else if (key.compare("AF Interval") == 0)
		{
			m_Grinding_AFInterval = stoi(value);
		}
		else if (key.compare("AF Full Threshold") == 0)
		{
			m_Grinding_AFFullThreshold = stoi(value);
		}
		else if (key.compare("Skip SpaceTime Rift") == 0)
		{
			m_Grinding_SkipSpaceTimeRift = stoi(value);
		}
		else if (key.compare("Skip Running") == 0)
		{
			m_Grinding_SkipRunning = stoi(value);
		}
		else if (key.compare("Direction") == 0)
		{
			if (value.compare("LEFT") == 0)
				m_Grinding_Direction = LEFT;
			else if (value.compare("RIGHT") == 0)
				m_Grinding_Direction = RIGHT;
			else if (value.compare("LR") == 0)
				m_Grinding_Direction = LR;
		}
		else if (key.compare("Run Steps") == 0)
		{
			m_Grinding_Step = stoi(value);
		}
		else if (key.compare("Run Count") == 0)
		{
			m_Grinding_Count = stoi(value);
		}
		else if (key.compare("SkillsHorrorSet") == 0)
		{
			m_Skill_HorrorSet = parseSkillsSet(file);
		}
		else if (key.compare("SkillsMobSet") == 0)
		{
			m_Skill_MobSet = parseSkillsSet(file);
		}
		else if (key.compare("GrindingSpots") == 0)
		{
			m_Grinding_Spots = parseGrindingSpotsList(file);
		}

		/***************************/
		/* fishing section         */
		/***************************/

		else if (key.compare("Horror Count") == 0)
		{
			m_Fishing_HorrorCount = stoi(value);
		}
		else if (key.compare("Pond Teleport") == 0)
		{
			m_Fishing_PondTeleport = stoi(value);
		}
		else if (key.compare("Harpoon Loop") == 0)
		{
			m_Harpoon_Loop = stoi(value);
		}
		else if (key.compare("Harpoon Mass Shooting") == 0)
		{
			m_Harpoon_MassShooting = stoi(value);
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

		/************************/
		/* LOM grinding section */
		/************************/

		else if (key.compare("LOM Heal") == 0)
		{
			m_Grinding_LOMHeal = value;
		}
		else if (key.compare("LOM Slime A") == 0)
		{
			m_Grinding_LOMSlimeA = value;
		}
		else if (key.compare("LOM Slime B") == 0)
		{
			m_Grinding_LOMSlimeB = value;
		}
		else if (key.compare("LOM Slime Run") == 0)
		{
			m_Grinding_LOMSlimeRun = value;
		}
		else if (key.compare("LOM turn") == 0)
		{
			m_Grinding_LOMTurn = stoi(value);
		}
		else if (key.compare("LOM Slime Reset") == 0)
		{
			m_Grinding_LOMReset = value;
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
		else if (key.compare("Grasta to Separate") == 0)
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
		else if (key.compare("AdsCloseButton") == 0)
		{
			m_Button_AdsClose = parseXYinfo(value);
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

	snprintf(m_debugMsg, 1024, "SkillsHorrorSet:::");
	dbgMsg(m_Debug_Type_Setting, debug_Brief);

	for (auto j = 0; j < m_Skill_HorrorSet.size(); j++)
	{
		snprintf(m_debugMsg, 1024, "Turn [%d]: %d %d %d %d", j, m_Skill_HorrorSet[j][0], m_Skill_HorrorSet[j][1], m_Skill_HorrorSet[j][2], m_Skill_HorrorSet[j][3]);
		dbgMsg(m_Debug_Type_Setting, debug_Brief);
	}

	snprintf(m_debugMsg, 1024, "SkillsMobSet:::");
	dbgMsg(m_Debug_Type_Setting, debug_Brief);

	for (auto j = 0; j < m_Skill_MobSet.size(); j++)
	{
		snprintf(m_debugMsg, 1024, "Turn [%d]: %d %d %d %d", j, m_Skill_MobSet[j][0], m_Skill_MobSet[j][1], m_Skill_MobSet[j][2], m_Skill_MobSet[j][3]);
		dbgMsg(m_Debug_Type_Setting, debug_Brief);
	}

	snprintf(m_debugMsg, 1024, "Grinding:::");
	dbgMsg(m_Debug_Type_Setting, debug_Brief);

	for (auto j = 0; j < m_Grinding_Spots.size(); j++)
	{
		snprintf(m_debugMsg, 1024, "---> %s %d", (m_Grinding_Spots[j].first).c_str(), m_Grinding_Spots[j].second);
		dbgMsg(m_Debug_Type_Setting, debug_Brief);
	}

	snprintf(m_debugMsg, 1024, "<<<< Setting profile loaded! >>>>");
	dbgMsg(m_Debug_Type_Setting, debug_Brief);
}

void CAEBot::loadFishingConfig()
{
	//Read in config file
	ifstream file("config_fishing.ini");

	m_Fishing_Spots.clear();
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
			int fishingOrder;
			string fishingLocation, pondName;

			int loc1 = (int)localKeyValue.value.find("(");
			int loc2 = (int)localKeyValue.value.find(")");

			fishingLocation = localKeyValue.value.substr(0, loc1);
			pondName = localKeyValue.value.substr(loc1 + 1, loc2 - loc1 - 1);

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
			newFishingSpot.pondName = pondName;

			m_Fishing_Spots.push_back(newFishingSpot);
			vector<baitInfo> tempBaitList;
			parseBaitForArea(file, false, m_Fishing_Spots.back(), baitsNeeded, tempBaitList);
		}

		/************************************/
		/* Harpoon Fishing Location section */
		/************************************/
		else if (localKeyValue.key.compare("HarpoonLocation") == 0)
		{
			m_Harpoon_Spots = parseGrindingSpotsList(file);
		}

	}

	snprintf(m_debugMsg, 1024, "Fishing:::");
	dbgMsg(m_Debug_Type_Setting, debug_Brief);
	for (auto j = 0; j < m_Fishing_Spots.size(); j++)
	{
		snprintf(m_debugMsg, 1024, "---> Fishing %s (%s) order %d", (m_Fishing_Spots[j].locationName).c_str(), (m_Fishing_Spots[j].pondName).c_str(), m_Fishing_Spots[j].orderNumber);
		dbgMsg(m_Debug_Type_Setting, debug_Brief);
	}
	for (auto j = 0; j < m_Harpoon_Spots.size(); j++)
	{
		snprintf(m_debugMsg, 1024, "---> Harpoon %s order %d", (m_Harpoon_Spots[j].first).c_str(), m_Harpoon_Spots[j].second);
		dbgMsg(m_Debug_Type_Setting, debug_Brief);
	}

	std::sort(m_Fishing_Spots.begin(), m_Fishing_Spots.end(), [](fishingSpot& lhs, fishingSpot& rhs) { return lhs.orderNumber < rhs.orderNumber; });

	std::sort(m_Harpoon_Spots.begin(), m_Harpoon_Spots.end(), [](pair<string, int> lhs, pair<string, int> rhs) { return lhs.second < rhs.second; });

	//Check to see if any spots actually use the bait we've selected to buy. If not, don't buy any for this run
	for (int i = 0; i <= bait_Premium_Crab_Cake; i++)
	{
		if (baitsNeeded.find((Bait_Type)i) == baitsNeeded.end())
			m_baitList[i].baitNumber = 0;
	}

	for (int i = 0; i < m_baitList.size(); i++)
	{
		snprintf(m_debugMsg, 1024, "Bait \"%s\" to buy %x number %d", m_baitList[i].baitName.c_str(), m_baitList[i].hasBait, m_baitList[i].baitNumber);
		dbgMsg(m_Debug_Type_Setting, debug_Brief);
	}

	snprintf(m_debugMsg, 1024, "<<<< Fishing profile loaded! >>>>");
	dbgMsg(m_Debug_Type_Setting, debug_Brief);
}

void CAEBot::loadPathConfig()
{
	//Read in config file
	ifstream file("config_path.ini");

	m_LocationList.clear();
	m_Grinding_StationSpot.pathStepsList.clear();
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
				parsePathList(file, m_Grinding_StationSpot);
			}
			else
			{
				m_LocationList.push_back(locInfo);
				parsePathList(file, m_LocationList.back());
			}
		}
	}

	snprintf(m_debugMsg, 1024, "<<<< Path profile loaded! >>>>");
	dbgMsg(m_Debug_Type_Setting, debug_Key);
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
			m_EmulatorIndex = stoi(value);
			parseEmulator(file);
		}

		/*****************/
		/* Mode section */
		/*****************/
		else if (key.compare("Baruoki Jump Rope") == 0)
		{
			if (stoi(value) == 1)
				m_SummaryInfo.botmode = baruokiJumpRopeMode;
		}
		else if (key.compare("Ratle Jump Rope") == 0)
		{
			if (stoi(value) == 1)
				m_SummaryInfo.botmode = ratleJumpRope;
		}
		else if (key.compare("Hit Bell") == 0)
		{
			if (stoi(value) == 1)
				m_SummaryInfo.botmode = silverHitBell30Mode;
			else if (stoi(value) == 2)
				m_SummaryInfo.botmode = silverHitBell999Mode;
		}
		else if (key.compare("Seperate Grasta") == 0)
		{
			if (stoi(value) == 1)
				m_SummaryInfo.botmode = separateGrastaMode;
		}
		else if (key.compare("Engage Fight") == 0)
		{
			if (stoi(value) == 1)
				m_SummaryInfo.botmode = engageFightMode;
		}
		else if (key.compare("Grinding") == 0)
		{
			if (stoi(value) == 1)
				m_SummaryInfo.botmode = grindingEndlessMode;
			else if (stoi(value) == 2)
				m_SummaryInfo.botmode = grindingTravelMode;
			else if (stoi(value) == 3)
				m_SummaryInfo.botmode = grindingStationMode;
			else if (stoi(value) == 4)
				m_SummaryInfo.botmode = grindingLOMSlimeMode;
		}
		else if (key.compare("Fishing") == 0)
		{
			if (stoi(value) == 1)
				m_SummaryInfo.botmode = fishingAnglerMode;
			else if (stoi(value) == 2)
				m_SummaryInfo.botmode = fishingHarpoonMode;
		}
		else if (key.compare("Capture Screen") == 0)
		{
			if (stoi(value) == 1)
				m_SummaryInfo.botmode = captureScreenMode;
		}
		else if (key.compare("Idle") == 0)
		{
			if (stoi(value) == 1)
				m_SummaryInfo.botmode = idleMode;
		}
		else if (key.compare("Dynamic Image") == 0)
		{
			parseDynamicImage(file);
		}
	}

	snprintf(m_debugMsg, 1024, "Total %d Emulators and choose [%d]::", m_EmulatorNumber, m_EmulatorIndex);
	dbgMsg(true, debug_Key);

	for (auto j = 0; j < m_EmulatorNumber; j++)
	{
		snprintf(m_debugMsg, 1024, "[%d] %s %s %s %s", j, m_EmulatorList[j].name.c_str(), m_EmulatorList[j].windowName.c_str(), m_EmulatorList[j].exeName.c_str(), m_EmulatorList[j].innerWindowName.c_str());
		dbgMsg(true, debug_Key);
	}

	snprintf(m_debugMsg, 1024, "<<<< Configuration loaded! >>>>");
	dbgMsg(true, debug_Key);
}

void CAEBot::init()
{
	loadConfig();
	loadSettingConfig();
	loadPathConfig();
	loadFishingConfig();

	snprintf(m_debugMsg, 1024, "<<<< Initiated! >>>>");
	dbgMsg(m_Debug_Type_Setting, debug_Key);
}

Status_Code CAEBot::setup()
{
	bool emuStatus;
	string emulator, windowName, innerWindowName;
	emulator = m_EmulatorList[m_EmulatorIndex].exeName;
	windowName = m_EmulatorList[m_EmulatorIndex].windowName;
	innerWindowName = m_EmulatorList[m_EmulatorIndex].innerWindowName;

	pair<string*, string*> enumInput = make_pair(&(emulator), &(windowName));
	emuStatus = EnumWindows(EnumWindowsProc, LPARAM(&enumInput));
	if (emuStatus)
		return status_WrongEmulator;
	emuStatus = EnumChildWindows(m_window, EnumChildWindowsProc, LPARAM(&(innerWindowName)));
	if (emuStatus)
		return status_WrongEmulator;

	m_ocr = OCRTesseract::create(NULL, NULL, NULL, OEM_TESSERACT_ONLY, PSM_SINGLE_LINE);

	RECT rect;
	GetWindowRect(m_window, &rect);
	m_height = rect.bottom - rect.top;
	m_width = rect.right - rect.left;

	m_xCenter = m_width / 2;
	m_yCenter = m_height / 2;
	m_heightPct = (double)(m_height / M_HEIGHT);
	m_widthPct = (double)(m_width / M_WIDTH);

	for (auto i = 0; i < m_DynamicImage.size(); i++)
	{
		string dynamicimagename = string("images\\") + m_DynamicImage[i].name;
		m_DynamicImage[i].image = imread(dynamicimagename, IMREAD_UNCHANGED);
	}

	if (m_heightPct != 1.0 || m_widthPct != 1.0)
	{
		for (auto i = 0; i < m_DynamicImage.size(); i++)
		{
			resize(m_DynamicImage[i].image, m_DynamicImage[i].image, Size(), m_widthPct, m_heightPct, m_heightPct >= 1.0 ? INTER_CUBIC : INTER_AREA);
		}
	}

	rng = std::mt19937(dev());
	boolRand = std::uniform_int_distribution<std::mt19937::result_type>(0, 1);

	m_hdc = GetWindowDC(m_window);
	m_hDest = CreateCompatibleDC(m_hdc);

	void* ptrBitmapPixels;

	BITMAPINFO bi;
	ZeroMemory(&bi, sizeof(BITMAPINFO));
	bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bi.bmiHeader.biWidth = m_width;
	bi.bmiHeader.biHeight = -m_height;
	bi.bmiHeader.biPlanes = 1;
	bi.bmiHeader.biBitCount = 32;
	m_hbDesktop = CreateDIBSection(m_hdc, &bi, DIB_RGB_COLORS, &ptrBitmapPixels, NULL, 0);

	SelectObject(m_hDest, m_hbDesktop);

	m_BitbltPic = Mat(m_height, m_width, CV_8UC4, ptrBitmapPixels, 0);

	snprintf(m_debugMsg, 1024, "<<<< Setup completed! >>>>");
	dbgMsg(m_Debug_Type_Setting, debug_Key);
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

	m_SummaryInfo.loopNumber = 0;
	m_SummaryInfo.locationNumber = 0;
	m_SummaryInfo.runFishCaught = 0;
	m_SummaryInfo.totalFishCaught = 0;
	m_SummaryInfo.runMobFought = 0;
	m_SummaryInfo.totalMobFought = 0;
	m_SummaryInfo.runHorrorFought = 0;
	m_SummaryInfo.totalHorrorFought = 0;
	m_SummaryInfo.currentLocation = "Local";
	m_SummaryInfo.startingtime = time(NULL);

	m_sharedThreadAEBot = this;

	if (m_SummaryInfo.isStopTimer)
	{
		tm utc_field = *std::gmtime(&(m_SummaryInfo.startingtime));
		utc_field.tm_isdst = -1;

		// JST is GMT +9
		int hh = 0;
		int mm = 0;
		int ss = 0;

		if (m_SummaryInfo.stopTimer.tm_sec >= utc_field.tm_sec)
		{
			ss = m_SummaryInfo.stopTimer.tm_sec - utc_field.tm_sec;
		}
		else
		{
			ss = m_SummaryInfo.stopTimer.tm_sec - utc_field.tm_sec + 60;
			mm -= 1;
		}

		if (m_SummaryInfo.stopTimer.tm_min + mm >= utc_field.tm_min)
		{
			mm = m_SummaryInfo.stopTimer.tm_min + mm - utc_field.tm_min;
		}
		else
		{
			mm = m_SummaryInfo.stopTimer.tm_min + mm - utc_field.tm_min + 60;
			hh -= 1;
		}

		hh = (m_SummaryInfo.stopTimer.tm_hour + hh - (utc_field.tm_hour + 9) + 48) % 24;

		snprintf(m_debugMsg, 1024, "Stop in %dH %dM %dS (Current GMT %d %d %d)", hh, mm, ss, utc_field.tm_hour, utc_field.tm_min, utc_field.tm_sec);
		dbgMsg(m_Debug_Type_Platform, debug_Key);

		//need to stop before the daily chrono stone at 00:00:00 UTC + 9
		std::thread timerstop(TimerforDailyChronoStone, this, (hh * 60 + mm) * 60 + ss, m_DCS_Waiting); // Register your call back function.
		timerstop.detach(); // this will be non-blocking thread.
		//timerstop.join(); // this will be blocking thread.
	}

	snprintf(m_debugMsg, 1024, "Run mode %x", m_SummaryInfo.botmode);
	dbgMsg(m_Debug_Type_Platform, debug_Key);

	switch (m_SummaryInfo.botmode) {
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
	case separateGrastaMode:
		m_resValue = stateSeparateGrasta();
		break;
	case engageFightMode:
		m_resValue = engageHorrorFightNow();
		break;
	case grindingEndlessMode:
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
	case fishingAnglerMode:
		m_resValue = stateFishing();
		break;
	case fishingHarpoonMode:
		m_resValue = stateHarpoonFishing();
		break;
	case captureScreenMode:
		m_resValue = captureScreenNow();
		break;
	case idleMode:
	default:
		m_resValue = idleNow();
		break;
	}

	return m_resValue;
}

void CAEBot::reloadConfig()
{
	// reload config files
	loadFishingConfig();
	loadPathConfig();
	loadSettingConfig();
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

	m_bot->SetMode(idleMode);
	m_bot->idleNow();
}
