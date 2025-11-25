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

#include "CNotificationPane.h"
#include <QGridLayout>
#include <QLabel>
#include <QTime>
#include <QDate>
#include <QPropertyAnimation>
#include <QToolButton>
#include <QListView>
#include <QMutexLocker>
#include <QComboBox>
#include <QLineEdit>
#include <QAction>
#include <QApplication>
#include <QWindow>
#include <QCheckBox>
#include <QGroupBox>
#include <QSpinBox>
#include <QToolBox>
#include "View/Common/CHtmlDelegate.h"

CNotificationPane::CNotificationPane(QWidget* pParent) : QWidget(pParent)
{
    init();
}

CNotificationPane::~CNotificationPane()
{
    if(m_pDelegate)
        delete m_pDelegate;
}

void CNotificationPane::init()
{
    // QString list Model
    m_pModel = new CNotificationModel;

    // Proxy model for filtering
    m_pProxy = new CNotificationProxyModel;
    m_pProxy->setSourceModel(m_pModel);

    // init layout
    initLayout();

    // init connections
    initConnections();
}

void CNotificationPane::initLayout()
{
    auto pal = qApp->palette();
    auto color = pal.base().color();
    // QListView for displaying items with custom delegate for HTML
    m_pListView = new QListView;
    QString styleSheet = m_pListView->styleSheet() + QString("QListView{ selection-background-color: %1; }").arg(color.name());
    m_pListView->setStyleSheet(styleSheet);
    m_pListView->setEditTriggers(QListView::EditTrigger::NoEditTriggers);
    m_pListView->setVerticalScrollMode(QListView::ScrollPerPixel);
    m_pListView->setModel(m_pProxy);
    m_pDelegate = new CHtmlDelegate;
    m_pListView->setItemDelegate(m_pDelegate);
    m_pListView->setWordWrap(true);

    // Title bar frame
    QFrame* pTitleBar = new QFrame;
    pTitleBar->setFixedHeight(30);
    pTitleBar->setProperty("class", "CHeaderNotifStyle");

    // Edit
    m_pSearchEdit = new QLineEdit;
    m_pSearchEdit->setPlaceholderText(tr("<Search by words>"));

    // Popup dialog
    initParams();

    // QPushButton clear
    QToolButton* pClearBtn = new QToolButton;
    pClearBtn->setProperty("class", "CBtnNotifStyle");
    pClearBtn->setIcon(QIcon(":/Images/delete.png"));

    connect(pClearBtn, &QPushButton::clicked, this, &CNotificationPane::onClearAllItems);

    QHBoxLayout* pHBox = new QHBoxLayout;
    pHBox->setContentsMargins(0,0,0,0);
    pHBox->setSpacing(5);
    pHBox->addWidget(pClearBtn);
    pHBox->addWidget(m_pSearchEdit);
    // Notification parameters standby
    //pHBox->addWidget(m_pParamsBtn);

    pTitleBar->setLayout(pHBox);

    // Main Layout
    QGridLayout* pLayout = new QGridLayout;
    pLayout->setContentsMargins(0,0,0,0);
    pLayout->setSpacing(0);
    pLayout->addWidget(pTitleBar);
    pLayout->addWidget(m_pParams);
    pLayout->addWidget(m_pListView);

    QWidget* pWidget = new QWidget;
    pWidget->setLayout(pLayout);

    QToolBox* pBox = new QToolBox;
    pBox->addItem(pWidget, tr("Notification center"));

    QVBoxLayout* pBoxLayout = new QVBoxLayout;
    pBoxLayout->addWidget(pBox);
    setLayout(pBoxLayout);
}

void CNotificationPane::initConnections()
{
    connect(m_pSearchEdit, &QLineEdit::textChanged, this, &CNotificationPane::filterText);
    connect(this, &CNotificationPane::doUpdateView, this, &CNotificationPane::onUpdateView);
}

