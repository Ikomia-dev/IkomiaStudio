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
