#pragma once

#include <wx/wx.h>

class InitFrame : public wxFrame {
private:
	wxTextCtrl* m_host_input = nullptr;
	wxTextCtrl* m_port_input = nullptr;
	wxTextCtrl* m_username_input = nullptr;
	wxTextCtrl* m_password_input = nullptr;
public:
	InitFrame();
	void ConnectToMysql();
	void OnButtonClicked(wxMouseEvent& evt);
	void OnKeyEvent(wxKeyEvent& evt);
};