// AnotherEdenTool.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "AnotherEdenTool.h"
#include "AEBot.h"

#define MAX_LOADSTRING 100
#define IDT_TIMER1 1001

// Global Variables:
HINSTANCE m_hInst;                          // current instance
TCHAR szTitle[MAX_LOADSTRING];              // The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];        // the main window class name
TCHAR strFormat[1024];                      // Must ensure size!

HWND m_hWnd;
CAEBot* m_AEBot=NULL;

//HWND m_hButtonExit;
//HWND m_hButtonStart;
//HWND m_hButtonStop;
//HWND m_hButtonCaptureScreen;

HWND m_hInfoBox;

#define GRINDING_ENDLESS 0
#define GRINDING_TRAVEL 1
#define GRINDING_STATION 2
#define GRINDING_LOMSLIME 3
#define GRINDING_TYPE_NUMBER 4

#define FISHING_ANGLER 0
#define FISHING_HARPOON 1
#define FISHING_TYPE_NUMBER 2

#define DEBUG_SUMMARY 0
#define DEBUG_ALERT 1
#define DEBUG_KEY 2
#define DEBUG_BRIEF 3
#define DEBUG_DETAIL 4
#define DEBUG_GRANULAR 5
#define DEBUG_LEVEL_NUMBER 6

string m_grindingType[GRINDING_TYPE_NUMBER] =
{
    "Endless", "Travel", "Station", "LOM Slime"
};

string m_Fishing_Type[FISHING_TYPE_NUMBER] =
{
    "Angler", "Harpoon"
};

string m_debugLevel[DEBUG_LEVEL_NUMBER] =
{
    "Summary", "Alert", "Key Message", "Brief Message", "Detail", "Granular"
};

void AEBotTimerThread(int n)
{
    Status_Code res;
    res = m_AEBot->run();
    return;
}

