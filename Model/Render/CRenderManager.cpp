// Copyright (C) 2021 Ikomia SAS
// Contact: https://www.ikomia.com
//
// This file is part of the IkomiaStudio software.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "CRenderManager.h"
#include "VolumeRender.h"
#include "Main/LogCategory.h"
#include "Model/ProgressBar/CProgressBarManager.h"

CRenderManager::CRenderManager()
{
    m_pVolumeRender = new CVolumeRender;
    m_pVolumeRender->setProgressSignalHandler(&m_progressSignal);
}

CRenderManager::~CRenderManager()
{
    delete m_pVolumeRender;
}

void CRenderManager::setManagers(CProgressBarManager *pProgressMgr)
{
    m_pProgressMgr = pProgressMgr;
}

void CRenderManager::onInitCL(const std::string& vendorName)
{
    // Set interoperability CL-GL
    m_pVolumeRender->setCLGLInterop(true);
    // Init CL with appropriate GL context
    auto ret = m_pVolumeRender->initCL(vendorName);
    if(ret == RenderError::SUCCESS)
    {
        m_bInit = true;
        qCDebug(logRender) << "InitCL passed.";
    }
    else
    {
        m_bInit = false;
        qCDebug(logRender) << "InitCL failed.";
    }
    emit doEnableRender(m_bInit);
}

void CRenderManager::onInitPBO(unsigned int pbo, unsigned int pbo_depth)
{
    if(!m_bInit)
        return;

    try
    {
        m_pVolumeRender->initPBO(pbo, pbo_depth);
        m_pVolumeRender->initKernelArg();
    }
    catch(cl::Error& e)
    {
        qCCritical(logRender).noquote() << e.what() << "(" << QString::fromStdString(ocl::utils::getOpenCLErrorCodeStr(e.err())) << ")";
    }
}

void CRenderManager::updateVolumeRenderInput(CMat volume)
{
    assert(m_pProgressMgr);

    if(!m_bInit)
        return;

    m_pVolumeRender->setInputData(volume);

    try
    {
        m_pProgressMgr->launchProgress(&m_progressSignal, tr("Volume rendering..."), true);
        m_pVolumeRender->init();
        emit doUpdateVolumeRender();
    }
    catch(cl::Error& e)
    {
        qCCritical(logRender).noquote() << e.what() << "(" << QString::fromStdString(ocl::utils::getOpenCLErrorCodeStr(e.err())) << ")";
    }
}

void CRenderManager::onRenderVolume(QMatrix4x4& model, QMatrix4x4& view, QMatrix4x4& projection)
{
    if(!m_bInit)
        return;

    try
    {
        m_pVolumeRender->setMatrix(model, view, projection);
        m_pVolumeRender->execute();
    }
    catch(cl::Error& e)
    {
        qCCritical(logRender).noquote() << e.what() << "(" << QString::fromStdString(ocl::utils::getOpenCLErrorCodeStr(e.err())) << ")";
    }
}

void CRenderManager::onUpdateWindowSize(size_t width, size_t height)
{
    if(!m_bInit)
        return;

    m_pVolumeRender->setWndSize(width, height);
}

void CRenderManager::onUpdateRenderMode(size_t mode)
{
    if(!m_bInit)
        return;

    switch(mode)
    {
        case 0:
            m_pVolumeRender->setRenderMode(RenderMode::RAYCAST);
            break;
        case 1:
            m_pVolumeRender->setRenderMode(RenderMode::MIP);
            break;
        case 2:
            m_pVolumeRender->setRenderMode(RenderMode::MinIP);
            break;
        case 3:
            m_pVolumeRender->setRenderMode(RenderMode::ISOSURFACE);
            break;
        default:
            break;
    }

    emit doUpdateVolumeRender();
}

void CRenderManager::onUpdateColormap(size_t colormap)
{
    if(!m_bInit)
        return;

    switch(colormap)
    {
        case 0:
            m_pVolumeRender->setColormap(RenderColormap::CLASSIC);
            break;
        case 1:
            m_pVolumeRender->setColormap(RenderColormap::SKIN);
            break;
        case 2:
            m_pVolumeRender->setColormap(RenderColormap::JET);
            break;
        case 3:
            m_pVolumeRender->setColormap(RenderColormap::GRAYCOLOR);
            break;
        default:
            break;
    }

    emit doUpdateVolumeRender();
}

void CRenderManager::onUpdateRenderParam(size_t type, float value)
{
    if(!m_bInit)
        return;

    m_pVolumeRender->setRayParam(static_cast<RenderParamType>(type), value);
    emit doUpdateVolumeRender();
}

void CRenderManager::addBinary(CMat binary)
{
    if(!m_bInit)
        return;

    try
    {
        m_pVolumeRender->setBinaryData(binary);
        m_pVolumeRender->init();
        emit doUpdateVolumeRender();
    }
    catch(cl::Error& e)
    {
        qCCritical(logRender).noquote() << e.what() << "(" << QString::fromStdString(ocl::utils::getOpenCLErrorCodeStr(e.err())) << ")";
    }
}

void CRenderManager::clearBinary()
{
    if(!m_bInit)
        return;

    try
    {
        m_pVolumeRender->setBinaryData(CMat());
        m_pVolumeRender->init();
        emit doUpdateVolumeRender();
    }
    catch(cl::Error& e)
    {
        qCCritical(logRender).noquote() << e.what() << "(" << QString::fromStdString(ocl::utils::getOpenCLErrorCodeStr(e.err())) << ")";
    }
}

#include "moc_CRenderManager.cpp"
