#pragma once

#include <wx/wx.h>
#include <wx/dataview.h>
#include <functional>
#include <mysqlx/xdevapi.h>

class TPopup : public wxFrame {
private:
	std::shared_ptr<mysqlx::Session> m_session;

	wxTextCtrl* table_name = nullptr;
	wxDataViewListCtrl* table = nullptr;

	int m_drop_row_index;
public:
	wxString m_active_db;
	std::function<void()> callback;

	TPopup(std::shared_ptr<mysqlx::Session> temp_session, std::function<void()> cb, const wxString& active_db, const wxString& title = "");

	void HandleAddRowClick(wxMouseEvent& evt);
	void HandleCreateTableClick(wxMouseEvent& evt);

	void HandleTableRightClick(wxDataViewEvent& evt);
	void HandleTableRightClickMenu(wxCommandEvent& evt);
};