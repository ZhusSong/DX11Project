//--------------------------------------------------------------------------------------
// File: Mouse.h
//
// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.
//
// http://go.microsoft.com/fwlink/?LinkId=248929
// http://go.microsoft.com/fwlink/?LinkID=615561
//--------------------------------------------------------------------------------------

#pragma once

#include<windows.h>
#include<memory>

namespace DirectX
{
	class Mouse
	{
	public:
		Mouse() noexcept(false);
		Mouse(Mouse&& moveFrom) noexcept;
		Mouse& operator= (Mouse&& moveFrom) noexcept;

		Mouse(Mouse const&) = delete;
		Mouse& operator=(Mouse const&) = delete;

		virtual ~Mouse();

		enum Mode
		{
			//绝对坐标模式，每次状态更新xy值为屏幕像素坐标，且鼠标可见
			MODE_ABSOLUTE = 0,
			//相对运动模式，每次状态更新xy值为每一帧之间的像素位移量，且鼠标不可见
			MODE_RELATIVE,

		};
		struct State
		{
			//鼠标左键
			bool    leftButton;
			//鼠标中键
			bool    middleButton;
			//鼠标右键
			bool    rightButton;
			//自定义按键1
			bool    xButton1;
			//自定义按键2
			bool    xButton2;
			//绝对坐标x或相对偏移量
			int     x;
			//绝对坐标y或相对偏移量
			int     y;
			//鼠标滚轮
			int     scrollWheelValue;
			//鼠标模式
			Mode    positionMode;
		};

		class ButtonStateTracker
		{
		public:
			enum ButtonState
			{ 
				// Button is up
				//按钮未被按下
				UP = 0,        
				// Button is held down
				//按钮长按中
				HELD = 1,       
				// Button was just released
				//按钮刚被放开
				RELEASED = 2,  
				// Buton was just pressed
				//按钮刚被按下
				PRESSED = 3,    
			};
			ButtonState leftButton;
			ButtonState middleButton;
			ButtonState rightButton;
			ButtonState xButton1;
			ButtonState xButton2;
			//获取上一帧的鼠标状态，应于Update前调用，否则为获取当前状态
			State __cdecl GetLastState() const { return lastState; }
			//每一帧提供鼠标当前状态并更新
			void __cdecl Update(const State& state);
			void __cdecl Reset() noexcept;

#pragma prefast(suppress: 26495, "Reset() performs the initialization")
			ButtonStateTracker() noexcept { Reset(); }
			

		private:
			State lastState;
		};
		// Retrieve the current state of the mouse
		//读取鼠标当前状态
		State __cdecl GetState() const;

		// Resets the accumulated scroll wheel value
		//重置鼠标滚轮的累计值
		void __cdecl ResetScrollWheelValue();

		// Sets mouse mode (defaults to absolute)
		//设置鼠标模式(默认为absolute状态)
		void __cdecl SetMode(Mode mode);

		// Feature detection
		//检测鼠标是否连接
		bool __cdecl IsConnected() const;

		// Cursor visibility
		//设置鼠标光标可见度
		bool __cdecl IsVisible() const;
		void __cdecl SetVisible(bool visible);

#if !defined(WINAPI_FAMILY) || (WINAPI_FAMILY == WINAPI_FAMILY_DESKTOP_APP) && defined(WM_USER)
		//为鼠标设置需要绑定的窗口句柄
		void __cdecl SetWindow(HWND window);
		static void __cdecl ProcessMessage(UINT message, WPARAM wParam, LPARAM lParam);
#endif
		// Singleton
		static Mouse& __cdecl Get();

	private:
		// Private implementation.
		class Impl;

		std::unique_ptr<Impl> pImpl;

	};
}