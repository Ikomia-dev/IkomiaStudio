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

#ifndef CRESULTTABLEDISPLAY_H
#define CRESULTTABLEDISPLAY_H

#include "View/DoubleView/CDataDisplay.h"

class CResultTableView;
class CMeasuresTableModel;
class CFeaturesTableModel;

class CResultTableDisplay : public CDataDisplay
{
    Q_OBJECT

    public:

        enum CResultTableDisplayBar
        {
            NONE = 0x00000000,
            CHECKBOX = 0x00000001,
            TITLE = 0x00000002,
            MAXIMIZE_BUTTON = 0x0000004,
            CLOSE_BUTTON = 0x0000008,
            SAVE_BUTTON = 0x00000010,
            EXPORT_BUTTON = 0x00000020,
            DEFAULT = TITLE | EXPORT_BUTTON | MAXIMIZE_BUTTON
        };

        CResultTableDisplay(QWidget* parent = nullptr, const QString &name = "", int flags=DEFAULT);

        void            setModel(CMeasuresTableModel* pModel);
        void            setModel(CFeaturesTableModel* pModel);

    signals:

        void            doSave();
        void            doExport(const QString& path);

    public slots:

        void            onExportBtnClicked();

    protected:

        bool            eventFilter(QObject* obj, QEvent* event) override;

    private:

        void            initLayout();
        void            initConnections();

        QPushButton*    createButton(const QIcon& icon);

    private:

        CResultTableView*   m_pView = nullptr;
        QPushButton*        m_pSaveBtn = nullptr;
        QPushButton*        m_pExportBtn = nullptr;
};

#endif // CRESULTTABLEDISPLAY_H
