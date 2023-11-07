#ifndef CTEXTDISPLAY_H
#define CTEXTDISPLAY_H

#include "CDataDisplay.h"

class CTextDisplay : public CDataDisplay
{
    public:

        enum TextDisplayBar
        {
            NONE = 0x00000000,
            CHECKBOX = 0x00000001,
            TITLE = 0x00000002,
            MAXIMIZE_BUTTON = 0x0000004,
            CLOSE_BUTTON = 0x0000008,
            EXPORT_BUTTON = 0x00000010,
            DEFAULT = TITLE | EXPORT_BUTTON | MAXIMIZE_BUTTON
        };

        CTextDisplay(const QString& name, QWidget* parent=nullptr, int flags=DEFAULT);
        CTextDisplay(const QString& name, const QString& text, QWidget* parent=nullptr, int flags=DEFAULT);

        void            setText(const QString& text);

    protected:

        bool            eventFilter(QObject* obj, QEvent* event) override;

    private:

        void            initLayout(const QString &text="");
        void            initConnections();

        QPushButton*    createButton(const QIcon& icon);

    private slots:

        void            onExportBtnClicked();

    private:

        QPlainTextEdit* m_pTextEdit = nullptr;
        QPushButton*    m_pExportBtn = nullptr;
};

#endif // CTEXTDISPLAY_H
