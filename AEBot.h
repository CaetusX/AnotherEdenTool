#include <opencv2/opencv.hpp>
#include <opencv2/text/ocr.hpp>

using namespace cv;
using namespace std;
using namespace cv::text;

enum Bot_Mode { initialMode, 
	grindingEndlessMode, grindingTravelMode, grindingStationMode, grindingLOMSlimeMode, 
	fishingAnglerMode, fishingHarpoonMode, 
	ratleJumpRope, baruokiJumpRopeMode, 
	silverHitBell30Mode, silverHitBell999Mode, 
	seperateGrastaMode, 
	captureScreenMode, 
	engageFightMode
};

enum Status_Code { 
	status_NoError				= 0x00000000,
	status_MajorError			= 0x10000000,
	status_MinorError			= 0x01000000,

	status_WrongEmulator		= 0x10000001,
	status_Stop					= 0x10000002,

	status_Timeout				= 0x01000001,
	status_BreakRun				= 0x01000002,

	status_NotFight				= 0x00100001, 
	status_FightFail			= 0x00100002,

	status_NoFishing			= 0x00101001,
	status_FishingNoBait		= 0x00101002,
	status_FishingHorrorMax		= 0x00101003,
	status_NoHarpoonCatch		= 0x00101101,
	status_HarpoonCatchFail		= 0x00101102, 
	status_HarpoonTrapWaiting	= 0x00101103,
	status_HarpoonTrapFail		= 0x00101104
};
enum Grasta_Type { grasta_Attack, grasta_Life, grasta_Support, grasta_Special };

enum OCR_Type { ocr_Alphabetic, ocr_Numeric, ocr_AlphaNumeric };

enum Debug_Level { debug_None = 0, debug_Key, debug_Summary, debug_Brief, debug_Detail };

enum Bait_Type { 
	bait_Fishing_Dango = 0, bait_Worm, bait_Unexpected_Worm, bait_Shopaholic_Clam, bait_Spree_Snail, bait_Dressed_Crab, bait_Tear_Crab, 
	bait_Foamy_Worm, bait_Bubbly_Worm, bait_Snitch_Sardines, bait_Blabber_Sardines, bait_Crab_Cake, bait_Premium_Crab_Cake, 
	harpoonBait_Seaworm, harpoonBait_Shrimp, harpoonBait_WreckerCrab, harpoonBait_BrightSeashell,
	bait_Error_Max
};

enum Direction_Info { LEFT, RIGHT, DOWN, UP, LR, NOWHERE };

enum {
	Grasta_Slot_0, Grasta_Slot_1, Grasta_Slot_2, Grasta_Slot_3, Grasta_Slot_4, 
	Grasta_Action_Bind, Grasta_Action_Separate, Grasta_Action_Rescind,
	Grasta_Type_Attack, Grasta_Type_Life, Grasta_Type_Support, Grasta_Type_Special, Grasta_Type_All,
	Grasta_Filter_None, Grasta_Filter_Crystal
};

struct buttonInfo
{
	string buttonName;
	pair<int, int> xyPosition;
};

struct emulatorInfo
{
	string name;
	string windowName;
	string exeName;
	string innerWindowName;
};

struct imageInfo
{
	string id;
	string name;
	int width;
	int height;
	int coordx;
	int coordy;
	Mat image;
};

struct baitInfo
{
	string baitName;
	double baitPrice;
	int baitNumber;
	bool hasBait;
};

struct fishingSpot
{
	vector<Bait_Type> baitsToUse;
	int orderNumber;
	string locationName;
};

struct pathInfo
{
	string type;
	string value1;
	string value2;
};

struct keyvalueInfo
{
	string key;
	string value;
};

struct locationInfo
{
	string locationName;
	vector<pathInfo> pathStepsList;
	bool skipRunning;
};

class CAEBot
{
	// Construction
public:
	CAEBot(HWND pParent = NULL);   // standard constructor
	~CAEBot();

	void SetStatus(Status_Code statusCode);
	Status_Code GetStatus();
	Debug_Level GetDebugLevel();
	void SetDebugLevel(Debug_Level debuglevel);
	string GetOutputMsg();
	Bot_Mode GetMode();

	void SetMode(Bot_Mode botMode);
	int GetEmulatorNumber();
	void SetEmulator(int emulatorIndex);
	string GetEmulatorName(int emulatorIndex);

	Status_Code captureScreenNow(const char* nameSuffix = NULL);
	void init();
	Status_Code setup();
	Status_Code run();
	void reloadConfig();

private:

	vector<emulatorInfo> m_emulatorList;
	int m_emulatorIndex;
	int m_emulatorNumber;

	Bot_Mode m_botMode;

	char m_debugMsg[1024];
	string m_outputMsg;
	char m_timeString[80];

	Status_Code m_resValue;