void Interface_Init(HWND hDlg, bool botEnabled)
{
    Bot_Mode botMode;
    Debug_Level debuglevel;

    SendDlgItemMessage(hDlg, IDC_COMBO_GrindingType, CB_SETCURSEL, GRINDING_ENDLESS, 0);
    SendDlgItemMessage(hDlg, IDC_COMBO_FishingType, CB_SETCURSEL, FISHING_ANGLER, 0);

    botMode = m_AEBot->GetMode();
    debuglevel = m_AEBot->GetDebugLevel();
    SendDlgItemMessage(hDlg, IDC_COMBO_DebugLevel, CB_SETCURSEL, debuglevel, 0);

    switch (botMode)
    {
    case grindingEndlessMode:
        CheckRadioButton(hDlg, IDC_RADIO_Grinding, IDC_RADIO_SeparateGrasta, IDC_RADIO_Grinding);
        SendDlgItemMessage(hDlg, IDC_COMBO_GrindingType, CB_SETCURSEL, GRINDING_ENDLESS, 0);
        break;
    case grindingTravelMode:
        CheckRadioButton(hDlg, IDC_RADIO_Grinding, IDC_RADIO_SeparateGrasta, IDC_RADIO_Grinding);
        SendDlgItemMessage(hDlg, IDC_COMBO_GrindingType, CB_SETCURSEL, GRINDING_TRAVEL, 0);
        break;
	case grindingStationMode:
		CheckRadioButton(hDlg, IDC_RADIO_Grinding, IDC_RADIO_SeparateGrasta, IDC_RADIO_Grinding);
		SendDlgItemMessage(hDlg, IDC_COMBO_GrindingType, CB_SETCURSEL, GRINDING_STATION, 0);
		break;
	case grindingLOMSlimeMode:
        CheckRadioButton(hDlg, IDC_RADIO_Grinding, IDC_RADIO_SeparateGrasta, IDC_RADIO_Grinding);
        SendDlgItemMessage(hDlg, IDC_COMBO_GrindingType, CB_SETCURSEL, GRINDING_LOMSLIME, 0);
        break;
    case fishingAnglerMode:
        CheckRadioButton(hDlg, IDC_RADIO_Grinding, IDC_RADIO_SeparateGrasta, IDC_RADIO_Fishing);
        SendDlgItemMessage(hDlg, IDC_COMBO_FishingType, CB_SETCURSEL, FISHING_ANGLER, 0);
        break;
    case fishingHarpoonMode:
        CheckRadioButton(hDlg, IDC_RADIO_Grinding, IDC_RADIO_SeparateGrasta, IDC_RADIO_Fishing);
        SendDlgItemMessage(hDlg, IDC_COMBO_FishingType, CB_SETCURSEL, FISHING_HARPOON, 0);
        break;
    case ratleJumpRope:
        CheckRadioButton(hDlg, IDC_RADIO_Grinding, IDC_RADIO_SeparateGrasta, IDC_RADIO_JumpRopes);
        CheckDlgButton(hDlg, IDC_CHECK_FigureEight, 0);
        break;
    case baruokiJumpRopeMode:
        CheckRadioButton(hDlg, IDC_RADIO_Grinding, IDC_RADIO_SeparateGrasta, IDC_RADIO_JumpRopes);
        CheckDlgButton(hDlg, IDC_CHECK_FigureEight, 1);
        break;
    case silverHitBell30Mode:
        CheckRadioButton(hDlg, IDC_RADIO_Grinding, IDC_RADIO_SeparateGrasta, IDC_RADIO_BellStrike);
        CheckDlgButton(hDlg, IDC_CHECK_Target999, 0);
        break;
    case silverHitBell999Mode:
        CheckRadioButton(hDlg, IDC_RADIO_Grinding, IDC_RADIO_SeparateGrasta, IDC_RADIO_BellStrike);
        CheckDlgButton(hDlg, IDC_CHECK_Target999, 1);
        break;
    case separateGrastaMode:
        CheckRadioButton(hDlg, IDC_RADIO_Grinding, IDC_RADIO_SeparateGrasta, IDC_RADIO_SeparateGrasta);
        break;
    /*
    case engageFightMode:
        CheckRadioButton(hDlg, IDC_RADIO_Grinding, IDC_RADIO_EngageHorror, IDC_RADIO_EngageHorror);
        break;
    */
    case captureScreenMode:
    default:
        break;
    }

    for (auto k = IDC_CONTROLUNIT_START; k <= IDC_CONTROLUNIT_END; k++)
    {
        EnableWindow(GetDlgItem(hDlg, k), botEnabled);
    }
    EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_Stop), false);
}

void Interface_Grinding(HWND hDlg)
{
    UINT nIndex = (UINT)SendDlgItemMessage(hDlg, IDC_COMBO_GrindingType, CB_GETCURSEL, 0, 0);
    if (m_AEBot)
    {
        switch (nIndex)
        {
        case GRINDING_TRAVEL:
            m_AEBot->SetMode(grindingTravelMode);
            wsprintf(strFormat, _T("Travel world to grind.\r\nYou can start anywhere. Please config where to grind and what skill to use in config_setting file\r\n"));
            SetWindowText(GetDlgItem(hDlg, IDC_InfoText), strFormat);
            break;
		case GRINDING_STATION:
			m_AEBot->SetMode(grindingStationMode);
			wsprintf(strFormat, _T("Stay at the place to collect sparkles.\r\nYou can start anywhere. Please config where to grind in config_setting file\r\n"));
			SetWindowText(GetDlgItem(hDlg, IDC_InfoText), strFormat);
			break;
		case GRINDING_LOMSLIME:
            m_AEBot->SetMode(grindingLOMSlimeMode);
            wsprintf(strFormat, _T("Lord of Mana plantium slime grinding.\r\nYou can start anywhere\r\n"));
            SetWindowText(GetDlgItem(hDlg, IDC_InfoText), strFormat);
            break;
        default:
            SendDlgItemMessage(hDlg, IDC_COMBO_GrindingType, CB_SETCURSEL, GRINDING_ENDLESS, 0);
        case GRINDING_ENDLESS:
            m_AEBot->SetMode(grindingEndlessMode);
            wsprintf(strFormat, _T("Grinding here.\r\nYou can start here\r\n"));
            SetWindowText(GetDlgItem(hDlg, IDC_InfoText), strFormat);
            break;
        }
        CheckRadioButton(hDlg, IDC_RADIO_Grinding, IDC_RADIO_SeparateGrasta, IDC_RADIO_Grinding);
    }
}

