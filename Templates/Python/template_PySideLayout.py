# Create layout : QGridLayout by default
        self.gridLayout = QtWidgets.QGridLayout()
        # PySide -> Qt wrapping
        layout_ptr = qtconversion.PySideToQt(self.gridLayout)