void CNotificationPane::initParams()
{
    // Init button in toolbar

    //Disable access to parameters for the moment
    //m_pParamsBtn = new QToolButton;
    //m_pParamsBtn->setIcon(QIcon(":/Images/properties_white.png"));
    //m_pParamsBtn->setProperty("class", "CBtnNotifStyle");

    // Init frame containing parameters
    m_pParams = new QFrame;
    m_pParams->setProperty("class", "CHeaderNotifStyle");
    QGridLayout* pParamLayout = new QGridLayout;
    m_pParams->setLayout(pParamLayout);

    // Parameters

    // Debug categories
    QGroupBox* pGroupCategory = new QGroupBox(tr("Debug categories"));
    QGridLayout* pCategLayout = new QGridLayout;
    pCategLayout->setContentsMargins(10,10,10,10);
    pGroupCategory->setLayout(pCategLayout);

    QToolButton* pAllBtn = new QToolButton;
    pAllBtn->setCheckable(true);
    pAllBtn->setText(tr("All"));

    pCategLayout->addWidget(pAllBtn, 0, 0);

    m_categoryList = {
        "Default",
        "Graphics",
        "Hub",
        "Image",
        "Plugin",
        "Process",
        "Progress",
        "Project",
        "Python",
        "Render",
        "Results",
        "User",
        "Video",
        "Wizard",
        "Workflow"
    };

    int ind = 1;
    for(auto&& it : m_categoryList)
    {
        // Create check box and add it to layout
        QCheckBox* pCheck = new QCheckBox(it);
        pCategLayout->addWidget(pCheck, ind++, 0);
        // Connect "All" button with every check box
        connect(pAllBtn, &QToolButton::toggled, pCheck, &QCheckBox::setChecked);
        // Connect every check box with category management
        connect(pCheck, &QCheckBox::toggled, [=](bool bChecked){ manageCategory(bChecked, pCheck->text()); });
    }

    // Connect show/hide parameters with toolbar button
    connect(m_pParamsBtn, &QToolButton::clicked, [this]{
        if(m_pParams->isHidden())
            m_pParams->show();
        else
            m_pParams->hide();
    });

    // Button text according to selected check boxes
    connect(pAllBtn, &QToolButton::toggled, [pAllBtn]{
        if(pAllBtn->isChecked())
            pAllBtn->setText(tr("None"));
        else
            pAllBtn->setText(tr("All"));
    });
    m_pParams->hide();
    pAllBtn->setChecked(true);

    // Debug Activation
    QCheckBox* pCheckActivate = new QCheckBox(tr("Activate debug"));
    pCheckActivate->setChecked(true);

    connect(pCheckActivate, &QCheckBox::toggled, this, &CNotificationPane::onActivateDebugLog);

    // Historic size
    QFrame* pContainer = new QFrame;
    QHBoxLayout* pHLayout = new QHBoxLayout;
    pContainer->setLayout(pHLayout);
    QSpinBox* pSpinHistory = new QSpinBox;
    QLabel* pLabelHistory = new QLabel(tr("Max size (in rows)"));
    pLabelHistory->setWordWrap(true);
    pSpinHistory->setValue(m_maxItems);
    pHLayout->addWidget(pLabelHistory);
    pHLayout->addWidget(pSpinHistory);

    connect(pSpinHistory, QOverload<int>::of(&QSpinBox::valueChanged), this, &CNotificationPane::onSetMaxItems);

    // Make layout
    QGroupBox* pFrame = new QGroupBox(tr("Settings"));
    QVBoxLayout* pVLayout = new QVBoxLayout;
    pVLayout->setAlignment(Qt::AlignTop);
    pFrame->setLayout(pVLayout);
    pVLayout->addWidget(pCheckActivate);
    pVLayout->addWidget(pContainer);

    pParamLayout->addWidget(pGroupCategory, 0, 0);
    pParamLayout->addWidget(pFrame, 0, 1);

    pParamLayout->setColumnStretch(0, 1);
    pParamLayout->setColumnStretch(1, 1);
}

void CNotificationPane::addNotification(const QString& text)
{
    auto date = QDate::currentDate();
    auto time = QTime::currentTime();

    QString str = QString("<b>%1</b> <i>%2</i> : %3").arg(date.toString()).arg(time.toString()).arg(text);
    QString newLine = QString("<font size='%2'>%1</font><br/>").arg(str, QString::number(m_htmlFontSize));
    setText(newLine, "");
}

void CNotificationPane::setPadding(size_t padding)
{
    m_padding = padding;
}

void CNotificationPane::setHtmlFontSize(size_t ftSize)
{
    m_htmlFontSize = ftSize;
}

void CNotificationPane::setAnimation(QByteArray name, int min, int max)
{
    m_animationName = name;
    m_animationMinValue = min;
    m_animationMaxValue = max;
}

QString CNotificationPane::activateDebugLog(bool bEnable, const QString& categoryName)
{
    assert(!categoryName.isEmpty());

    if(categoryName == "Default")
        return QString();

    QString str;
    if(bEnable)
    {
        str = QString("%1.debug=true\n"
                      "%1.info=true\n"
                      "%1.warning=true\n"
                      "%1.critical=true\n").arg(categoryName);

    }
    else
    {
        str = QString("%1.debug=false\n"
                      "%1.info=true\n"
                      "%1.warning=true\n"
                      "%1.critical=true\n").arg(categoryName);
    }
    return str;
}

