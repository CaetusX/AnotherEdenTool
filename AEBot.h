#include <opencv2/opencv.hpp>
#include <opencv2/text/ocr.hpp>

using namespace cv;
using namespace std;
using namespace cv::text;

enum Bot_Mode { grindingMode, grindingTravelMode, grindingLOMSlimeMode, fishingMode, ratleJumpRope, baruokiJumpRopeMode, silverHitBell30Mode, silverHitBell999Mode, seperateGrastaMode, engageFightMode, captureScreenMode};
enum Status_Code { status_NoError, status_Timeout, status_NotFight, status_FightFail, status_Stop};
enum Grasta_Type { grasta_Attack, grasta_Life, grasta_Support, grasta_Special};
enum Bait_Type { bait_Fishing_Dango, bait_Worm, bait_Unexpected_Worm, bait_Shopaholic_Clam, bait_Spree_Snail, bait_Dressed_Crab, bait_Tear_Crab, bait_Foamy_Worm, bait_Bubbly_Worm, bait_Snitch_Sardines, bait_Blabber_Sardines, bait_Crab_Cake, bait_Premium_Crab_Cake, bait_Error_Max };
enum Door_Type { door_Default, door_Kunlun, door_LOM };
enum Stair_Type { stair_Default, stair_Down, stair_LOM };
enum Excl_Type { excl_Default, excl_Rift, excl_Sepcter, excl_Grasta, excl_Chamber, excl_ChamberPlasma, excl_KMS };
enum Direction_Info { LEFT, RIGHT, DOWN, UP, LR };

struct buttonInfo
{
	string buttonName;
	pair<int, int> xyPosition;
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
};

class CAEBot
{
	// Construction
public:
	CAEBot(HWND pParent = NULL);   // standard constructor
	~CAEBot();

	void SetStatus(Status_Code statusCode);
	Status_Code GetStatus();
	string GetdbgMsg();
	Bot_Mode GetMode();
	void SetMode(Bot_Mode botMode);

	void captureScreenNow(const char* nameSuffix = NULL);
	void setup();
	void run();

private:

	Bot_Mode m_botMode;

	char m_debugMsg[1024];

	Status_Code m_resValue;

	int m_lastGrasta;
	int m_SectionHeight;
	int m_grastaType;
	vector<string> m_grastaNames;

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
	bool m_IsDebug_Platform;

	bool m_IsPrint;

	Ptr<OCRTesseract> ocr;
	HDC hdc;
	HDC hDest;
	HBITMAP hbDesktop;
	Mat bitbltPic, fishIcon, exclDefaultIcon, exclRiftIcon, exclSepcterIcon, exclGrastaIcon, exclChamberIcon, exclChamberPlasmaIcon, exclKMSIcon, swampFishIcon, doorDefaultIcon, doorKunlunIcon, doorLOMIcon, stairDefaultIcon, stairDownIcon, stairLOMIcon;
	Mat afBarEmptyPic, afBarFullPic, hitBellPic, jmpRopePic1, jmpRopePic2, jmpRopePic3, jmpRopePic4;

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

	int m_Fight_HorrorCount;
	int m_Fight_GrindingStep;
	int m_Fight_AFInterval;
	int m_Fight_AFFullThreshold;
	int m_Fight_GrindingCount;
	int m_Fight_Timeout;
	int m_Fight_ActionInterval;
	bool m_Fight_EndlessGrinding;

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

	vector<fishingSpot> m_fishingSpots;
	vector<locationInfo> m_LocationList;
	vector<pair<string, int>> m_grindingSpots;

	vector<pair<bool, int>> m_baitList; //Index is the type, the boolean is whether or not you have greater than 0 currently held, int is how many to buy for this run
	vector<Bait_Type>* m_currentBaitsToUse;

	pair<int, int> m_currentFishIconLoc;
	string m_currentLocation;
	bool m_hasHorror;

	Direction_Info m_Fight_GrindingDirection;

	void dbgMsg(int debugLevel);
	void bitBltWholeScreen();
	void copyPartialPic(Mat& partialPic, int cols, int rows, int x, int y);
	string runOCR(Mat& pic);
	string getText(Mat& pic);
	int getNumber(Mat& pic);
	string ocrPictureText(int columns, int rows, int x, int y);
	int ocrPictureNumber(int columns, int rows, int x, int y);

	pair<int, int> findIcon(Mat& tmp);
	pair<int, int> findExclamationIcon(Excl_Type whichExcl = excl_Default);
	pair<int, int> findDoorIcon(Door_Type whichDoor);
	pair<int, int> findStairIcon(Stair_Type whichStair);
	void findAndClickSwampFishIcon();
	void findAndClickFishIcon();

	Bait_Type hashBait(string str);

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

	void leftClick(int x, int y, int sTime = 2000, bool changeLoc = true);
	void leftClick(pair<int, int>& coord, int sTime = 2000);
	void drag(Direction_Info botDirection, int slideDistance, int xStart, int yStart);
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
	void stateFishing();

	void stateSilverHitBell(Bot_Mode silverHitBellstate);
	void stateJumpRopeRatle();
	void stateJumpRopeBaruoki();
	void stateSeparateGrasta();

	Status_Code grindingRun(bool endlessGrinding = false);
	void stateGrindingTravel();
	void stateGrindingLOMSlime();
	void stateGrinding();

	void loadSettingConfig();
	void loadPathConfig();
	void loadFishingConfig();

};