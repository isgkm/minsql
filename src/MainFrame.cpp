#include <wx/wx.h>
#include <wx/msgdlg.h>
#include <wx/menu.h>
#include <wx/listctrl.h>
#include <wx/dataview.h>
#include <wx/clipbrd.h>

#include <mysqlx/xdevapi.h>
#include <mysqlx/devapi/result.h>

#include <cstdint>
#include <format>
#include <string>

#include "MainFrame.h"
#include "DBPopup.h"
#include "TPopup.h"
#include "Util.h"

using namespace mysqlx;

MainFrame::MainFrame(std::shared_ptr<mysqlx::Session> temp_session) : wxFrame(nullptr, wxID_ANY, "minsql"), m_session(std::move(temp_session)) {
	wxPanel* panel = new wxPanel(this, wxID_ANY);
	wxFont font = wxFont{ 11, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD };

	wxStaticText* dbs_text = new wxStaticText(panel, wxID_ANY, "DBs", wxPoint(0, 0), wxSize(150, 10), wxALIGN_CENTER_HORIZONTAL);
	dbs_text->SetFont(font);
	dbs_listbox = new wxListBox(panel, wxID_ANY, wxPoint(0, 20), wxSize(150, 200), wxArrayString{}, wxLB_HSCROLL);

	tables_text = new wxStaticText(panel, wxID_ANY, "Tables", wxPoint(155, 0), wxSize(150, 10), wxALIGN_CENTER_HORIZONTAL);
	tables_text->SetFont(font);
	tables_listbox = new wxListBox(panel, wxID_ANY, wxPoint(150, 20), wxSize(150, 200), wxArrayString{}, wxLB_HSCROLL);
	tables_text->Hide();
	tables_listbox->Hide();

	display_current_table = new wxStaticText(panel, wxID_ANY, "", wxPoint(0, 225), wxSize(800, 10), wxALIGN_CENTER_HORIZONTAL);
	display_current_table->SetFont(font);

	results_structure_listbox = new wxDataViewListCtrl(panel, wxID_ANY, wxPoint(0, 250), wxDefaultSize, wxDV_ROW_LINES | wxDV_HORIZ_RULES | wxDV_VERT_RULES | wxDV_SINGLE);
	results_structure_listbox->SetSize(wxSize(800, 300));

	results_data_listbox = new wxDataViewListCtrl(panel, wxID_ANY, wxPoint(0, 250), wxDefaultSize, wxDV_ROW_LINES | wxDV_HORIZ_RULES | wxDV_VERT_RULES | wxDV_SINGLE);
	results_data_listbox->SetSize(wxSize(800, 300));
	results_data_listbox->Hide();

	rb_structure = new wxRadioButton(panel, wxID_ANY, "Structure", wxPoint(310, 20));
	rb_data = new wxRadioButton(panel, wxID_ANY, "Data", wxPoint(390, 20));
	rb_structure->SetValue(true);
	rb_structure->Hide();
	rb_data->Hide();

	add_column_to_data_listbox_btn = new wxButton(panel, wxID_ANY, "Add row", wxPoint(310, 40), wxSize(125, -1));
	add_column_to_data_listbox_btn->Hide();

	m_active_structure_data_listbox = results_structure_listbox;

	RefreshDBList();

	dbs_listbox->Bind(wxEVT_LISTBOX_DCLICK, &MainFrame::HandleDBsListboxDBLClick, this);
	dbs_listbox->Bind(wxEVT_RIGHT_DOWN, &MainFrame::SetDropDbIndex, this);
	dbs_listbox->Bind(wxEVT_CONTEXT_MENU, &MainFrame::OnDBsContextMenu, this);

	tables_listbox->Bind(wxEVT_LISTBOX_DCLICK, &MainFrame::HandleTablesListboxDBLClick, this);
	tables_listbox->Bind(wxEVT_RIGHT_DOWN, &MainFrame::SetDropTableIndex, this);
	tables_listbox->Bind(wxEVT_CONTEXT_MENU, &MainFrame::OnTablesContextMenu, this);

	results_structure_listbox->Bind(wxEVT_DATAVIEW_ITEM_CONTEXT_MENU, &MainFrame::HandleResultsContextMenu, this);
	results_structure_listbox->Bind(wxEVT_DATAVIEW_ITEM_START_EDITING, &MainFrame::HandleResultsEditStart, this);
	results_structure_listbox->Bind(wxEVT_DATAVIEW_ITEM_EDITING_DONE, &MainFrame::HandleResultsEditEnd, this);

	results_data_listbox->Bind(wxEVT_DATAVIEW_ITEM_CONTEXT_MENU, &MainFrame::HandleResultsContextMenu, this);
	results_data_listbox->Bind(wxEVT_DATAVIEW_ITEM_START_EDITING, &MainFrame::HandleResultsEditStart, this);
	results_data_listbox->Bind(wxEVT_DATAVIEW_ITEM_EDITING_DONE, &MainFrame::HandleResultsEditEnd, this);

	add_column_to_data_listbox_btn->Bind(wxEVT_LEFT_DOWN, &MainFrame::HandleAddRowToResultsDataListbox, this);

	rb_structure->Bind(wxEVT_RADIOBUTTON, &MainFrame::HandleResultsStructureRB, this);
	rb_data->Bind(wxEVT_RADIOBUTTON, &MainFrame::HandleResultsDataRB, this);

	CreateStatusBar();
}