	int m_Grasta_Target;
	int m_Grasta_SectionHeight;
	int m_Grasta_ScrollHeight;
	int m_Grasta_ScrollRatio;
	int m_Grasta_NumberCap;
	string m_Grasta_TypeSelection;
	string m_Grasta_FilterSelection;
	vector<string> m_Grasta_Names;
	vector<buttonInfo> m_Grasta_Button;

	double M_WIDTH;
	double M_HEIGHT;
	double M_WIDTH_1280;
	double M_HEIGHT_720;
	double M_ABOVE_MENU;

	int m_height, m_width;
	double m_heightPct, m_widthPct;
	WORD m_xCenter, m_yCenter;

	int m_Image_Threshold;
	int m_loadTime;

	Debug_Level m_Debug_Level;

	bool m_IsDebug_Setting;
	bool m_IsDebug_Platform;
	bool m_IsDebug_Path;
	bool m_IsDebug_Fighting;
	bool m_IsDebug_Grinding;
	bool m_IsDebug_Fishing;
	bool m_IsDebug_Grasta;
	bool m_IsDebug_LOM;

	bool m_IsPrint;

	Ptr<OCRTesseract> m_ocr;
	HDC m_hdc;
	HDC m_hDest;
	HBITMAP m_hbDesktop;

	Mat m_BitbltPic;

	vector<Mat> m_Icons;
	vector<Mat> m_Pictures;

	vector<pair<int, int>> m_Fishing_Locs_Acteul;
	vector<pair<int, int>> m_Fishing_Locs_Baruoki;
	vector<pair<int, int>> m_Fishing_Locs_DragonPalace;
	vector<pair<int, int>> m_Fishing_Locs_Elzion;
	vector<pair<int, int>> m_Fishing_Locs_DimensionRift;
	vector<pair<int, int>> m_Fishing_Locs_KiraBeach;
	vector<pair<int, int>> m_Fishing_Locs_RucyanaSands;
	vector<pair<int, int>> m_Fishing_Locs_Vasu;
	vector<pair<int, int>> m_Fishing_Locs_Igoma;
	vector<pair<int, int>> m_Fishing_Locs_Moonlight;
	vector<pair<int, int>> m_Fishing_Locs_AncientBattlefield;
	vector<pair<int, int>> m_Fishing_Locs_ZolPlains;
	vector<pair<int, int>> m_Fishing_Locs_Default;

	bool m_Fishing_HasHorror;
	bool m_Fishing_HasMob;

	vector<buttonInfo> m_Button_MapButtons;
	vector<buttonInfo> m_Button_Characters;
	vector<buttonInfo> m_Button_Skills;
	vector<buttonInfo> m_Button_Baits;

	int m_Skill_Normal;
	int m_Skill_Exchange;
	int m_Skill_Exchange_A;
	int m_Skill_Exchange_B;
	int m_Skill_AF;
	int m_Skill_for_AF;
	int m_CharacterFrontline;

	int m_Action_Interval;
	int m_Fast_Action_Interval;
	int m_Smart_DownUp_Interval;
	int m_Smart_DownUp_Threshold;

	double m_Walk_Distance_Ratio;

	int m_Fight_AFInterval;
	int m_Fight_AFFullThreshold;

	bool m_Fight_GrindingSkipSpaceTimeRift;
	bool m_Fight_GrindingSkipRunning;
	Direction_Info m_Fight_GrindingDirection;

	int m_Fight_GrindingStep;
	int m_Fight_GrindingCount;
	int m_Time_Out;
	int m_Fight_HorrorCount;

	string m_Fight_LOMHeal;
	string m_Fight_LOMSlimeA;
	string m_Fight_LOMSlimeB;
	string m_Fight_LOMSlimeRun;
	string m_Fight_LOMReset;
	int m_Fight_LOMTurn;

	int m_currentGrindingCounter;

	int m_Harpoon_Loop;
	int m_Harpoon_Xinc;
	int m_Harpoon_Yinc;
	int m_Harpoon_Xmin;
	int m_Harpoon_Xmax;
	int m_Harpoon_Ymin;
	int m_Harpoon_Ymax;
	bool m_Harpoon_SkipVendor;
	int m_Harpoon_Interval;

	vector<vector<int>> m_skillsHorrorSet;
	vector<vector<int>> m_skillsMobSet;

	TCHAR m_CurrentPath[MAX_PATH];

	pair<int, int> m_Button_Yes;
	pair<int, int> m_Button_Leave;
	pair<int, int> m_Button_X;
	pair<int, int> m_Button_Menu;
	pair<int, int> m_Button_Food;
	pair<int, int> m_Button_Attack;
	pair<int, int> m_Button_AF;
	pair<int, int> m_Button_Map;
	pair<int, int> m_Button_Antiquity;
	pair<int, int> m_Button_Present;
	pair<int, int> m_Button_Future;
	pair<int, int> m_Button_EndOfTimeLoc;
	pair<int, int> m_Button_SpacetimeRift;
	pair<int, int> m_Button_Tree;
	pair<int, int> m_Button_PassThrough;

