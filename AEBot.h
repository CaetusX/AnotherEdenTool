#include <opencv2/opencv.hpp>
#include <opencv2/text/ocr.hpp>

using namespace cv;
using namespace std;
using namespace cv::text;

enum Bot_Mode { initialMode, grindingMode, grindingTravelMode, grindingStationMode, grindingLOMSlimeMode, fishingMode, ratleJumpRope, baruokiJumpRopeMode, silverHitBell30Mode, silverHitBell999Mode, seperateGrastaMode, engageFightMode, captureScreenMode};
enum Status_Code { status_NoError, status_Timeout, status_NotFight, status_FightFail, status_Stop, status_WrongEmulator};
enum Grasta_Type { grasta_Attack, grasta_Life, grasta_Support, grasta_Special};
enum Bait_Type { bait_Fishing_Dango, bait_Worm, bait_Unexpected_Worm, bait_Shopaholic_Clam, bait_Spree_Snail, bait_Dressed_Crab, bait_Tear_Crab, bait_Foamy_Worm, bait_Bubbly_Worm, bait_Snitch_Sardines, bait_Blabber_Sardines, bait_Crab_Cake, bait_Premium_Crab_Cake, bait_Error_Max };
enum Door_Type { door_Default, door_Small, door_LOM };
enum Stair_Type { stair_Default, stair_Down, stair_LOM };
enum Excl_Type { excl_Default, excl_Fishing, excl_Small, excl_Large, excl_Grasta, excl_Chamber, excl_ChamberPlasma };
enum Direction_Info { LEFT, RIGHT, DOWN, UP, LR };

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

