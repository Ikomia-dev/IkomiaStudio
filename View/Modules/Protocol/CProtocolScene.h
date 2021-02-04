#ifndef CPROTOCOLSCENE_H
#define CPROTOCOLSCENE_H

#include "Main/forwards.hpp"
#include <QGraphicsScene>
#include "Core/CProtocol.h"
#include "../../Process/CProcessPopupDlg.h"
#include "View/Graphics/CGraphicsLayerChoiceDlg.h"

class CProtocolItem;
class CProtocolPortItem;
class CProtocolConnection;
class CProtocolIOArea;
class CProtocolDummyItem;

class CProtocolScene : public QGraphicsScene
{
    Q_OBJECT

    public:

        CProtocolScene(QWidget* pParent = nullptr);

        //Getters
        QSize               getCellSize() const;
        CProtocolIOArea*    getInputArea() const;

        //Setters
        void                setSelectedItem(QGraphicsItem* pItem);
        void                setRootId(const ProtocolVertex& id);
        void                setProtocolStarted(bool bStarted);
        void                setTaskActionFlag(const ProtocolVertex& id, CProtocolTask::ActionFlag action, bool bEnable);
        void                setCellSize(QSize cellSize);
        void                setItemSize(QSize itemSize);
        void                setInputInfo(const CDataInfoPtr& info, int index);

        //Methods
        void                initInputs();

        void                updateInput(int index, const ProtocolTaskIOPtr &inputPtr, bool isBatch);

        void                addTaskItem(CProtocolItem* pItem);
        void                addTaskItem(CProtocolItem *pItem, CProtocolItem* pParent);

        void                deleteConnection(CProtocolConnection* pConnection, bool bLater, bool bNotifyView, bool bNotifyModel);
        void                deleteInput(int index);

        QRectF              resize(const QRectF& viewRect);

        void                updateCandidateTask();

        void                clearAll();

        void                queryProjectDataProxyModel(const TreeItemType &inputType, const std::vector<DataDimension>& filters);

        void                retrieveIOInfo(const ProtocolVertex& taskId, size_t index, bool bInput);

    signals:

        void                doSelectionChange(const ProtocolVertex& id);

        void                doItemDeleted(const ProtocolVertex& id);
        void                doItemConnection(const ProtocolVertex& srcId, size_t srcIndex, const ProtocolVertex& dstId, size_t dstIndex);

        void                doDeleteConnection(const ProtocolEdge& id, bool bNotifyModel);
        void                doDeleteInput(int index);

        void                doSetTaskActionFlag(const ProtocolVertex& id, CProtocolTask::ActionFlag action, bool bEnable);
        void                doSetGraphicsLayerInput(const ProtocolVertex& id, int portIndex, const QModelIndex& layerIndex);
        void                doSetInput(int inputIndex, const CProtocolInput& input);
        void                doSetFolderPathInput(const ProtocolVertex& id, size_t index, const QString& path);

        void                doQueryGraphicsProxyModel();
        void                doQueryProjectDataProxyModel(const std::vector<TreeItemType>& dataTypes, const std::vector<DataDimension>& filters);

        void                doAddInput(const CProtocolInput& input);

        void                doShowProcessPopup();

        void                doQueryIOInfo(const ProtocolVertex& taskId, int index, bool bInput);

    public slots:

        void                onSetGraphicsProxyModel(CProjectGraphicsProxyModel* pModel);
        void                onSetProjectDataProxyModel(CProjectDataProxyModel *pModel);
        void                onSelectionChanged();
        void                onIOAreaChanged();

    protected:

        void                mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent) Q_DECL_OVERRIDE;
        void                mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent) Q_DECL_OVERRIDE;
        void                mouseReleaseEvent(QGraphicsSceneMouseEvent *mouseEvent) Q_DECL_OVERRIDE;
        void                keyPressEvent(QKeyEvent *event) Q_DECL_OVERRIDE;

    private:

        void                setGraphGridItem(QGraphicsItem* pItem, QPair<int,int>& cell);
        void                setPosition(QGraphicsItem *pItem, int row, int column);
        void                setDialogPosition(QDialog* pDialog, const QPointF& pos);

        QRectF              getItemsBoundingRect() const;
        QRectF              getViewRect() const;
        int                 getTaskItemCount() const;
        QPair<int, int>     getCandidateCell(CProtocolItem* pItem) const;
        QPair<int, int>     getCellFromParent(CProtocolItem* pItem, CProtocolItem* pParent, int childIndex) const;
        size_t              getGraphGridIndex(const QPair<int,int>& cell) const;

        bool                isGraphGridRowEmpty(int row, int firstColumn, int lastColumn) const;
        bool                isValidDropCell(const QPair<int,int>& cell) const;

        void                createIOAreas();

        void                addItemToGraphGrid(QGraphicsItem* pItem, QGraphicsItem* pParent);

        void                deleteSelectedItems();
        void                deleteTaskItem(CProtocolItem* pItem);

        void                removeGraphGridEmptyRows();

        void                finalizeCurrentConnection(QPointF pos);

        void                updateNearestPorts(QPointF pos);
        void                updatePortZoom(QPointF pos);
        void                updateDropCellRubber(QPointF pos);
        void                updateGraphGridSize(const QPair<int,int>& cell);
        void                updateSceneFromGraphGrid();
        void                updateAllConnections();

        void                endItemDrag(QPointF pos);

        QPoint              mapToScreen(QPointF scenePos);

        QPair<int,int>      positionToGrid(const QPointF& pos) const;
        QPointF             gridToPosition(int row, int column) const;

        bool                isWidthReducible(QRectF viewRect);
        bool                isHeightReducible(QRectF viewRect);

        void                checkSceneRectChange();
        void                centerInputArea();

        void                onPortClicked(CProtocolPortItem *pPort, const QPointF& pos);

        void                openProcessDlg();
        void                openGraphicsLayerChoiceDlg(CProtocolPortItem *pPort, const QPointF &pos);
        void                openFolderPathChoiceDlg(CProtocolPortItem *pPort, const QPointF &pos);

    private:

        //Memory structure of the graph for the view
        CndArray<QGraphicsItem*> m_graphGrid;
        //Size of the cells in the grid.
        QSize                   m_cellSize = QSize(120, 120);
        QSize                   m_itemSize = QSize(90, 90);
        QPointF                 m_dragStartPos = QPointF(0, 0);
        bool                    m_bProtocolStarted = false;
        bool                    m_bItemDragged = false;
        bool                    m_bNewItemClicked = false;
        ProtocolVertex          m_rootId;
        QGraphicsItem*          m_pCurProtocolItem = nullptr;
        CProtocolConnection*    m_pProtocolConnectionTmp = nullptr;
        QGraphicsProxyWidget*   m_pDropCellRubber = nullptr;
        CProtocolIOArea*        m_pInputArea = nullptr;
        CProtocolIOArea*        m_pOutputArea = nullptr;
        CProtocolDummyItem*     m_pDummyItem = nullptr;
        QGraphicsItem*          m_pLastItem = nullptr;
        CGraphicsLayerChoiceDlg m_layerChoiceDlg;
};

#endif // CPROTOCOLSCENE_H
