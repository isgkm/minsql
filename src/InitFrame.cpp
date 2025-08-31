#include <wx/textctrl.h>
#include <wx/valnum.h>

#include "InitFrame.h"
#include "MainFrame.h"

InitFrame::InitFrame() : wxFrame(nullptr, wxID_ANY, "Connect to MySQL") {
  wxPanel *panel =
      new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize);

  wxStaticText *host =
      new wxStaticText(panel, wxID_ANY, "Host", wxPoint(90, 5), wxDefaultSize);
  m_host_input = new wxTextCtrl(panel, wxID_ANY, "",
                                wxPoint(90, 25));

  wxStaticText *port =
      new wxStaticText(panel, wxID_ANY, "Port", wxPoint(90, 55));
  m_port_input =
      new wxTextCtrl(panel, wxID_ANY, "", wxPoint(90, 75),
                     wxDefaultSize, 0L, wxIntegerValidator<int>());

  wxStaticText *username =
      new wxStaticText(panel, wxID_ANY, "Username", wxPoint(90, 105));
  m_username_input =
      new wxTextCtrl(panel, wxID_ANY, "", wxPoint(90, 125));

  wxStaticText *password =
      new wxStaticText(panel, wxID_ANY, "Password", wxPoint(90, 155));
  m_password_input =
      new wxTextCtrl(panel, wxID_ANY, "",
                     wxPoint(90, 175), wxDefaultSize, wxTE_PASSWORD);

  wxButton *connect_button =
      new wxButton(panel, wxID_ANY, "Connect", wxPoint(105, 220), wxDefaultSize,
                   wxWANTS_CHARS);

  connect_button->Bind(wxEVT_CHAR_HOOK, &InitFrame::OnKeyEvent, this);
  connect_button->Bind(wxEVT_LEFT_DOWN, &InitFrame::OnButtonClicked, this);
}

void InitFrame::ConnectToMysql() {
  if (!m_host_input->IsEmpty() && !m_port_input->IsEmpty() &&
      !m_username_input->IsEmpty()) {
    try {
      using mysqlx::SessionOption;

      std::shared_ptr<mysqlx::Session> _temp =
          std::make_unique<mysqlx::Session>(
              SessionOption::HOST,
              static_cast<std::string>(m_host_input->GetValue()),
              SessionOption::PORT, wxAtoi(m_port_input->GetValue()),
              SessionOption::USER,
              static_cast<std::string>(m_username_input->GetValue()),
              SessionOption::PWD,
              static_cast<std::string>(m_password_input->GetValue()));

      if (_temp) {
        MainFrame *frame = new MainFrame(std::move(_temp));
        frame->SetSize(850, 650);
        frame->Center();
        frame->Show();
        this->Destroy();
      }
    } catch (const mysqlx::Error &err) {
      int a = wxMessageBox(static_cast<std::string>(err.what()),
                           "Couldn't connect to MySQL Server",
                           wxOK | wxICON_WARNING, this);
    }
  }
}

void InitFrame::OnButtonClicked(wxMouseEvent &evt) { this->ConnectToMysql(); }

void InitFrame::OnKeyEvent(wxKeyEvent &evt) {
  wxChar c = evt.GetUnicodeKey();

  if (c == WXK_RETURN) {
    this->ConnectToMysql();
  }
}