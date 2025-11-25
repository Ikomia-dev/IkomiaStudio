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

#include "CHubPluginListViewDelegate.h"
#include <QStaticText>
#include <QTimer>
#include "Main/CoreTools.hpp"
#include "Model/Hub/CHubQueryModel.h"

CHubPluginListViewDelegate::CHubPluginListViewDelegate(int pluginSource, QObject *parent)
    : CListViewDelegate(parent)
{
    m_source = pluginSource;
    if (m_source == WORKSPACE)
        m_actionBtnCount = 3;

    // Size redefinition
    m_sizeHint = QSize(400,200);
    m_iconSize = QSize(96,96);
    m_btnSize = QSize(32, 32);
}

void CHubPluginListViewDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    paintBoundingRect(painter, option);
    paintIcon(painter, option, index);
    paintText(painter, option, index);
    paintLanguageIcon(painter, option, index);
    paintStars(painter, option, index);
    paintActionButtons(painter, option, index);
}

bool CHubPluginListViewDelegate::editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index)
{
    if(event->type() == QEvent::MouseMove)
    {
        auto ev = static_cast<QMouseEvent*>(event);

        if(getRibbonRect(option).containsPoint(ev->pos(), Qt::OddEvenFill))
            showTooltip(index, ev->globalPosition().toPoint(), 2);
    }

    return CListViewDelegate::editorEvent(event, model, option, index);
}

QString CHubPluginListViewDelegate::getActionIconPath(int action) const
{
    QString path;
    switch(action)
    {
        case NONE: break;
        case INFO: path = ":/Images/info-color2.png"; break;
        case INSTALL: path = ":/Images/download-color.png"; break;
        case PUBLISH: path = ":/Images/share-color.png"; break;
    }
    return path;
}

bool CHubPluginListViewDelegate::isBtnEnabled(const QModelIndex &itemIndex, int index) const
{
    if (!itemIndex.isValid())
        return false;

    bool bEnable = true;
    switch(index)
    {
        case 0:
        case 1:
            bEnable = true;
            break;
        case 2:
            bEnable = (m_source == WORKSPACE);
            break;
    }
    return bEnable;
}

void CHubPluginListViewDelegate::executeAction(int action, const QModelIndex &index)
{
    assert(index.isValid());

    switch(action)
    {
        case NONE:
            break;

        case INFO:
            emit doShowInfo(index);
            break;

        case INSTALL:            
            emit doInstallPlugin(index);
            break;

        case PUBLISH:            
            emit doPublishPlugin(index);
            break;
    }
}

int CHubPluginListViewDelegate::getBtnAction(int index) const
{
    int action = NONE;
    switch(index)
    {
        case 0:
            action = INFO;
            break;

        case 1:
            (m_source == HUB || m_source == WORKSPACE) ? action = INSTALL : action = PUBLISH;
            break;

        case 2:
            action = PUBLISH;
            break;
    }
    return action;
}

QPolygon CHubPluginListViewDelegate::getRibbonRect(const QStyleOptionViewItem& option) const
{
    QRect rcCertification(option.rect.right() - m_contentMargins.left() - m_btnSize.width(),
                          option.rect.top() + m_contentMargins.top(),
                          m_headerHeight,
                          m_headerHeight);

    QPolygon poly;
    poly << rcCertification.topLeft() << rcCertification.topRight() << rcCertification.bottomRight();
    return poly;
}

void CHubPluginListViewDelegate::paintText(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (!index.isValid())
        return;

    auto pModel = static_cast<const CHubQueryModel*>(index.model());
    if (!pModel)
        return;

    QBrush brushHighlight = option.palette.highlight();
    QColor colorHighlight = brushHighlight.color();
    QBrush brushText = option.palette.text();
    QColor colorText = brushText.color();

    QFont fontNormal = qApp->font();
    fontNormal.setPointSize(11);
    QFont fontBold = qApp->font();
    fontBold.setBold(true);
    fontBold.setPointSize(12);

    QRect lastRect;

    painter->setPen(colorText);

    // Header height
    int bodyLeft = option.rect.left() + m_iconSize.width() + m_contentMargins.left() + m_spacings.width();
    int bodyWidth = option.rect.width() - m_iconSize.width() - m_contentMargins.left() - m_spacings.width() - m_contentMargins.right();

    /**********************************************/
    // Plugin name
    /**********************************************/
    lastRect = paintName(painter, option, pModel, index, fontBold, colorHighlight);
    /**********************************************/
    // Plugin short description
    /**********************************************/
    lastRect = paintShortDescription(painter, bodyLeft, lastRect.bottom(), bodyWidth, pModel, index, fontNormal, colorText);
    /**********************************************/
    // Contributor
    /**********************************************/
    lastRect = paintContributor(painter, bodyLeft, lastRect.bottom(), bodyWidth, pModel, index, fontBold, colorText);
    /**********************************************/
    // Plugin certification
    /**********************************************/
    paintCertification(painter, option, pModel, index);
}

