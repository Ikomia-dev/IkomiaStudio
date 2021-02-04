#ifndef CPROCESSEDITDOCFRAME_H
#define CPROCESSEDITDOCFRAME_H

#include <QFrame>
#include "Core/CProcessInfo.h"
#include "Model/User/CUser.h"

class CProcessEditDocFrame : public QFrame
{
    Q_OBJECT

    public:

        CProcessEditDocFrame(QWidget *parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags());

        void        setCurrentUser(const CUser& user);
        void        setProcessInfo(const CProcessInfo& info);

    signals:

        void        doSave(bool bFullEdit, const CProcessInfo& info);
        void        doCancel();

    private:

        void        initLayout();

        QWidget*    createFullEditWidget();
        QWidget*    createPartialEditWidget();

        void        fillForm();

    private slots:

        void        onSave();
        void        onBrowseIconPath();

    private:

        bool            m_bFullEdit = false;
        CUser           m_currentUser;
        CProcessInfo    m_originalInfo;
        QStackedWidget* m_pStackWidget = nullptr;
        QTextEdit*      m_pTextEditDescription = nullptr;
        QTextEdit*      m_pTextEditKeywords = nullptr;
        QTextEdit*      m_pTextEditKeywords2 = nullptr;
        QTextEdit*      m_pTextEditAuthors = nullptr;
        QTextEdit*      m_pTextEditArticle = nullptr;
        QTextEdit*      m_pTextEditJournal = nullptr;
        QSpinBox*       m_pSpinYear = nullptr;
        QLineEdit*      m_pEditDocLink = nullptr;
        QLineEdit*      m_pEditVersion = nullptr;
        QLineEdit*      m_pEditIconPath = nullptr;
        QRadioButton*   m_pRadioLinux = nullptr;
        QRadioButton*   m_pRadioMac = nullptr;
        QRadioButton*   m_pRadioWin = nullptr;
};

#endif // CPROCESSEDITDOCFRAME_H
