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

#include "CStorePluginListViewDelegate.h"
#include <QStaticText>
#include <QTimer>
#include "Main/CoreTools.hpp"
#include "Model/Store/CStoreQueryModel.h"
#include "Core/CTaskFactory.hpp"

CStorePluginListViewDelegate::CStorePluginListViewDelegate(int pluginSource, QObject *parent)
    : CListViewDelegate(parent)
{
    m_source = pluginSource;

    // Size redefinition
    m_sizeHint = QSize(400,200);
    m_iconSize = QSize(96,96);
    m_btnSize = QSize(32, 32);
}

void CStorePluginListViewDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    paintBoundingRect(painter, option);
    paintIcon(painter, option, index);
    paintText(painter, option, index);
    paintLanguageIcon(painter, option, index);
    paintStars(painter, option, index);
    paintActionButtons(painter, option, index);
}

bool CStorePluginListViewDelegate::editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index)
{
    if(event->type() == QEvent::MouseMove)
    {
        auto ev = static_cast<QMouseEvent*>(event);

        if(getRibbonRect(option).containsPoint(ev->pos(), Qt::OddEvenFill))
            showTooltip(index, ev->globalPos(), 2);
    }

    return CListViewDelegate::editorEvent(event, model, option, index);
}

QString CStorePluginListViewDelegate::getActionIconPath(int action) const
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

bool CStorePluginListViewDelegate::isBtnEnabled(const QModelIndex &itemIndex, int index) const
{
    assert(itemIndex.isValid());
    bool bEnable = true;

    switch(index)
    {
        case 0:
            bEnable = true;
            break;

        case 1:
        {
            auto pModel = static_cast<const CStoreQueryModel*>(itemIndex.model());
            int language = pModel->record(itemIndex.row()).value("language").toInt();
            auto state = getProcessState(itemIndex);

            if(language == ApiLanguage::CPP)
                bEnable = (state == PluginState::VALID);
            else
                bEnable = (state == PluginState::VALID || state == PluginState::UPDATED);
            break;
        }
    }
    return bEnable;
}

void CStorePluginListViewDelegate::executeAction(int action, const QModelIndex &index)
{
    assert(index.isValid());

    switch(action)
    {
        case NONE:
            break;

        case INFO:
            emit doShowInfo(index); break;

        case INSTALL:
        {
            auto pModel = static_cast<const CStoreQueryModel*>(index.model());
            int language = pModel->record(index.row()).value("language").toInt();
            auto state = getProcessState(index);

            if( (language == ApiLanguage::CPP && state == PluginState::VALID) ||
                (language == ApiLanguage::PYTHON && (state == PluginState::VALID || state == PluginState::UPDATED)))
            {
                emit doInstallPlugin(index);
            }
            break;
        }

        case PUBLISH:
        {
            auto state = getProcessState(index);
            if(state == PluginState::VALID)
                emit doPublishPlugin(index);
            break;
        }
    }
}

int CStorePluginListViewDelegate::getBtnAction(int index) const
{
    int action = NONE;
    switch(index)
    {
        case 0:
            action = INFO;
            break;

        case 1:
            if(m_source == SERVER)
                action = INSTALL;
            else
                action = PUBLISH;
            break;
    }
    return action;
}

QPolygon CStorePluginListViewDelegate::getRibbonRect(const QStyleOptionViewItem& option) const
{
    QRect rcCertification(option.rect.right()-m_ribbonSize+9, option.rect.top()-9, m_ribbonSize, m_ribbonSize);
    QPolygon poly;
    poly << rcCertification.topLeft() << rcCertification.topRight() << rcCertification.bottomRight();
    return poly;
}

PluginState CStorePluginListViewDelegate::getProcessState(const QModelIndex &index) const
{
    assert(index.isValid());
    auto pModel = static_cast<const CStoreQueryModel*>(index.model());
    assert(pModel);
    int language = pModel->record(index.row()).value("language").toInt();
    auto ikomiaVersion = pModel->record(index.row()).value("ikomiaVersion").toString();

    if(language == ApiLanguage::CPP)
        return Utils::Plugin::getCppState(ikomiaVersion);
    else
        return Utils::Plugin::getPythonState(ikomiaVersion);
}

QString CStorePluginListViewDelegate::getStatusMessage(const QModelIndex &index) const
{
    // Check version compatibility with App & API
    auto pModel = static_cast<const CStoreQueryModel*>(index.model());
    assert(pModel);

    QString msg;
    int language = pModel->record(index.row()).value("language").toInt();
    auto ikomiaVersion = pModel->record(index.row()).value("ikomiaVersion").toString();

    if(language == ApiLanguage::CPP)
    {
        auto state = Utils::Plugin::getCppState(ikomiaVersion);
        if(state == PluginState::DEPRECATED)
            msg = QString("<br><b><i><font color=#9a0000>Deprecated</font></i></b>");
        else if(state == PluginState::UPDATED)
            msg = QString("<br><b><i><font color=#9a0000>Ikomia Studio update required</font></i></b>");
    }
    else
    {
        auto state = Utils::Plugin::getPythonState(ikomiaVersion);
        if(state == PluginState::DEPRECATED)
            msg = QString("<br><b><i><font color=#9a0000>Deprecated</font></i></b>");
        else if(state == PluginState::UPDATED)
            msg = QString("<br><b><i><font color=#de7207>Ikomia Studio update adviced</font></i></b>");
    }
    return msg;
}

