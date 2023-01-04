#ifndef CHARTCANVAS_H
#define CHARTCANVAS_H

#include <wx/window.h>
#include <wx/scrolwin.h>
#include <wx/graphics.h>
#include <wx/dcbuffer.h>
#include <ValueColumn.h>
#include <map>
#include <cstring>

class ChartCanvas : public wxWindow 
{
    public:
        ChartCanvas(wxScrolled<wxWindow>* parent, wxWindowID id,
                    ValueColumn* valueLabels);
        virtual void ScrollWindow(int dx, int dy, const wxRect* rect) wxOVERRIDE;
        void SetValues(std::map<std::string, int> inValues);
        void SetTitle(std::string);
        void SetYLineCalcs(std::tuple<int, double, double>);
        void SetTitleFont(wxFont);
        void SetBarSizes(double, double, double);
        void SetNormalFont(wxFont);
    private:
        wxFont titleFont;
        wxFont textFont;
        wxScrolled<wxWindow>* myParent;
        ValueColumn* myValueCol;
        double myWidth;
        double barWidth;
        double barMargins;

        std::map<std::string, int> myValues;
        std::string myTitle;
        void OnPaint(wxPaintEvent &evt);
        std::tuple<int, double, double> yLineValues;
        
};
#endif
