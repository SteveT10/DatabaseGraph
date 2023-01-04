#include <ChartWindow.h>
#include <wx/sizer.h>
#include <sqlite3.h>
#include <iostream>
#include <map>
#include <cstring>
#include <iostream>

ChartWindow::ChartWindow(wxWindow *parent)
                        : wxScrolled<wxWindow>(parent, wxID_ANY) {
    
    //Children:
    myValueCol = new ValueColumn(this, wxID_ANY, wxDefaultPosition, wxSize(60, 0));
    myCanvas = new ChartCanvas(this, wxID_ANY, myValueCol);

    //Retrieving Value from DB
    ReadDatabase("../data/TCEScourses.db");
    SetUpGraph();

    //Organizing children
    wxFlexGridSizer *mainSizer = new wxFlexGridSizer(2, 0, 0);
    mainSizer->Add(myValueCol, wxSizerFlags().Expand());
    mainSizer->Add(myCanvas, wxSizerFlags().Expand());
    mainSizer->AddGrowableRow(0);
    mainSizer->AddGrowableCol(1);
    SetSizer(mainSizer);

    //Setting up labels
    wxFont titleFont = wxFont(wxNORMAL_FONT->GetPointSize() * 2.0,
                                  wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL,
                                wxFONTWEIGHT_BOLD);
    wxFont textFont = wxFont(wxNORMAL_FONT->GetPointSize(),
                                  wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL,
                                wxFONTWEIGHT_BOLD);
                                
    myCanvas->SetTitleFont(titleFont);
    myCanvas->SetNormalFont(textFont);
    myValueCol->SetTitleFont(titleFont); //To account for text margins
    myValueCol->SetNormalFont(textFont);

    //Events
    Bind(wxEVT_SIZE, &ChartWindow::OnSize, this);

}

std::tuple<int, double, double> ChartWindow::CalcChartHlines(
                                            double origLow, double origHigh) 
{
    constexpr double rangeMults[] = {0.2, 0.25, 0.5, 1.0, 2.0, 2.5, 5.0};
    constexpr int maxSegments = 6;

    double magnitude = std::floor(std::log10(origHigh - origLow));
    
    for (auto r : rangeMults)
    {
        double stepSize = r * std::pow(10.0, magnitude);
        double low = std::floor(origLow / 10) * stepSize;
        double high = std::ceil(origHigh / stepSize) * stepSize;

        int segments = round((high - low) / stepSize);

        if (segments <= maxSegments)
        {
            return std::make_tuple(segments, low, high);
        }
    }

    // return some defaults in case rangeMults and maxSegments are mismatched
    return std::make_tuple(10, origLow, origHigh); 
}

wxSize ChartWindow::GetSizeAvailableForScrollTarget(const wxSize& size)
{
    // decrease the total size by the size of the non-scrollable parts
    // above/to the left of the canvas
    wxSize sizeCanvas(size);
    sizeCanvas.x -= (60 * 2);
    //sizeCanvas.x -= (myCanvas->GetSize().GetWidth() * 2); //60 width per non-scrollable column.
    return sizeCanvas;
}

void ChartWindow::ReadDatabase(const char* dbName) 
{
    if(myGraphValues.size() != 0) {
        myGraphValues.clear();
    }

    sqlite3* db;

    sqlite3_open(dbName, &db);

    sqlite3_stmt *stmt;
    const char *command = "SELECT Code FROM courses";
    int exit = sqlite3_prepare_v2(db, command, -1, &stmt, nullptr);
    if (exit != SQLITE_OK) {
        std::cout << "Database is empty, setting default values of 10, 20, 50" << std::endl;
        myGraphValues["Course 1"] = 10;
        myGraphValues["Course 2"] = 20;
        myGraphValues["Course 3"] = 50;
        myCanvas->SetTitle("Sample Graph");
    } else {
        while ((exit = sqlite3_step(stmt)) == SQLITE_ROW) {
            std::string code = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
            //printf(code);
            
            if (myGraphValues.find(code) == myGraphValues.end()) {
                myGraphValues[code] = 1;
            } else { //It already exists
                myGraphValues[code] += 1;
            }
        }
        myCanvas->SetTitle("Frequency of TCES courses from 2007 to Now");
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);
}

void ChartWindow::SetUpGraph()
{
    //Sending graph values
    myCanvas->SetValues(myGraphValues);
    myValueCol->SetValues(myGraphValues);

    //Calculating Bar Width
    double barWidth = FromDIP(30);
    double barSpacing = barWidth * 1.5;
    double chartWidth = 2 * barSpacing + myGraphValues.size() * (barWidth + barSpacing);
    myCanvas->SetBarSizes(barWidth, barSpacing, chartWidth);

    //Calculating horizontal lines specs
    std::pair<std::string, int> lowValue = *std::min_element(myGraphValues.begin(), myGraphValues.end(), 
                                                [](const std::pair<std::string, int> & p1,
                                                const std::pair<std::string, int> & p2) {
                                                return p1.second < p2.second; });
    std::pair<std::string, int> highValue = *std::max_element(myGraphValues.begin(), myGraphValues.end(), 
                                                [](const std::pair<std::string, int> & p1,
                                                const std::pair<std::string, int> & p2) {
                                                return p1.second < p2.second; });

    std::tuple<int, double, double> yLineCalcs = CalcChartHlines(static_cast<double>(lowValue.second),
                                                                static_cast<double>(highValue.second));

    myCanvas->SetYLineCalcs(yLineCalcs);
    myValueCol->SetYLineCalcs(yLineCalcs);    

    SetTargetWindow(myCanvas);
    SetScrollbars(10, 0, myGraphValues.size() * (barWidth + barSpacing) / 10, 0);
}

void ChartWindow::OnSize(wxSizeEvent& WXUNUSED(event))
{

    Layout(); //Use sizers for positioning.

    AdjustScrollbars(); //But do not use entire sizer as scrollable
}
