#include "CProcessDocContent.h"

CProcessDocContent::CProcessDocContent(QObject *parent) : QObject(parent)
{
}

void CProcessDocContent::setText(const QString &text)
{
    if(text == m_text)
        return;

    m_text = text;
    emit doTextChanged(m_text);
}
