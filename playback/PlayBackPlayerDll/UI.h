#ifndef UI_H

#define UI_H

#include "..\..\logic\LocalUIDll\BasicWindow.h"
#include <array>
#include <memory>
#include <string>

class BasicModelessDialog
{
protected:

	HWND m_hDlg = NULL;
	HINSTANCE m_hInst = NULL;
	BOOL Init(HINSTANCE hInstance, LPCTSTR c_lpszTemplateName, HWND hWndParent);
	virtual bool MessageProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) = 0;

	bool IsButtonCheck(int buttonID) const
	{
		return SendMessage(GetDlgItem(m_hDlg, buttonID), BM_GETCHECK, 0, 0) == BST_CHECKED;
	}

private:

	static INT_PTR CALLBACK DlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
};

class UI : public BasicWindow
{
public:

	int Begin(System::String^ FinitialFileName);
	UI();

private:

	virtual bool MessageProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) override;

	int width = 0;
	int height = 0;
	POINT pixelPerCell;
	int appendCx = 0;
	int appendCy = 0;
	int basicSize = 600;

	HBITMAP hBmBkGnd = NULL;
	BITMAP bkGnd;

	long long player1ID = 0;
	long long player2ID = 0;

	std::wstring m_initialFileName;

public:

	value class MessageReaderWrapper
	{
	public:
		playback::MessageReader^ MessageReader() { return messageReader; }
		MessageReaderWrapper(System::String^ fileName);
		Communication::Proto::MessageToClient^ recentMsg;
	private:
		playback::MessageReader^ messageReader;
	};

	value class MonitorWrapper
	{
	public:
		void Init() { if (lockObj == nullptr) lockObj = gcnew System::Object(); }
		void Lock() { System::Threading::Monitor::Enter(lockObj); }
		void Unlock() { System::Threading::Monitor::Exit(lockObj); }

	private:
		System::Object^ lockObj;
	};

private:

	MessageReaderWrapper* volatile pMR = nullptr;
	volatile bool finishConstruct = false;
	volatile bool successConstruct = false;
	void GetPMR(System::String^ fileName, std::shared_ptr<volatile bool> messager);
	std::array<int, 8> teamScores;

	void ChooseFile(bool first = false);
	std::shared_ptr<volatile bool> msgParserMessager = std::make_shared<volatile bool>(false);

	MonitorWrapper* volatile pauseLock = nullptr;

	class ChooseFileDialog : BasicModelessDialog
	{
	public:
		virtual bool MessageProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) override;
		int TimeInterval() const { return timeInterval; }
		BOOL Begin(HINSTANCE hInst, LPCTSTR hTempName, HWND hWndParent);
		HWND GetHangle() const { return m_hDlg; }
	private:
		int timeInterval = 50;
	};

	ChooseFileDialog chooseFileDlg;

};

#endif
