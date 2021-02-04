#include "CResultTableDisplay.h"
#include "CResultTableView.h"
#include "Model/Data/CMeasuresTableModel.h"
#include "Model/Data/CFeaturesTableModel.h"
#include "View/Common/CDialog.h"
#include "Main/AppTools.hpp"

CResultTableDisplay::CResultTableDisplay(QWidget *parent, const QString& name, int flags) : CDataDisplay(parent, name, flags)
{
    initLayout();
    initConnections();
    m_typeId = DisplayType::TABLE_DISPLAY;
}

void CResultTableDisplay::setModel(CMeasuresTableModel *pModel)
{
    assert(pModel);

    while (pModel->canFetchMore())
        pModel->fetchMore();

    m_pView->setModel(pModel);

    //Hide primary key column - auto id
    m_pView->setColumnHidden(0, true);
}

void CResultTableDisplay::setModel(CFeaturesTableModel *pModel)
{
    m_pView->setModel(pModel);
}

void CResultTableDisplay::onExportBtnClicked()
{
    auto fileName = Utils::File::saveFile(this, tr("Export table"), "", tr("Table formats(*.csv)"), QStringList("csv"), ".csv");

    if(fileName.isEmpty())
        return;

    emit doExport(fileName);
}

void CResultTableDisplay::initLayout()
{
    setObjectName("CResultTableDisplay");

    m_pSaveBtn = createButton(QIcon(":/Images/save.png"));
    m_pExportBtn = createButton(QIcon(":/Images/export.png"));

    int index = 0;
    if(m_flags & CHECKBOX)
        index++;
    if(m_flags & TITLE)
        index++;

    if(m_flags & SAVE_BUTTON || m_flags & EXPORT_BUTTON)
        m_pHbox->insertStretch(index++, 1);

    if(m_flags & SAVE_BUTTON)
        m_pHbox->insertWidget(index++, m_pSaveBtn);

    if(m_flags & EXPORT_BUTTON)
        m_pHbox->insertWidget(index++, m_pExportBtn);

    if(m_flags & SAVE_BUTTON || m_flags & EXPORT_BUTTON)
        m_pHbox->insertStretch(index++, 1);

    m_pView = new CResultTableView;
    m_pLayout->addWidget(m_pView);
}

void CResultTableDisplay::initConnections()
{
    connect(m_pSaveBtn, &QPushButton::clicked, [&]{ emit doSave(); });
    connect(m_pExportBtn, &QPushButton::clicked, this, &CResultTableDisplay::onExportBtnClicked);
}

QPushButton *CResultTableDisplay::createButton(const QIcon &icon)
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

bool CResultTableDisplay::eventFilter(QObject* obj, QEvent* event)
{
    if(obj == this)
    {
        if(event->type() == QEvent::MouseButtonDblClick)
        {
            emit doDoubleClicked(this);
            return true;
        }
    }
    return CDataDisplay::eventFilter(obj, event);
}
