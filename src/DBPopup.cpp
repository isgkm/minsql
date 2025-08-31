#include "DBPopup.h"

DBPopup::DBPopup(std::shared_ptr<mysqlx::Session> temp_session, std::function<void()> cb, const wxString& title) : wxFrame(nullptr, wxID_ANY, title), m_session(temp_session), callback(cb) {
	wxPanel* panel = new wxPanel(this, wxID_ANY);
	dbname = new wxTextCtrl(panel, wxID_ANY, "", wxPoint(25, 20), wxDefaultSize);
	wxButton* create_db = new wxButton(panel, wxID_ANY, "Create", wxPoint(175, 20));

	create_db->Bind(wxEVT_LEFT_DOWN, &DBPopup::HandleCreateDBClick, this);
}

void DBPopup::HandleCreateDBClick(wxMouseEvent& evt) {
	m_session->createSchema(static_cast<std::string>(dbname->GetValue()));
	this->callback();
	this->Destroy();
}