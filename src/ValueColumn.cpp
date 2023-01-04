#include <ValueColumn.h>
#include <wx/dcbuffer.h>
#include <wx/graphics.h>

ValueColumn::ValueColumn(wxScrolled<wxWindow>* parent,
                         wxWindowID id, const wxPoint &pos,
                        const wxSize &size) 
                        : wxWindow(parent, id, pos, size,
                        wxFULL_REPAINT_ON_RESIZE) 
{
    myParent = parent;
    this->SetBackgroundColour("WHEAT");
    this->SetBackgroundStyle(wxBG_STYLE_PAINT);
    Bind(wxEVT_PAINT, &ValueColumn::OnPaint, this);
}

void ValueColumn::OnPaint(wxPaintEvent &evt) 
{
    //Graphics Setup, must be declared for window to show up.
    wxAutoBufferedPaintDC dc(this);
    dc.Clear();
    wxGraphicsContext *gc = wxGraphicsContext::Create(dc);

    if(gc && myValues.size() > 0) {

        const auto &[segmentCount, rangeLow, rangeHigh] = yLineValues;
        double yValueSpan = rangeHigh - rangeLow;
        double yLinesCount = segmentCount + 1;

        double labelsMargin = FromDIP(10);
        double titleWidth, titleHeight, textWidth, textHeight;
        double textXPos;
        double yMargin, topMargin, botMargin;

        //Using title and text to determine minimum top margin.
        gc->SetFont(titleFont, *wxWHITE);
        gc->GetTextExtent("S", &titleWidth, &titleHeight); //Height still same

        gc->SetFont(textFont, *wxWHITE);
        gc->GetTextExtent("S", &textWidth, &textHeight);

        //Margins
        yMargin = GetSize().GetHeight() / 8.0; //Standard margin, minimum below.
        topMargin = std::max(yMargin, labelsMargin * 2.0 + titleHeight);
        botMargin = std::max(yMargin, labelsMargin * 2.0 + textHeight);

        wxAffineMatrix2D normalizedToValue{};
        wxAffineMatrix2D normalizedToChartArea{};

        wxRect2DDouble columnArea{
            0,
            topMargin, //Top Margin has minimum size.
            static_cast<double>(GetSize().GetWidth() - FromDIP(1)), //Y Position of line
            GetSize().GetHeight() - topMargin - botMargin //Length of Line
        };

        //Matrix Setup.
        normalizedToChartArea.Translate(columnArea.GetLeft(), columnArea.GetTop());
        normalizedToChartArea.Scale(columnArea.GetSize().GetWidth(),  columnArea.GetSize().GetHeight());

        normalizedToValue.Translate(0, rangeHigh);
        normalizedToValue.Scale(1, -1);
        normalizedToValue.Scale(static_cast<double>(myValues.size() - 1), yValueSpan);

        //Drawing the vertical axis
        wxPoint2DDouble top(columnArea.GetSize().GetWidth(), columnArea.GetTop());
        wxPoint2DDouble bot(columnArea.GetSize().GetWidth(), columnArea.GetSize().GetHeight() + columnArea.GetTop());
        wxPoint2DDouble axisLine[] = {top, bot};
        gc->SetPen(wxPen(wxColor(128, 128, 128)));
        gc->StrokeLines(2, axisLine);

        //Drawing the horizontal lines
        gc->SetFont(textFont, wxSystemSettings::GetAppearance().IsDark() ?
                     *wxWHITE : *wxBLACK);
        for (int i = 0; i < yLinesCount; i++)
        {
            double normalizedLineY = static_cast<double>(i) / (yLinesCount - 1);
            wxPoint2DDouble lineStartPoint = normalizedToChartArea.TransformPoint({1, normalizedLineY});
            double valueAtLineY = normalizedToValue.TransformPoint({0, normalizedLineY}).m_y;

            wxString text = wxString::Format("%.2f", valueAtLineY);
            gc->GetTextExtent(text, &textWidth, &textHeight);
            textXPos = lineStartPoint.m_x - labelsMargin - textWidth;

            text = wxControl::Ellipsize(text, dc, wxELLIPSIZE_MIDDLE, textXPos + textWidth);
            gc->DrawText(text, textXPos, (lineStartPoint.m_y - textHeight / 2.0));
        }
    }

    delete gc;
}

void ValueColumn::SetYLineCalcs(std::tuple<int, double, double> lineCalcs) 
{
    yLineValues = lineCalcs;
}

void ValueColumn::SetValues(std::map<std::string, int> inValues) 
{
    myValues = inValues;
}

void ValueColumn::SetTitleFont(wxFont titleFont) 
{
    this->titleFont = titleFont;
}

void ValueColumn::SetNormalFont(wxFont textFont) 
{
    this->textFont = textFont;
}