private:

	vector<emulatorInfo> m_emulatorList;
	int m_emulatorIndex;
	int m_emulatorNumber;

	Bot_Mode m_botMode;

	char m_debugMsg[1024];
	string m_outputMsg;

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

	int m_height, m_width;
	double m_heightPct, m_widthPct;
	WORD m_xCenter, m_yCenter;

	int m_msdThreshold;
	int m_loadTime;
	bool m_IsDebug_Key;
	bool m_IsDebug_Path;
	bool m_IsDebug_Fighting;
	bool m_IsDebug_Grinding;
	bool m_IsDebug_Fishing;
	bool m_IsDebug_Grasta;
	bool m_IsDebug_LOM;
	bool m_IsDebug_Setting;
	bool m_IsDebug_Platform;

	bool m_IsPrint;

	Ptr<OCRTesseract> ocr;
	HDC hdc;
	HDC hDest;
	HBITMAP hbDesktop;
	Mat bitbltPic, fishIcon, exclDefaultIcon, exclFishingIcon, exclSmallIcon, exclLargeIcon, exclGrastaIcon, exclChamberIcon, exclChamberPlasmaIcon, swampFishIcon, doorDefaultIcon, doorSmallIcon, doorLOMIcon, stairDefaultIcon, stairDownIcon, stairLOMIcon;
	Mat afBarEmptyPic, afBarFullPic, hitBellPic, jmpRopePic1, jmpRopePic2, jmpRopePic3, jmpRopePic4, catHokoraPic;

	string ocrNumbers;
	string ocrLetters;

	vector<pair<int, int>> m_Locs_baits;
	vector<pair<int, int>> m_Locs_Acteul;
	vector<pair<int, int>> m_Locs_Baruoki;
	vector<pair<int, int>> m_Locs_DragonPalace;
	vector<pair<int, int>> m_Locs_Elzion;
	vector<pair<int, int>> m_Locs_DimensionRift;
	vector<pair<int, int>> m_Locs_KiraBeach;
	vector<pair<int, int>> m_Locs_RucyanaSands;
	vector<pair<int, int>> m_Locs_Vasu;
	vector<pair<int, int>> m_Locs_Igoma;
	vector<pair<int, int>> m_Locs_Moonlight;
	vector<pair<int, int>> m_Locs_AncientBattlefield;
	vector<pair<int, int>> m_Locs_ZolPlains;
	vector<pair<int, int>> m_Locs_Default;

	vector<buttonInfo> m_Button_MapButtons;
	vector<buttonInfo> m_Button_Characters;
	vector<buttonInfo> m_Button_Skills;

	int m_Skill_Normal;
	int m_Skill_Exchange;
	int m_Skill_Exchange_A;
	int m_Skill_Exchange_B;
	int m_Skill_AF;
	int m_Skill_for_AF;
	int m_CharacterFrontline;

	int m_Fight_AFInterval;
	int m_Fight_AFFullThreshold;
	int m_Fight_ActionInterval;

	bool m_Fight_GrindingSkipSpaceTimeRift;
	bool m_Fight_GrindingSkipRunning;
	Direction_Info m_Fight_GrindingDirection;

	int m_Fight_GrindingStep;
	int m_Fight_GrindingCount;
	int m_Fight_Timeout;
	int m_Fight_HorrorCount;

	string m_Fight_LOMHeal;
	string m_Fight_LOMSlimeA;
	string m_Fight_LOMSlimeB;
	string m_Fight_LOMSlimeRun;
	string m_Fight_LOMReset;
	int m_Fight_LOMTurn;

	int m_currentGrindingCounter;

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

	vector<Mat>* m_currentMonsterVec = NULL;
	vector<Mat> m_MonsterVec_Baruoki;
	vector<Mat> m_MonsterVec_Acteul;
	vector<Mat> m_MonsterVec_Vasu;
	vector<Mat> m_MonsterVec_Serena;
	vector<Mat> m_MonsterVec_Rucyana;
	vector<Mat> m_MonsterVec_Elzion;
	vector<Mat> m_MonsterVec_LastIsland;
	vector<Mat> m_MonsterVec_DimensionRift;
	vector<Mat> m_MonsterVec_LOMPSlime;

	vector<string> m_baitNames;

	vector<imageInfo> m_DynamicImage;

	vector<locationInfo> m_LocationList;
	vector<fishingSpot> m_fishingSpots;
	vector<pair<string, int>> m_grindingSpots;
	locationInfo m_stationGrindingSpot;

	vector<pair<bool, int>> m_baitList; //Index is the type, the boolean is whether or not you have greater than 0 currently held, int is how many to buy for this run
	vector<Bait_Type>* m_currentBaitsToUse;

	pair<int, int> m_currentFishIconLoc;
	string m_currentLocation;
	bool m_hasHorror;

	void dbgMsg(int debugLevel);
	void outputMsg();
	void bitBltWholeScreen();
	void copyPartialPic(Mat& partialPic, int cols, int rows, int x, int y);
	string runOCR(Mat& pic);
	string getText(Mat& pic);
	int getNumber(Mat& pic);
	bool hokoraReady();
	string ocrPictureText(int columns, int rows, int x, int y);
	int ocrPictureNumber(int columns, int rows, int x, int y);

	pair<int, int> findIcon(Mat& tmp);
	pair<int, int> findExclamationIcon(Excl_Type whichExcl = excl_Default);
	pair<int, int> findDoorIcon(Door_Type whichDoor = door_Default);
	pair<int, int> findStairIcon(Stair_Type whichStair);
	void findAndClickSwampFishIcon();
	void findAndClickFishIcon();

	Bait_Type hashBait(string str);

	void leftClick(int x, int y, int sTime = 2000, bool changeLoc = true);
	void leftClick(pair<int, int>& coord, int sTime = 3000);
	void drag(Direction_Info botDirection, int slideDistance, int xStart, int yStart, int sleepTime = 1000, int scrollRatio = 1);
	void dragMap(Direction_Info botDirection, int slideDistance);
	void Walk(Direction_Info botDirection, int time, int sleepTime = 3000);
	void clickAttack(int time = 3000);
	bool inBattle();
	bool endBattle();
	Status_Code fightUntilEnd();
	Status_Code WalkUntilBattle(Direction_Info botdirection = LR);

	Status_Code engageMobFightNow(int horrorThreshold = 7000);
	Status_Code engageHorrorFightNow(bool restoreHPMP = true);

	void fish(vector<pair<int, int>>& sections, int m_msdThreshold = 10000, int horrorThreshold = 7000);
	void changeBait(Bait_Type type);
	Status_Code goToTargetLocation(vector<pathInfo> pathInfoList);
	void goToFishingLocation();
	void goToSpacetimeRift(bool heal = true);
	void goToFishVendor();
	void fishFunction();
	void fishIconClickFunction();

	Status_Code stateFishing();
	Status_Code stateSilverHitBell(Bot_Mode silverHitBellstate);
	Status_Code stateJumpRopeRatle();
	Status_Code stateJumpRopeBaruoki();
	Status_Code stateSeparateGrasta();
	Status_Code grindingRun(bool endlessGrinding = false);
	Status_Code stateTravelGrinding();
	Status_Code stateStationGrinding();
	Status_Code stateLOMSlimeGrinding();
	Status_Code stateEndlessGrinding();

	keyvalueInfo parseKeyValue(string str, string parser, bool order = true);
	pair<int, int> parseXYinfo(string str);
	pathInfo parsePathStep(string str);
	void parseDynamicImage(ifstream& file);
	vector<buttonInfo> parseButtons(ifstream& file);
	vector<vector<int>> parseSkillsSet(ifstream& file);
	vector<pair<string, int>> parseGrindingSpotsList(ifstream& file);
	vector<string> parseGrastaNames(ifstream& file);
	void parseBaitForArea(ifstream& file, bool maxBait, fishingSpot& area, set<int>& baitsNeeded, vector<pair<bool, int>>& baitsToPurchase);
	void parsePathList(ifstream& file, locationInfo& location);
	void parseEmulator(ifstream& file);

	void loadSettingConfig();
	void loadPathConfig();
	void loadFishingConfig();
	void loadConfig();
};