void CNotificationPane::onClearAllItems()
{
    clearAllItems();
}

void CNotificationPane::onActivateDebugLog(bool bEnable)
{
    QString str;
    for(auto&& it : m_categoryList)
        str += activateDebugLog(bEnable, it);

    QLoggingCategory::setFilterRules(str);
}

void CNotificationPane::onSetMaxItems(int maxItems)
{
    m_maxItems = maxItems;
    updateModelRows();
    emit doUpdateView();
}

void CNotificationPane::onUpdateView()
{
    // Select last inserted item and scroll to it
    auto index = m_pModel->index(m_pModel->rowCount()-1, 0);
    QModelIndex proxyIndex = m_pProxy->mapFromSource(index);
    m_pListView->setCurrentIndex(proxyIndex);
    m_pListView->scrollTo(proxyIndex);
    m_pListView->scrollToBottom();
}

void CNotificationPane::onDisplayLogMsg(int type, const QString& msg, const QString& categoryName)
{
    QColor color;

    switch(type)
    {
        case QtDebugMsg:
            color = QColor(70,195,230);
#ifdef QT_NO_DEBUG
            // Do not print debug information in notification center
            return;
#endif
            break;

        case QtInfoMsg:
            color = QColor(45,255,130);
            break;

        case QtWarningMsg:
            color = QColor(255,110,45);
            break;

        case QtCriticalMsg:
            color = QColor(255,45,45);
            break;

        case QtFatalMsg:
            color = QColor(255,45,45);
            break;

        default:
            color = QColor(255,255,255);
            break;
    }
    auto date = QDate::currentDate();
    auto time = QTime::currentTime();

    QString htmlMsg = msg.toHtmlEscaped();
    htmlMsg.replace("\n", "<br>");
    QString str = QString("<b>%1</b> <i>%2</i> : %3").arg(date.toString(Qt::ISODate)).arg(time.toString(Qt::ISODate)).arg(htmlMsg);
    const QString fontTemplate = "<font size='%3' color='%1'>%2</font><br/>";
    QString newLine = fontTemplate.arg(color.name(), str, QString::number(m_htmlFontSize));

    setText(newLine, categoryName);
}

void CNotificationPane::filterText(const QString& text)
{
    if(text.isEmpty())
    {
        m_pProxy->invalidate();
        m_pProxy->setSourceModel(m_pModel);
        return;
    }

    QRegularExpression rx("[, ]");// match a comma or a space
    QStringList list = text.split(rx, Qt::SkipEmptyParts);
    QString str = "^.*(";
    for(auto&& it : list)
    {
        str += it+"|";
    }
    str.chop(1);
    str += ").*$";
    QRegularExpression regExp(str, QRegularExpression::CaseInsensitiveOption);
    m_pProxy->setFilterRegularExpression(regExp);
}

void CNotificationPane::setText(const QString& text, const QString& category)
{
    int r = m_pModel->rowCount();
    if(m_pModel->insertRows(r, 1))
    {
        QModelIndex index = m_pModel->index(m_pModel->rowCount() - 1, 0);
        m_pModel->setData(index, text);
        m_pModel->setData(index, category, CNotificationModel::categoryRole);
    }

    updateModelRows();
    emit doUpdateView();
}

void CNotificationPane::clearAllItems()
{
    m_pModel->setStringList(QStringList{});
}

void CNotificationPane::updateModelRows()
{
    int r = m_pModel->rowCount();
    // Check in order to keep only the last m_maxItems items
    while(r > m_maxItems)
    {
        m_pModel->deleteItem(m_pModel->index(0,0));
        r--;
    }
    m_pModel->update();
}

void CNotificationPane::manageCategory(bool bChecked, const QString& text)
{
    if(bChecked)
        m_pProxy->addCategory(text);
    else
        m_pProxy->removeCategory(text);

    m_pModel->update();
}

void CNotificationPane::animate()
{
    QPropertyAnimation *animation = new QPropertyAnimation(this, m_animationName);
    animation->setDuration(500);

    if(m_bIsOpened == true)
    {
        animation->setStartValue(m_animationMaxValue);
        animation->setEndValue(m_animationMinValue);
        m_bIsOpened = false;
    }
    else
    {
        show();
        animation->setStartValue(m_animationMinValue);
        animation->setEndValue(m_animationMaxValue);
        m_bIsOpened = true;
    }

    connect(animation, &QPropertyAnimation::finished, [&]
    {
        if(m_bIsOpened == false)
            this->hide();
    });

    animation->setEasingCurve(QEasingCurve::OutQuad);
    animation->start(QPropertyAnimation::DeleteWhenStopped);
}

#include "moc_CNotificationPane.cpp"
