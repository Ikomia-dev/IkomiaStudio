#ifndef C3DANIMATIONDLG_H
#define C3DANIMATIONDLG_H

#include "View/Common/CDialog.h"
#include "Model/Render/C3dAnimation.h"

class C3dAnimationDlg : public CDialog
{
    Q_OBJECT

    public:

        C3dAnimationDlg(QWidget *parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags());

        void            addAnimationSequence(const C3dAnimationSequence& sequence);

    private:

        void            initLayout();
        void            initConnections();

        QPushButton*    createButton(const QString& text, const QIcon& icon);

        QString         getSequenceText(const C3dAnimationSequence &sequence) const;

    signals:

        void            doStartRecording();
        void            doStopRecording();
        void            doPlayAnimation(const C3dAnimation& animation);
        void            doMakeVideoAnimation(const QString& path, const C3dAnimation& animation);

    private slots:

        void            onActionChanged(int index);
        void            onAddSequence();
        void            onPlayAnimation();
        void            onRemoveSequence();
        void            onClearAnimation();
        void            onExportAnimation();
        void            onLoadAnimation();
        void            onCreateAnimation();

    private:

        QComboBox*          m_pComboAction = nullptr;
        QLabel*             m_pLabelParam = nullptr;
        QLineEdit*          m_pEditFps = nullptr;
        QLineEdit*          m_pEditParam = nullptr;
        QPushButton*        m_pBtnAdd = nullptr;
        QPushButton*        m_pBtnPlay = nullptr;
        QPushButton*        m_pBtnRemove = nullptr;
        QPushButton*        m_pBtnClear = nullptr;
        QPushButton*        m_pBtnExport = nullptr;
        QPushButton*        m_pBtnLoad = nullptr;
        QPushButton*        m_pBtnCreate = nullptr;
        QPushButton*        m_pBtnCancel = nullptr;
        QListView*          m_pAnimationView = nullptr;
        QStringListModel*   m_pAnimationModel = nullptr;
        bool                m_bRecording = false;
        C3dAnimation        m_animation;
};

#endif // C3DANIMATIONDLG_H
