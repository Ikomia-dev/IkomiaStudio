// Copyright (C) 2021 Ikomia SAS
// Contact: https://www.ikomia.com
//
// This file is part of the IkomiaStudio software.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "CPlotDisplay.h"
#include <QLineEdit>
#include <QComboBox>
#include <QColorDialog>
#include <QCheckBox>
#include <QGroupBox>
#include <QStackedWidget>
#include <QSpinBox>
#include <QRadioButton>

CPlotDisplay::CPlotDisplay(QWidget* parent, const QString& name, int flags) : CDataDisplay(parent, name, flags)
{
    initLayout();
    initSettings();
    initConnections();
    m_typeId = DisplayType::PLOT_DISPLAY;
}

void CPlotDisplay::initLayout()
{
    setObjectName("CPlotDisplay");

    m_pExportBtn = createButton(QIcon(":/Images/export.png"));
    m_pSettingsBtn = createButton(QIcon(":/Images/properties_white.png"));

    QComboBox* pCombo = new QComboBox;
    pCombo->addItem("Histogram");
    pCombo->addItem("Curve");
    pCombo->addItem("Bar");
    pCombo->addItem("MultiBar");
    pCombo->addItem("Pie");
    pCombo->setCurrentIndex(0);
    pCombo->view()->setTextElideMode(Qt::ElideNone);
    pCombo->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLength);

    int index = 0;
    if(m_flags & CHECKBOX)
        index++;
    if(m_flags & TITLE)
        index++;

    if(m_flags & SAVE_BUTTONS)
    {
        m_pHbox->insertStretch(index++, 1);
        m_pHbox->insertWidget(index++, pCombo, 2);
        m_pHbox->insertWidget(index++, m_pSettingsBtn);
        m_pHbox->insertWidget(index++, m_pExportBtn);
        m_pHbox->insertStretch(index++, 1);
    }

    connect(pCombo, &QComboBox::currentTextChanged, this, &CPlotDisplay::onChangePlot);
}

void CPlotDisplay::initSettings()
{
    QGroupBox* pGlobalSettings = new QGroupBox(tr("Global settings"));
    QGridLayout* pGlobalLayout = new QGridLayout;
    pGlobalSettings->setLayout(pGlobalLayout);

    m_pSettings = new QWidget;
    m_pSettings->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    m_pSettings->hide();

    QGridLayout* pSettingsLayout = new QGridLayout(m_pSettings);

    m_pEditTitle = new QLineEdit;
    m_pEditAxisX = new QLineEdit;
    m_pEditAxisY = new QLineEdit;

    pGlobalLayout->addWidget(new QLabel("Title : "), 0, 0);
    pGlobalLayout->addWidget(m_pEditTitle, 0, 1);

    pGlobalLayout->addWidget(new QLabel("Axis X : "), 1, 0);
    pGlobalLayout->addWidget(m_pEditAxisX, 1, 1);

    pGlobalLayout->addWidget(new QLabel("Axis Y : "), 2, 0);
    pGlobalLayout->addWidget(m_pEditAxisY, 2, 1);

    m_pLegendCheck = new QCheckBox(tr("Legend"));
    m_pLegendCheck->setChecked(false);
    pGlobalLayout->addWidget(m_pLegendCheck, 3, 1);

    m_pGridCheck = new QCheckBox(tr("Grid"));
    m_pGridCheck->setChecked(false);
    pGlobalLayout->addWidget(m_pGridCheck, 4, 1);

    m_pPlotSettings = new QGroupBox;
    m_pStacked = new QStackedWidget;
    QVBoxLayout* pLayout = new QVBoxLayout;
    pLayout->addWidget(m_pStacked);
    m_pPlotSettings->setLayout(pLayout);

    pSettingsLayout->addWidget(pGlobalSettings, 0, 0);
    pSettingsLayout->addWidget(m_pPlotSettings, 0, 1);

    initHistogramSettings();
    initCurveSettings();
    initBarSettings();
    initMultiBarSettings();
    initPieSettings();

    m_pVbox->insertWidget(1, m_pSettings);
}

