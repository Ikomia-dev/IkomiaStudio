#include "CGraphicsDeletableButton.h"
#include "View/Common/CSvgButton.h"

CGraphicsDeletableButton::CGraphicsDeletableButton(QGraphicsItem *parent, Qt::WindowFlags wFlags)
    : QGraphicsProxyWidget(parent, wFlags)
{
}

void CGraphicsDeletableButton::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    Q_UNUSED(event);
    showDeleteButton();
    assert(widget());
    auto pal = qApp->palette();
    auto hoverBorderColor = pal.highlight().color();
    auto styleStr = QString("border: 2px solid %1; border-radius: 5px; background: transparent; padding-left: 0px; padding-right: 0px; padding-top: 0px; padding-bottom: 0px;").arg(hoverBorderColor.name());
    widget()->setStyleSheet(styleStr);
}

void CGraphicsDeletableButton::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    Q_UNUSED(event);
    hideDeleteButton();
    assert(widget());
    widget()->setStyleSheet("border: none; background: transparent; padding-left: 0px; padding-right: 0px; padding-top: 0px; padding-bottom: 0px;");
}

void CGraphicsDeletableButton::showDeleteButton()
{
    if(m_pProxyDeleteBtn == nullptr)
    {
        CSvgButton* pBtn = new CSvgButton(":/Images/close-connect.svg", true);
        pBtn->setToolTip(tr("Delete"));
        connect(pBtn, &CSvgButton::clicked, this, [&]{ emit doDelete(); });
        m_pProxyDeleteBtn = new QGraphicsProxyWidget(this);
        m_pProxyDeleteBtn->setWidget(pBtn);
    }

    QRectF rect = boundingRect();
    m_pProxyDeleteBtn->setPos(rect.right()-m_pProxyDeleteBtn->size().width()/2, rect.top()-m_pProxyDeleteBtn->size().height()/2);
    m_pProxyDeleteBtn->show();
}

void CGraphicsDeletableButton::hideDeleteButton()
{
    m_pProxyDeleteBtn->hide();
}
