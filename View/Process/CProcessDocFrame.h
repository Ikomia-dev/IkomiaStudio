#ifndef CPROCESSDOCFRAME_H
#define CPROCESSDOCFRAME_H

#include "CProcessDocContent.h"
#include "Core/CProcessInfo.h"

class CProcessDocFrame : public QFrame
{
    public:

        CProcessDocFrame(QWidget *parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags());

        void    setProcessInfo(const CProcessInfo& info);

    private:

        void    initLayout();

        QString getMarkdownTemplate() const;

        QString generateMarkdown(const CProcessInfo& info) const;

    private:

        CProcessDocContent  m_content;
        QTextBrowser*       m_pBrowser = nullptr;
        QTextDocument*      m_pDoc = nullptr;
};

#endif // CPROCESSDOCFRAME_H