void CPlotDisplay::initHistogramSettings()
{
    m_pComboHisto = new QComboBox;
    m_pHistoParam = new CPlotDisplayParam;

    //auto [pLayout, pColorBtn, pEdit] = createSettings(m_pComboHisto, m_colorHisto);
    std::tuple<QGridLayout*, QPushButton*, QLineEdit*> tp;
    tp = createSettings(m_pComboHisto, m_colorHisto);
    auto pLayout = std::get<0>(tp);
    auto pColorBtn = std::get<1>(tp);
    auto pEdit = std::get<2>(tp);
    Q_UNUSED(pLayout);
    initSettingsConnect(pColorBtn, m_pComboHisto, pEdit, m_pHistoParam, m_colorHisto);
}

void CPlotDisplay::initCurveSettings()
{
    m_pComboCurve = new QComboBox;
    m_pCurveParam = new CCurveDisplayParam;

    //auto [pLayout, pColorBtn, pEdit] = createSettings(m_pComboCurve, m_colorCurve);
    std::tuple<QGridLayout*, QPushButton*, QLineEdit*> tp;
    tp = createSettings(m_pComboCurve, m_colorCurve);
    auto pLayout = std::get<0>(tp);
    auto pColorBtn = std::get<1>(tp);
    auto pEdit = std::get<2>(tp);
    initSettingsConnect(pColorBtn, m_pComboCurve, pEdit, m_pCurveParam, m_colorCurve);

    auto pThicknessLabel = new QLabel(tr("Thickness"));
    auto pThicknessSpin = new QSpinBox;
    pLayout->addWidget(pThicknessLabel, 2, 0);
    pLayout->addWidget(pThicknessSpin, 2, 1);

    auto pLineTypeLabel = new QLabel(tr("LineType"));
    auto pComboLineType = new QComboBox;
    QStringList items;
    items << "Default" << "-." << "..-" << "-" << ".";
    pComboLineType->addItems(items);
    pLayout->addWidget(pLineTypeLabel, 3, 0);
    pLayout->addWidget(pComboLineType, 3, 1);

    auto pMarkerLabel = new QLabel(tr("MarkerType"));
    auto pComboMarkerType = new QComboBox;
    QStringList markers;
    markers << "Default" << "o" << "d" << "s" << "+" << "x" << "^" << "v" << ">" << "<" << "*" << "h";
    pComboMarkerType->addItems(markers);
    pLayout->addWidget(pMarkerLabel, 4, 0);
    pLayout->addWidget(pComboMarkerType, 4, 1);

    auto pMarkerColorInBtn = createColorButton("Inside marker color", m_markerColorIn);
    auto pMarkerColorOutBtn = createColorButton("Outside marker color", m_markerColorOut);

    pLayout->addWidget(pMarkerColorInBtn, 4, 2);
    pLayout->addWidget(pMarkerColorOutBtn, 4, 3);

    auto pCheckArea = new QCheckBox(tr("Area"));
    pCheckArea->setChecked(false);
    pLayout->addWidget(pCheckArea, 5, 1);

    connect(pThicknessSpin, QOverload<int>::of(&QSpinBox::valueChanged), [this](int value){
        auto pPlot = static_cast<CCurvePlot*>(m_pCurrentPlot);
        int id = m_pComboCurve->currentIndex();
        pPlot->setThickness(value, id);
        m_pCurveParam->m_thickness[id] = value;
    });

    connect(pComboLineType, &QComboBox::currentTextChanged, [this](const QString& text){
        auto pPlot = static_cast<CCurvePlot*>(m_pCurrentPlot);
        int id = m_pComboCurve->currentIndex();
        pPlot->setLineType(text, id);
        m_pCurveParam->m_lineType[id] = text;
    });

    connect(pComboMarkerType, &QComboBox::currentTextChanged, [this](const QString& text){
        auto pPlot = static_cast<CCurvePlot*>(m_pCurrentPlot);
        int index = m_pComboCurve->currentIndex();
        pPlot->setMarker(text, pPlot->getMarkerBrushColor(index), pPlot->getMarkerPenColor(index), index);
        m_pCurveParam->m_markerType[index] = text;
    });

    connect(pMarkerColorInBtn, &QPushButton::clicked, [this, pComboMarkerType, pMarkerColorInBtn]{
        auto pPlot = static_cast<CCurvePlot*>(m_pCurrentPlot);
        int id = m_pComboCurve->currentIndex();
        auto currentColor = pPlot->getMarkerBrushColor(id);
        auto color = QColorDialog::getColor(currentColor);
        if(color != currentColor)
        {
            m_markerColorIn = color;
            pPlot->setMarker(pComboMarkerType->currentText(), m_markerColorIn, m_markerColorOut, id);
            updateColorButton(pMarkerColorInBtn, m_markerColorIn);
            m_pCurveParam->m_colorIn[id] = m_markerColorIn;
        }
    });

    connect(pMarkerColorOutBtn, &QPushButton::clicked, [this, pComboMarkerType, pMarkerColorOutBtn]{
        auto pPlot = static_cast<CCurvePlot*>(m_pCurrentPlot);
        int id = m_pComboCurve->currentIndex();
        auto currentColor = pPlot->getMarkerPenColor(id);
        auto color = QColorDialog::getColor(currentColor);
        if(color != currentColor)
        {
            m_markerColorOut = color;
            pPlot->setMarker(pComboMarkerType->currentText(), m_markerColorIn, m_markerColorOut, id);
            updateColorButton(pMarkerColorOutBtn, m_markerColorOut);
            m_pCurveParam->m_colorOut[id] = m_markerColorOut;
        }
    });

    connect(m_pComboCurve, QOverload<int>::of(&QComboBox::currentIndexChanged), [this, pThicknessSpin, pComboLineType, pComboMarkerType, pMarkerColorInBtn, pMarkerColorOutBtn](int index){
        if(index<0)
            return;
        auto pPlot = static_cast<CCurvePlot*>(m_pCurrentPlot);

        pThicknessSpin->setValue(pPlot->getThickness(index));
        pComboLineType->setCurrentText(pPlot->getLineType(index));
        pComboMarkerType->setCurrentText(pPlot->getMarker(index));
        updateColorButton(pMarkerColorInBtn, pPlot->getMarkerBrushColor(index));
        updateColorButton(pMarkerColorOutBtn, pPlot->getMarkerPenColor(index));
    });

    connect(pCheckArea, &QCheckBox::clicked, [this](bool checked){
        auto pPlot = static_cast<CCurvePlot*>(m_pCurrentPlot);
        pPlot->setCurveArea(checked);
        m_pCurveParam->m_bArea = checked;
    });
}