void MainFrame::HandleResultsStructureRB(wxCommandEvent& WXUNUSED(evt)) {
	if (results_data_listbox->IsShown()) {
		results_data_listbox->Hide();

		results_structure_listbox->Show();
		add_column_to_data_listbox_btn->Hide();
		
		m_active_structure_data_listbox = results_structure_listbox;
	}
}

void MainFrame::HandleResultsDataRB(wxCommandEvent& WXUNUSED(evt)) {
	if (results_structure_listbox->IsShown()) {
		results_structure_listbox->Hide();

		results_data_listbox->Show();
		add_column_to_data_listbox_btn->Show();

		m_active_structure_data_listbox = results_data_listbox;
	}
}

void MainFrame::HandleAddRowToResultsDataListbox(wxMouseEvent& evt) {
	return;
}

void MainFrame::HandleResultsEditStart(wxDataViewEvent& WXUNUSED(evt)) {
	wxDataViewItem item;
	wxDataViewColumn* col = nullptr;

	m_active_structure_data_listbox->HitTest(m_active_structure_data_listbox->ScreenToClient(::wxGetMousePosition()), item, col);
	int row = m_active_structure_data_listbox->ItemToRow(item);

	wxDataViewListStore* store = m_active_structure_data_listbox->GetStore();
	wxVariant value;
	store->GetValueByRow(value, row, col->GetModelColumn());

	m_editing_start = value.GetString();
}

void MainFrame::HandleResultsEditEnd(wxDataViewEvent& evt) {
	wxVariant v = evt.GetValue();
	int temp_col_index = evt.GetColumn();
	wxDataViewColumn* col = m_active_structure_data_listbox->GetColumn(temp_col_index);
}

void MainFrame::OnDeleteRowClicked(wxCommandEvent& evt) {
	return;
}

void MainFrame::OnAddRowClicked(wxCommandEvent& evt) {
	return;
}

void MainFrame::OnCopyItemClicked(wxCommandEvent& evt) {
	if (wxTheClipboard->Open()) {
		wxTheClipboard->SetData(new wxTextDataObject(m_results_structure_rclick_item.GetString()));
		wxTheClipboard->Close();
	}
}

void MainFrame::OnCopyRowClicked(wxCommandEvent& evt) {
	wxDataViewItemArray items;
	m_active_structure_data_listbox->GetSelections(items);
	wxString result;

	for (size_t i = 0; i < items.GetCount(); ++i) {
		wxDataViewItem ite = items[i];
		int row = m_active_structure_data_listbox->ItemToRow(ite);

		for (int j = 0; j < m_active_structure_data_listbox->GetColumnCount(); ++j) {
			wxVariant variant;
			m_active_structure_data_listbox->GetValue(variant, row, j);

			result += variant.GetString() + " ";
		}
	}

	if (wxTheClipboard->Open()) {
		wxTheClipboard->SetData(new wxTextDataObject(result));
		wxTheClipboard->Close();
	}
}

void MainFrame::HandleResultsContextMenu(wxDataViewEvent& evt) {
	wxPoint b = evt.GetPosition();
	wxDataViewItem item;
	wxDataViewColumn* colptr = nullptr;
	m_active_structure_data_listbox->HitTest(b, item, colptr);

	wxMenu* menu = new wxMenu();

	if (item.IsOk()) {
		int row = m_active_structure_data_listbox->ItemToRow(item);
		int col = colptr->GetModelColumn();

		wxDataViewListStore* store = m_active_structure_data_listbox->GetStore();

		store->GetValueByRow(m_results_structure_rclick_item, row, col);

		menu->Append(wxID_DELETE, "Delete row");
		Bind(wxEVT_MENU, &MainFrame::OnDeleteRowClicked, this, wxID_DELETE);

		menu->Append(wxID_COPY, "Copy item");
		Bind(wxEVT_MENU, &MainFrame::OnCopyItemClicked, this, wxID_COPY);

		menu->Append(wxID_INFO, "Copy row");
		Bind(wxEVT_MENU, &MainFrame::OnCopyRowClicked, this, wxID_INFO);
	}
	else {
		menu->Append(wxID_NEW, "Add row");
		Bind(wxEVT_MENU, &MainFrame::OnAddRowClicked, this, wxID_NEW);
	}

	menu->SetNextHandler(this);
	m_active_structure_data_listbox->PopupMenu(menu);
}

