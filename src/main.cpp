#include <wx/wx.h>

#include <ChartWindow.h>

class MyApp : public wxApp 
{
    public:
        virtual bool OnInit();
};

class MyFrame : public wxFrame 
{
    public:
        MyFrame(const wxString &title, const wxPoint &pos, const wxSize &size);
    private:
        ChartWindow* myChart;
        void OnExit(wxCommandEvent& event);
        void OnAbout(wxCommandEvent& event);
        void OnRefreshData(wxCommandEvent& event);
        //events, ctrlPanel, chart canvas, 
    
};

enum {
    ID_RefreshData = 1
};

wxIMPLEMENT_APP(MyApp);

bool MyApp::OnInit() 
{
    MyFrame *frame = new MyFrame("Number of TCES Classes up to now", 
                                wxPoint(50, 50), wxDefaultSize);
    frame->Show(true);
    return true;
}

MyFrame::MyFrame(const wxString &title, const wxPoint &pos, const wxSize &size)
    : wxFrame(NULL, wxID_ANY, title, pos, size) 
{
    wxMenuBar* menuBar = new wxMenuBar;
    wxMenu* menuFile = new wxMenu;
    wxMenu* menuHelp = new wxMenu;
    wxMenu* menuSettings = new wxMenu;

    menuBar->Append(menuFile, "&File" );
    menuBar->Append(menuHelp, "&Help" );
    menuBar->Append(menuSettings, "&Settings");

    menuFile->Append(wxID_EXIT);
    menuHelp->Append(wxID_ABOUT);

    menuSettings->Append(ID_RefreshData, "&Refresh Data",
                     "Checks for new UWT courses and displays them");
    SetMenuBar(menuBar);

    CreateStatusBar();
    SetStatusText( "Welcome!" );

    //To attach events to this application.
    Bind(wxEVT_MENU, &MyFrame::OnRefreshData, this, ID_RefreshData);
    Bind(wxEVT_MENU, &MyFrame::OnAbout, this, wxID_ABOUT);
    Bind(wxEVT_MENU, &MyFrame::OnExit, this, wxID_EXIT);

    CreateStatusBar(1);
    SetStatusText("Ready", 0);

    myChart = new ChartWindow(this);

}

void MyFrame::OnExit(wxCommandEvent& event) //File -> Quit
{
    Close( true ); //Ask window to close
}
void MyFrame::OnAbout(wxCommandEvent& event) //Help -> About
{
    wxMessageBox( "This program web scrapes the UWT Time schedule and counts the number of TCES programs from 2007 to now. Click Settings -> Refresh data if you are on the sample graph.",
                  "About this program", wxOK | wxICON_INFORMATION );
    //(message, window title, ???)
}

void MyFrame::OnRefreshData(wxCommandEvent& event) //Settings -> Refresh Data
{
    wxExecuteEnv env;

    wxLogStatus("Checking for new UWT courses and adding them, please wait...");

    int code = wxExecute("PowerShell Set-ExecutionPolicy RemoteSigned -Scope Process; ../.venv/Scripts/activate.ps1; python ../src/RetrieveData.py", wxEXEC_SYNC, NULL, &env);

    wxLogMessage("Finished gathering new data From UWT Time Schedules.");
    wxLogStatus("Process terminated with exit code %d.", code);
    myChart->ReadDatabase("../data/TCEScourses.db");
    myChart->SetUpGraph();

}