void CPlotDisplay::initBarSettings()
{
    m_pComboBar = new QComboBox;
    m_pBarParam = new CBarDisplayParam;

    //auto [pLayout, pColorBtn, pEdit] = createSettings(m_pComboBar, m_colorBar);
    std::tuple<QGridLayout*, QPushButton*, QLineEdit*> tp;
    tp = createSettings(m_pComboBar, m_colorBar);
    auto pLayout = std::get<0>(tp);
    auto pColorBtn = std::get<1>(tp);
    auto pEdit = std::get<2>(tp);
    initSettingsConnect(pColorBtn, m_pComboBar, pEdit, m_pBarParam, m_colorBar);

    auto pRadioHorizontal = new QRadioButton("Horizontal");
    auto pRadioVertical = new QRadioButton("Vertical");

    pRadioVertical->setChecked(true);

    pLayout->addWidget(pRadioHorizontal, 2, 0);
    pLayout->addWidget(pRadioVertical, 3, 0);

    connect(pRadioHorizontal, &QRadioButton::toggled, [this](bool checked){
        auto pPlot = static_cast<CSimpleBarPlot*>(m_pCurrentPlot);
        if(checked == true)
            pPlot->setOrientation(Qt::Horizontal);
        else
            pPlot->setOrientation(Qt::Vertical);
        m_pBarParam->m_bHorizontal = checked;
    });
}

