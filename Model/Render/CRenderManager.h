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

#ifndef CRENDERMANAGER_H
#define CRENDERMANAGER_H

#include <QObject>
#include "Data/CMat.hpp"
#include "CProgressSignalHandler.h"

class CVolumeRender;
class CProgressBarManager;

class CRenderManager : public QObject
{
    Q_OBJECT

    public:

        CRenderManager();
        ~CRenderManager();

        void    setManagers(CProgressBarManager* pProgressMgr);

        void    updateVolumeRenderInput(CMat volume);

        void    addBinary(CMat binary);
        void    clearBinary();

    signals:

        void    doUpdateVolumeRender();
        void    doEnableRender(int bEnable);

    public slots:

        void    onInitCL(const std::string& vendorName);
        void    onInitPBO(unsigned int pbo, unsigned int pbo_depth);
        void    onRenderVolume(QMatrix4x4& model, QMatrix4x4& view, QMatrix4x4& projection);
        void    onUpdateWindowSize(size_t width, size_t height);
        void    onUpdateRenderMode(size_t mode);
        void    onUpdateColormap(size_t colormap);
        void    onUpdateRenderParam(size_t type, float value);

    private:

        CVolumeRender*          m_pVolumeRender = nullptr;
        bool                    m_bInit = false;
        CProgressBarManager*    m_pProgressMgr = nullptr;
        CProgressSignalHandler  m_progressSignal;
};

#endif // CRENDERMANAGER_H
