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

#include "CProcessParameters.h"
#include <QStackedWidget>
#include <QLayout>
#include <QSqlTableModel>
#include "View/Common/CResizeStackedWidget.h"
#include "CProcessDocDlg.h"
#include "CProcessDocWidget.h"

CProcessParameters::CProcessParameters(QWidget *parent, Qt::WindowFlags f) :
    CDialog(parent, NO_TITLE_BAR, f)
{
    setSizeGripEnabled(false);
    setWindowOpacity(0.9);
    initLayout();
    initConnections();
}

void CProcessParameters::addWidget(QWidget *pWidget, QString processName)
{
    int index = m_pParamWidgets->indexOf(pWidget);
    if(index == -1)
    {
        index = m_pParamWidgets->addWidget(pWidget);
        m_widgetNames.append(processName);
    }
    m_pParamWidgets->setCurrentIndex(index);
}

void CProcessParameters::setCurrentWidget(const QString& processName)
{
    int index = m_widgetNames.indexOf(processName);
    if(index != -1)
    {
        m_pParamWidgets->setCurrentIndex(index);
        m_pLabel->setText(processName);
    }
}

void CProcessParameters::setProcessInfo(const CTaskInfo &info)
{
    if(m_pDocDlg)
        m_pDocDlg->setProcessInfo(info);
}

void CProcessParameters::setCurrentUser(const CUser &user)
{
    if(m_pDocDlg)
        m_pDocDlg->setCurrentUser(user);
}

QWidget* CProcessParameters::getWidget()
{
    return m_pParamWidgets->currentWidget();
}

bool CProcessParameters::isCurrent(QString name) const
{
    int index = m_widgetNames.indexOf(name);
    return index == m_pParamWidgets->currentIndex();
}

void CProcessParameters::fitToContent()
{
    m_pParamWidgets->updateGeometry();
    m_pFrame->updateGeometry();
    updateGeometry();
    adjustSize();

    QSize size = sizeHint();
    QRect rec = QApplication::desktop()->availableGeometry(this);

    if(size.height() < rec.height())
        resize(size.width(), size.height());
    else
        resize(size.width(), rec.height());
}

void CProcessParameters::clear()
{
    m_widgetNames.clear();

    for(int i = m_pParamWidgets->count(); i >= 0; i--)
    {
        QWidget* pWidget = m_pParamWidgets->widget(i);
        if(pWidget)
        {
            m_pParamWidgets->removeWidget(pWidget);
            pWidget->deleteLater();
        }
    }
}

void CProcessParameters::remove(const QString &name)
{
    int index = m_widgetNames.indexOf(name);
    if(index != -1)
    {
        m_widgetNames.removeAt(index);
        QWidget* pWidget = m_pParamWidgets->widget(index);
        m_pParamWidgets->removeWidget(pWidget);
        pWidget->deleteLater();
    }
}

void CProcessParameters::hideEvent(QHideEvent *event)
{
    Q_UNUSED(event);
    m_pDocDlg->hide();
}

void CProcessParameters::onShowDocumentation()
{
    if(m_pDocDlg->isHidden())
    {
        m_pDocDlg->show();
        updateWidgetPosition(m_pDocDlg, getBorderSize());
    }
    else
        m_pDocDlg->hide();
}

void CProcessParameters::initLayout()
{
    //Popup documentation - non modal
    m_pDocDlg = new CProcessDocDlg(this);

    //Label + doc button with line underneath
    m_pLabel = new QLabel(tr("Process parameters"));
    m_pLabel->setAlignment(Qt::AlignCenter);

    m_pBtnDocumentation = new QPushButton(QIcon(":/Images/info-color2.png"), "");
    m_pBtnDocumentation->setFixedSize(22, 22);
    m_pBtnDocumentation->setToolTip(tr("Documentation"));
    m_pBtnDocumentation->setStyleSheet(QString("QPushButton { background: transparent; border: none;} "
                                               "QPushButton:hover {border: 1px solid %1;}")
                                       .arg(qApp->palette().highlight().color().name()));

    auto pTopHLayout = new QHBoxLayout;
    pTopHLayout->addWidget(m_pLabel);
    pTopHLayout->addWidget(m_pBtnDocumentation);

    //Line separator
    auto pBottomLine = createLine();

    //Process params
    m_pParamWidgets = new CResizeStackedWidget;

    //Add to layout
    auto pLayout = getContentLayout();
    pLayout->addLayout(pTopHLayout);
    pLayout->addWidget(pBottomLine);
    pLayout->addWidget(m_pParamWidgets);
}

void CProcessParameters::initConnections()
{
    connect(m_pBtnDocumentation, &QPushButton::clicked, this, &CProcessParameters::onShowDocumentation);
    connect(m_pDocDlg->getDocWidget(), &CProcessDocWidget::doSave, [&](bool bFullEdit, const CTaskInfo& info)
    {
        emit doUpdateProcessInfo(bFullEdit, info);
    });
}

QFrame *CProcessParameters::createLine()
{
    auto pLine = new QFrame;
    pLine->setLineWidth(1);
    pLine->setFrameStyle(QFrame::Plain);
    pLine->setFrameShape(QFrame::HLine);
    auto pal = qApp->palette();
    pLine->setStyleSheet(QString("color:%1;").arg(pal.highlight().color().name()));
    return pLine;
}

void CProcessParameters::updateWidgetPosition(QWidget* pWidget, int borderSize)
{
    QRect screen = QApplication::desktop()->availableGeometry(this);
    QRect rc = frameGeometry();
    auto size = pWidget->size();
    int yCenter = (rc.top() + rc.bottom()) / 2;
    int x = rc.right() - 2*borderSize;
    int y = yCenter - size.height()/2;

    if(y + size.height() > screen.bottom())
        y = screen.bottom() - size.height();
    else if(y < 0)
        y = 0;

    pWidget->move(QPoint(x, y));
}

#include "moc_CProcessParameters.cpp"