void CPlotDisplay::initMultiBarSettings()
{
    m_pComboMultiBar = new QComboBox;
    m_pMultiBarParam = new CMultiBarDisplayParam;

    //auto [pLayout, pColorBtn, pEdit] = createSettings(m_pComboMultiBar, m_colorMultiBar);
    std::tuple<QGridLayout*, QPushButton*, QLineEdit*> tp;
    tp = createSettings(m_pComboMultiBar, m_colorMultiBar);
    auto pLayout = std::get<0>(tp);
    auto pColorBtn = std::get<1>(tp);
    auto pEdit = std::get<2>(tp);
    initSettingsConnect(pColorBtn, m_pComboMultiBar, pEdit, m_pMultiBarParam, m_colorMultiBar);

    auto pOrientationGroup = new QGroupBox(tr("Orientation"));
    auto pOrientationLayout = new QGridLayout;
    pOrientationGroup->setLayout(pOrientationLayout);
    auto pRadioHorizontal = new QRadioButton("Horizontal");
    auto pRadioVertical = new QRadioButton("Vertical");

    pRadioVertical->setChecked(true);

    pOrientationLayout->addWidget(pRadioHorizontal, 0, 0);
    pOrientationLayout->addWidget(pRadioVertical, 1, 0);

    pLayout->addWidget(pOrientationGroup, 2, 0);

    auto pModeGroup = new QGroupBox(tr("Mode"));
    auto pModeLayout = new QGridLayout;
    pModeGroup->setLayout(pModeLayout);
    auto pGrouped = new QRadioButton("Grouped");
    auto pStacked = new QRadioButton("Stacked");

    pGrouped->setChecked(true);

    pModeLayout->addWidget(pGrouped, 0, 0);
    pModeLayout->addWidget(pStacked, 1, 0);

    pLayout->addWidget(pModeGroup, 2, 1);

    connect(pRadioHorizontal, &QRadioButton::toggled, [this](bool checked){
        auto pPlot = static_cast<CMultiBarPlot*>(m_pCurrentPlot);
        if(checked == true)
            pPlot->setOrientation(Qt::Horizontal);
        else
            pPlot->setOrientation(Qt::Vertical);
        m_pMultiBarParam->m_bHorizontal = checked;
    });

    connect(pGrouped, &QRadioButton::toggled, [this](bool checked){
        auto pPlot = static_cast<CMultiBarPlot*>(m_pCurrentPlot);
        if(checked == true)
            pPlot->setMode(0);
        else
            pPlot->setMode(1);
        m_pMultiBarParam->m_mode = checked ? 0 : 1;
    });
}

void CPlotDisplay::initPieSettings()
{
    auto pWidget = new QWidget;
    auto pPieLayout = new QGridLayout;

    pWidget->setLayout(pPieLayout);

    m_pStacked->addWidget(pWidget);
}

void CPlotDisplay::initSettingsConnect(QPushButton* pColorBtn, QComboBox* pCombo, QLineEdit* pEdit, CPlotDisplayParam* pParam, QColor& plotColor)
{
    connect(pColorBtn, &QPushButton::clicked, [this, pColorBtn, pCombo, pParam, &plotColor]{
        auto id = pCombo->currentIndex();
        auto currentColor = m_pCurrentPlot->getColor(id);
        auto color = QColorDialog::getColor(currentColor);
        if(color != currentColor)
        {
            plotColor = color;
            m_pCurrentPlot->setColor(plotColor, id);
            pParam->m_plotColorList[id] = plotColor;
            updateColorButton(pColorBtn, plotColor);
        }
    });
    connect(pCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), [this, pColorBtn, pEdit, pCombo](int index){
        if(index<0)
            return;

        auto color = m_pCurrentPlot->getColor(index);
        updateColorButton(pColorBtn, color);
        pEdit->setText(pCombo->currentText());
    });
    connect(pEdit, &QLineEdit::textChanged, [this, pCombo, pParam](const QString& text){
        if(m_pCurrentPlot)
        {
            auto id = pCombo->currentIndex();
            m_pCurrentPlot->setPlotName(text, id);
            pParam->m_plotNameList[id] = text;
            pCombo->setItemText(id, text);
        }
    });
}

