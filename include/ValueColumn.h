#ifndef VALUECOL_H
#define VALUECOL_H

#include <wx/window.h>
#include <wx/scrolwin.h>
#include <map>
#include <cstring>

class ValueColumn : public wxWindow {
    public:
        ValueColumn(wxScrolled<wxWindow>* parent, wxWindowID id,
                     const wxPoint &pos, const wxSize &size);
        //void DrawAxis(double startHeight, double endHeight);
        //void DrawValueLabel(double yPos, double value);
        void SetValues(std::map<std::string, int> inValues);
        void SetYLineCalcs(std::tuple<int, double, double>);
        void SetTitleFont(wxFont);
        void SetNormalFont(wxFont);
    private:
        wxFont titleFont;
        wxFont textFont;
        std::map<std::string, int> myValues;
        void OnPaint(wxPaintEvent &evt);
        wxScrolled<wxWindow>* myParent;
        std::tuple<int, double, double> yLineValues;

};
#endif