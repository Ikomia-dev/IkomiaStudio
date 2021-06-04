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

#ifndef CPROCESSPARAMETERS_H
#define CPROCESSPARAMETERS_H

#include "View/Common/CDialog.h"
#include "Main/forwards.hpp"
#include "Core/CTaskInfo.h"
#include "Model/User/CUser.h"

class CResizeStackedWidget;
class CProcessDocDlg;

class CProcessParameters : public CDialog
{
    Q_OBJECT

    public:

        CProcessParameters(QWidget *parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags());

        void            addWidget(QWidget* pWidget, QString processName);

        void            setCurrentWidget(const QString& processName);
        void            setProcessInfo(const CTaskInfo& info);
        void            setCurrentUser(const CUser& user);

        QWidget*        getWidget();

        bool            isCurrent(QString name) const;

        void            fitToContent();

        void            clear();

        void            remove(const QString& name);

    signals:

        void            doUpdateProcessInfo(bool bFullEdit, const CTaskInfo& info);

    protected:

        void            hideEvent(QHideEvent* event) override;

    private slots:

        void            onShowDocumentation();

    private:

        void            initLayout();
        void            initConnections();

        QFrame*         createLine();

        CTaskInfo    getProcessInfo() const;

        void            updateWidgetPosition(QWidget* pWidget, int borderSize);

    private:

        CResizeStackedWidget*   m_pParamWidgets = nullptr;
        QLabel*                 m_pLabel = nullptr;
        QPushButton*            m_pBtnDocumentation = nullptr;
        QList<QString>          m_widgetNames;
        CProcessDocDlg*         m_pDocDlg = nullptr;
};

#endif // CPROCESSPARAMETERS_H
