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

#ifndef CROLLUPBTN_H
#define CROLLUPBTN_H

#include <QFrame>
#include <QLabel>
#include <QToolButton>
#include <QVBoxLayout>

class CLabelBtn : public QToolButton
{
    public:
        CLabelBtn(QWidget* pParent = nullptr) : QToolButton(pParent)
        {
            init();
        }

        inline void init()
        {
            setProperty("class", "CLabelBtn");

            setCursor(Qt::PointingHandCursor);
            setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
            setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
            setFocusPolicy(Qt::StrongFocus);
        }

        inline QSize sizeHint() const
        {
            ensurePolished();

            int w = 0, h = 0;

            QStyleOptionToolButton opt;
            initStyleOption(&opt);

            QString s(text());
            bool empty = s.isEmpty();
            if (empty)
                s = QString::fromLatin1("XXXX");
            QFontMetrics fm = fontMetrics();
            QSize sz = fm.size(Qt::TextShowMnemonic, s);
            if(!empty || !w)
                w += sz.width();
            if(!empty || !h)
                h = qMax(h, sz.height());
            opt.rect.setSize(QSize(w, h)); // PM_MenuButtonIndicator depends on the height

            if (!icon().isNull()) {
                int ih = opt.iconSize.height();
                int iw = opt.iconSize.width() + 4;
                w += iw;
                h = qMax(h, ih);
            }

            if (menu())
                w += style()->pixelMetric(QStyle::PM_MenuButtonIndicator, &opt, this);

            h += 4;
            w += 8;

            QSize sizeHint = style()->sizeFromContents(QStyle::CT_PushButton, &opt, QSize(w, h), this);
            return sizeHint;
        }

        inline QSize minimumSizeHint() const
        {
            return sizeHint();
        }


};

class CRollupBtn : public QFrame
{
        Q_OBJECT
    public:
        CRollupBtn(const QIcon& icon, const QString& title, bool bExpandable, QWidget* pParent = nullptr);

        void            setExpandable(bool bExpandable);
        bool            isExpandable();

        bool            isFold();
        void            setFold(bool bFold);

        void            setText(const QString& text);
        void            setWidget(QWidget* pWidget);
        void            removeWidget(QWidget* pWidget);
        void            replaceWidget(QWidget* pOldWidget, QWidget* pNewWidget);

        void            toggleWidget(QWidget* pWidget);

    signals:
        void            doClicked();

    public slots:
        void            onFold();

    private:
        virtual void    keyPressEvent (QKeyEvent* event);
        virtual void    keyReleaseEvent (QKeyEvent* event);

        bool            eventFilter(QObject *obj, QEvent *event);
        void            changeIcons();

        void            processShow(QWidget* pWidget);
        void            processHide(QWidget* pWidget);

    private:
        QVBoxLayout* m_pVBoxLayout = nullptr;
        QHBoxLayout* m_pHBoxLayout = nullptr;
        CLabelBtn*  m_pLabelBtn = nullptr;
        QLabel*     m_pLabelIcon = nullptr;
        QWidget*     m_pFrame = nullptr;
        double      m_opacity = 0.1;
        bool        m_bExpandable = true;
        bool        m_bFold = true;
        bool        m_bOverIcon = false;
        bool        m_bOver = false;
};

#endif // CROLLUPBTN_H
