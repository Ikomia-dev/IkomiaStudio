# _algorithm_name_

[Put algorithm description here]

[Insert illustrative image here. Image must be accessible publicly, in algorithm Github repository for example.
<img src="images/illustration.png"  width="30%" height="30%">]

## :rocket: Use with Ikomia API

1. Install Ikomia API

We strongly recommend using a virtual environment. If you're not sure where to start, we offer a tutorial [here](https://ikomia-dev.github.io/python-api-documentation/bonus/virtual_env.html).

```sh
pip install ikomia
```

2. Create your workflow

```python
import ikomia
from ikomia.dataprocess.workflow import Workflow

# Init your workflow
wf = Workflow()

# Add algorithm
algo = wf.add_task(name="_algorithm_name_", auto_connect=True)

# Run on your image  
wf.run_on(url="https://raw.githubusercontent.com/Ikomia-dev/notebooks/main/examples/img/img_dog.png")
```

## :rocket: Use with Ikomia Studio

1. If you are not an Ikomia Studio user yet, you can install it from the [following page](https://www.ikomia.ai/studio).
2. Start Ikomia Studio software
3. Open Ikomia HUB
4. Search and install **_algorithm_name_**
5. Add **_algorithm_name_** in a workflow and visualize results

If you need more resources for your first steps with Ikomia Studio, you can consult the [following blog post](https://www.ikomia.ai/blog/how-to-use-ikomia-studio).

## :pencil: Algorithm parameters

[Explain each algorithm parameters]

```python
import ikomia
from ikomia.dataprocess.workflow import Workflow

# Init your workflow
wf = Workflow()

# Add algorithm
algo = wf.add_task(name="_algorithm_name_", auto_connect=True)

algo.set_parameters({
    "param1": "value1",
    "param2": "value2",
    ...
})

# Run on your image  
wf.run_on(url="https://raw.githubusercontent.com/Ikomia-dev/notebooks/main/examples/img/img_dog.png")

```

## :fast_forward: Advanced usage 

[optional]
