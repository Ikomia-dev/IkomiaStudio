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

#ifndef CPATHNAVIGATOR_H
#define CPATHNAVIGATOR_H

#include <QFrame>
#include <QHBoxLayout>
#include <QPushButton>
#include <QScrollArea>

class CPathButton : public QPushButton
{
        Q_OBJECT
    public:
        CPathButton(QWidget* pParent = nullptr) : QPushButton(pParent)
        {
            setIcon(QIcon(":/Images/chevron.png"));
            setLayoutDirection(Qt::RightToLeft);
        }
        void    setIndex(const QPersistentModelIndex& index)
        {
            m_index = index;
        }

        void    initConnections()
        {
            connect(this, &CPathButton::clicked, this, &CPathButton::onUpdateIndex);
        }

        void    setFullText(const QString text)
        {
            m_text = text;
            QFontMetrics metrics(font());
            QString elidedText = metrics.elidedText(text, Qt::ElideRight, width()-32);
            setText(elidedText);
        }

        void    updateText()
        {
            // Update elided text according to button size
            QFontMetrics metrics(font());
            QString elidedText = metrics.elidedText(m_text, Qt::ElideRight, width()-32);
            setText(elidedText);
        }

        void    resizeEvent(QResizeEvent* event)
        {
            QPushButton::resizeEvent(event);
            updateText();
        }

        QSize   sizeHint() const
        {
            QFontMetrics metrics(font());
            int textWidth = metrics.horizontalAdvance(m_text);

            // Check if text size fits in button and return appropriate button size
            if(textWidth < maximumWidth())
                return QSize(textWidth + 42, 26);
            else
                return QSize(maximumWidth(), 26);
        }
    signals:
        void doUpdateIndex(const QModelIndex& index);

    public slots:
        void onUpdateIndex()
        {
            emit doUpdateIndex(m_index);
        }

    private:
        QPersistentModelIndex           m_index;
        QString                         m_text;
};

class CPathNavigator : public QFrame
{
        Q_OBJECT
    public:
        CPathNavigator();

        void    clear();
        void    addPath(const QPersistentModelIndex& index);

        void    resizeEvent(QResizeEvent* event);
        QSize   sizeHint() const;

    signals:
        void    doUpdateIndex(const QModelIndex& index);
        void    doGoHome(const QModelIndex& index);

    public slots:
        void    onUpdateIndex(const QModelIndex& index);
        void    onGoHome();

    private:
        bool    isDataItem(const QModelIndex& index);

    private:
        QHBoxLayout*            m_pLayout = nullptr;
        QList<CPathButton*>     m_buttons;
        QPersistentModelIndex   m_parentIndex;
};

#endif // CPATHNAVIGATOR_H
