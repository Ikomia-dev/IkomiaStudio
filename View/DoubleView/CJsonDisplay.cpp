#include "CJsonDisplay.h"

#include <QStandardItem>
#include <QStandardItemModel>
#include <QTreeView>
#include <QPlainTextEdit>
#include <QPushButton>

#include "CException.h"
#include "Main/AppTools.hpp"



CJsonDisplay::CJsonDisplay(const QString &name, const QJsonDocument &jsonDocument, QWidget *parent, int flags) :
    CDataDisplay(parent, name, flags)
{
    m_typeId = DisplayType::JSON_DISPLAY;
    initLayout();
    initConnections();

    m_pAutoresizeBtn->setChecked(true);
    m_pSwitchModeBtn->setChecked(true);

    setJsonDocument(jsonDocument);
}

void CJsonDisplay::initLayout()
{
    setObjectName("CJsonDisplay");

    // Button used to resize treeview's sections
    m_pAutoresizeBtn = createButton(QIcon(":/Images/stretch.svg"));
    m_pAutoresizeBtn->setCheckable(true);

    // Button used to switch between treeview and plain text view
    m_pSwitchModeBtn = createButton(QIcon(":/Images/treeview.svg"));
    m_pSwitchModeBtn->setCheckable(true);

    // Button used to export data
    m_pExportBtn = createButton(QIcon(":/Images/export.png"));

    // Add buttons to the toolbar
    int index = 0;
    if(m_flags & CHECKBOX)
        index++;
    if(m_flags & TITLE)
        index++;

    m_pHbox->insertStretch(index++, 1);
    m_pHbox->insertWidget(index++, m_pAutoresizeBtn);
    m_pHbox->insertWidget(index++, m_pSwitchModeBtn);

    if(m_flags & EXPORT_BUTTON)
    {
        m_pHbox->insertWidget(index++, m_pExportBtn);
    }

    m_pHbox->insertStretch(index++, 1);

    // Initialization of the model
    m_pTreeModel = new QStandardItemModel(this);
    m_pTreeModel->setHorizontalHeaderLabels({"", ""});

    // Initialisation of the treeview
    m_pTreeView = new QTreeView(this);
    // Relation between the view and the model
    m_pTreeView->setModel(m_pTreeModel);
    m_pTreeView->setAnimated(true);
    m_pTreeView->setExpandsOnDoubleClick(true);
    m_pTreeView->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    // All items will have same height: enable some optimizations
    m_pTreeView->setUniformRowHeights(true);
    m_pLayout->addWidget(m_pTreeView);

    // Initialisation of the plain text component
    m_pPlainTextEdit = new QPlainTextEdit(this);
    m_pPlainTextEdit->setReadOnly(true);
    m_pLayout->addWidget(m_pPlainTextEdit);
}

void CJsonDisplay::initConnections()
{
    connect(m_pExportBtn, &QPushButton::clicked, this, &CJsonDisplay::onExportBtnClicked);
    connect(m_pAutoresizeBtn, &QPushButton::toggled, this, &CJsonDisplay::onAutoresizeBtnToggled);
    connect(m_pSwitchModeBtn, &QPushButton::toggled, this, &CJsonDisplay::onSwitchModeBtnToggled);
}

void CJsonDisplay::computeTreeModel()
{
    // Old data are removed
    m_pTreeModel->removeRows(0, m_pTreeModel->rowCount());

    // Creation of the root node. Here, this node is not shown.
    QStandardItem *root = m_pTreeModel->invisibleRootItem();
    root->setFlags(Qt::NoItemFlags);

    // The first element of a valid JSON can be an object or an array.
    // So, a specific treatment must be done for this root node.
    if(m_pJsonDocument.isObject())
    {
        QJsonObject obj = m_pJsonDocument.object();
        processObject(obj, root);
    }
    else if(m_pJsonDocument.isArray())
    {
        QJsonArray arr = m_pJsonDocument.array();
        processArray(arr, root);
    }
    else if(m_pJsonDocument.isEmpty())
    {
        // Display if there is not data inside the document
        //
        // WARNING : Currently, this code is not executed because
        // of a previous test into available data
        //
        root->appendRow(
            new QStandardItem(QString("No data to display..."))
        );
    }
    else
    {
        // Null document or invalid type...
        throw CException(CoreExCode::INVALID_JSON_FORMAT, "This document is not a valid JSON!", __func__, __FILE__, __LINE__);
    }
}

QStandardItem* CJsonDisplay::processValue(QStandardItem *key, const QJsonValue &value)
{
    QStandardItem *currentValue = nullptr;

    switch(value.type())
    {
        case QJsonValue::Null:
        {
            currentValue = new QStandardItem("null");
            currentValue->setFlags(Qt::NoItemFlags);

            // The 'null' value is display using an italic font
            QFont font = currentValue->font();
            font.setItalic(true);
            currentValue->setFont(font);
        }
        break;

        case QJsonValue::Bool:
        case QJsonValue::Double:
        case QJsonValue::String:
        {
            currentValue = new QStandardItem(value.toVariant().toString());
            currentValue->setFlags(Qt::NoItemFlags);

            // Values are display using a bold font
            QFont font = currentValue->font();
            font.setBold(true);
            currentValue->setFont(font);
        }
        break;

        case QJsonValue::Object:
        {
            currentValue = new QStandardItem("{ }");
            currentValue->setFlags(Qt::NoItemFlags);

            // The current value is an object: child elements are treated
            processObject(value.toObject(), key);
        }
        break;

        case QJsonValue::Array:
        {
            currentValue = new QStandardItem("[ ]");
            currentValue->setFlags(Qt::NoItemFlags);

            // The current value is an arrey: child elements are treated
            processArray(value.toArray(), key);
        }
        break;

        case QJsonValue::Undefined:
        default:
        {
            // The current value is undefined: data seem to be invalid!
            throw CException(CoreExCode::INVALID_JSON_FORMAT, QString("This JSON file is not valid ! The key '%1' has an incorrect value.").arg(key->text()).toStdString(), __func__, __FILE__, __LINE__);
        }
        break;
    }

    return currentValue;
}

