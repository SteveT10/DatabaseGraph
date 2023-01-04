#include <ChartCanvas.h>
#include <wx/graphics.h>
#include <wx/dcbuffer.h>

//Scroll_x added/minused to full area, ellpise maxwidth, and Title position

ChartCanvas::ChartCanvas(wxScrolled<wxWindow>* parent, wxWindowID id,
                        ValueColumn* valueCol) 
                         : wxWindow(parent, id, wxDefaultPosition,
                         wxDefaultSize, wxFULL_REPAINT_ON_RESIZE) {
    myParent = parent;
    myValueCol = valueCol;

    this->SetBackgroundStyle(wxBG_STYLE_PAINT);
    this->SetBackgroundColour("WHEAT");

    Bind(wxEVT_PAINT, &ChartCanvas::OnPaint, this);
}

void ChartCanvas::OnPaint(wxPaintEvent &evt) 
{
    wxAutoBufferedPaintDC dc(this);
    dc.Clear();
    wxGraphicsContext *gc = wxGraphicsContext::Create(dc);

    if(gc && myValues.size() > 0) {

        //Rectange is drawned when within this range
        double drawRange= FromDIP(10);
  
        //Title Font Setup
        gc->SetFont(titleFont, wxSystemSettings::GetAppearance().IsDark() ?
                     *wxWHITE : *wxBLACK);

        //Grab Text Samples From Gc to help with sizing.
        double titleWidth, titleHeight, textWidth, textHeight;

        gc->SetFont(textFont, wxSystemSettings::GetAppearance().IsDark() ?
                     *wxWHITE : *wxBLACK);
        gc->GetTextExtent(this->myValues.begin()->first, &textWidth, &textHeight);

        gc->SetFont(titleFont, wxSystemSettings::GetAppearance().IsDark() ?
                     *wxWHITE : *wxBLACK);
        gc->GetTextExtent(this->myTitle, &titleWidth, &titleHeight);

        //How far we are from original x,y position after scrolling.
        int scroll_x = 0;
        int scroll_y = 0; //Currently unused, but may be used in future.
        myParent->CalcUnscrolledPosition(scroll_x, scroll_y, 
                                        &scroll_x, &scroll_y);

        //Full Area of Canvas
        wxRect2DDouble fullArea{
            static_cast<double>(0 - scroll_x),
            0,
            static_cast<double>(myWidth),
            static_cast<double>(GetSize().GetHeight())
        };

        //Margin Variables
        double titleMinMargin = this->FromDIP(10);
        double labelsToChartMargin = this->FromDIP(10);
        double marginX = fullArea.GetSize().GetWidth() / 8.0;
        double marginTop = std::max(fullArea.GetSize().GetHeight() / 8.0,
                             titleMinMargin * 2.0 + titleHeight);

        double marginBottom = std::max(fullArea.GetSize().GetHeight() / 8.0,
                            labelsToChartMargin * 2.0 + textHeight);

        //Setting up chartArea for drawing axes and lines
        wxRect2DDouble chartArea = fullArea;
        chartArea.Inset(0, marginTop, marginX, marginBottom);

        //Setting up plottable area for plotting our values
        wxRect2DDouble plotArea = chartArea;
        plotArea.Inset(barMargins, 0, barWidth + barMargins, 0);

        //Value array
        wxPoint2DDouble *pointArray = new wxPoint2DDouble[myValues.size()];

        //Calculating Value and Horizontal Line position
        const auto &[segmentCount, rangeLow, rangeHigh] = yLineValues;
        double yValueSpan = rangeHigh - rangeLow;
        double yLinesCount = segmentCount + 1;

        //Affine Matrix Variables
        wxAffineMatrix2D normalizedToChart{};
        wxAffineMatrix2D normalizedToPlot{};
        wxAffineMatrix2D normalizedToValue{};
        wxAffineMatrix2D valueToNormalized{};
        wxAffineMatrix2D valueToPlot{};

        //Affine Matrices Setup:
        //Note: wxWidgets graphs 0,0 to top left corner, 
        //positive y is down and positive x is right
        // | scaleX shearX 0 |
        // | shearY scaleY 0 |
        // | deltaX deltaY 1 |
        //Rotation can be done with scaling and shearing with trig funcs.
        normalizedToChart.Translate(chartArea.GetLeft(), chartArea.GetTop());
        normalizedToChart.Scale(chartArea.m_width, chartArea.m_height);

        normalizedToPlot.Translate(plotArea.GetLeft(), plotArea.GetTop());
        normalizedToPlot.Scale(plotArea.m_width, plotArea.m_height);

        valueToNormalized.Translate(0, rangeHigh);
        valueToNormalized.Scale(1, -1);
        valueToNormalized.Scale(static_cast<double>(myValues.size() - 1),
                                 yValueSpan);
        valueToNormalized.Invert();

        valueToPlot = normalizedToPlot;
        valueToPlot.Concat(valueToNormalized);
        //valueToNormalized x normalizedToPlot = valueToPlot.

        //Drawing title with fonts and margins
        gc->DrawText(this->myTitle, std::max(chartArea.GetLeft(), (fullArea.GetSize().GetWidth() - titleWidth) / 2.0 - scroll_x),
                                     (marginTop - titleHeight) / 2.0);

        //Drawing Horizontal Lines and values
        gc->SetPen(wxPen(wxColor(128, 128, 128)));
                     
        for (int i = 0; i < yLinesCount; i++)
        {
            double normalizedLineY = static_cast<double>(i) / (yLinesCount - 1);

            auto lineStartPoint = normalizedToChart.TransformPoint({0, normalizedLineY});
            auto lineEndPoint = normalizedToChart.TransformPoint({1, normalizedLineY});

            wxPoint2DDouble linePoints[] = {{0, lineStartPoint.m_y}, lineEndPoint};
            gc->StrokeLines(2, linePoints);
        }

        //Drawing Right Vertical line, Left is drawn in ValueColumn
        wxPoint2DDouble rightHLinePoints[] = {
            normalizedToChart.TransformPoint({1, 0}),
            normalizedToChart.TransformPoint({1, 1})};

        //gc->StrokeLines(2, leftHLinePoints);
        gc->StrokeLines(2, rightHLinePoints);

        //Plotting our values are points then drawing bars for them
        gc->SetBrush(*wxCYAN_BRUSH);
        gc->SetFont(textFont, wxSystemSettings::GetAppearance().IsDark() ?
                     *wxWHITE : *wxBLACK);
        
        int count = 0;
        std::map<std::string, int>::iterator it;
        for (it = myValues.begin(); it != myValues.end(); it++)
        {
            pointArray[count] = valueToPlot.TransformPoint({static_cast<double>(count), static_cast<double>(it->second)});
            if(IsExposed(pointArray[count].m_x - drawRange, pointArray[count].m_y, std::max(textWidth, barWidth) + (2 * drawRange), 1)) {
                //Rectange has a negative value for height since that directs it upward.
                gc->DrawRectangle(pointArray[count].m_x, plotArea.GetBottom(), barWidth, (pointArray[count].m_y - plotArea.GetBottom()));
                gc->DrawText(it->first, pointArray[count].m_x, plotArea.GetBottom() + textHeight);
                gc->DrawText(std::to_string(it->second), pointArray[count].m_x, pointArray[count].m_y - labelsToChartMargin - textHeight);
            }

            count++;
        }
        
        delete[] pointArray;
        delete gc;
    }
}

void ChartCanvas::SetYLineCalcs(std::tuple<int, double, double> lineCalcs) 
{
    yLineValues = lineCalcs;
}

void ChartCanvas::ScrollWindow(int dx, int dy, const wxRect* rect) 
{
    wxWindow::ScrollWindow( dx, dy, rect );
    myValueCol->ScrollWindow( 0, dy, rect );
}

void ChartCanvas::SetValues(std::map<std::string, int> inValues) 
{
    myValues = inValues;
}

void ChartCanvas::SetTitleFont(wxFont titleFont) 
{
    this->titleFont = titleFont;
}

void ChartCanvas::SetNormalFont(wxFont textFont) 
{
    this->textFont = textFont;
}

void ChartCanvas::SetBarSizes(double barWidth, double barMargins, double canvasWidth) 
{
    this->barWidth = barWidth;
    this->barMargins = barMargins;
    myWidth = canvasWidth;
}

void ChartCanvas::SetTitle(std::string inTitle) 
{
    myTitle = inTitle;
}