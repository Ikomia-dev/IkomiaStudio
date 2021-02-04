# Create layout : QGridLayout by default
        self.gridLayout = QtWidgets.QGridLayout()
        # PySide -> Qt wrapping
        layout_ptr = utils.PySideToQt(self.gridLayout)