void Interface_fishing(HWND hDlg)
{
    UINT nIndex = (UINT)SendDlgItemMessage(hDlg, IDC_COMBO_FishingType, CB_GETCURSEL, 0, 0);
    if (m_AEBot)
    {
        switch (nIndex)
        {
        case FISHING_HARPOON:
            m_AEBot->SetMode(fishingHarpoonMode);
            wsprintf(strFormat, _T("Harpoon Fishing mode.\r\nYou can start anywhere\nPlease config where to fish and what bait to use in config_fishing file\r\n"));
            SetWindowText(GetDlgItem(hDlg, IDC_InfoText), strFormat);
            break;
        default:
            SendDlgItemMessage(hDlg, IDC_COMBO_FishingType, CB_SETCURSEL, FISHING_ANGLER, 0);
        case FISHING_ANGLER:
            m_AEBot->SetMode(fishingAnglerMode);
            wsprintf(strFormat, _T("Fishing mode.\r\nYou can start anywhere\nPlease config where to fish and what bait to use in config_fishing file\r\n"));
            SetWindowText(GetDlgItem(hDlg, IDC_InfoText), strFormat);
            break;
        }
        CheckRadioButton(hDlg, IDC_RADIO_Grinding, IDC_RADIO_SeparateGrasta, IDC_RADIO_Fishing);
    }
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.

    // Initialize global strings
    LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadString(hInstance, IDC_ANOTHEREDENTOOL, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_ANOTHEREDENTOOL));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEX wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ANOTHEREDENTOOL));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH) (COLOR_WINDOW + 1);
    wcex.lpszMenuName   = MAKEINTRESOURCE(IDC_ANOTHEREDENTOOL);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    m_hInst = hInstance; // Store instance handle in our global variable

    HWND m_hWnd = CreateDialog(m_hInst, MAKEINTRESOURCE(IDD_ANOTHEREDENTOOL_DIALOG), nullptr, AEToolBoxCallback);
    HICON hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SMALL));
    SendMessage(m_hWnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
    SendMessage(m_hWnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
    ShowWindow(m_hWnd, SW_SHOW);

/*
    HWND m_hWnd = CreateWindowEx(WS_EX_TRANSPARENT, szWindowClass, szTitle, (WS_CAPTION | WS_SYSMENU) & ~(WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_THICKFRAME), //WS_OVERLAPPEDWINDOW
        CW_USEDEFAULT, CW_USEDEFAULT, 640, 480, nullptr, nullptr, hInstance, nullptr);
    if (!m_hWnd)
    {
        return FALSE;
    }

    ShowWindow(m_hWnd, nCmdShow);
    UpdateWindow(m_hWnd);
   
    m_hButtonStart = CreateWindowEx(
        0,
        "BUTTON",  // Predefined class; Unicode assumed 
        "Start",      // Button text 
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON | WS_DLGFRAME,  // Styles 
        450,         // x position 
        20,         // y position 
        150,        // Button width
        40,        // Button height
        m_hWnd,     // Parent window
        reinterpret_cast<HMENU>(IDC_BUTTON_Start),       // No menu.
        (HINSTANCE)GetWindowLongPtr(m_hWnd, GWLP_HINSTANCE),
        nullptr);      // Pointer not needed.5

    m_hButtonStop = CreateWindowEx(
        0,
        "BUTTON",  // Predefined class; Unicode assumed 
        "Stop",      // Button text 
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON | WS_DLGFRAME,  // Styles 
        450,         // x position 
        70,         // y position 
        150,        // Button width
        40,        // Button height
        m_hWnd,     // Parent window
        reinterpret_cast<HMENU>(IDC_BUTTON_Stop),       // No menu.
        (HINSTANCE)GetWindowLongPtr(m_hWnd, GWLP_HINSTANCE),
        nullptr);      // Pointer not needed.

    m_hButtonCaptureScreen = CreateWindowEx(
        0,
        "BUTTON",  // Predefined class; Unicode assumed 
        "Capture Screen",      // Button text 
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON | WS_DLGFRAME,  // Styles 
        450,         // x position 
        120,         // y position 
        150,        // Button width
        40,        // Button height
        m_hWnd,     // Parent window
        reinterpret_cast<HMENU>(IDC_BUTTON_CaptureScreen),       // No menu.
        (HINSTANCE)GetWindowLongPtr(m_hWnd, GWLP_HINSTANCE),
        nullptr);      // Pointer not needed.

    m_hInfoBox = CreateWindowEx(
        0,
        "STATIC",  // Predefined class; Unicode assumed 
        "",      // Button text 
        WS_VISIBLE | WS_CHILD | SS_SIMPLE,  // Styles 
        20,         // x position 
        300,         // y position 
        580,        // Button width
        100,        // Button height
        m_hWnd,     // Parent window
        reinterpret_cast<HMENU>(IDC_InfoText),       // No menu.
        (HINSTANCE)GetWindowLongPtr(m_hWnd, GWLP_HINSTANCE),
        nullptr);      // Pointer not needed.

    CreateWindowEx(
        0,
        "STATIC",  // Predefined class; Unicode assumed 
        "",      // Button text 
        WS_VISIBLE | WS_CHILD | SS_SIMPLE | WS_BORDER,  // Styles 
        20,         // x position 
        20,         // y position 
        400,        // Button width
        260,        // Button height
        m_hWnd,     // Parent window
        nullptr,       // No menu.
        (HINSTANCE)GetWindowLongPtr(m_hWnd, GWLP_HINSTANCE),
        nullptr);      // Pointer not needed.

*/
    return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(m_hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC m_hdc = BeginPaint(hWnd, &ps);
            // TODO: Add any drawing code that uses m_hdc here...
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

// Message handler for Another Eden Tool box.
INT_PTR CALLBACK AEToolBoxCallback(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);

    switch (message)
    {
    case WM_INITDIALOG:
        //        wsprintf(strFormat, _T("Init: wParam [%d] lParam [%x]"), wParam, lParam);
        //        MessageBox(NULL, strFormat, _T("Information"), MB_ICONINFORMATION);

        // Try to auto select emulator

        m_AEBot = new CAEBot();
        m_AEBot->init();

        for (auto i = 0; i < m_AEBot->GetEmulatorNumber(); i++)
        {
            string emuname = m_AEBot->GetEmulatorName(i);
            SendDlgItemMessage(hDlg, IDC_EMULATORLIST, CB_ADDSTRING, 0, LPARAM(&emuname));
        }

        for (auto i = 0; i < GRINDING_TYPE_NUMBER; i++)
        {
            string grindingtype = m_grindingType[i];
            SendDlgItemMessage(hDlg, IDC_COMBO_GrindingType, CB_ADDSTRING, 0, LPARAM(&grindingtype));
        }

        for (auto i = 0; i < FISHING_TYPE_NUMBER; i++)
        {
            string fishingtype = m_Fishing_Type[i];
            SendDlgItemMessage(hDlg, IDC_COMBO_FishingType, CB_ADDSTRING, 0, LPARAM(&fishingtype));
        }

        for (auto i = 0; i < DEBUG_LEVEL_NUMBER; i++)
        {
            string debuglevel = m_debugLevel[i];
            SendDlgItemMessage(hDlg, IDC_COMBO_DebugLevel, CB_ADDSTRING, 0, LPARAM(&debuglevel));
        }

        HWND htext;
        htext = GetDlgItem(hDlg, IDC_InfoText);
        SendMessage(htext, EM_SETLIMITTEXT, 0, 0);

        bool botEnabled;
        botEnabled = false;
        for (auto i = 0; i < m_AEBot->GetEmulatorNumber(); i++)
        {
            m_AEBot->SetEmulator(i);
            Status_Code emuStatus = m_AEBot->setup();
            if (emuStatus == status_NoError)
            {
                SendDlgItemMessage(hDlg, IDC_EMULATORLIST, CB_SETCURSEL, i, 0);
                
                wsprintf(strFormat, _T("Auto Select Emulator [%d] %s successfully\r\n"), i, m_AEBot->GetEmulatorName(i));
                SetWindowText(GetDlgItem(hDlg, IDC_InfoText), strFormat);
                botEnabled = true;
                break;
            }
        }

        Interface_Init(hDlg, botEnabled);

        return (INT_PTR)TRUE;

    case WM_QUIT:
    case WM_CLOSE:
        if (m_AEBot)
        {
            m_AEBot->SetStatus(status_Stop);

            delete m_AEBot;
            m_AEBot = NULL;
        }
        PostQuitMessage(0);
        return (INT_PTR)TRUE;

    case WM_TIMER:
        switch (wParam)
        {
        case IDT_TIMER1:
            if (m_AEBot)
            {
                if (m_AEBot->GetStatus() == status_Stop)
                {
                    for (auto k = IDC_CONTROLUNIT_START; k <= IDC_CONTROLUNIT_END; k++)
                    {
                        EnableWindow(GetDlgItem(hDlg, k), true);
                    }

                    EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_Refresh), true);
                    EnableWindow(GetDlgItem(hDlg, IDC_EMULATORLIST), true);
                    EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_Stop), false);

                    KillTimer(hDlg, IDT_TIMER1);
                }
                else 
                {
                    UINT nIndex;
                    string debugmsg;
                    Debug_Level debuglevel = m_AEBot->GetDebugLevel();

                    nIndex = (UINT)SendDlgItemMessage(hDlg, IDC_COMBO_DebugLevel, CB_GETCURSEL, 0, 0);

                    if (nIndex != debuglevel)
                    {
                        m_AEBot->SetDebugLevel((Debug_Level)nIndex);
                    }

                    switch (nIndex)
                    {
                    case debug_None:
                        debugmsg = m_AEBot->GetSummaryMsg();
                        SetWindowText(GetDlgItem(hDlg, IDC_InfoText), debugmsg.c_str());
                        break;

                    case debug_Key:
                    case debug_Brief:
                    case debug_Detail:
                    default:
                        debugmsg = m_AEBot->GetOutputMsg();
                        if (debugmsg.size() > 0)
                        {
                            HWND htext;
                            htext = GetDlgItem(hDlg, IDC_InfoText);
                            int currentlength;
                            currentlength = GetWindowTextLength(htext);
                            SendMessage(htext, EM_SETSEL, (WPARAM)currentlength, (LPARAM)currentlength);
                            SendMessage(htext, EM_REPLACESEL, 0, (LPARAM)(debugmsg.c_str()));
                        }
                        break;
                    }
                }

            }
        }
        return  (INT_PTR)TRUE;

    case WM_COMMAND:
        // Parse the menu selections:
        UINT nCheck;

        switch (LOWORD(wParam))
        {
        case IDC_BUTTON_Refresh:
            UINT nIndex;

            CheckRadioButton(hDlg, IDC_RADIO_Grinding, IDC_RADIO_SeparateGrasta, 0);
            CheckDlgButton(hDlg, IDC_CHECK_FigureEight, 0);
            CheckDlgButton(hDlg, IDC_CHECK_Target999, 0);
            SendDlgItemMessage(hDlg, IDC_COMBO_GrindingType, CB_SETCURSEL, -1, 0);
            SendDlgItemMessage(hDlg, IDC_EMULATORLIST, CB_SETCURSEL, -1, 0);

            for (auto k = IDC_CONTROLUNIT_START; k <= IDC_CONTROLUNIT_END; k++)
            {
                EnableWindow(GetDlgItem(hDlg, k), false);
            }
            EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_Stop), false);

            wsprintf(strFormat, _T(""));
            SetWindowText(GetDlgItem(hDlg, IDC_InfoText), strFormat);

            delete m_AEBot;
            m_AEBot = new CAEBot();
            m_AEBot->init();

            nIndex = (UINT)SendDlgItemMessage(hDlg, IDC_COMBO_DebugLevel, CB_GETCURSEL, 0, 0);
            m_AEBot->SetDebugLevel((Debug_Level)nIndex);

            break;

        case IDC_BUTTON_Start:
            SetTimer(hDlg, IDT_TIMER1, 1000, NULL);

            if (m_AEBot)
            {
                //fork a thread for CAEBot
                std::thread(AEBotTimerThread, 1).detach();

                for (auto k = IDC_CONTROLUNIT_START; k <= IDC_CONTROLUNIT_END; k++)
                {
                    EnableWindow(GetDlgItem(hDlg, k), false);
                }

                EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_Refresh), false);
                EnableWindow(GetDlgItem(hDlg, IDC_EMULATORLIST), false);
                EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_Stop), true);
            }
            break;

        case IDC_BUTTON_Stop:
            KillTimer(hDlg, IDT_TIMER1);
            if (m_AEBot)
            {
                m_AEBot->SetStatus(status_Stop);

                //delete m_AEBot;
                //m_AEBot = NULL;

                for (auto k = IDC_CONTROLUNIT_START; k <= IDC_CONTROLUNIT_END; k++)
                {
                    EnableWindow(GetDlgItem(hDlg, k), true);
                }

                EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_Refresh), true);
                EnableWindow(GetDlgItem(hDlg, IDC_EMULATORLIST), true);
                EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_Stop), false);

            }
            //PostQuitMessage(0);
            return (INT_PTR)TRUE;

        case IDC_BUTTON_Reload:
            if (m_AEBot)
            {
                m_AEBot->reloadConfig();

                Debug_Level debuglevel = m_AEBot->GetDebugLevel();
                SendDlgItemMessage(hDlg, IDC_COMBO_DebugLevel, CB_SETCURSEL, debuglevel, 0);
            }
            break;

        case IDC_BUTTON_CaptureScreen:
            if (m_AEBot)
            {
                m_AEBot->captureScreenNow();
                SetWindowText(GetDlgItem(hDlg, IDC_InfoText), (m_AEBot->GetOutputMsg()).c_str());
            }
            break;

        case IDC_RADIO_Grinding:
            Interface_Grinding(hDlg);
            break;

        case IDC_RADIO_Fishing:
            Interface_fishing(hDlg);
            break;

        case IDC_RADIO_JumpRopes:
        case IDC_CHECK_FigureEight:
             nCheck = IsDlgButtonChecked(hDlg, IDC_CHECK_FigureEight);
            if (m_AEBot)
            {
                if (nCheck)
                {
                    m_AEBot->SetMode(baruokiJumpRopeMode);
                    wsprintf(strFormat, _T("Baruoki Jump Rope.\r\nGet to the section of text that says \"Ready!\", and then start\r\n"));
                }
                else
                {
                    m_AEBot->SetMode(ratleJumpRope);
                    wsprintf(strFormat, _T("Ratle Jump Rope.\r\nGet to the section of text that says \"Are you ready ? GO!\" and then start\r\n"));
                }
                SetWindowText(GetDlgItem(hDlg, IDC_InfoText), strFormat);
            }
            CheckRadioButton(hDlg, IDC_RADIO_Grinding, IDC_RADIO_SeparateGrasta, IDC_RADIO_JumpRopes);
            break;

        case IDC_RADIO_BellStrike:
        case IDC_CHECK_Target999:
            nCheck = IsDlgButtonChecked(hDlg, IDC_CHECK_Target999);
            if (m_AEBot)
            {
                if (nCheck)
                {
                    m_AEBot->SetMode(baruokiJumpRopeMode);
                    wsprintf(strFormat, _T("Hit Bell 999 times.\r\nUse characters with small head models. Mariel recommended.\r\nGet to the section of text that says \"...All right, here we go!\" and then start\r\n"));
                }
                else
                {
                    m_AEBot->SetMode(ratleJumpRope);
                    wsprintf(strFormat, _T("Hit Bell 30 times multiple runs to win Silver Coin.\r\nUse characters with small head models. Mariel recommended.\r\nGet to the section of text that says \"...All right, here we go!\" and then start\r\n"));
                }
                SetWindowText(GetDlgItem(hDlg, IDC_InfoText), strFormat);
            }
            CheckRadioButton(hDlg, IDC_RADIO_Grinding, IDC_RADIO_SeparateGrasta, IDC_RADIO_BellStrike);
            break;

        case IDC_RADIO_SeparateGrasta:
            if (m_AEBot)
            {
                m_AEBot->SetMode(separateGrastaMode);
            }
            wsprintf(strFormat, _T("Seperate Grasta.\r\nGo and click separate in grasta panel, then start.\r\nPlease config what grasta to separate in config_setting file\r\n"));
            SetWindowText(GetDlgItem(hDlg, IDC_InfoText), strFormat);
            break;
        /*
        case IDC_RADIO_EngageHorror:
            if (m_AEBot)
            {
                m_AEBot->SetMode(engageFightMode);
            }
            wsprintf(strFormat, _T("Engage a horror fight now.\r\nPlease config what skill to use in config_setting file\r\n"));
            SetWindowText(GetDlgItem(hDlg, IDC_InfoText), strFormat);
            break;
        */
        case IDC_COMBO_GrindingType:
            switch (HIWORD(wParam))
            {
            case CBN_SELCHANGE:
                Interface_Grinding(hDlg);
                break;
            }
            break;

        case IDC_EMULATORLIST:
            switch (HIWORD(wParam))
            {
            case CBN_SELCHANGE:
                UINT nIndex = (UINT) SendDlgItemMessage(hDlg, IDC_EMULATORLIST, CB_GETCURSEL, 0, 0);
                if (nIndex != CB_ERR && m_AEBot)
                {
                    delete m_AEBot;
                    m_AEBot = new CAEBot;
                    m_AEBot->init();
                    
                    m_AEBot->SetEmulator(nIndex);
                    Status_Code emuStatus = m_AEBot->setup();
                    if ( emuStatus == status_NoError)
                    {
                        botEnabled = true;
                        wsprintf(strFormat, _T("Select Emulator [%d] %s successfully\r\n"), nIndex, m_AEBot->GetEmulatorName(nIndex));
                    }
                    else 
                    {
                        botEnabled = false;
                        wsprintf(strFormat, _T("Select Emulator [%d] %s failed\r\n"), nIndex, m_AEBot->GetEmulatorName(nIndex));
                    }
                    SetWindowText(GetDlgItem(hDlg, IDC_InfoText), strFormat);

                    Interface_Init(hDlg, botEnabled);
                }
                break;
            }
            break;

        }
        break;
    }
    return (INT_PTR)FALSE;
}
