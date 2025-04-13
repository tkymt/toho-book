= ウインドウを画面へ表示させる

== Windowsプログラミング

_tWinMain関数は、Windowsアプリケーションのエントリーポイントです。
_tWinMainは、Unicodeとマルチバイト文字セットの両方に対応するためのマクロです。
SAL注釈を使用して、関数の引数の使われ方を示すことができます。
_Use_decl_annotations_は、定義に宣言と同じSAL注釈を付与します。
使用しないと警告がうるさいです。注釈は付けておきましょう。

//listnum[windows_main.cpp][windows_main.cpp]{
#include <Window.h>
#include <tchar.h>

_Use_decl_annotations_                                    // 宣言と同じSAL注釈を付与するマクロだ
int APIENTRY _tWinMain(HINSTANCE, HINSTANCE, LPTSTR, int) // エントリーポイントとなる関数である
{
	return 0; // 今のところは0を返しておく。一般的に0なら正常終了を意味する。
}
//}



Windowsヘッダーには、C++標準でない型がたくさんあります。
リファレンスを参照して、型を確認してください。
//listnum[Win32Application.ixx][Win32Application.ixx]{
module;
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN // Windowsヘッダーからあまり使用されないものを除外します。
#endif
#include <Windows.h>
#include <cstdlib>
#include <tchar.h>

export module Win32Application;

class LunaEngine;

export class Win32Application
{
public:
	static int Run(LunaEngine *, HINSTANCE, int);
	static HWND GetHwnd();

protected:
	static LRESULT CALLBACK WindowProcedure(HWND, UINT, WPARAM, LPARAM);

private:
	static void ErrorExit();
	static HWND _hwnd;
};
//}
//listnum[Win32Application::Run][Win32Application.cppm]{
int Win32Application::Run(LunaEngine *luna, HINSTANCE hInstance, int nCmdShow)
{
	WNDCLASSEX windowClassEx{
		.cbSize{sizeof(WNDCLASSEX)},
		.style{CS_HREDRAW | CS_VREDRAW},
		.lpfnWndProc{WindowProcedure},
		.hInstance{hInstance},
		.hCursor{LoadIcon(NULL, IDC_ARROW)},
		.lpszClassName{_TEXT("Luna Window Class")}};

	if (RegisterClassEx(&windowClassEx) == 0)
	{
		// エラーハンドリング
		ErrorExit();
	}

	// ウィンドウのタイトルバーを除いた部分であるクライアント領域のサイズを指定
	RECT windowRect{
		.right{1280},
		.bottom{960}};
	if (AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE) == 0)
	{
		// エラーハンドリング
		ErrorExit();
	}

	if ((_hwnd = CreateWindow(
			windowClassEx.lpszClassName,
			_TEXT("Luna Application"),
			WS_OVERLAPPEDWINDOW,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			windowRect.right - windowRect.left,
			windowRect.bottom - windowRect.top,
			NULL,
			NULL,
			hInstance,
			luna)) == NULL)
	{
		MessageBox(NULL, _TEXT("ウインドウの作成に失敗しました。"), _TEXT("エラー"), MB_OK | MB_ICONERROR);
		return EXIT_FAILURE;
	}

	ShowWindow(_hwnd, nCmdShow);

	MSG msg{};
	BOOL msgReturn{};
	while ((msgReturn = GetMessage(&msg, _hwnd, 0, 0)) != 0)
	{
		if (msgReturn == -1)
		{
			ErrorExit();
		}
		else
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return static_cast<int>(msg.wParam);
}
//}
//listnum[Win32Application::GetHwnd][Win32Application.cppm]{
HWND Win32Application::GetHwnd()
{
	return _hwnd;
}
//}


WindowProcは、OSからのメッセージを受け取るコールバック関数です。
メッセージで分岐させ、適切な処理をします。
基本的には、0を返して終了しますが、その他の値を返す意味があることもあります。
WM_CREATEなら、0を返すとウインドウが作成されますが、-1を返すとウインドウが作成されずCreateWindowEx関数がNULLを返します。
ウインドウプロシージャで返すべき値は、リファレンスを参照してください。
WM_DESTROYを受け取ったらPostQuitMessage(0)を呼び出してWM_QUITメッセージを送ります。
WM_QUITメッセージはPeekMessage関数で取得して、ループの継続条件に用います。

AdjustWindowRect は、指定されたウィンドウスタイル (dwStyle) とメニューフラグ (bMenu) に基づいて、ウィンドウのクライアント領域を囲むために必要なウィンドウの外側のサイズを調整します。結果は、指定された矩形 (lpRect) に反映されます。


//listnum[Win32Application::WindowProcedure][Win32Application.cppm]{
LRESULT CALLBACK Win32Application::WindowProcedure(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	LunaEngine *luna = reinterpret_cast<LunaEngine *>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
	switch (msg)
	{
	case WM_CREATE:
	{
		CREATESTRUCT *createStruct = reinterpret_cast<CREATESTRUCT *>(lParam);
		luna = reinterpret_cast<LunaEngine *>(createStruct->lpCreateParams);
		SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(luna));
		return 0;
	}
	case WM_DESTROY:
		PostQuitMessage(EXIT_SUCCESS);
		return 0;
	default:
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}
}
//}
ウインドウプロシージャ概要
@<href>{https://learn.microsoft.com/ja-jp/windows/win32/learnwin32/writing-the-window-procedure}
ウインドウメッセージリファレンス
@<href>{https://learn.microsoft.com/ja-jp/windows/win32/winmsg/window-notifications}

SAL注釈概要
@<href>{https://learn.microsoft.com/ja-jp/cpp/code-quality/using-sal-annotations-to-reduce-c-cpp-code-defects}
Windows データ型
@<href>{https://learn.microsoft.com/ja-jp/windows/win32/winprog/windows-data-types}


//listnum[Win32Application::ErrorExit][Win32Application.cppm]{
void Win32Application::ErrorExit()
{
	// Retrieve the system error message for the last-error code

	LPVOID lpMsgBuf;
	DWORD dw = GetLastError();

	if (FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER |
				FORMAT_MESSAGE_FROM_SYSTEM |
				FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			dw,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPTSTR)&lpMsgBuf,
			0, NULL) == 0)
	{
		MessageBox(NULL, TEXT("FormatMessage failed"), TEXT("Error"), MB_OK);
		ExitProcess(dw);
	}

	MessageBox(NULL, (LPCTSTR)lpMsgBuf, TEXT("Error"), MB_OK);

	LocalFree(lpMsgBuf);
	ExitProcess(dw);
}
//}
//listnum[Win32Application::_hwnd][Win32Application.cppm]{
/// @brief 値初期化によるゼロ初期化でnullptrを初期値にする
HWND Win32Application::_hwnd{};
//}

このメッセージループは、多くのサンプルコードで見られるものです。
ですがもっと良い方法があります。それは別スレッドでゲームループを実行する方法です。
なぜかというと、if文を読んでわかるようにメッセージがあったときはゲームループが実行されないからです。
また、メッセージの処理に時間がかかると、ゲームが実行されない時間が長くなります。
このような別の処理で時間がかかり発生する待ち時間をデッドタイムといいます。

//listnum[MessageLoop][MessageLoop]{
MSG msg{};
while (msg.message != WM_QUIT)
{
	if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) 
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	else 
	{
		luna->Update(); // メッセージが無かったときだけ、ゲームの更新や描画を行う
	}
}
//}

