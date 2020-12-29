// AnotherEdenTool.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "AnotherEdenTool.h"
#include "AEBot.h"

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE m_hInst;                                // current instance
TCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
TCHAR strFormat[1024]; // Must ensure size!

HWND m_hWnd;
CAEBot* m_AEBot=NULL;

HWND m_hButtonExit;

HWND m_hButtonStart;
HWND m_hButtonStop;
HWND m_hButtonCaptureScreen;
HWND m_hInfoBox;

void AEBotThread(int n)
{
    m_AEBot->run();
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
            case IDC_BUTTON_Start:
                wsprintf(strFormat, _T("Start: wParam [%d] lParam [%x]"), wParam, lParam);
                MessageBox(NULL, strFormat, _T("Information"), MB_ICONINFORMATION);
                break;
            case IDC_BUTTON_Stop:
                wsprintf(strFormat, _T("Stop: wParam [%d] lParam [%x]"), wParam, lParam);
                MessageBox(NULL, strFormat, _T("Information"), MB_ICONINFORMATION);
                break;
            case IDC_BUTTON_CaptureScreen:
                wsprintf(strFormat, _T("%s\n%s\n\n"), szTitle, szWindowClass);
                SetWindowText(m_hInfoBox, strFormat);

                MessageBox(NULL, strFormat, _T("Information"), MB_ICONINFORMATION);

                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: Add any drawing code that uses hdc here...
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
    Bot_Mode botMode;

    switch (message)
    {
    case WM_INITDIALOG:
        EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_Stop), false);

        CheckRadioButton(hDlg, IDC_RADIO_Grinding, IDC_RADIO_EngageHorror,0);

