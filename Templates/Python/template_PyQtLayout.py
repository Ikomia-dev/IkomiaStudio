# Create layout : QGridLayout by default
        self.gridLayout = QGridLayout()
        # PyQt -> Qt wrapping
        layout_ptr = utils.PyQtToQt(self.gridLayout)
