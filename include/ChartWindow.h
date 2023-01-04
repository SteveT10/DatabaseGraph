#ifndef CHARTWIN_H
#define CHARTWIN_H

#include <wx/scrolwin.h>
#include <ValueColumn.h>
#include <ChartCanvas.h>
#include <map>
#include <cstring>

class ChartWindow : public wxScrolled<wxWindow> {
    public:
        ChartWindow(wxWindow *parent);
        void ReadDatabase(const char* dbName);
        void SetUpGraph();
    protected: 
        virtual wxSize GetSizeAvailableForScrollTarget(const wxSize& size) wxOVERRIDE;
    private: 
        void OnSize(wxSizeEvent& WXUNUSED(event));

        std::map<std::string, int> myGraphValues;
        ChartCanvas* myCanvas;
        ValueColumn* myValueCol;
        std::tuple<int, double, double>CalcChartHlines(double, double); 
};

#endif