void CStorePluginListViewDelegate::paintText(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    assert(index.isValid());
    auto pModel = static_cast<const CStoreQueryModel*>(index.model());
    assert(pModel);

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

void CStorePluginListViewDelegate::paintLanguageIcon(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    assert(index.isValid());
    auto pModel = static_cast<const CStoreQueryModel*>(index.model());
    assert(pModel);

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

void CStorePluginListViewDelegate::paintOSIcon(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    assert(index.isValid());
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

    auto pModel = static_cast<const CStoreQueryModel*>(index.model());
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

void CStorePluginListViewDelegate::paintStars(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    assert(index.isValid());
    auto pModel = static_cast<const CStoreQueryModel*>(index.model());
    assert(pModel);
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

void CStorePluginListViewDelegate::showTooltip(const QModelIndex& modelIndex, const QPoint& pos, int index) const
{
    switch(index)
    {
        case 0:
            QToolTip::showText(pos, tr("Info"));
            break;

        case 1:
            if(m_source == SERVER)
                QToolTip::showText(pos, tr("Install"));
            else
                QToolTip::showText(pos, tr("Publish"));
            break;
        case 2:
        {
            auto pModel = static_cast<const CStoreQueryModel*>(modelIndex.model());
            auto certification = pModel->record(modelIndex.row()).value("certification").toInt();

            switch (certification)
            {
                case 0: break;
                case 1: QToolTip::showText(pos, tr("Tested")); break;
                case 2: QToolTip::showText(pos, tr("Certified")); break;
                default: break;
            }
            break;
        }
    }
}

QRect CStorePluginListViewDelegate::paintName(QPainter* painter, const QStyleOptionViewItem &option, const CStoreQueryModel* pModel, const QModelIndex& index, QFont font, const QColor& color) const
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

QRect CStorePluginListViewDelegate::paintShortDescription(QPainter* painter, int left, int top, int width, const CStoreQueryModel* pModel, const QModelIndex& index, QFont font, const QColor& color) const
{
    auto shortDescription = pModel->record(index.row()).value("shortDescription").toString();
    if(shortDescription.isEmpty())
        shortDescription = pModel->record(index.row()).value("description").toString();

    QString displayShortDescription = Utils::String::getElidedString(shortDescription, font, width, 4);
    displayShortDescription += getStatusMessage(index);

    auto shortDescriptionSize = paintStaticText(painter, left, top + m_contentMargins.top()*4, width, displayShortDescription, font, color);
    QRect rcShortDescription(left, top, width, shortDescriptionSize.height());
    return rcShortDescription;
}

QRect CStorePluginListViewDelegate::paintContributor(QPainter* painter, int left, int top, int width, const CStoreQueryModel* pModel, const QModelIndex& index, QFont font, const QColor& color) const
{
    QBrush brushHighlight = qApp->palette().highlight();
    QColor colorHighlight = brushHighlight.color();

    auto user = pModel->record(index.row()).value("user").toString();
    auto userReputation = pModel->record(index.row()).value("userReputation").toString();

    if(user.isEmpty())
        user = "Unknown";

    user = QString("%1 (%2)").arg(user).arg(userReputation);
    auto displayUser = QString("<b>Contributor : <font color=%2>%1</font></b>").arg(user).arg(colorHighlight.name());
    auto userSize = paintStaticText(painter, left, top + m_contentMargins.top()*8, width, displayUser, font, color);
    QRect rcUser(left, top, width, userSize.height());
    return rcUser;
}

void CStorePluginListViewDelegate::paintCertification(QPainter* painter, const QStyleOptionViewItem& option, const CStoreQueryModel* pModel, const QModelIndex& index) const
{
    auto certification = pModel->record(index.row()).value("certification").toInt();
    if(certification != 0)
    {
        QRect rcCertification(option.rect.right()-m_ribbonSize+9, option.rect.top()-9, m_ribbonSize, m_ribbonSize);
        // Choix de l'image à gérer en fonction du type de label (tested, quality...)
        QPixmap certiIcon(QString(":/Images/quality%1.png").arg(certification));
        // Draw ribbon corner at the top right corner
        painter->drawPixmap(rcCertification, certiIcon.scaled(m_ribbonSize, m_ribbonSize, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
}

#include "moc_CStorePluginListViewDelegate.cpp"
