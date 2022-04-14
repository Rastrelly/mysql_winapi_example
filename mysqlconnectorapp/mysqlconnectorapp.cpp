#ifndef UNICODE
#define UNICODE
#endif 

//#include "stdafx.h"
#include <cstdio>
#include <conio.h>
#include <stdlib.h>
#include <iostream>
#include <iomanip>

#include <mysql.h>

#include <string>
#include <sstream>
#include <windows.h>
#include <commctrl.h> 

#define BTN_CONNECT 1000
#define BTN_WRITEDB 1005
#define BTN_READDB 1006
#define EDT_COL 1001
#define EDT_ROW 1002
#define EDT_DATA 1003
#define STT_STATUS 1004

using namespace std;

HWND edit1, edit2, edit3, button1, button2, button3, static1; //windows for controls

//MySQL specific variables
MYSQL *conn;
MYSQL_RES *mresult;
MYSQL_ROW row;
MYSQL_FIELD *fld;

string port = "3306";
string host = "localhost";
string user = "root";
string pass = "DOOM";
string dbname = "test_schema";

bool off = false;
int qr;


LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR pCmdLine, int nCmdShow)
{
	// Register the window class.
	const wchar_t CLASS_NAME[] = L"Sample Window Class";

	WNDCLASS wc = { };

	wc.lpfnWndProc = WindowProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = CLASS_NAME;

	RegisterClass(&wc);

	// Create the window.

	HWND hwnd = CreateWindowEx(
		0,                              // Optional window styles.
		CLASS_NAME,                     // Window class
		L"MYSQL Connector Example",    // Window text
		WS_OVERLAPPEDWINDOW,            // Window style

		// Size and position
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,

		NULL,       // Parent window    
		NULL,       // Menu
		hInstance,  // Instance handle
		NULL        // Additional application data
	);

	if (hwnd == NULL)
	{
		return 0;
	}

	ShowWindow(hwnd, nCmdShow);

	//Creating window controls

	button1 = CreateWindow(
		L"BUTTON",  // Predefined class; Unicode assumed 
		L"Connect",      // Button text 
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // Styles 
		10,         // x position 
		10,         // y position 
		100,        // Button width
		25,        // Button height
		hwnd,     // Parent window
		(HMENU)BTN_CONNECT,       // No menu.
		hInstance,	//Instance
		NULL);      // Pointer not needed.

	button2 = CreateWindow(
		L"BUTTON",  // Predefined class; Unicode assumed 
		L"Create DB",      // Button text 
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // Styles 
		10,         // x position 
		490,         // y position 
		100,        // Button width
		25,        // Button height
		hwnd,     // Parent window
		(HMENU)BTN_WRITEDB,       // No menu.
		hInstance,	//Instance
		NULL);      // Pointer not needed.

	button3 = CreateWindow(
		L"BUTTON",  // Predefined class; Unicode assumed 
		L"Read DB",      // Button text 
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // Styles 
		200,         // x position 
		490,         // y position 
		100,        // Button width
		25,        // Button height
		hwnd,     // Parent window
		(HMENU)BTN_READDB,       // No menu.
		hInstance,	//Instance
		NULL);      // Pointer not needed.

	static1 = CreateWindowEx(WS_EX_CLIENTEDGE, L"STATIC", L"Not connected",
		WS_VISIBLE | WS_CHILD | ES_LEFT,
		130, 10, 150, 25,
		hwnd,
		(HMENU)EDT_COL, hInstance, NULL);

	edit1 = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", L"10",
		WS_VISIBLE | WS_CHILD | WS_BORDER | ES_LEFT,
		10, 40, 150, 25,
		hwnd,
		(HMENU)EDT_COL, hInstance, NULL);

	edit2 = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", L"10",
		WS_VISIBLE | WS_CHILD | WS_BORDER | ES_LEFT,
		170, 40, 150, 25,
		hwnd,
		(HMENU)EDT_ROW, hInstance, NULL);

	edit3 = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", L"10",
		WS_CHILD | WS_VISIBLE | WS_VSCROLL |
		ES_LEFT | ES_MULTILINE | ES_AUTOVSCROLL,
		10, 75, 910, 400,
		hwnd,
		(HMENU)EDT_DATA, hInstance, NULL);


	// Run the message loop.

	MSG msg = { };
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return 0;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;

	case WM_COMMAND: //Processing user input
	{
		if (wParam == BTN_CONNECT) //If user clicked on CONNECT button
		{
			// Получаем дескриптор соединения
			conn = mysql_init(NULL);
			if (conn == NULL)
			{
				// Если дескриптор не получен – выводим сообщение об ошибке
				SetWindowTextA(static1, LPCSTR("No descriptor"));
			}
			// Подключаемся к серверу
			if (!mysql_real_connect(conn, host.c_str(), user.c_str(), pass.c_str(), dbname.c_str(), atoi(port.c_str()), NULL, 0))
			{
				// Если нет возможности установить соединение с сервером
				// базы данных выводим сообщение об ошибке
				SetWindowTextA(static1, LPCSTR("Connection error"));
			}
			else
			{
				// Если соединение успешно установлено выводим фразу - "Success!"
				SetWindowTextA(static1, LPCSTR("Connected"));
			}
		}

		if (wParam == BTN_WRITEDB) //If user clicked on write button
		{
			bool done = false;
			int attempt = 0;

			int ln, rn;
			WCHAR ln_s[256];  //reading lines and rows numbers
			GetWindowText(edit1, ln_s, 256);
			ln = _wtoi(ln_s);
			WCHAR rn_s[256];
			GetWindowText(edit2, rn_s, 256);
			rn = _wtoi(rn_s);

			string requestblock = ""; //блок запроса
			string request = "";	//формулировка запроса

			while (!done)
			{
				done = true;
				requestblock = "";
				//формируем блок запроса
				for (int i = 0; i < ln; i++)
				{
					requestblock += "id_" + to_string(i) + " INT";
					if (i != ln - 1) { requestblock += ", "; }
				}
				request = "CREATE TABLE test_table(" + requestblock + ")";

				//порождаем таблицу на ln столбцов
				qr = mysql_query(conn, request.c_str());

				if (qr != 0) {
					mysql_query(conn, "DROP TABLE test_table"); //если не удалось - удаляем и порождаем
					done = false;
					attempt++;
				}

				if (attempt > 10) { done = true; } //аварийнный выход из цикла

			}


			//формируем таблицу из рядов
			for (int i = 0; i < rn; i++)
			{

				requestblock = "";
				//формируем блок запроса для строки
				for (int j = 0; j < ln; j++)
				{
					requestblock += to_string(rand() % 100);
					if (j != ln - 1) { requestblock += ", "; }
				}

				//отправляем запрос
				request = "INSERT INTO test_table VALUES(" + requestblock + ")";
				qr = mysql_query(conn, request.c_str());
				if (qr == 0) {
					SetWindowTextA(edit3, LPCSTR("Database written"));
				}
				else
				{
					SetWindowTextA(edit3, LPCSTR("Database writing with error"));
				}
			}
		}

		if (wParam == BTN_READDB) //user clicked red button
		{

			stringstream finoutp;
			//делаем запрос на содержимое таблицы
			qr = mysql_query(conn, "SELECT * FROM test_table");
			mresult = mysql_store_result(conn);
			if ((mresult == NULL) || (mresult->row_count == 0))
			{
				//если таблица пуста или не существует
				printf("The table is empty or does not exist!\n");
				if (mresult)
					mysql_free_result(mresult);
			}
			else
			{
				//таблица существует и не пуста
				//1. выводим заголовки колонок
				finoutp << left << setw(5) << "";
				while (fld = mysql_fetch_field(mresult))
					//запрашиваем данные по полю из результата
				{
					finoutp << right << setw(5) << setfill(' ') << fld->name;
					finoutp << "|";
				}
				finoutp << "\r\n" << endl;
				//2. выводим данные

				int rn = 0; //инициализируем счетчик рядов
				do
				{
					int coln = mysql_field_count(conn); //выясняем число столбцов
					row = mysql_fetch_row(mresult);

					if (row != NULL)
					{
						finoutp << right << setw(4) << setfill(' ') << to_string(rn);
						finoutp << ")";
						for (int i = 0; i < coln; i++)
						{
							string s;
							if (row[i] != NULL) { s = row[i]; }
							else { s = "nil"; }
							finoutp << right << setw(5) << setfill(' ') << s;
							finoutp << "|";
						}
						finoutp << "\r\n" << endl;

					}
					rn++;
				} while (row);

				string fos = finoutp.str();

				SetWindowTextA(edit3, LPCSTR(fos.c_str()));

				mysql_free_result(mresult);
			}
		}
	}
	return 0;

	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hwnd, &ps);



		FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW + 1));

		EndPaint(hwnd, &ps);
	}
	return 0;

	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}
