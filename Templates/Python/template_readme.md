<div align="center">
  <img src="images/icon.png" alt="Algorithm icon" width="100%" height="100%">
  <h1 align="center">_algorithm_name_</h1>
</div>
<br />
<p align="center">
    <a href="https://github.com/Ikomia-hub/_algorithm_name_">
        <img alt="Stars" src="https://img.shields.io/github/stars/Ikomia-hub/_algorithm_name_">
    </a>
    <a href="https://app.ikomia.ai/hub/">
        <img alt="Website" src="https://img.shields.io/website/http/app.ikomia.ai/en.svg?down_color=red&down_message=offline&up_message=online">
    </a>
    <a href="https://github.com/Ikomia-hub/_algorithm_name_/blob/main/LICENSE.md">
        <img alt="GitHub" src="https://img.shields.io/github/license/Ikomia-hub/_algorithm_name_.svg?color=blue">
    </a>    
    <br>
    <a href="https://discord.com/invite/82Tnw9UGGc">
        <img alt="Discord community" src="https://img.shields.io/badge/Discord-white?style=social&logo=discord">
    </a> 
</p>

[Put algorithm description here]

[Insert illustrative image here. Image must be accessible publicly, in algorithm Github repository for example.
<img src="images/illustration.png"  alt="Illustrative image" width="30%" height="30%">]

## :rocket: Use with Ikomia API

1. Install Ikomia API

We strongly recommend using a virtual environment. If you're not sure where to start, we offer a tutorial [here](https://ikomia-dev.github.io/python-api-documentation/bonus/virtual_env.html).

```sh
pip install ikomia
```

2. Create your workflow

[Change the sample image URL to fit algorithm purpose]

```python
import ikomia
from ikomia.dataprocess.workflow import Workflow

# Init your workflow
wf = Workflow()

# Add algorithm
algo = wf.add_task(name="_algorithm_name_", auto_connect=True)

# Run on your image  
wf.run_on(url="example_image.png")
```

## :rocket: Use with Ikomia Studio

1. If you are not an Ikomia Studio user yet, you can install it from the [following page](https://www.ikomia.ai/studio).
2. Start Ikomia Studio software
3. Open Ikomia HUB
4. Search and install **_algorithm_name_**
5. Add **_algorithm_name_** in a workflow and visualize results

If you need more resources for your first steps with Ikomia Studio, you can consult the [following blog post](https://www.ikomia.ai/blog/how-to-use-ikomia-studio).

## :pencil: Set algorithm parameters

[Explain each algorithm parameters]

[Change the sample image URL to fit algorithm purpose]

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
wf.run_on(url="example_image.png")

```

## :tada: Explore algorithm outputs

Each algorithm has specific outputs, but you can explore them the same way with Ikomia API. Please consult the [documentation](https://ikomia-dev.github.io/python-api-documentation/advanced_guide/IO_management.html) to go deeper in algorithm outputs management.

```python
import ikomia
from ikomia.dataprocess.workflow import Workflow

# Init your workflow
wf = Workflow()

# Add algorithm
algo = wf.add_task(name="_algorithm_name_", auto_connect=True)

# Run on your image  
wf.run_on(url="example_image.png")

# Iterate over outputs
for output in algo.get_outputs()
    # Print information
    print(output)
    # Export it to JSON
    output.to_json()
```

## :fast_forward: Advanced usage 

[optional]