void CHubPluginListViewDelegate::paintLanguageIcon(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (!index.isValid())
        return;

    auto pModel = static_cast<const CHubQueryModel*>(index.model());
    if (!pModel)
        return;

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);

    QRect rcName(option.rect.left() + m_contentMargins.left() + m_spacings.width(),
                 option.rect.top() + m_contentMargins.top(),
                 m_headerHeight,
                 m_headerHeight);

    QString pixmapPath;
    int language = pModel->record(index.row()).value("language").toInt();

    if(language == ApiLanguage::CPP)
        pixmapPath = ":/Images/C++-language-logo.png";
    else
        pixmapPath = ":/Images/python-language-logo.png";

    QPixmap pixmap = QPixmap(pixmapPath).scaled(m_headerHeight, m_headerHeight, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    painter->drawPixmap(rcName, pixmap);
    painter->restore();
}

void CHubPluginListViewDelegate::paintOSIcon(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    if (!index.isValid())
        return;

    auto pModel = static_cast<const CHubQueryModel*>(index.model());
    if (!pModel)
        return;

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);

    QRect iconRect(option.rect.left() + m_contentMargins.left(),
                   option.rect.top() + m_contentMargins.top(),
                   m_iconSize.width(),
                   option.rect.height()*0.5);

    QRect rc(   iconRect.center().x() - m_btnSize.width() - m_spacings.width(),
                option.rect.bottom() - m_contentMargins.bottom() - m_btnSize.height(),
                m_btnSize.width(),
                m_btnSize.height());

    int os = pModel->record(index.row()).value("os").toInt();
    QString pixmapPath;

    if(os == OSType::LINUX)
        pixmapPath = ":/Images/linux.png";
    else if(os == OSType::WIN)
        pixmapPath = ":/Images/win.png";
    else if(os == OSType::OSX)
        pixmapPath = ":/Images/mac.png";
    else
        pixmapPath = ":/Images/all.png";

    QPixmap pixmap = QPixmap(pixmapPath).scaled(m_btnSize.width(), m_btnSize.height(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    painter->drawPixmap(rc, pixmap);
    painter->restore();
}

void CHubPluginListViewDelegate::paintStars(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    if (!index.isValid())
        return;

    auto pModel = static_cast<const CHubQueryModel*>(index.model());
    if (!pModel)
        return;

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);

    QFont font = qApp->font();
    font.setBold(true);
    font.setPointSize(12);
    QBrush brushHighlight = option.palette.highlight();
    QColor colorHighlight = brushHighlight.color();

    QRect iconRect(option.rect.left() + m_contentMargins.left(),
                   option.rect.top() + m_contentMargins.top() + m_headerHeight+m_contentMargins.top()+m_spacings.height(),
                   m_iconSize.width(),
                   option.rect.height()*0.5);

    QRect starRect(iconRect.center().x(), iconRect.bottom()+m_spacings.height(), m_headerHeight, m_headerHeight);

    // Get rating or stars from database
    QString stars = QString::number(pModel->record(index.row()).value("votes").toInt());
    QStaticText staticStars(stars);
    painter->drawPixmap(starRect, QPixmap(":/Images/star-selected.png").scaled(m_headerHeight, m_headerHeight, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    paintStaticText(painter, starRect.left()-5-staticStars.size().width(), starRect.top(), 0, stars, font, colorHighlight);
    painter->restore();
}

void CHubPluginListViewDelegate::showTooltip(const QModelIndex& modelIndex, const QPoint& pos, int index) const
{
    QString msg;
    switch(index)
    {
        case 0:
            msg = tr("Info");
            break;
        case 1:
            (m_source == LOCAL) ? msg = tr("Publish") : msg = tr("Install");
            break;
        case 2:
            msg = tr("Publish");
            break;
    }
    QToolTip::showText(pos, msg);
}

QRect CHubPluginListViewDelegate::paintName(QPainter* painter, const QStyleOptionViewItem &option, const CHubQueryModel* pModel, const QModelIndex& index, QFont font, const QColor& color) const
{
    QRect rcName(option.rect.left() + m_contentMargins.left() + m_spacings.width(),
                 option.rect.top() + m_contentMargins.top(),
                 option.rect.width() - m_contentMargins.left() - m_spacings.width() - m_contentMargins.right(),
                 m_headerHeight);

    QFontMetrics fontMetrics(font);

    auto name = pModel->record(index.row()).value("name").toString();
    auto displayName = fontMetrics.elidedText(name, Qt::TextElideMode::ElideRight, rcName.width());
    auto w = fontMetrics.horizontalAdvance(displayName);

    paintStaticText(painter, rcName.center().x()-w/2, rcName.top(), rcName.width(), displayName, font, color);

    return rcName;
}

QRect CHubPluginListViewDelegate::paintShortDescription(QPainter* painter, int left, int top, int width, const CHubQueryModel* pModel, const QModelIndex& index, QFont font, const QColor& color) const
{
    auto shortDescription = pModel->record(index.row()).value("shortDescription").toString();
    if(shortDescription.isEmpty())
        shortDescription = pModel->record(index.row()).value("description").toString();

    QString displayShortDescription = Utils::String::getElidedString(shortDescription, font, width, 4);
    auto shortDescriptionSize = paintStaticText(painter, left, top + m_contentMargins.top()*4, width, displayShortDescription, font, color);
    QRect rcShortDescription(left, top, width, shortDescriptionSize.height());
    return rcShortDescription;
}

QRect CHubPluginListViewDelegate::paintContributor(QPainter* painter, int left, int top, int width, const CHubQueryModel* pModel, const QModelIndex& index, QFont font, const QColor& color) const
{
    //QBrush brushHighlight = qApp->palette().highlight();
    //QColor colorHighlight = brushHighlight.color();

    auto user = pModel->record(index.row()).value("user").toString();
    //auto userReputation = pModel->record(index.row()).value("userReputation").toString();

    if(user.isEmpty())
        user = "Unknown";

    //user = QString("%1 (%2)").arg(user).arg(userReputation);
    //auto displayUser = QString("<b>Contributor : <font color=%2>%1</font></b>").arg(user).arg(colorHighlight.name());
    auto displayUser = QString("<b>Contributor : %1</b>").arg(user);
    auto userSize = paintStaticText(painter, left, top + m_contentMargins.top()*8, width, displayUser, font, color);
    QRect rcUser(left, top, width, userSize.height());
    return rcUser;
}

void CHubPluginListViewDelegate::paintCertification(QPainter* painter, const QStyleOptionViewItem& option, const CHubQueryModel* pModel, const QModelIndex& index) const
{
    auto certification = pModel->record(index.row()).value("certification").toInt();
    if(certification != 0)
    {
        /* Certification displayed as ribbon
        QRect rcCertification(option.rect.right()-m_ribbonSize+9, option.rect.top()-9, m_ribbonSize, m_ribbonSize);
        // Choix de l'image à gérer en fonction du type de label (tested, quality...)
        QPixmap certiIcon(QString(":/Images/quality%1.png").arg(certification));
        // Draw ribbon corner at the top right corner
        painter->drawPixmap(rcCertification, certiIcon.scaled(m_ribbonSize, m_ribbonSize, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        */

        painter->save();
        painter->setRenderHint(QPainter::Antialiasing);

        QRect rc(option.rect.right() - m_contentMargins.left() - m_btnSize.width(),
                 option.rect.top() + m_contentMargins.top(),
                 m_headerHeight,
                 m_headerHeight);

        QPixmap pixmap = QPixmap(":/Images/approved.png").scaled(m_headerHeight, m_headerHeight, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        painter->drawPixmap(rc, pixmap);
        painter->restore();
    }
}

#include "moc_CHubPluginListViewDelegate.cpp"
