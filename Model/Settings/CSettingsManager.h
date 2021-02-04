#ifndef CSETTINGSMANAGER_H
#define CSETTINGSMANAGER_H

#include <QSqlDatabase>
#include <QObject>
#include <QFileDialog>

class CWizardManager;

class CSettingsManager : public QObject
{
    Q_OBJECT

    public:

        CSettingsManager();

        void        init();
        void        notifyViewShow();

        std::string getProtocolSaveFolder() const;

        bool        isNativeDlgEnabled() const;
        bool        isTutorialEnabled() const;

        void        enableTutorial(bool bEnable);

        static QFileDialog::Options dialogOptions();

    signals:

        void        doEnableNativeDialog(bool bEnable);
        void        doEnableTutorialHelper(bool bEnable);
        void        doSetProtocolSaveFolder(const QString& path);

    public slots:

        void        onUseNativeDlg(bool bEnable);
        void        onEnableTutorialHelper(bool bEnable);
        void        onSetProtocolSaveFolder(const QString& path);

    private:

        void        initMainDb();
        void        initNativeDialogOption();
        void        initTutorialHelperOption();
        void        initProtocolOption();

        void        setSettings(const QString& category, const QJsonObject &jsonData);
        void        setUseNativeDlg(bool bEnable);
        void        setTutoEnabled(bool bEnable);

        static void setDialogOptions(QFileDialog::Options options);

        QJsonObject getSettings(const QString& category) const;

    private:

        static QFileDialog::Options m_dlgOptions;

        QSqlDatabase    m_mainDb;
        bool            m_bUseNativeDlg = false;
        bool            m_bShowTuto = true;
        std::string     m_protocolSaveFolder;
};

#endif // CSETTINGSMANAGER_H
