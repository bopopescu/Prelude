// Copyright (c) 2018 The Prelude Authors.
// Use of this source code is governed by MIT license that can be
// found in MIT-LICENSE file.
#define STRICT
#include <windows.h>
#include <commctrl.h>
#include <ole2.h>

#include "base/at_exit.h"
#include "base/bind.h"
#include "base/location.h"
#include "base/message_loop/message_loop.h"
#include "base/run_loop.h"
#include <stdio.h>

// this sample shows how to do the same as in hello_dialog, but with chromium
// message loop

namespace {
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam,
                            LPARAM lParam) {
  switch (uMsg) {
  case WM_DESTROY: {
    PostQuitMessage(0);
    break;
  }
  case WM_PAINT: {
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hwnd, &ps);

    FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW + 1));
    DrawText(hdc, TEXT("Pumped by message loop"), -1, &ps.rcPaint,
             DT_SINGLELINE | DT_CENTER | DT_VCENTER);

    EndPaint(hwnd, &ps);
    break;
  }
  case WM_SIZE: {
    InvalidateRect(hwnd, NULL, FALSE);
    break;
  }
    return 0;
  }
  return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void CreateAndShowWindow() {
  // Register the window class.
  const wchar_t CLASS_NAME[] = L"Sample Window Class";

  WNDCLASS wc = {};
  HINSTANCE hInstance = GetModuleHandle(NULL);
  wc.lpfnWndProc = WindowProc;
  wc.hInstance = hInstance;
  wc.lpszClassName = CLASS_NAME;

  if (!RegisterClass(&wc))
    return;

  InitCommonControls();
  // Create the window.
  HWND hwnd = CreateWindowEx(0,          // Optional window styles.
                             CLASS_NAME, // Window class
                             L"Learn to Program Windows", // Window text
                             WS_OVERLAPPEDWINDOW,         // Window style

                             // Size and position
                             CW_USEDEFAULT, CW_USEDEFAULT, 600, 400,

                             NULL,      // Parent window
                             NULL,      // Menu
                             hInstance, // Instance handle
                             NULL       // Additional application data
  );

  if (hwnd == NULL) {
    return;
  }
  ShowWindow(hwnd, SW_SHOW);
}

} // namespace

int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR pCmdLine,
                      int nCmdShow) {
  // The exit manager is in charge of calling the dtors of singleton objects.
  base::AtExitManager exit_manager;
  if (FAILED(CoInitialize(NULL)))
    return 1;

  // a default message loop is not enough,
  // use a ui message loop. all the work is encapsulated into tasks and posted
  // instead of directly done here. These tasks will be performed later in the
  // main thread.
  base::MessageLoopForUI main_loop;
  base::RunLoop run_loop;
  main_loop.task_runner()->PostTask(FROM_HERE,
                                    base::BindOnce(&CreateAndShowWindow));

  run_loop.Run();

  CoUninitialize();
  return 0;
}