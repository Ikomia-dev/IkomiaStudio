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

#ifndef CDIALOG_H
#define CDIALOG_H

#include <QDialog>
#include <QFileDialog>
#include "Main/forwards.hpp"
#include "View/Common/CFramelessHelper.h"
#include "Model/Settings/CSettingsManager.h"
#include <memory>

class CDialog : public QDialog
{
    Q_OBJECT

    public:

        enum CDialogStyle
        {
            NONE = 0x00000000,
            LABEL = 0x00000001,
            MAXIMIZE_BUTTON = 0x00000002,
            CLOSE_BUTTON = 0x00000004,
            EFFECT_ENABLED = 0x00000008,
            NO_TITLE_BAR = 0x00000010,
            RESIZABLE = 0x00000020,
            MOVABLE = 0x00000040,
            DEFAULT = LABEL | CLOSE_BUTTON | RESIZABLE | MOVABLE,
            DEFAULT_FIXED = LABEL | CLOSE_BUTTON,
            DEFAULT_MOVABLE = DEFAULT_FIXED | MOVABLE
        };

        CDialog(QWidget * parent = Q_NULLPTR, int style=DEFAULT, Qt::WindowFlags f = Qt::WindowFlags());
        CDialog(const QString title, QWidget * parent = Q_NULLPTR, int style=DEFAULT, Qt::WindowFlags f = Qt::WindowFlags());

        void            setTitle(const QString& title);

        QVBoxLayout*    getContentLayout() const;
        int             getBorderSize() const;

        void            showCustomMaximized();

    private:

        void            initLayout();
        void            initTitleBar();
        void            initFramelessHelper();

        void            applyEffects();

        QPushButton*    createTitleButton(const QString iconPath);

    private:

        int             m_styleFlags = DEFAULT;
        QString         m_title = "";
        QLabel*         m_pLabelTitle = nullptr;
        QFrame*         m_pTitleBar = nullptr;
        QVBoxLayout*    m_pContentLayout = nullptr;
        int             m_titleBarHeight = 22;
        bool            m_bIsMaximized = false;

        std::unique_ptr<CFramelessHelper> m_pViewMover = nullptr;

    protected:

        QFrame*         m_pFrame = nullptr;
};

#endif // CDIALOG_H
