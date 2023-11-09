#ifndef CJSONDISPLAY_H
#define CJSONDISPLAY_H

#include <QJsonDocument>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QTreeView>


#include "CDataDisplay.h"



/**
 * @brief Class used to display JSON data. Two viewers are available :
 * - a treeview: data can be parsed interactively
 * - a plain text view: data are display in a simple text format, with indentation
 */
class CJsonDisplay : public CDataDisplay
{
    public:
        enum JsonDisplayBar
        {
            NONE = 0x00000000,
            CHECKBOX = 0x00000001,
            TITLE = 0x00000002,
            MAXIMIZE_BUTTON = 0x0000004,
            CLOSE_BUTTON = 0x0000008,
            EXPORT_BUTTON = 0x00000010,
            DEFAULT = TITLE | EXPORT_BUTTON | MAXIMIZE_BUTTON
        };

        /**
         * @brief Constructor.
         */
        CJsonDisplay(const QString& name, const QJsonDocument &jsonDocument=QJsonDocument(), QWidget* parent=nullptr, int flags=DEFAULT);

        /**
         * @brief Set a new JSON document to the viewer. Views are updated.
         */
        void setJsonDocument(const QJsonDocument &jsonDocument);

    protected:
        bool eventFilter(QObject* obj, QEvent* event) override;

    private:
        /**
         * @brief Initialize the window's widgets.
         */
        void initLayout();

        /**
         * @brief Initialize widget's connections.
         */
        void initConnections();

        /**
         * brief Compute the model, using data contain in 'm_pJsonDocument'.
         */
        void computeTreeModel();

        /**
         * @brief Inner method used to add data (= value) into the model.
         */
        QStandardItem* processValue(QStandardItem *key, const QJsonValue &value);

        /**
         * @brief Inner method used to add data (= object) into the model.
         */
        void processObject(const QJsonObject &obj, QStandardItem *parent);

        /**
         * @brief Inner method used to add data (= array) into the model.
         */
        void processArray(const QJsonArray &arr, QStandardItem *parent);

        /**
         * @brief Generate a button into the toolbar.
         */
        QPushButton* createButton(const QIcon& icon);

    private slots:
        void onExportBtnClicked();
        void onAutoresizeBtnToggled(bool checked);
        void onSwitchModeBtnToggled(bool checked);

    private:
        /**
         * @brief Data to display.
         */
        QJsonDocument m_pJsonDocument;

        /**
         * @brief Model used by the widget 'QTreeView'.
         */
        QStandardItemModel *m_pTreeModel;

        /**
         * @brief View used to display data contain in the associated model.
         */
        QTreeView *m_pTreeView;

        /**
         * @brief Component used to display JSON data as plain text.
         */
        QPlainTextEdit *m_pPlainTextEdit;

        /**
         * @brief Enable/disable the auto-resize of the treeview.
         */
        QPushButton *m_pAutoresizeBtn = nullptr;

        /**
         * @brief Switch between treeview display and plain text display.
         */
        QPushButton *m_pSwitchModeBtn = nullptr;

        /**
         * @brief Open a dialog box used to save data into a JSON file.
         */
        QPushButton *m_pExportBtn = nullptr;
};

#endif // CJSONDISPLAY_H