void CPlotDisplay::initConnections()
{
    connect(m_pExportBtn, &QPushButton::clicked, this, &CPlotDisplay::onExportBtnClicked);
    connect(m_pSettingsBtn, &QPushButton::clicked, [&]{
        if(m_pSettings->isHidden())
            m_pSettings->show();
        else
            m_pSettings->hide();
    });
    connect(m_pLegendCheck, &QCheckBox::clicked, this, &CPlotDisplay::setLegend);
    connect(m_pGridCheck, &QCheckBox::clicked, this, &CPlotDisplay::setGrid);
}

void CPlotDisplay::setDataPlot(CDataPlot* pPlot)
{
    assert(pPlot);

    if(m_pDataPlot)
        delete m_pDataPlot;  

    m_pDataPlot = pPlot;

    updatePlot();
}

void CPlotDisplay::setType(CDataPlot::Type type)
{
    if(m_pDataPlot->getType() == type)
        return;

    m_pDataPlot->setType(type);

    updatePlot();
}

void CPlotDisplay::setLegend(bool bEnable)
{
    assert(m_pCurrentPlot);

    m_pCurrentPlot->setLegend(bEnable);
}

void CPlotDisplay::setGrid(bool bEnable)
{
    assert(m_pCurrentPlot);
    m_pCurrentPlot->setGrid(bEnable);
}

void CPlotDisplay::updatePlot()
{
    if(m_pCurrentPlot)
        m_pCurrentPlot->disconnect();
    // Remove and delete current widget from view
    auto pItem = m_pLayout->itemAtPosition(0, 0);
    if(pItem)
    {
        m_pLayout->removeItem(pItem);
        pItem->widget()->deleteLater();
    }
    // Create new plot according to the right type
    m_pCurrentPlot = m_pDataPlot->create();

    // Create encapsulating widget for plot
    QWidget* pWidget = new QWidget;
    pWidget->setObjectName("CWidget");
    QVBoxLayout* pLayout = new QVBoxLayout;
    pLayout->setContentsMargins(5,5,5,5);
    pLayout->addWidget(m_pCurrentPlot);
    pWidget->setLayout(pLayout);

    // Add this widget to layout
    m_pLayout->addWidget(pWidget, 0, 0);

    // Connect signals to current plot
    connect(m_pEditTitle, &QLineEdit::textChanged, m_pCurrentPlot, &CPlot::onSetTitle);
    connect(m_pEditAxisX, &QLineEdit::textChanged, m_pCurrentPlot, &CPlot::onSetLabelX);
    connect(m_pEditAxisY, &QLineEdit::textChanged, m_pCurrentPlot, &CPlot::onSetLabelY);

    // Update settings widget
    updateSettings(m_pDataPlot->getType());
}

void CPlotDisplay::updateSettings(CDataPlot::Type type)
{
    // Take into account current settings
    // Title
    if(!m_pEditTitle->text().isEmpty())
        m_pCurrentPlot->setTitle(m_pEditTitle->text());
    // Axis X
    if(!m_pEditAxisX->text().isEmpty())
        m_pCurrentPlot->onSetLabelX(m_pEditAxisX->text());
    // Axis Y
    if(!m_pEditAxisY->text().isEmpty())
        m_pCurrentPlot->onSetLabelY(m_pEditAxisY->text());
    // Legend and grid
    m_pCurrentPlot->setLegend(m_pLegendCheck->isChecked());
    m_pCurrentPlot->setGrid(m_pGridCheck->isChecked());

    switch(type)
    {
        case CDataPlot::Type::Histogram:
        {
            updateHistogramSettings(0, tr("Histogram settings"));
            break;
        }
        case CDataPlot::Type::Curve:
        {
            updateCurveSettings(1, tr("Curve settings"));
            break;
        }
        case CDataPlot::Type::Bar:
        {
            updateBarSettings(2, tr("Bar settings"));
            break;
        }
        case CDataPlot::Type::MultiBar:
        {
            updateMultiBarSettings(3, tr("MultiBar settings"));
            break;
        }
        case CDataPlot::Type::Pie:
        {
            m_pPlotSettings->setTitle(tr("Pie settings"));
            m_pStacked->setCurrentIndex(4);
            break;
        }
    }
}

