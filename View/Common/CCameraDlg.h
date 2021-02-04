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
