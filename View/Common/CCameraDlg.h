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

#ifndef CCAMERADLG_H
#define CCAMERADLG_H

#include "View/Common/CDialog.h"

class CCameraDlg : public CDialog
{
    public:
        explicit    CCameraDlg(QWidget *parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags());

        void        addCam(const QString& name, int id);
        void        addCam(const QString& name, const QString& path);

        void        findCamera();

        QString     getCameraName();
        QString     getCameraMode();

    private:
        void        init();
        void        addIPCam();
        void        addManualCam();
        void        addCamUSB_Linux(std::vector<QString>& camName, std::vector<QString>& camPath);
        void        addCamUSB_Mac(std::vector<QString>& camName, std::vector<QString>& camPath);
        void        addCamUSB_Win(std::vector<QString>& camName, std::vector<QString>& camPath);

    private:
        QVBoxLayout*    m_pLayout = nullptr;
        QString         m_cameraName;
        QString         m_cameraMode;
        bool            m_bUseOpenNI = false;
};

#endif // CCAMERADLG_H
