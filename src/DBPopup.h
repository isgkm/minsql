#pragma once

#include <wx/wx.h>
#include <mysqlx/xdevapi.h>
#include <functional>

class MainFrame;

class DBPopup : public wxFrame {
private:
	wxTextCtrl* dbname = nullptr;
	std::shared_ptr<mysqlx::Session> m_session;
public:
	std::function<void()> callback;

	DBPopup(std::shared_ptr<mysqlx::Session> temp_session, std::function<void()> cb, const wxString& title = "");

	void HandleCreateDBClick(wxMouseEvent& evt);
};