	vector<baitInfo> m_baitList; //Index is the type, the boolean is whether or not you have greater than 0 currently held, int is how many to buy for this run
	vector<Bait_Type>* m_currentBaitsToUse;

	vector<imageInfo> m_DynamicImage;

	vector<locationInfo> m_LocationList;
	vector<pair<string, int>> m_grindingSpots;
	vector<fishingSpot> m_fishingSpots;
	vector<pair<string, int>> m_harpoonSpots; //string is the harpoon location name, int is the order

	locationInfo m_stationGrindingSpot;

	pair<int, int> m_currentFishIconLoc;
	string m_currentLocation;

	void dbgMsg(int debugGroup, Debug_Level debugLevel);
	char* timeString();
	bool checkStatus(Status_Code statuscode);

	void bitBltWholeScreen();
	void copyPartialPic(Mat& partialPic, int cols, int rows, int x, int y);
	string runOCR(Mat& pic);
	string getText(Mat& pic);
	string getTextNumber(Mat& pic);
	int getNumber(Mat& pic);
	string ocrPicture(OCR_Type ocrtype, int columns, int rows, int x, int y);

	pair<int, int> findIcon(Mat& tmp);
	pair<int, int> findIconInRegion(Mat& tmp, int cols, int rows, int x, int y);
	imageInfo retrieveImage(string imageID);
	bool compareImage(string imageID);
	pair<bool, pair<int, int>> findClick(string imageID, int cols, int rows, int x, int y);

	void leftClick(int x, int y, bool changeLoc = true);
	void leftClick(int x, int y, int sTime, bool changeLoc = true);
	void leftClick(pair<int, int>& coord);
	void leftClick(pair<int, int>& coord, int sTime);
	void drag(Direction_Info botDirection, int slideDistance, int xStart, int yStart, int sleepTime = 1000, int scrollRatio = 1);
	void dragMap(Direction_Info botDirection, int slideDistance);
	void walk(Direction_Info botDirection, int time);
	void walk(Direction_Info botDirection, int time, int sleepTime);
	bool inBattle();
	bool endBattle();

	Status_Code smartWorldMap(pair<int, int>& coord);
	Status_Code smartMiniMap(pair<int, int>& coord);
	Status_Code smartLoadMap(pair<int, int>& coord);
	Status_Code smartDownUp(Direction_Info updownDirection, Direction_Info leftrightDirection);
	Status_Code sleepLoadTime();
	Status_Code fightUntilEnd();
	Status_Code walkUntilBattle(Direction_Info botdirection);

	Status_Code engageMobFightNow();
	Status_Code engageHorrorFightNow(bool restoreHPMP = true);

	Status_Code fish(vector<pair<int, int>>& sections);
	Status_Code changeBait(Bait_Type type);
	Status_Code goToTargetLocation(vector<pathInfo> pathInfoList);
	void goToFishingLocation();
	void goToSpacetimeRift(bool heal = true);
	void goToFishVendor();
	void goToHarpoonVendor();
	void buyBaitsFromVendor();
	Status_Code fishFunction();
	void fishIconClickFunction();
	Status_Code harpoonFunction();
	Status_Code harpoonHorror();
	Status_Code harpoonMassShooting();
	Status_Code harpoonTrapFunction(string trapRef = "");
	Status_Code harpoonSetTrap(string trapRef = "");

	Status_Code stateFishing();
	Status_Code stateHarpoonFishing();
	Status_Code stateSilverHitBell(Bot_Mode silverHitBellstate);
	Status_Code stateJumpRopeRatle();
	Status_Code stateJumpRopeBaruoki();
	Status_Code stateSeparateGrasta();
	Status_Code grindingRun(bool endlessGrinding, int forcetimeout = 0);
	Status_Code stateTravelGrinding();
	Status_Code stateStationGrinding();
	Status_Code stateLOMSlimeGrinding();
	Status_Code stateEndlessGrinding();

	Status_Code captureImageNow(Mat imagePicCrop, const char* nameSuffix);

	keyvalueInfo parseKeyValue(string str, string parser, bool reverseorder = false);
	pair<int, int> parseXYinfo(string str);
	pathInfo parsePathStep(string str);
	void parseDynamicImage(ifstream& file);
	vector<buttonInfo> parseButtons(ifstream& file);
	vector<vector<int>> parseSkillsSet(ifstream& file);
	vector<pair<string, int>> parseGrindingSpotsList(ifstream& file);
	vector<string> parseGrastaNames(ifstream& file);
	void parseBaitForArea(ifstream& file, bool constructBait, fishingSpot& area, set<Bait_Type>& baitsNeeded, vector<baitInfo>& baitsToPurchase);
	void parsePathList(ifstream& file, locationInfo& location);
	void parseEmulator(ifstream& file);

	void loadSettingConfig();
	void loadPathConfig();
	void loadFishingConfig();
	void loadConfig();
};