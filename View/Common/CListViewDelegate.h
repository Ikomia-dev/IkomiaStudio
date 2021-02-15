/*
 * Copyright (C) 2021 Ikomia SAS
 * Contact: https://www.ikomia.com
 *
 * This file is part of the IkomiaStudio software.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef CLISTVIEWDELEGATE_H
#define CLISTVIEWDELEGATE_H

#include <QStyledItemDelegate>

/**
 * @brief
 *
 */
class CListViewDelegate : public QStyledItemDelegate
{
    Q_OBJECT

    public:

        CListViewDelegate(QObject* parent = nullptr);

        void            setSizeHint(QSize sizeHint);
        void            setIconSize(QSize size);

    protected:

        virtual bool    editorEvent(QEvent *event, QAbstractItemModel *model,
                                    const QStyleOptionViewItem &option, const QModelIndex &index) Q_DECL_OVERRIDE;

        virtual QSize   sizeHint(const QStyleOptionViewItem & option, const QModelIndex & index ) const override;

    protected:

        virtual void    initBrush();

        void            paintBoundingRect(QPainter* painter, const QStyleOptionViewItem& option) const;
        void            paintIcon(QPainter* painter, const QStyleOptionViewItem &option, const QModelIndex& index) const;
        void            paintStars(QPainter* painter, QRect& rect, const QModelIndex& index) const;
        void            paintActionButtons(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex &index) const;
        void            paintBtn(QPainter* painter, const QRect& rect, const QString &pixmapPath, bool bEnabled, bool bHovered) const;
        QSize           paintStaticText(QPainter* painter, int x, int y, int w, const QString& text, QFont font, const QColor& color) const;

        void            updateStarRating(const QModelIndex& index, int nbStar);

        virtual void    executeAction(int action, const QModelIndex &index) = 0;

        QRect           getBtnRect(int index, const QStyleOptionViewItem &option) const;        
        QIcon           getDataIcon(const QModelIndex& index) const;
        QString         getDataName(const QModelIndex& index) const;
        QString         getShortDescription(const QModelIndex& index) const;
        QString         getDescription(const QModelIndex& index) const;
        QIcon           getRoundedIcon(QIcon icon) const;

        // Pure virtual -> must be defined
        virtual QString getActionIconPath(int action) const = 0;
        virtual int     getBtnAction(int index) const = 0;

        virtual bool    isBtnEnabled(const QModelIndex& itemIndex, int index) const = 0;

        virtual void    showTooltip(const QModelIndex& modelIndex, const QPoint& pos, int index) const = 0;

    protected:

        int                     m_actionBtnCount = 2;
        QSize                   m_sizeHint = QSize(200, 200);
        QSize                   m_iconSize = QSize(48, 48);
        QSize                   m_btnSize = QSize(22, 22);
        QSize                   m_spacings = QSize(5, 2);
        QLinearGradient         m_headerGradient;
        QLinearGradient         m_headerGradientLight;
        QLinearGradient         m_bodyGradient;
        QLinearGradient         m_bodyGradientLight;
        QMargins                m_contentMargins = QMargins(5, 5, 5, 5);
        QMap<QModelIndex, int>  m_mapBtnHover;
        int                     m_headerHeight = 24;
};

#endif // CLISTVIEWDELEGATE_H
