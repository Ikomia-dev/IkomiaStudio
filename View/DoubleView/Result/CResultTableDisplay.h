#ifndef CRESULTTABLEDISPLAY_H
#define CRESULTTABLEDISPLAY_H

#include "View/DoubleView/CDataDisplay.h"

class CResultTableView;
class CMeasuresTableModel;
class CFeaturesTableModel;

class CResultTableDisplay : public CDataDisplay
{
    Q_OBJECT

    public:

        enum CResultTableDisplayBar
        {
            NONE = 0x00000000,
            CHECKBOX = 0x00000001,
            TITLE = 0x00000002,
            MAXIMIZE_BUTTON = 0x0000004,
            CLOSE_BUTTON = 0x0000008,
            SAVE_BUTTON = 0x00000010,
            EXPORT_BUTTON = 0x00000020,
            DEFAULT = TITLE | EXPORT_BUTTON | MAXIMIZE_BUTTON
        };

        CResultTableDisplay(QWidget* parent = nullptr, const QString &name = "", int flags=DEFAULT);

        void            setModel(CMeasuresTableModel* pModel);
        void            setModel(CFeaturesTableModel* pModel);

    signals:

        void            doSave();
        void            doExport(const QString& path);

    public slots:

        void            onExportBtnClicked();

    protected:

        bool            eventFilter(QObject* obj, QEvent* event) override;

    private:

        void            initLayout();
        void            initConnections();

        QPushButton*    createButton(const QIcon& icon);

    private:

        CResultTableView*   m_pView = nullptr;
        QPushButton*        m_pSaveBtn = nullptr;
        QPushButton*        m_pExportBtn = nullptr;
};

#endif // CRESULTTABLEDISPLAY_H
