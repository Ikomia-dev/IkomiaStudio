#ifndef CPLOTDISPLAY_H
#define CPLOTDISPLAY_H

#include "View/DoubleView/CDataDisplay.h"
#include "CDataPlot.h"

class CPlotDisplayParam
{
    public:
        QList<QString>  m_plotNameList;
        QList<QColor>   m_plotColorList;
};

class CCurveDisplayParam : public CPlotDisplayParam
{
    public:
        QList<int>     m_thickness ;
        QList<QString> m_lineType;
        QList<QString> m_markerType;
        QList<QColor>  m_colorIn;
        QList<QColor>  m_colorOut;
        bool    m_bArea = false;

};

class CBarDisplayParam : public CPlotDisplayParam
{
    public:
        bool    m_bHorizontal = false;
};

class CMultiBarDisplayParam : public CPlotDisplayParam
{
    public:
        bool    m_bHorizontal = false;
        int     m_mode = 0;
};

class CPlotDisplay : public CDataDisplay
{
    public:

        enum CResultTableDisplayBar
        {
            NONE = 0x00000000,
            CHECKBOX = 0x00000001,
            TITLE = 0x00000002,
            MAXIMIZE_BUTTON = 0x0000004,
            CLOSE_BUTTON = 0x0000008,
            SAVE_BUTTONS = 0x00000010,
            DEFAULT = TITLE | SAVE_BUTTONS | MAXIMIZE_BUTTON
        };

        CPlotDisplay(QWidget* parent = nullptr, const QString &name = "", int flags=DEFAULT);

        void            initLayout();
        void            initConnections();
        void            initSettings();
        void            initHistogramSettings();
        void            initCurveSettings();
        void            initBarSettings();
        void            initMultiBarSettings();
        void            initPieSettings();

        void            initSettingsConnect(QPushButton* pColorBtn, QComboBox* pCombo, QLineEdit* pEdit, CPlotDisplayParam* pParam, QColor& plotColor);

        void            setDataPlot(CDataPlot* pPlot);

        void            setType(CDataPlot::Type type);
        void            setLegend(bool bEnable);
        void            setGrid(bool bEnable);

    public slots:
        void            onExportBtnClicked();
        void            onChangePlot(const QString& text);

    private:
        void            updatePlot();
        void            updateSettings(CDataPlot::Type type);
        void            makeUpdate(QComboBox* pCombo, CPlotDisplayParam* pParam);
        void            updateCommonSettings(int index, const QString& title, CPlotDisplayParam* pParam, QColor& color);
        void            updateHistogramSettings(int index, const QString& title);
        void            updateCurveSettings(int index, const QString& title);
        void            updateBarSettings(int index, const QString& title);
        void            updateMultiBarSettings(int index, const QString& title);
        QPushButton*    createButton(const QIcon& icon);
        QPushButton*    createColorButton(const QString& title, const QColor& color);
        void            updateColorButton(QPushButton* pBtn, const QColor& color);

        std::tuple<QGridLayout*, QPushButton*, QLineEdit*> createSettings(QComboBox* pCombo, QColor& plotColor);

    private:
        CDataPlot*      m_pDataPlot = nullptr;
        CPlot*          m_pCurrentPlot = nullptr;
        QPushButton*    m_pExportBtn = nullptr;
        QPushButton*    m_pSettingsBtn = nullptr;
        //QPushButton*    m_pColorBtn = nullptr;
        QWidget*        m_pSettings = nullptr;
        QLineEdit*      m_pEditTitle = nullptr;
        QLineEdit*      m_pEditAxisX = nullptr;
        QLineEdit*      m_pEditAxisY = nullptr;
        QGroupBox*      m_pPlotSettings = nullptr;
        QStackedWidget* m_pStacked = nullptr;
        QCheckBox*      m_pLegendCheck = nullptr;
        QCheckBox*      m_pGridCheck = nullptr;
        QComboBox*      m_pComboHisto = nullptr;
        QComboBox*      m_pComboCurve = nullptr;
        QComboBox*      m_pComboBar = nullptr;
        QComboBox*      m_pComboMultiBar = nullptr;
        QColor          m_colorHisto = Qt::red;
        QColor          m_colorCurve = Qt::green;
        QColor          m_colorBar = Qt::gray;
        QColor          m_colorMultiBar = Qt::gray;
        QColor          m_markerColorIn = Qt::darkBlue;
        QColor          m_markerColorOut = Qt::magenta;

        CPlotDisplayParam*          m_pHistoParam = nullptr;
        CCurveDisplayParam*         m_pCurveParam = nullptr;
        CBarDisplayParam*           m_pBarParam = nullptr;
        CMultiBarDisplayParam*      m_pMultiBarParam = nullptr;
};

#endif // CPLOTDISPLAY_H
