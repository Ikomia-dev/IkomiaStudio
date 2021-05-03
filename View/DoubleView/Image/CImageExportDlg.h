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

#ifndef CIMAGEEXPORTDLG_H
#define CIMAGEEXPORTDLG_H

#include "View/Common/CDialog.h"

class CImageExportDlg: public CDialog
{
    public:

        enum DataType { IMAGE, IMAGE_SEQUENCE, VIDEO};

        CImageExportDlg(const QString& title, DataType type, QWidget * parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags());

        QString     getFileName() const;
        bool        isGraphicsExported() const;

    private:

        void        initLayout();
        void        initConnections();

    private slots:

        void        onBrowse();
        void        onValidate();

    private:

        DataType        m_dataType = IMAGE;
        QCheckBox*      m_pCheckBurnGraphics = nullptr;
        QLineEdit*      m_pEditPath = nullptr;
        QPushButton*    m_pBrowseBtn = nullptr;
        QPushButton*    m_pOkBtn = nullptr;
        QPushButton*    m_pCancelBtn = nullptr;
};

#endif // CIMAGEEXPORTDLG_H