void MainFrame::HandleTablesListboxDBLClick(wxCommandEvent& evt) {
	results_structure_listbox->ClearColumns();
	results_structure_listbox->DeleteAllItems();
	results_data_listbox->ClearColumns();
	results_data_listbox->DeleteAllItems();

	m_active_table = evt.GetString();
	display_current_table->SetLabel(m_active_db + "." + m_active_table);

	SqlResult z = m_session->sql("SHOW COLUMNS FROM `" + WXSTR_TO_STD(m_active_table) + "` FROM `" + WXSTR_TO_STD(m_active_db) + "`").execute();

	const RowResult::Columns& structure_cols = z.getColumns();

	for (auto& co : structure_cols) {
		results_structure_listbox->AppendTextColumn(static_cast<std::string>(co.getColumnLabel()), wxDATAVIEW_CELL_EDITABLE, -1, wxALIGN_CENTER, wxCOL_SORTABLE | wxCOL_RESIZABLE);
	}

	wxVector<wxVariant> data;

	for (const mysqlx::Row& row : z.fetchAll()) {
		for (mysqlx::col_count_t i = 0; i < row.colCount(); ++i) {
			if (!row[i].isNull()) {
				data.push_back(wxVariant(WXSTR_TO_STD(row[i])));
			}
			else {
				data.push_back(wxVariant("NULL"));
			}
		}
		results_structure_listbox->AppendItem(data);
		data.clear();
	}
	data.clear();

	SqlResult res = m_session->sql("SELECT * FROM `" + WXSTR_TO_STD(m_active_db) + "`.`" + WXSTR_TO_STD(m_active_table) + "`").execute();

	const RowResult::Columns& data_cols = res.getColumns();

	for (auto& col : data_cols) {
		if (col.getType() == mysqlx::Type::TINYINT) {
			results_data_listbox->AppendToggleColumn(static_cast<std::string>(col.getColumnLabel()), wxDATAVIEW_CELL_ACTIVATABLE, -1, wxALIGN_LEFT, wxCOL_SORTABLE | wxCOL_RESIZABLE);
		}
		else {
			results_data_listbox->AppendTextColumn(static_cast<std::string>(col.getColumnLabel()), wxDATAVIEW_CELL_EDITABLE, -1, wxALIGN_LEFT, wxCOL_SORTABLE | wxCOL_RESIZABLE);
		}
		m_types_map.insert(std::pair<std::string, mysqlx::Type>(col.getColumnLabel(), col.getType()));
	}

	for (const mysqlx::Row& row : res.fetchAll()) {
		for (mysqlx::col_count_t i = 0; i < row.colCount(); ++i) {
			if (!row[i].isNull()) {
				switch (row[i].getType())
				{
				case mysqlx::Value::Type::INT64:
					data.push_back(wxVariant(std::to_string(row[i].get<int64_t>())));
					break;
				case mysqlx::Value::Type::UINT64:
					data.push_back(wxVariant(std::to_string(row[i].get<uint64_t>())));
					break;
				case mysqlx::Value::Type::RAW: {
					auto time_point = Util::read_date_time(row[i]);
					std::string s = std::format("{:%F %T}", time_point);

					data.push_back(wxVariant(s));
					break;
				}
				default:
					data.push_back(wxVariant(WXSTR_TO_STD(row[i])));
					break;
				}
			}
			else {
				data.push_back(wxVariant("NULL"));
			}
		}

		results_data_listbox->AppendItem(data);
		data.clear();
	}

	rb_structure->Show();
	rb_data->Show();
}

void MainFrame::RefreshTablesList() {
	Schema sch = m_session->getSchema(static_cast<std::string>(m_active_db));

	tables_listbox->Clear();

	std::list<mysqlx::Table> res = sch.getTables();
	for (auto& r : res) {
		if (!r.isView()){
			tables_listbox->Append(WXSTR_TO_STD(r.getName()));
		}
	}
}

void MainFrame::HandleDBsListboxDBLClick(wxCommandEvent& evt) {
	tables_listbox->Clear();

	m_active_db = evt.GetString();
	tables_text->Show();
	tables_listbox->Show();

	RefreshTablesList();
}

bool filterDBS(const std::string& name) {
	return (name == "sys") || (name == "mysql") || (name == "performance_schema") || (name == "information_schema");
}

