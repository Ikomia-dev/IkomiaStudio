from ikomia import dataprocess
import _PluginName__process as processMod
import _PluginName__widget as widgetMod


# --------------------
# - Interface class to integrate the process with Ikomia application
# - Inherits PyDataProcess.CPluginProcessInterface from Ikomia API
# --------------------
class _PluginName_(dataprocess.CPluginProcessInterface):

    def __init__(self):
        dataprocess.CPluginProcessInterface.__init__(self)

    def getProcessFactory(self):
        # Instantiate process object
        return processMod._PluginName_ProcessFactory()

    def getWidgetFactory(self):
        # Instantiate associated widget object
        return widgetMod._PluginName_WidgetFactory()
