#ifndef CCPPNEWPLUGINWIDGET_H
#define CCPPNEWPLUGINWIDGET_H

#include <QWidget>

class CCppNewPluginWidget : public QWidget
{
    Q_OBJECT

    public:

        explicit CCppNewPluginWidget(QWidget *parent = nullptr);

    signals:

        void    doPluginCreated(const QString& pluginName);

    private slots:

        void    onBrowseSrcFolder();
        void    onBrowseApiFolder();
        void    onGeneratePlugin();

    private:

        void    initLayout();
        void    initApiFolder();
        void    initConnections();

        QString getSelectedFolder();

    private:

        QLineEdit*          m_pEditCppName = nullptr;
        QLineEdit*          m_pEditCppSrcFolder = nullptr;
        QLineEdit*          m_pEditCppApiFolder = nullptr;
        QPushButton*        m_pBtnBrowseSrcFolder = nullptr;
        QPushButton*        m_pBtnBrowseApiFolder = nullptr;
        QComboBox*          m_pComboCppProcessType = nullptr;
        QComboBox*          m_pComboCppWidgetType = nullptr;
        QPushButton*        m_pBtnGenerate = nullptr;
};

#endif // CCPPNEWPLUGINWIDGET_H
