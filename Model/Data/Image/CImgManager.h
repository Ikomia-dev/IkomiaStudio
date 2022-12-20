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

#ifndef CIMGMANAGER_H
#define CIMGMANAGER_H

#include "CImageDataManager.h"

class CImageScene;
class CProgressBarManager;
class CProjectManager;
class CResultManager;
class CRenderManager;
class CGraphicsManager;
class CViewPropertyIO;

using CImageDataMgrPtr = std::shared_ptr<CImageDataManager>;

class CImgManager : public QObject
{
    Q_OBJECT

    public:

        CImgManager();

        void                    setManagers(CProjectManager* pProjectMgr, CGraphicsManager* pGraphicsMgr, CResultManager* pResultMgr,
                                            CRenderManager* pRenderMgr, CProgressBarManager* pProgressMgr);
        void                    setProgressSignalHandler(CProgressSignalHandler* pHandler);

        CImageDataMgrPtr        getImageMgrPtr();
        CMat                    getImage(const QModelIndex& wrapIndex);
        CMat                    getImage(const QModelIndex& datasetWrapIndex, int imgIndex);

        void                    displaySimpleImage(CImageScene* pScene, const QModelIndex& index, const QModelIndex& wrapIndex, size_t inputIndex, bool bNewSequence);
        void                    displayVolumeImage(CImageScene* pScene, const QModelIndex& index, const QModelIndex& wrapIndex, bool bNewSequence);

        void                    exportImage(const QModelIndex& index, const QString &path, bool bWithGraphics);
        void                    exportImage(const CMat& image, const std::vector<ProxyGraphicsItemPtr> &graphics, const QString &path);

        void                    enableInfoUpdate(bool bEnable);

    public slots:

        void                    onCloseWorkflow();

    private:

        void                    displayImageInfo(const CMat& image, const QModelIndex& wrapIndex);

    signals:

        void                    doDisplayImage(int index, CImageScene* pScene, QImage image, QString name, CViewPropertyIO* pViewProp);
        void                    doDisplayImageInfo(const VectorPairString& infoMap);
        void                    doDisplayVolume(CImageScene* pScene, QImage image, QString imgName, bool bNewSequence, CViewPropertyIO* pViewProp);

        void                    doInputDataChanged(const QModelIndex& index, size_t inputIndex, bool bNewSequence);
        void                    doCurrentDataChanged(const QModelIndex& itemIndex, bool bNewSequence);

        void                    doUpdateCurrentImgIndex(size_t currentImgIndex);
        void                    doUpdateNbImg(size_t nbImg);

    private:

        CImageDataMgrPtr        m_pImgMgr = nullptr;
        CProjectManager*        m_pProjectMgr = nullptr;
        CProgressBarManager*    m_pProgressMgr = nullptr;
        CRenderManager*         m_pRenderMgr = nullptr;
        CGraphicsManager*       m_pGraphicsMgr = nullptr;
        CResultManager*         m_pResultMgr = nullptr;
        CProgressSignalHandler* m_pProgressSignal = nullptr;
        CDataInfoPtr            m_pCurrentDataInfo = nullptr;
        bool                    m_bInfoUpdate = false;
};

#endif // CIMGMANAGER_H
