#ifndef CPROCESSDOCCONTENT_H
#define CPROCESSDOCCONTENT_H

#include <QObject>

class CProcessDocContent : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString text MEMBER m_text NOTIFY doTextChanged FINAL)

    public:

        CProcessDocContent(QObject *parent = nullptr);

        void setText(const QString &text);

    signals:

        void doTextChanged(const QString &text);

    private:

        QString m_text = "";
};

#endif // CPROCESSDOCCONTENT_H
