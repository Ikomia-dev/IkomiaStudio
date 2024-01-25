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

#ifndef CNOTIFICATIONPANE_H
#define CNOTIFICATIONPANE_H

#include <QDialog>
#include <QMutex>
#include <QSortFilterProxyModel>
#include <QStringListModel>
#include <QDockWidget>
#include <mutex>

class QPlainTextEdit;
class QListView;
class QListWidget;
class QComboBox;
class QFrame;
class QLineEdit;
class QToolButton;
class CHtmlDelegate;

class CNotificationModel : public QStringListModel
{
    public:
        enum NotificationRole
        {
            categoryRole = Qt::UserRole + 1
        };
        CNotificationModel(QObject* parent = nullptr)
            :    QStringListModel(parent)
        {}

        QVariant data(const QModelIndex & index, int role = Qt::EditRole) const override
        {
            if (role == categoryRole)
            {
                auto itr = m_categoryMap.find(index.row());
                if (itr != m_categoryMap.end())
                    return itr->second;
            }
            else if (role == Qt::FontRole)
            {
                QFont font;
                font.setPointSize(1);
                return font;
            }

            return QStringListModel::data(index, role);
        }

        bool setData(const QModelIndex & index, const QVariant & value, int role = Qt::EditRole) override
        {
            if (role == categoryRole)
            {
                m_categoryMap[index.row()] = value.toString();
                emit dataChanged(index, index);
                return true;
            }

            return QStringListModel::setData(index, value, role);
        }

        void deleteItem(const QModelIndex& index)
        {
            if (!index.isValid() || index.row() >= stringList().size())
                return;

            removeRows(index.row(), 1);
        }

        void update()
        {
            emit dataChanged(createIndex(0,0), createIndex(rowCount()-1,0));
        }

    private:

        mutable std::mutex m_mutex;
        std::map<int, QString> m_categoryMap;
};

class CNotificationProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

    public:

        CNotificationProxyModel(QObject *parent = nullptr) : QSortFilterProxyModel(parent){}

        void addCategory(const QString& text)
        {
            m_categoryList.push_back(text);
        }

        void removeCategory(const QString& text)
        {
            m_categoryList.removeAll(text);
        }

        void clearCategory()
        {
            m_categoryList.clear();
        }

        QList<QString> getCategoryList()
        {
            return m_categoryList;
        }

        QVariant data(const QModelIndex &proxyIndex, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE
        {
            return QSortFilterProxyModel::data(proxyIndex, role);
        }
        void setSourceModel(QAbstractItemModel *sourceModel) Q_DECL_OVERRIDE
        {
            QSortFilterProxyModel::setSourceModel(sourceModel);
        }

    protected:

        virtual bool filterAcceptsRow(int source_row, const QModelIndex & source_parent) const override
        {
            // check the current item
            QModelIndex currIndex = sourceModel()->index(source_row, 0, source_parent);
            if(!currIndex.isValid())
                return false;

            auto text = currIndex.data().toString();
            auto categoryName = currIndex.data(CNotificationModel::categoryRole).toString();

            // Select row if contains some words from search box
            bool bRet = text.contains(filterRegExp());
            if(!m_categoryList.empty())
            {
                bRet = bRet && m_categoryList.contains(categoryName);
                for(auto&& it : m_categoryList)
                    bRet = bRet || text.contains(it, Qt::CaseInsensitive);
            }

            return bRet;
        }

    private:

        mutable std::mutex m_mutex;
        QList<QString> m_categoryList = {};
};

class CNotificationPane : public QWidget
{
    Q_OBJECT

    public:

        CNotificationPane(QWidget* pParent = nullptr);
        ~CNotificationPane();

        void    init();
        void    initLayout();
        void    initConnections();
        void    initParams();

        void    addNotification(const QString& text);

        void    setPadding(size_t padding);
        void    setHtmlFontSize(size_t ftSize);
        void    setAnimation(QByteArray name, int min, int max);

        QString activateDebugLog(bool bEnable, const QString& categoryName);

        void    animate();

    signals:

        void    doUpdateView();

    public slots:

        void    onClearAllItems();
        void    onActivateDebugLog(bool bEnable);
        void    onSetMaxItems(int maxItems);
        void    onUpdateView();
        void    onDisplayLogMsg(int type, const QString &msg, const QString &categoryName);

    private:

        void    filterText(const QString& text);
        void    setText(const QString& text, const QString& category);
        void    clearAllItems();
        void    updateModelRows();
        void    manageCategory(bool bChecked, const QString& text);

    private:

        std::mutex                  m_mutex;
        QListView*                  m_pListView = nullptr;
        QListWidget*                m_pListWidget = nullptr;
        CNotificationModel*         m_pModel = nullptr;
        CNotificationProxyModel*    m_pProxy = nullptr;
        CHtmlDelegate*              m_pDelegate = nullptr;
        QLineEdit*                  m_pSearchEdit = nullptr;
        QFrame*                     m_pParams = nullptr;
        QToolButton*                m_pParamsBtn = nullptr;
        size_t                      m_padding = 0;
        size_t                      m_htmlFontSize = 3;
        QByteArray                  m_animationName = "";
        QList<QString>              m_categoryList;
        int                         m_animationMinValue = 0;
        int                         m_animationMaxValue = 0;
        bool                        m_bIsOpened = false;
        int                         m_maxItems = 100;
};

#endif // CNOTIFICATIONPANE_H
