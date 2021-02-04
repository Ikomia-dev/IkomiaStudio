#ifndef CCONTEXTMENU_HPP
#define CCONTEXTMENU_HPP

#include <boost/container/flat_map.hpp>
#include <QAction>
#include <QMenu>
#include <functional>

using menu_sig = std::function<void(QModelIndex&)>;
Q_DECLARE_METATYPE(menu_sig)

template< class context_sig, class hash_type = size_t>
class CContextMenu
{
    public:

        void registerAction(hash_type type_hash, const QString& text, const context_sig& sig, QObject* parent)
        {
            QList<QAction*>& la = m_typeToMenu[type_hash];
            la.push_back(new QAction(text, parent));
            la.back()->setData(QVariant::fromValue<context_sig>(sig));
        }

        template<class...args>
        void displayMenu(hash_type type_hash, QPoint pos, args&&... a)
        {
            if(m_typeToMenu.find(type_hash) == m_typeToMenu.end())//some items might have no submenu...
                return;

            auto action = QMenu::exec(m_typeToMenu[type_hash], pos);
            if(action)
                action->data().template value< context_sig >()(std::forward<args>(a)...);
        }

    private:

        boost::container::flat_map<hash_type,QList<QAction*> > m_typeToMenu;
};


template<class context_sig, class hash_type = size_t>
class CMultiContextMenu
{
    public:

        void    addAction(hash_type type, const QString& text, const context_sig& sig, const QIcon& icon = QIcon(), const QKeySequence& shortcut = QKeySequence())
        {
            auto retPair = m_typeToMenu.insert(std::make_pair(type, new QMenu()));
            QAction* pAction = retPair.first->second->addAction(text);
            pAction->setData(QVariant::fromValue<context_sig>(sig));
            pAction->setShortcut(shortcut);
            pAction->setIcon(icon);
        }

        void    addSubMenuAction(hash_type type, const QString& text, const context_sig& sig)
        {
            auto it = m_typeToSubMenu.find(type);
            if(it != m_typeToSubMenu.end())
            {
                QAction* pAction = it->second->addAction(text);
                pAction->setData(QVariant::fromValue<context_sig>(sig));
            }
        }

        void    clearSubMenu(hash_type type)
        {
            auto it = m_typeToSubMenu.find(type);
            if(it != m_typeToSubMenu.end())
                it->second->clear();
        }

        void    addMenu(hash_type type, const QString& text)
        {            
            auto retPair = m_typeToMenu.insert(std::make_pair(type, new QMenu()));
            auto it = m_typeToSubMenu.find(type);

            if(it != m_typeToSubMenu.end())
            {
                QAction *menuToBeRemoved = it->second->menuAction();
                retPair.first->second->removeAction(menuToBeRemoved);
                it->second = retPair.first->second->addMenu(text);
            }
            else
                m_typeToSubMenu.insert(std::make_pair(type, retPair.first->second->addMenu(text)));
        }

        QMenu*  menu(hash_type type)
        {
            auto it = m_typeToSubMenu.find(type);
            if(it != m_typeToSubMenu.end())
                return it->second;
            else
                return nullptr;
        }

        template<class...args>
        void    displayMenu(hash_type type, QPoint pos, args&&... a)
        {
            auto itMenu = m_typeToMenu.find(type);
            if(itMenu == m_typeToMenu.end())//some items might have no submenu...
                return;

            auto action = itMenu->second->exec(pos);
            if(action)
                action->data().template value< context_sig >()(std::forward<args>(a)...);
        }

    private:

        std::unordered_map<hash_type, QMenu*>   m_typeToMenu;
        std::unordered_map<hash_type, QMenu*>   m_typeToSubMenu;
        size_t                                  m_subMenuId = 0;
};

#endif // CCONTEXTMENU_HPP
