#include <wx/wx.h>

#include "App.h"
#include "MainFrame.h"
#include "InitFrame.h"

wxIMPLEMENT_APP(App);

bool App::OnInit() {
	InitFrame* initFrame = new InitFrame();
	initFrame->SetClientSize(300, 300);
	initFrame->SetMaxSize(wxSize(300, 300));
	initFrame->SetMinSize(wxSize(300, 300));
	initFrame->Center();
	initFrame->Show();
	
	return true;
}