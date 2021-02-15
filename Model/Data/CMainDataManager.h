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

#ifndef CMAINDATAMANAGER_H
#define CMAINDATAMANAGER_H

#include <QObject>
#include "Image/CImgManager.h"
#include "Video/CVideoManager.h"
#include "Main/AppDefine.hpp"

class CProgressSignalHandler;
class CImageScene;

class CMainDataManager : public QObject
{
    Q_OBJECT

    public:

        CMainDataManager();
        ~CMainDataManager();

        CImgManager*    getImgMgr();
        CVideoManager*  getVideoMgr();
        QString         getDataPath(const QModelIndex& wrapIndex);
        CDataInfoPtr    getDataInfoPtr(const QModelIndex& wrapIndex);
        int             getSelectedDisplayCategory() const;
        int             getSelectedDisplayIndex() const;

        void            setManagers(CProjectManager* pProjectMgr, CProtocolManager* pProtocolMgr, CGraphicsManager* pGraphicsMgr,
                                    CResultManager *pResultMgr, CRenderManager* pRenderMgr, CProgressBarManager* pProgressMgr);
        void            setProgressSignalHandler(CProgressSignalHandler* pHandler);

        void            closeData(const QModelIndex &index);

        void            displaySimpleImage(CImageScene* pScene, const QModelIndex& index, const QModelIndex& wrapIndex, size_t inputIndex, bool bNewSequence);
        void            displayVolumeImage(CImageScene* pScene, const QModelIndex& index, const QModelIndex& wrapIndex, bool bNewSequence);
        void            displayVideoImage(const QModelIndex& index, size_t inputIndex, bool bNewSequence);
        void            displayImageSequence(const QModelIndex& index, size_t inputIndex, bool bNewSequence);

        void            beforeProjectClose(int projectIndex, bool bWithCurrentImage);

        void            reloadCurrent();

    public slots:

        void            onSetSelectedDisplay(DisplayCategory category, int index);

        void            onSaveCurrentVideoFrame(const QModelIndex &modelIndex, int index);

        void            onExportCurrentImage(int index, const QString &path, bool bWithGraphics);
        void            onExportCurrentVideoFrame(const QModelIndex &index, int inputIndex, const QString &path, bool bWithGraphics);

        void            onEnableInfo(bool bEnable);

        void            onPlayVideo(int index);

        void            onStopVideo(const QModelIndex &index);

    private:

        CImgManager         m_imgMgr;
        CVideoManager       m_videoMgr;
        CProjectManager*    m_pProjectMgr = nullptr;
        CProtocolManager*   m_pProtocolMgr = nullptr;
        int                 m_selectedDisplayCategory = DisplayCategory::INPUT;
        int                 m_selectedDisplayIndex = 0;
};

#endif // CMAINDATAMANAGER_H