void MainFrame::RefreshDBList() {
	std::list<mysqlx::Schema> dbs = m_session->getSchemas();

	dbs_listbox->Clear();

	for (const mysqlx::Schema& schema : dbs) {
		if (!filterDBS(schema.getName())){
			dbs_listbox->Append(static_cast<std::string>(schema.getName()));
		}
	}
}

void MainFrame::SetDropDbIndex(wxMouseEvent& evt) {
	int item = dbs_listbox->HitTest(evt.GetPosition());
	dbs_listbox->SetSelection(item);

	m_drop_db_index = item;

	evt.Skip();
}

void MainFrame::OnCreateDBClicked(wxCommandEvent& evt) {
	DBPopup* popup = new DBPopup(m_session, std::bind(&MainFrame::RefreshDBList, this), "Create Database");
	popup->SetMinSize(wxSize(300, 100));
	popup->SetMaxSize(wxSize(300, 100));
	popup->Center();
	popup->Show();
}

void MainFrame::OnDropDBClicked(wxCommandEvent& evt) {
	wxString to_drop = dbs_listbox->GetString(m_drop_db_index);
	wxMessageDialog* dia = new wxMessageDialog(this, "Drop database '" + to_drop + "' ?", "minsql", wxYES_NO);
	int a = dia->ShowModal();
	if (a == wxID_YES) {
		m_session->dropSchema(static_cast<std::string>(to_drop));
		RefreshDBList();
	}
}

void MainFrame::OnRefreshDBClicked(wxCommandEvent& evt) {
	RefreshDBList();
}

void MainFrame::OnDBsContextMenu(wxContextMenuEvent& evt) {
	wxMenu* menu = new wxMenu();

	if (m_drop_db_index != wxNOT_FOUND) {
		menu->Append(wxID_DELETE, "Drop Database");
		Bind(wxEVT_MENU, &MainFrame::OnDropDBClicked, this, wxID_DELETE);
	}
	else {
		menu->Append(wxID_NEW, "Create Database");
		menu->Append(wxID_REFRESH, "Refresh");

		Bind(wxEVT_MENU, &MainFrame::OnCreateDBClicked, this, wxID_NEW);
		Bind(wxEVT_MENU, &MainFrame::OnRefreshDBClicked, this, wxID_REFRESH);
	}

	menu->SetNextHandler(this);

	dbs_listbox->PopupMenu(menu, dbs_listbox->ScreenToClient(evt.GetPosition()));
}

void MainFrame::SetDropTableIndex(wxMouseEvent& evt) {
	int item = tables_listbox->HitTest(evt.GetPosition());
	tables_listbox->SetSelection(item);

	m_drop_table_index = item;

	evt.Skip();
}

void MainFrame::OnCreateTableClicked(wxCommandEvent& evt) {
	TPopup* popup = new TPopup(m_session, std::bind(&MainFrame::RefreshTablesList, this), m_active_db, "Create Table");
	popup->SetMinSize(wxSize(635, 435));
	popup->SetMaxSize(wxSize(635, 435));
	popup->Center();
	popup->Show();
}

void MainFrame::OnDropTableClicked(wxCommandEvent& evt) {
	wxString to_drop = tables_listbox->GetString(m_drop_table_index);
	wxMessageDialog* dia = new wxMessageDialog(this, "Drop table '" + to_drop + "' ?", "minsql", wxYES_NO);
	int a = dia->ShowModal();
	if (a == wxID_YES) {
		m_session->sql("DROP TABLE `" + WXSTR_TO_STD(m_active_db) + "`.`" + WXSTR_TO_STD(to_drop) + "`").execute();
		RefreshTablesList();
	}
}

void MainFrame::OnRefreshTableClicked(wxCommandEvent& evt) {
	RefreshTablesList();
}

void MainFrame::OnTablesContextMenu(wxContextMenuEvent& evt) {
	wxMenu* menu = new wxMenu();

	if (m_drop_table_index != wxNOT_FOUND) {
		menu->Append(wxID_DELETE, "Drop table");
		Bind(wxEVT_MENU, &MainFrame::OnDropTableClicked, this, wxID_DELETE);
	}
	else {
		menu->Append(wxID_NEW, "Create table");
		menu->Append(wxID_REFRESH, "Refresh");

		Bind(wxEVT_MENU, &MainFrame::OnCreateTableClicked, this, wxID_NEW);
		Bind(wxEVT_MENU, &MainFrame::OnRefreshTableClicked, this, wxID_REFRESH);
	}

	menu->SetNextHandler(this);

	tables_listbox->PopupMenu(menu, tables_listbox->ScreenToClient(evt.GetPosition()));
}