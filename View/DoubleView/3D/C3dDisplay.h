#ifndef C3DDISPLAY_H
#define C3DDISPLAY_H

/**
 * @file      C3dDisplay.h
 * @author    Guillaume Demarcq and Ludovic Barusseau
 * @brief     Header file including C3dDisplay definition
 *
 * @details   Details
 */

#include "../CDataDisplay.h"
#include "View/Common/CGLWidget.h"

class C3dAnimationDlg;

/**
 * @brief The C3dDisplay class for displaying 3D data using OpenGL
 */
class C3dDisplay : public CDataDisplay
{
    Q_OBJECT

    public:

        C3dDisplay(QWidget* parent = nullptr, const QString& name = QString());

        bool            eventFilter(QObject *obj, QEvent *event) override;

        void            initLayout();
        void            initConnections();
        void            initToolBar();
        void            initSettings();
        void            initGL();

        void            changeVolume();
        void            updateRenderVolume();

        CGLWidget*      getGLWidget();

    signals:

        void            doUpdateRenderMode(size_t mode);
        void            doUpdateColormap(size_t colormap);
        void            doUpdateParam(size_t type, float value);

    public slots:

        void            onUseRaycasting();
        void            onUseMIP();
        void            onUseMinIP();
        void            onUseIso();
        void            onUpdateOffset(int value);
        void            onExportScreenshot();
        void            onExportAnimation();

    private:

        QPushButton*    createToolbarBtn(const QString &tooltip, const QIcon &icon);

    private:

        CGLWidget*          m_pGLWidget = nullptr;
        QPushButton*        m_pRaycastBtn = nullptr;
        QPushButton*        m_pMIPBtn = nullptr;
        QPushButton*        m_pMinIPBtn = nullptr;
        QPushButton*        m_pIsoBtn = nullptr;
        QPushButton*        m_pSettingsBtn = nullptr;
        QPushButton*        m_pScreenshotBtn = nullptr;
        QPushButton*        m_pAnimationBtn = nullptr;
        QComboBox*          m_pColorMapBtn = nullptr;
        QWidget*            m_pToolbar = nullptr;
        QVBoxLayout*        m_pVBoxLayout = nullptr;
        QWidget*            m_pSettings = nullptr;
        C3dAnimationDlg*    m_pAnimationDlg = nullptr;
};

#endif // C3DDISPLAY_H
