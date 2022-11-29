import logging
from ikomia.core import task
from ikomia.utils.tests import run_for_test


logger = logging.getLogger(__name__)


def test(t, data_dict):
    logger.info(f"===== Test::{t.name} =====")
    # 1. Set task parameter if necessary. ex: task.set_parameters(t, {"iteration": 10, "size":3})
    # 2. Get input data from data_dict. ex: img = cv2.imread(data_dict["images"]["detection"]["coco"])[::-1]
    # 3. For each input to set
    #   3.1 Get task input instance. ex: input_0 = t.getInput(0)
    #   3.2 Set input data. ex: input_0.setImage(img)
    return run_for_test(t)
