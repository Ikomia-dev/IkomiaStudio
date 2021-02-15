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

#ifndef CNOTIFICATIONPOPUP_H
#define CNOTIFICATIONPOPUP_H

#include <QDialog>

class CLabelBtn;
class QLabel;
class QToolButton;
class QGridLayout;

class CNotificationPopup : public QDialog
{
    Q_OBJECT

    public:

        CNotificationPopup(QWidget* pParent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
        ~CNotificationPopup();

        void    showPopup(const QString& title, const QString description, const QIcon& icon = QIcon());

        void    addWidget(QWidget* pWidget, int x, int y);
        void    removeWidget(QWidget* pWidget);

        void    setDescriptionPos(int x, int y);
        void    setPadding(size_t padding);
        void    setDescription(const QString& text);

        void    mousePressEvent(QMouseEvent*);

    signals:

        void    doClose();
        void    doClicked();

    public slots:

        void    onClosePopup();

    private:

        void    initLayout();
        void    initConnections();

        void    updatePos();

    private:

        QToolButton*    m_pCloseBtn  = nullptr;
        QGridLayout*    m_pBodyLayout = nullptr;
        CLabelBtn*      m_pTitle  = nullptr;
        QLabel*         m_pDescription = nullptr;
        size_t          m_padding = 0;
        bool            m_bDescription = false;
};

class CNotificationPopupFactory
{
    public:
        inline CNotificationPopup* create(QWidget* pParent = nullptr)
        {
            return new CNotificationPopup(pParent);
        }
};

#endif // CNOTIFICATIONPOPUP_H