void CPlotDisplay::makeUpdate(QComboBox* pCombo, CPlotDisplayParam* pParam)
{
    // Update all values by emitting signal currentIndexChanged from addItems
    pCombo->clear();
    pCombo->addItems(pParam->m_plotNameList);
}

void CPlotDisplay::updateCommonSettings(int index, const QString& title, CPlotDisplayParam* pParam, QColor& color)
{
    m_pPlotSettings->setTitle(title);
    m_pStacked->setCurrentIndex(index);
    if(pParam->m_plotNameList.isEmpty())
        pParam->m_plotNameList = m_pCurrentPlot->getPlotTitles();
    if(pParam->m_plotColorList.isEmpty())
    {
        for(int i = 0; i<pParam->m_plotNameList.size(); ++i)
        {
            auto c = m_pCurrentPlot->getColor(i);
            if(c.isValid())
                pParam->m_plotColorList.push_back(c);
            else
                pParam->m_plotColorList.push_back(color);
        }
        auto c = m_pCurrentPlot->getColor(0);
        if(c.isValid())
            m_pCurrentPlot->setColor(c, 0);
        else
            m_pCurrentPlot->setColor(color, 0);
    }
    else
    {
        for(int i=0; i<pParam->m_plotColorList.size(); ++i)
        {
            m_pCurrentPlot->setColor(pParam->m_plotColorList[i], i);
        }
    }
}

void CPlotDisplay::updateHistogramSettings(int index, const QString& title)
{
    updateCommonSettings(index, title, m_pHistoParam, m_colorHisto);
    makeUpdate(m_pComboHisto, m_pHistoParam);
}

void CPlotDisplay::updateCurveSettings(int index, const QString& title)
{
    auto pPlot = static_cast<CCurvePlot*>(m_pCurrentPlot);

    if(pPlot == nullptr)
        return;

    // Update Common parameters
    updateCommonSettings(index, title, m_pCurveParam, m_colorCurve);

    // Update thickness
    if(m_pCurveParam->m_thickness.isEmpty())
    {
        for(int i=0; i<m_pCurrentPlot->getNumberOfPlots(); ++i)
            m_pCurveParam->m_thickness.push_back(4);
    }

    for(int i=0; i<m_pCurveParam->m_thickness.size(); ++i)
        pPlot->setThickness(m_pCurveParam->m_thickness[i], i);

    // Update line type
    if(m_pCurveParam->m_lineType.isEmpty())
    {
        for(int i=0; i<m_pCurrentPlot->getNumberOfPlots(); ++i)
            m_pCurveParam->m_lineType.push_back(tr("Default"));
    }

    for(int i=0; i<m_pCurveParam->m_lineType.size(); ++i)
        pPlot->setLineType(m_pCurveParam->m_lineType[i], i);

    // Update marker type and color marker
    if(m_pCurveParam->m_markerType.isEmpty())
    {
        for(int i=0; i<m_pCurrentPlot->getNumberOfPlots(); ++i)
            m_pCurveParam->m_markerType.push_back(tr("Default"));
    }
    if(m_pCurveParam->m_colorIn.isEmpty())
    {
        for(int i=0; i<m_pCurrentPlot->getNumberOfPlots(); ++i)
            m_pCurveParam->m_colorIn.push_back(m_markerColorIn);
    }
    if(m_pCurveParam->m_colorOut.isEmpty())
    {
        for(int i=0; i<m_pCurrentPlot->getNumberOfPlots(); ++i)
            m_pCurveParam->m_colorOut.push_back(m_markerColorOut);
    }

    assert(m_pCurveParam->m_markerType.size() == m_pCurveParam->m_colorIn.size() && m_pCurveParam->m_colorIn.size() == m_pCurveParam->m_colorOut.size());
    for(int i=0; i<m_pCurveParam->m_markerType.size(); ++i)
        pPlot->setMarker(m_pCurveParam->m_markerType[i], m_pCurveParam->m_colorIn[i], m_pCurveParam->m_colorOut[i], i);

    // Update Area check
    pPlot->setCurveArea(m_pCurveParam->m_bArea);

    makeUpdate(m_pComboCurve, m_pCurveParam);
}

