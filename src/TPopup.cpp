#include "TPopup.h"
#include "Util.h"

TPopup::TPopup(std::shared_ptr<mysqlx::Session> temp_session, std::function<void()> cb, const wxString& active_db, const wxString& title) : wxFrame(nullptr, wxID_ANY, title), m_session(temp_session), callback(cb), m_active_db(active_db) {
	wxPanel* panel = new wxPanel(this, wxID_ANY);

	wxStaticText* table_name_label = new wxStaticText(panel, wxID_ANY, "Table name:", wxPoint(5, 17));
	table_name = new wxTextCtrl(panel, wxID_ANY, "", wxPoint(75, 15));

	wxButton* add_row_btn = new wxButton(panel, wxID_ANY, "Add row", wxPoint(200, 15));

	table = new wxDataViewListCtrl(panel, wxID_ANY, wxPoint(0, 50), wxDefaultSize, wxDV_ROW_LINES | wxDV_HORIZ_RULES | wxDV_VERT_RULES | wxDV_SINGLE);
	table->SetSize(590, 300);

	wxArrayString data_types;

	auto type_list = {
		"INT", "FLOAT", "DOUBLE", "CHAR", "VARCHAR", "TEXT", "NCHAR", "NVARCHAR", "DATETIME", "DATE", "TIME", "YEAR", "TIMESTAMP", "BOOLEAN", "ENUM", "BIGINT", "DECIMAL", "SET", "TINYINT", "SMALLINT", "MEDIUMINT", "JSON", "LONGTEXT", "MEDIUMTEXT", "TINYTEXT", "BIT", "TINYBLOB", "BLOB", "MEDIUMBLOB", "LONGBLOB", "BINARY", "VARBINARY", "GEOMETRY", "GEOMETRYCOLLECTION", "LINESTRING", "MULTILINESTRING", "POINT", "MULTIPOINT", "POLYGON", "MULTIPOLYGON", "REAL"
	};

	for (const auto& type : type_list) {
		data_types.Add(type);
	}

	table->AppendTextColumn("Name", wxDATAVIEW_CELL_EDITABLE);
	table->AppendColumn(new wxDataViewColumn("Type", new wxDataViewChoiceRenderer(data_types), 1, 100));
	table->AppendTextColumn("Length", wxDATAVIEW_CELL_EDITABLE);
	table->AppendToggleColumn("Unsigned", wxDATAVIEW_CELL_ACTIVATABLE, -1, wxALIGN_CENTER);
	table->AppendToggleColumn("Not Null", wxDATAVIEW_CELL_ACTIVATABLE, -1, wxALIGN_CENTER);
	table->AppendToggleColumn("Unique", wxDATAVIEW_CELL_ACTIVATABLE, -1, wxALIGN_CENTER);
	table->AppendTextColumn("Default", wxDATAVIEW_CELL_EDITABLE);

	wxButton* create_table_btn = new wxButton(panel, wxID_ANY, "Create", wxPoint(517, 360));

	add_row_btn->Bind(wxEVT_LEFT_DOWN, &TPopup::HandleAddRowClick, this);

	create_table_btn->Bind(wxEVT_LEFT_DOWN, &TPopup::HandleCreateTableClick, this);

	table->Bind(wxEVT_DATAVIEW_ITEM_CONTEXT_MENU, &TPopup::HandleTableRightClick, this);
}

void TPopup::HandleTableRightClickMenu(wxCommandEvent& evt) {
	wxMessageDialog* dia = new wxMessageDialog(this, "Delete row '" + std::to_string(m_drop_row_index + 1) + "' ?", "minsql", wxYES_NO);
	int a = dia->ShowModal();
	if (a == wxID_YES) {
		table->DeleteItem(m_drop_row_index);
	}
}

void TPopup::HandleTableRightClick(wxDataViewEvent& evt) {
	wxMenu* menu = new wxMenu();

	wxPoint b = evt.GetPosition();
	wxDataViewItem item;
	wxDataViewColumn* colptr = nullptr;
	table->HitTest(b, item, colptr);

	if (item.IsOk()) {
		m_drop_row_index = table->ItemToRow(item);

		menu->Append(wxID_DELETE, "Delete row");
		Bind(wxEVT_MENU, &TPopup::HandleTableRightClickMenu, this, wxID_DELETE);
		menu->SetNextHandler(this);
		table->PopupMenu(menu);
	}
}

void TPopup::HandleAddRowClick(wxMouseEvent& evt) {
	wxVector<wxVariant> vec;
	vec.push_back("id");
	vec.push_back("INT");
	vec.push_back("");
	vec.push_back(false);
	vec.push_back(false);
	vec.push_back(false);
	vec.push_back("");

	table->AppendItem(vec);
	table->SelectRow(table->GetStore()->GetCount() - 1);
}

void TPopup::HandleCreateTableClick(wxMouseEvent& evt) {
	std::vector<std::string> query;
	std::string s;
	for (unsigned int i = 0; i < table->GetItemCount(); ++i) {
		for (unsigned int j = 0; j < table->GetColumnCount(); ++j) {
			wxVariant val;
			table->GetValue(val, i, j);
			wxLogStatus(val.GetString());

			query.push_back(WXSTR_TO_STD(val.GetString()));
		}
		s += query.at(0) + " " + query.at(1) + (query.at(2).length() > 0 ? "(" + query.at(2) + ") " : " ") + (query.at(3) == "0" ? "" : "UNSIGNED ") + (query.at(4) == "0" ? "" : "NOT NULL ") + (query.at(5) == "0" ? "" : "UNIQUE") + (query.at(6).length() > 0 ? "DEFAULT (" + query.at(6) + ")" : "") + (i != table->GetItemCount() - 1 ? ",\n" : "");
		query.clear();
	}
	try {
		m_session->sql("CREATE TABLE `" + WXSTR_TO_STD(m_active_db) + "`.`" + WXSTR_TO_STD(table_name->GetValue()) + "` (" + s + ")").execute();
		this->callback();
		int res = MESSAGE_BOX_INFORMATION("Successfully created table " + table_name->GetValue());
		if (res == wxID_OK) {
			this->Destroy();
		}
	}
	catch (const mysqlx::Error& err) {
		MESSAGE_BOX_WARNING(err.what())
	}
}