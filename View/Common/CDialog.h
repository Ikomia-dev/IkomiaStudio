#ifndef CDIALOG_H
#define CDIALOG_H

#include <QDialog>
#include <QFileDialog>
#include "Main/forwards.hpp"
#include "View/Common/CFramelessHelper.h"
#include "Model/Settings/CSettingsManager.h"
#include <memory>

class CDialog : public QDialog
{
    Q_OBJECT

    public:

        enum CDialogStyle
        {
            NONE = 0x00000000,
            LABEL = 0x00000001,
            MAXIMIZE_BUTTON = 0x00000002,
            CLOSE_BUTTON = 0x00000004,
            EFFECT_ENABLED = 0x00000008,
            NO_TITLE_BAR = 0x00000010,
            RESIZABLE = 0x00000020,
            MOVABLE = 0x00000040,
            DEFAULT = LABEL | CLOSE_BUTTON | RESIZABLE | MOVABLE,
            DEFAULT_FIXED = LABEL | CLOSE_BUTTON,
            DEFAULT_MOVABLE = DEFAULT_FIXED | MOVABLE
        };

        CDialog(QWidget * parent = Q_NULLPTR, int style=DEFAULT, Qt::WindowFlags f = Qt::WindowFlags());
        CDialog(const QString title, QWidget * parent = Q_NULLPTR, int style=DEFAULT, Qt::WindowFlags f = Qt::WindowFlags());

        void            setTitle(const QString& title);

        QVBoxLayout*    getContentLayout() const;
        int             getBorderSize() const;

        void            showCustomMaximized();

    private:

        void            initLayout();
        void            initTitleBar();
        void            initFramelessHelper();

        void            applyEffects();

        QPushButton*    createTitleButton(const QString iconPath);

    private:

        int             m_styleFlags = DEFAULT;
        QString         m_title = "";
        QLabel*         m_pLabelTitle = nullptr;
        QFrame*         m_pTitleBar = nullptr;
        QVBoxLayout*    m_pContentLayout = nullptr;
        int             m_titleBarHeight = 22;
        bool            m_bIsMaximized = false;

        std::unique_ptr<CFramelessHelper> m_pViewMover = nullptr;

    protected:

        QFrame*         m_pFrame = nullptr;
};

#endif // CDIALOG_H