//        wsprintf(strFormat, _T("Init: wParam [%d] lParam [%x]"), wParam, lParam);
//        MessageBox(NULL, strFormat, _T("Information"), MB_ICONINFORMATION);

        m_AEBot = new CAEBot();
        m_AEBot->setup();

        botMode = m_AEBot->GetMode();

        switch (botMode)
        {
        case grindingMode:
            CheckRadioButton(hDlg, IDC_RADIO_Grinding, IDC_RADIO_EngageHorror, IDC_RADIO_Grinding);
            break;
        case grindingTravelMode:
            CheckRadioButton(hDlg, IDC_RADIO_Grinding, IDC_RADIO_EngageHorror, IDC_RADIO_Grinding);
            CheckDlgButton(hDlg, IDC_CHECK_Travel, 1);
            break;
        case grindingLOMSlimeMode:
            CheckRadioButton(hDlg, IDC_RADIO_Grinding, IDC_RADIO_EngageHorror, IDC_RADIO_Grinding);
            CheckDlgButton(hDlg, IDC_CHECK_LOMSmile, 1);
            break;
        case fishingMode:
            CheckRadioButton(hDlg, IDC_RADIO_Grinding, IDC_RADIO_EngageHorror, IDC_RADIO_Fishing);
            break;
        case ratleJumpRope:
            CheckRadioButton(hDlg, IDC_RADIO_Grinding, IDC_RADIO_EngageHorror, IDC_RADIO_JumpRopes);
            break;
        case baruokiJumpRopeMode:
            CheckRadioButton(hDlg, IDC_RADIO_Grinding, IDC_RADIO_EngageHorror, IDC_RADIO_JumpRopes);
            CheckDlgButton(hDlg, IDC_CHECK_FigureEight, 1);
            break;
        case silverHitBell30Mode:
            CheckRadioButton(hDlg, IDC_RADIO_Grinding, IDC_RADIO_EngageHorror, IDC_RADIO_BellStrike);
            break;
        case silverHitBell999Mode:
            CheckRadioButton(hDlg, IDC_RADIO_Grinding, IDC_RADIO_EngageHorror, IDC_RADIO_BellStrike);
            CheckDlgButton(hDlg, IDC_CHECK_Target999, 1);
            break;
        case seperateGrastaMode:
            CheckRadioButton(hDlg, IDC_RADIO_Grinding, IDC_RADIO_EngageHorror, IDC_RADIO_SeparateGrasta);
            break;
        case engageFightMode:
            CheckRadioButton(hDlg, IDC_RADIO_Grinding, IDC_RADIO_EngageHorror, IDC_RADIO_EngageHorror);
            break;
        case captureScreenMode:
        default:
            break;
        }
 
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

    case WM_COMMAND:
        // Parse the menu selections:
        int resourceID = (int)wParam;
        string str;
        UINT nCheck;

        switch (resourceID)
        {
        case IDC_BUTTON_Start:
            //fork a thread for CAEBot
            std::thread (AEBotThread, 1).detach();

            EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_Stop), true);
            EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_Start), false);
            EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_Refresh), false);
            break;

        case IDC_BUTTON_Refresh:
            CheckRadioButton(hDlg, IDC_RADIO_Grinding, IDC_RADIO_EngageHorror, 0);
            CheckDlgButton(hDlg, IDC_CHECK_Travel, 0);
            CheckDlgButton(hDlg, IDC_CHECK_LOMSmile, 0);
            CheckDlgButton(hDlg, IDC_CHECK_FigureEight, 0);
            CheckDlgButton(hDlg, IDC_CHECK_Target999, 0);

            wsprintf(strFormat, _T(""));
            SetWindowText(GetDlgItem(hDlg, IDC_InfoText), strFormat);

            delete m_AEBot;
            m_AEBot = new CAEBot();
            m_AEBot->setup();

            break;

        case IDC_BUTTON_CaptureScreen:
            if (m_AEBot)
            {
                m_AEBot->captureScreenNow();
                SetWindowText(GetDlgItem(hDlg, IDC_InfoText), (m_AEBot->GetdbgMsg()).c_str());
                CheckRadioButton(hDlg, IDC_RADIO_Grinding, IDC_RADIO_EngageHorror, 0);
            }
            break;

        case IDC_BUTTON_Stop:
            if (m_AEBot)
            {
                m_AEBot->SetStatus(status_Stop);

                delete m_AEBot;
                m_AEBot = NULL;
            }
            PostQuitMessage(0);
            return (INT_PTR)TRUE;

        case IDC_RADIO_Grinding:
            if (m_AEBot)
            {
                m_AEBot->SetMode(grindingMode);
            }
            wsprintf(strFormat, _T("Grinding here.\n\nYou can start here\n"));
            SetWindowText(GetDlgItem(hDlg, IDC_InfoText), strFormat);
            break;

        case IDC_RADIO_Fishing:
            if (m_AEBot)
            {
                m_AEBot->SetMode(fishingMode);
            }
            wsprintf(strFormat, _T("Fishing mode.\n\nYou can start anywhere\nPlease config where to fish and what bait to use in config_fishing file\n"));
            SetWindowText(GetDlgItem(hDlg, IDC_InfoText), strFormat);
            break;

        case IDC_RADIO_JumpRopes:
            if (m_AEBot)
            {
                m_AEBot->SetMode(ratleJumpRope);
            }
            wsprintf(strFormat, _T("Ratle Jump Rope.\n\nGet to the section of text that says \"Are you ready ? GO!\" and then start\n"));
            SetWindowText(GetDlgItem(hDlg, IDC_InfoText), strFormat);
            break;

        case IDC_RADIO_BellStrike:
            if (m_AEBot)
            {
                m_AEBot->SetMode(silverHitBell30Mode);
            }
            wsprintf(strFormat, _T("Hit Bell 30 times multiple runs to win Silver Coin.\n\nUse characters with small head models. Mariel recommended.\nGet to the section of text that says \"...All right, here we go!\" and then start\n"));
            SetWindowText(GetDlgItem(hDlg, IDC_InfoText), strFormat);
            break;

        case IDC_RADIO_SeparateGrasta:
            if (m_AEBot)
            {
                m_AEBot->SetMode(seperateGrastaMode);
            }
            wsprintf(strFormat, _T("Seperate Grasta.\n\nGet to Izana and go to a place near the shrine. As long as the exclamation shows, then start.\nPlease config what grasta to separate in config_setting file\n"));
            SetWindowText(GetDlgItem(hDlg, IDC_InfoText), strFormat);
            break;

        case IDC_RADIO_EngageHorror:
            if (m_AEBot)
            {
                m_AEBot->SetMode(engageFightMode);
            }
            wsprintf(strFormat, _T("Engage a horror fight now.\n\nPlease config what skill to use in config_setting file\n"));
            SetWindowText(GetDlgItem(hDlg, IDC_InfoText), strFormat);
            break;

        case IDC_CHECK_Travel:
            nCheck = IsDlgButtonChecked(hDlg, IDC_CHECK_Travel);
            if (m_AEBot)
            {
                if (nCheck)
                {
                    CheckDlgButton(hDlg, IDC_CHECK_LOMSmile, 0);
                    m_AEBot->SetMode(grindingTravelMode);
                    wsprintf(strFormat, _T("Travel world to grind.\n\nYou can start anywhere. Please config where to grind and what skill to use in config_setting file\n"));
                    SetWindowText(GetDlgItem(hDlg, IDC_InfoText), strFormat);
                }
                else
                { 
                    m_AEBot->SetMode(grindingMode);
                    wsprintf(strFormat, _T("Grinding here.\n\nYou can start here\n"));
                    SetWindowText(GetDlgItem(hDlg, IDC_InfoText), strFormat);
                }
            }
            CheckRadioButton(hDlg, IDC_RADIO_Grinding, IDC_RADIO_EngageHorror, IDC_RADIO_Grinding);
            break;

        case IDC_CHECK_LOMSmile:
            nCheck = IsDlgButtonChecked(hDlg, IDC_CHECK_LOMSmile);
            if (m_AEBot)
            {
                if (nCheck)
                {
                    CheckDlgButton(hDlg, IDC_CHECK_Travel, 0);
                    m_AEBot->SetMode(grindingLOMSlimeMode);
                    wsprintf(strFormat, _T("Lord of Mana plantium slime grinding.\n\nYou can start anywhere\n"));
                    SetWindowText(GetDlgItem(hDlg, IDC_InfoText), strFormat);
                }
                else
                {
                    m_AEBot->SetMode(grindingMode);
                    wsprintf(strFormat, _T("Grinding here.\n\nYou can start here\n"));
                    SetWindowText(GetDlgItem(hDlg, IDC_InfoText), strFormat);
                }
            }
            CheckRadioButton(hDlg, IDC_RADIO_Grinding, IDC_RADIO_EngageHorror, IDC_RADIO_Grinding);
            break;

        case IDC_CHECK_FigureEight:
            nCheck = IsDlgButtonChecked(hDlg, IDC_CHECK_FigureEight);
            if (m_AEBot)
            {
                if (nCheck)
                {
                    m_AEBot->SetMode(baruokiJumpRopeMode);
                    wsprintf(strFormat, _T("Baruoki Jump Rope.\n\nGet to the section of text that says \"Ready!\", and then start\n"));
                    SetWindowText(GetDlgItem(hDlg, IDC_InfoText), strFormat);
                }
                else
                {
                    m_AEBot->SetMode(ratleJumpRope);
                    wsprintf(strFormat, _T("Ratle Jump Rope.\n\nGet to the section of text that says \"Are you ready ? GO!\" and then start\n"));
                    SetWindowText(GetDlgItem(hDlg, IDC_InfoText), strFormat);
                }
            }
            CheckRadioButton(hDlg, IDC_RADIO_Grinding, IDC_RADIO_EngageHorror, IDC_RADIO_JumpRopes);
            break;

        case IDC_CHECK_Target999:
            nCheck = IsDlgButtonChecked(hDlg, IDC_CHECK_Target999);
            if (m_AEBot)
            {
                if (nCheck)
                {
                    m_AEBot->SetMode(baruokiJumpRopeMode);
                    wsprintf(strFormat, _T("Hit Bell 999 times.\n\nUse characters with small head models. Mariel recommended.\nGet to the section of text that says \"...All right, here we go!\" and then start\n"));
                    SetWindowText(GetDlgItem(hDlg, IDC_InfoText), strFormat);
                }
                else
                {
                    m_AEBot->SetMode(ratleJumpRope);
                    wsprintf(strFormat, _T("Hit Bell 30 times multiple runs to win Silver Coin.\n\nUse characters with small head models. Mariel recommended.\nGet to the section of text that says \"...All right, here we go!\" and then start\n"));
                    SetWindowText(GetDlgItem(hDlg, IDC_InfoText), strFormat);
                }
            }
            CheckRadioButton(hDlg, IDC_RADIO_Grinding, IDC_RADIO_EngageHorror, IDC_RADIO_BellStrike);
            break;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
