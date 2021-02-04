#ifndef CMAINCTRL_H
#define CMAINCTRL_H

#include <QObject>
#include "View/Main/CMainView.h"
#include "Model/CMainModel.h"
#include "Model/Wizard/CWizardManager.h"

/**
 * @brief
 *
 */
class CMainCtrl : public QObject
{
    public:

        /**
         * @brief
         *
         * @param pModel
         * @param pView
         * @param pSplash
         */
        CMainCtrl(CMainModel* pModel, CMainView* pView, QSplashScreen* pSplash);

        void    show();
        void    launchWizard();

    private:

        void    initSplash();
        void    initConnections();
        void    initProjectConnections();
        void    initProcessConnections();
        void    initProtocolConnections();
        void    initInfoConnections();
        void    initRenderConnections();
        void    initProgressConnections();
        void    initGraphicsConnections();
        void    initResultsConnections();
        void    initVideoConnections();
        void    initUserConnections();
        void    initDataConnections();
        void    initStoreConnections();
        void    initWizardConnections();
        void    initSettingsConnections();
        void    initPluginConnections();

    private:

        CMainModel*         m_pModel = nullptr;
        CMainView*          m_pView = nullptr;
        QSplashScreen*      m_pSplash = nullptr;

        // Tutorial manager
        CWizardManager      m_tutorialMgr;
};

#endif // CMAINCTRL_H