void CPlotDisplay::updateBarSettings(int index, const QString& title)
{
    auto pPlot = static_cast<CSimpleBarPlot*>(m_pCurrentPlot);

    if(pPlot == nullptr)
        return;

    updateCommonSettings(index, title, m_pBarParam, m_colorBar);

    // Update orientation
    pPlot->setOrientation(m_pBarParam->m_bHorizontal ? Qt::Horizontal : Qt::Vertical);

    makeUpdate(m_pComboBar, m_pBarParam);
}

void CPlotDisplay::updateMultiBarSettings(int index, const QString& title)
{
    auto pPlot = static_cast<CMultiBarPlot*>(m_pCurrentPlot);

    if(pPlot == nullptr)
        return;

    updateCommonSettings(index, title, m_pMultiBarParam, m_colorMultiBar);

    // Update orientation
    pPlot->setOrientation(m_pMultiBarParam->m_bHorizontal ? Qt::Horizontal : Qt::Vertical);

    // Update orientation
    pPlot->setMode(m_pMultiBarParam->m_mode);

    makeUpdate(m_pComboMultiBar, m_pMultiBarParam);
}

QPushButton* CPlotDisplay::createButton(const QIcon &icon)
{
    auto pal = qApp->palette();
    auto color = pal.highlight().color();

    auto pBtn = new QPushButton;
    pBtn->setIcon(icon);
    pBtn->setIconSize(QSize(16,16));
    pBtn->setFixedSize(22,22);
    pBtn->setStyleSheet(QString("QPushButton { background: transparent; border: none;} QPushButton:hover {border: 1px solid %1;}").arg(color.name()));
    return pBtn;
}

void CPlotDisplay::onExportBtnClicked()
{
    m_pCurrentPlot->exportChart("untitled.pdf");
}

void CPlotDisplay::onChangePlot(const QString& text)
{
    if(text == "Histogram")
        setType(CDataPlot::Type::Histogram);
    else if(text == "Curve")
        setType(CDataPlot::Type::Curve);
    else if(text == "Bar")
        setType(CDataPlot::Type::Bar);
    else if(text == "MultiBar")
        setType(CDataPlot::Type::MultiBar);
    else if(text == "Pie")
        setType(CDataPlot::Type::Pie);
}

QPushButton *CPlotDisplay::createColorButton(const QString &title, const QColor& color)
{
    auto pal = qApp->palette();
    auto hoverBorderColor = pal.highlight().color();

    auto pBtn = new QPushButton("");
    pBtn->setFlat(true);
    pBtn->setToolTip(title);
    pBtn->setStyleSheet(QString("QPushButton { background-color: %1; border: none; } QPushButton:hover {border: 1px solid %2;}")
                        .arg(color.name())
                        .arg(hoverBorderColor.name()));
    return pBtn;
}

void CPlotDisplay::updateColorButton(QPushButton* pBtn, const QColor& color)
{
    auto pal = qApp->palette();
    auto hoverBorderColor = pal.highlight().color();
    pBtn->setStyleSheet(QString("QPushButton { background-color: %1; border: none; } QPushButton:hover {border: 1px solid %2;}")
                                .arg(color.name())
                        .arg(hoverBorderColor.name()));
}

std::tuple<QGridLayout*, QPushButton*, QLineEdit*> CPlotDisplay::createSettings(QComboBox* pCombo, QColor& plotColor)
{
    auto pWidget = new QWidget;
    auto pLayout = new QGridLayout;

    pWidget->setLayout(pLayout);

    m_pStacked->addWidget(pWidget);

    auto pColorBtn = createColorButton(tr("Color dialog"), plotColor);

    auto pEdit = new QLineEdit;
    auto pLabel = new QLabel(tr("Plot name"));

    pLayout->addWidget(pCombo, 0, 0);
    pLayout->addWidget(pLabel, 1, 0);
    pLayout->addWidget(pEdit, 1, 1);
    pLayout->addWidget(pColorBtn, 1, 2);

    return std::tuple<QGridLayout*, QPushButton*, QLineEdit*>(pLayout, pColorBtn, pEdit);
}
