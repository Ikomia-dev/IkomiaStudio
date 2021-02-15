/*
 * Copyright (C) 2021 Ikomia SAS
 * Contact: https://www.ikomia.com
 *
 * This file is part of the IkomiaStudio software.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

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