void CJsonDisplay::processObject(const QJsonObject &obj, QStandardItem *parent)
{
    // for each key/value of the current object...
    // NOTE: cbegin and cend are not defined for QJsonObject
    for(auto iter = obj.begin(); iter != obj.end(); ++iter)
    {
        // ... a new row (made up of two columns) is created
        QList<QStandardItem*> currentRow;

        // The first column contains the current key
        QStandardItem *currentKey = new QStandardItem(iter.key());
        currentKey->setFlags(Qt::NoItemFlags);
        currentRow.push_back(currentKey);

        // The second column contains the current value
        // This value can be a string, a double, an object, an array...
        currentRow.push_back(
            processValue(currentKey, iter.value())
        );

        // The current row becomes a child of the node 'parent'
        parent->appendRow(currentRow);
    }
}

void CJsonDisplay::processArray(const QJsonArray &arr, QStandardItem *parent)
{
    // for each element of this array...
    for(auto iter = arr.cbegin(); iter != arr.cend(); ++iter)
    {
        // ... a new row (made up of two columns) is created
        QList<QStandardItem*> currentRow;

        // The first column contains the index of the current element inside the array
        QStandardItem *currentKey = new QStandardItem(QString::number(iter - arr.cbegin()));
        currentKey->setFlags(Qt::NoItemFlags);
        currentRow.push_back(currentKey);

        // The second column contains the value of the current element inside the array
        // This value can be a string, a double, an object, an array...
        currentRow.push_back(
            processValue(currentKey, *iter)
        );

        // The current row becomes a child of the node 'parent'
        parent->appendRow(currentRow);
    }
}

QPushButton *CJsonDisplay::createButton(const QIcon &icon)
{
    auto pal = qApp->palette();
    auto color = pal.highlight().color();

    auto pBtn = new QPushButton(this);
    pBtn->setIcon(icon);
    pBtn->setIconSize(QSize(16,16));
    pBtn->setFixedSize(22,22);
    pBtn->setStyleSheet(
        QString(
            "QPushButton {"
                "background: transparent;"
                "border: none;"
            "}"
            ""
            "QPushButton:hover {"
                "border: 1px solid %1;"
            "}"
        ).arg(color.name())
    );
    return pBtn;
}

void CJsonDisplay::onExportBtnClicked()
{
    QString path = Utils::File::saveFile(this, tr("Export JSON file"), "", tr("JSON file (*.json)"), QStringList({"json"}), ".json");
    QFile file(path);

    if (file.open(QIODevice::WriteOnly))
    {
        // The content of the plain text viewer is directly copied into the file
        QTextStream stream(&file);
        stream << m_pPlainTextEdit->toPlainText();
    }
}

void CJsonDisplay::onAutoresizeBtnToggled(bool checked)
{
    // User has just clicked into the 'auto-resize' button...

    // ... the tooltip is updated
    m_pAutoresizeBtn->setToolTip(tr(QString("Auto-resize to show whole values (currently: %1)").arg(checked ? "on" : "off").toStdString().c_str()));

    // ... and the treeview's resize mode is selected, according to the current mode
    m_pTreeView->header()->setSectionResizeMode(
        checked ?
            QHeaderView::ResizeToContents
        :
            QHeaderView::Interactive
    );
}

void CJsonDisplay::onSwitchModeBtnToggled(bool checked)
{
    // User has just clicked into the 'switch mode' button...

    // The current viewer (treeview or plain text view) is switched
    m_pTreeView->setVisible(checked);
    m_pPlainTextEdit->setVisible(!checked);

    // Buttons are also updated (icons, tooltips...)
    m_pAutoresizeBtn->setEnabled(checked);
    m_pAutoresizeBtn->setToolTip(tr(QString("Auto-resize to show whole values (currently: %1)").arg(checked ? "on" : "off").toStdString().c_str()));

    m_pSwitchModeBtn->setIcon(QIcon(checked ? ":/Images/plain-text-view.svg" : ":/Images/treeview.svg"));
    m_pSwitchModeBtn->setToolTip(tr(QString("Display data in %1 mode").arg(checked ? "plain text" : "treeview").toStdString().c_str()));
}

void CJsonDisplay::setJsonDocument(const QJsonDocument &jsonDocument)
{
    // The new data are stored
    m_pJsonDocument = jsonDocument;

    // Data have changed: treeview model is recomputed
    computeTreeModel();

    // and plain text view is updated
    m_pPlainTextEdit->setPlainText(jsonDocument.toJson(QJsonDocument::Indented));
}

bool CJsonDisplay::eventFilter(QObject *obj, QEvent *event)
{
    if(obj == this)
    {
        if(event->type() == QEvent::MouseButtonDblClick)
        {
            emit doDoubleClicked(this);
            return true;
        }
    }
    return CDataDisplay::eventFilter(obj, event);
}
