/**
 * 网格操作可视化程序 (美化版)
 * 功能：实现2n×2m网格上的5种行列交换与块内交换操作，实时显示数字变化
 * 新增：
 *   - 支持动态修改 n, m（通过顶部输入框）
 *   - 圆角网格绘制，柔和配色
 *   - 按钮字体优化，窗口自适应大小
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <vector>
#include <string>
#include <sstream>
#include <cmath>

// ---------------------------- 全局变量 ----------------------------
int n = 2;                       // 2n行 = 4行
int m = 3;                       // 2m列 = 6列
std::vector<std::vector<int>> grid;

// 绘制参数
const int CELL_W = 70;           // 单元格宽度(像素)
const int CELL_H = 50;           // 单元格高度
const int TOP_AREA_HEIGHT = 100; // 顶部控件区高度
const int MARGIN_LEFT = 20;
const int MARGIN_TOP = TOP_AREA_HEIGHT + 10;

// 控件ID
#define IDC_EDIT_N      2001
#define IDC_EDIT_M      2002
#define IDC_BTN_APPLY   2003
#define IDC_BTN_OP1     2004
#define IDC_BTN_OP2     2005
#define IDC_BTN_OP3     2006
#define IDC_BTN_OP4     2007
#define IDC_BTN_OP5     2008
#define IDC_BTN_RESET   2009

// 窗口句柄
HWND g_hWnd = nullptr;
HFONT g_hButtonFont = nullptr;   // 全局按钮字体

// ---------------------------- 辅助函数 ----------------------------
void SetWindowSizeForGrid();
void InitGrid();

// 判断单元格颜色 (基于2x2块染色)
bool IsBlackCell(int i, int j) {
    int bi = i / 2;
    int bj = j / 2;
    return ((bi + bj) % 2 == 0);
}

// 初始化网格 (数字按行优先)
void InitGrid() {
    int rows = 2 * n;
    int cols = 2 * m;
    grid.assign(rows, std::vector<int>(cols, 0));
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            grid[i][j] = i * cols + j + 1;
        }
    }
}

// 操作1: 交换偶数列↔奇数列
void Operation1() {
    int cols = 2 * m, rows = 2 * n;
    for (int j = 0; j < m; ++j) {
        int col1 = 2 * j, col2 = 2 * j + 1;
        for (int i = 0; i < rows; ++i)
            std::swap(grid[i][col1], grid[i][col2]);
    }
}

// 操作2: 交换奇数列↔下一列
void Operation2() {
    int cols = 2 * m, rows = 2 * n;
    for (int j = 0; j < m - 1; ++j) {
        int col1 = 2 * j + 1, col2 = 2 * j + 2;
        for (int i = 0; i < rows; ++i)
            std::swap(grid[i][col1], grid[i][col2]);
    }
}

// 操作3: 交换偶数行↔奇数行
void Operation3() {
    int rows = 2 * n, cols = 2 * m;
    for (int i = 0; i < n; ++i) {
        int row1 = 2 * i, row2 = 2 * i + 1;
        for (int j = 0; j < cols; ++j)
            std::swap(grid[row1][j], grid[row2][j]);
    }
}

// 操作4: 交换奇数行↔下一行
void Operation4() {
    int rows = 2 * n, cols = 2 * m;
    for (int i = 0; i < n - 1; ++i) {
        int row1 = 2 * i + 1, row2 = 2 * i + 2;
        for (int j = 0; j < cols; ++j)
            std::swap(grid[row1][j], grid[row2][j]);
    }
}

// 操作5: 2x2块内对角线交换
void Operation5() {
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < m; ++j) {
            int r0 = 2 * i, c0 = 2 * j;
            bool isBlack = ((i + j) % 2 == 0);
            if (isBlack)
                std::swap(grid[r0][c0], grid[r0+1][c0+1]);
            else
                std::swap(grid[r0][c0+1], grid[r0+1][c0]);
        }
    }
}

// 重置网格 (按当前n,m)
void ResetGrid() {
    InitGrid();
    InvalidateRect(g_hWnd, nullptr, TRUE);
}

// 应用新的n,m输入
void ApplyNewDimensions() {
    // 获取编辑框文本
    wchar_t bufN[16], bufM[16];
    GetDlgItemTextW(g_hWnd, IDC_EDIT_N, bufN, 16);
    GetDlgItemTextW(g_hWnd, IDC_EDIT_M, bufM, 16);
    int newN = _wtoi(bufN);
    int newM = _wtoi(bufM);
    if (newN < 1) newN = 1;
    if (newM < 1) newM = 1;
    // 限制最大尺寸 (避免窗口超出屏幕)
    const int MAX_N = 15;
    const int MAX_M = 20;
    if (newN > MAX_N) newN = MAX_N;
    if (newM > MAX_M) newM = MAX_M;
    
    if (newN == n && newM == m) return; // 无变化
    
    n = newN;
    m = newM;
    InitGrid();
    // 调整窗口大小以适应新网格
    SetWindowSizeForGrid();
    InvalidateRect(g_hWnd, nullptr, TRUE);
    SetWindowTextW(g_hWnd, L"网格操作可视化 (当前网格: 2n×2m = ");
    wchar_t title[128];
    wsprintfW(title, L"网格操作可视化 (%d×%d)", 2*n, 2*m);
    SetWindowTextW(g_hWnd, title);
}

// 根据当前n,m计算所需窗口大小并调整
void SetWindowSizeForGrid() {
    int requiredWidth = MARGIN_LEFT + 2 * m * CELL_W + 30;
    int requiredHeight = MARGIN_TOP + 2 * n * CELL_H + 40;
    // 保证最小宽度能看到所有按钮
    if (requiredWidth < 750) requiredWidth = 750;
    
    RECT rcWindow;
    GetWindowRect(g_hWnd, &rcWindow);
    // 调整客户区大小
    RECT rcClient = {0, 0, requiredWidth, requiredHeight};
    AdjustWindowRectEx(&rcClient, GetWindowLongW(g_hWnd, GWL_STYLE), FALSE, GetWindowLongW(g_hWnd, GWL_EXSTYLE));
    int newWidth = rcClient.right - rcClient.left;
    int newHeight = rcClient.bottom - rcClient.top;
    SetWindowPos(g_hWnd, NULL, rcWindow.left, rcWindow.top, newWidth, newHeight, SWP_NOZORDER | SWP_NOMOVE);
}

// 绘制圆角矩形
void DrawRoundRect(HDC hdc, int left, int top, int right, int bottom, int radius, COLORREF color) {
    HBRUSH hBrush = CreateSolidBrush(color);
    HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, hBrush);
    HPEN hPen = CreatePen(PS_SOLID, 1, RGB(128,128,128));
    HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
    RoundRect(hdc, left, top, right, bottom, radius, radius);
    SelectObject(hdc, hOldPen);
    SelectObject(hdc, hOldBrush);
    DeleteObject(hPen);
    DeleteObject(hBrush);
}

// 绘制网格 (圆角单元格，渐变背景色)
void DrawGrid(HDC hdc, const RECT& clientRect) {
    int rows = 2 * n;
    int cols = 2 * m;
    int startX = MARGIN_LEFT;
    int startY = MARGIN_TOP;

    // 创建字体（使用宽字符版本）
    HFONT hFont = CreateFontW(22, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                              DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                              DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
    HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);
    SetBkMode(hdc, TRANSPARENT);
    
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            int x = startX + j * CELL_W;
            int y = startY + i * CELL_H;
            // 根据颜色填充背景（柔和色：黑色块淡蓝灰，白色块米白）
            bool isBlack = IsBlackCell(i, j);
            COLORREF bgColor = isBlack ? RGB(210, 225, 240) : RGB(252, 248, 235);
            DrawRoundRect(hdc, x, y, x + CELL_W, y + CELL_H, 8, bgColor);
            
            // 绘制数字
            std::wstring text = std::to_wstring(grid[i][j]);
            RECT cellRect = {x + 2, y + 2, x + CELL_W - 2, y + CELL_H - 2};
            SetTextColor(hdc, RGB(40, 40, 80));
            DrawTextW(hdc, text.c_str(), (int)text.length(), &cellRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        }
    }
    
    SelectObject(hdc, hOldFont);
    DeleteObject(hFont);
}

// 创建控件 (在WM_CREATE中调用)
void CreateControls(HWND hWnd) {
    // 创建全局字体
    g_hButtonFont = CreateFontW(16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                                DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
    
    // 标签: n =
    CreateWindowW(L"STATIC", L"n =", WS_VISIBLE | WS_CHILD, 20, 15, 30, 25, hWnd, NULL, NULL, NULL);
    CreateWindowW(L"EDIT", L"2", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_NUMBER, 55, 12, 40, 25, hWnd, (HMENU)IDC_EDIT_N, NULL, NULL);
    CreateWindowW(L"STATIC", L"m =", WS_VISIBLE | WS_CHILD, 110, 15, 30, 25, hWnd, NULL, NULL, NULL);
    CreateWindowW(L"EDIT", L"3", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_NUMBER, 145, 12, 40, 25, hWnd, (HMENU)IDC_EDIT_M, NULL, NULL);
    CreateWindowW(L"BUTTON", L"应用", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 195, 10, 60, 28, hWnd, (HMENU)IDC_BTN_APPLY, NULL, NULL);
    
    // 操作按钮
    CreateWindowW(L"BUTTON", L"操作1: 交换偶数列↔奇数列", WS_VISIBLE | WS_CHILD, 280, 10, 170, 32, hWnd, (HMENU)IDC_BTN_OP1, NULL, NULL);
    CreateWindowW(L"BUTTON", L"操作2: 交换奇数列↔下列", WS_VISIBLE | WS_CHILD, 460, 10, 170, 32, hWnd, (HMENU)IDC_BTN_OP2, NULL, NULL);
    CreateWindowW(L"BUTTON", L"操作3: 交换偶数行↔奇数行", WS_VISIBLE | WS_CHILD, 640, 10, 170, 32, hWnd, (HMENU)IDC_BTN_OP3, NULL, NULL);
    
    CreateWindowW(L"BUTTON", L"操作4: 交换奇数行↔下行", WS_VISIBLE | WS_CHILD, 280, 50, 170, 32, hWnd, (HMENU)IDC_BTN_OP4, NULL, NULL);
    CreateWindowW(L"BUTTON", L"操作5: 2x2块内对角线交换", WS_VISIBLE | WS_CHILD, 460, 50, 200, 32, hWnd, (HMENU)IDC_BTN_OP5, NULL, NULL);
    CreateWindowW(L"BUTTON", L"重置网格", WS_VISIBLE | WS_CHILD, 670, 50, 100, 32, hWnd, (HMENU)IDC_BTN_RESET, NULL, NULL);
    
    // 设置所有按钮字体
    HWND hButtons[] = {
        GetDlgItem(hWnd, IDC_BTN_APPLY), GetDlgItem(hWnd, IDC_BTN_OP1), GetDlgItem(hWnd, IDC_BTN_OP2),
        GetDlgItem(hWnd, IDC_BTN_OP3), GetDlgItem(hWnd, IDC_BTN_OP4), GetDlgItem(hWnd, IDC_BTN_OP5),
        GetDlgItem(hWnd, IDC_BTN_RESET)
    };
    for (HWND btn : hButtons) {
        if (btn) SendMessage(btn, WM_SETFONT, (WPARAM)g_hButtonFont, TRUE);
    }
    // 编辑框也设字体
    SendMessage(GetDlgItem(hWnd, IDC_EDIT_N), WM_SETFONT, (WPARAM)g_hButtonFont, TRUE);
    SendMessage(GetDlgItem(hWnd, IDC_EDIT_M), WM_SETFONT, (WPARAM)g_hButtonFont, TRUE);
}

// 窗口过程
LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE:
            CreateControls(hWnd);
            InitGrid();
            SetWindowSizeForGrid();
            SetWindowTextW(hWnd, L"网格操作可视化 (4×6)");
            break;
            
        case WM_COMMAND:
        {
            int id = LOWORD(wParam);
            switch (id) {
                case IDC_BTN_OP1:  Operation1(); InvalidateRect(hWnd, nullptr, TRUE); break;
                case IDC_BTN_OP2:  Operation2(); InvalidateRect(hWnd, nullptr, TRUE); break;
                case IDC_BTN_OP3:  Operation3(); InvalidateRect(hWnd, nullptr, TRUE); break;
                case IDC_BTN_OP4:  Operation4(); InvalidateRect(hWnd, nullptr, TRUE); break;
                case IDC_BTN_OP5:  Operation5(); InvalidateRect(hWnd, nullptr, TRUE); break;
                case IDC_BTN_RESET: ResetGrid(); break;
                case IDC_BTN_APPLY: ApplyNewDimensions(); break;
            }
        }
        break;
        
        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // 绘制背景渐变 (简单线性渐变，从浅灰到白)
            RECT rc;
            GetClientRect(hWnd, &rc);
            HDC memDC = CreateCompatibleDC(hdc);
            HBITMAP memBitmap = CreateCompatibleBitmap(hdc, rc.right, rc.bottom);
            SelectObject(memDC, memBitmap);
            // 填充背景
            for (int y = 0; y < rc.bottom; ++y) {
                COLORREF color = RGB(240 + y/20, 240 + y/20, 255 - y/30);
                HPEN pen = CreatePen(PS_SOLID, 1, color);
                SelectObject(memDC, pen);
                MoveToEx(memDC, 0, y, NULL);
                LineTo(memDC, rc.right, y);
                DeleteObject(pen);
            }
            DrawGrid(memDC, rc);
            BitBlt(hdc, 0, 0, rc.right, rc.bottom, memDC, 0, 0, SRCCOPY);
            DeleteDC(memDC);
            DeleteObject(memBitmap);
            EndPaint(hWnd, &ps);
        }
        break;
        
        case WM_DESTROY:
            if (g_hButtonFont) DeleteObject(g_hButtonFont);
            PostQuitMessage(0);
            break;
            
        default:
            return DefWindowProcW(hWnd, msg, wParam, lParam);
    }
    return 0;
}

// 程序入口
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    WNDCLASSW wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = L"GridOperationWindow";
    RegisterClassW(&wc);
    
    g_hWnd = CreateWindowW(L"GridOperationWindow", L"网格操作可视化",
                           WS_OVERLAPPEDWINDOW | WS_VISIBLE | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
                           CW_USEDEFAULT, CW_USEDEFAULT, 800, 600,
                           NULL, NULL, hInstance, NULL);
    if (!g_hWnd) return -1;
    
    ShowWindow(g_hWnd, nCmdShow);
    UpdateWindow(g_hWnd);
    
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return (int)msg.wParam;
}
/*
g++ main.cpp -o GridOperation.exe -mwindows -static
*/