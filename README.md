# FleXR: A System Enabling Flexibly Distributed Extended Reality

<img src="https://github.com/gt-flexr/FleXR_Page/blob/main/figs/ar1.gif?raw=true" width="200"><img src="https://github.com/gt-flexr/FleXR_Page/blob/main/figs/simplear.gif?raw=true" width="200"><img src="https://github.com/gt-flexr/FleXR_Page/blob/main/figs/ar2.gif?raw=true" width="200"><img src="https://github.com/gt-flexr/FleXR_Page/blob/main/figs/ar3.gif?raw=true" width="200"><img src="https://github.com/gt-flexr/FleXR_Page/blob/main/figs/vr.gif?raw=true" width="200">

## Introduction

**FleXR** is a fully open-source, flexibly configurable, and high-performance distributed stream processing system (DSPS) for distributed XR.
In FleXR, each XR functionality is implemented as a kernel (pipeline component).
After the kernel is implemented via our kernel template, it can be pipelined locally or remotely without any code modification.
Thus, a user can flexibly configure the kernels for diverse distribution scenarios.


## Starting Point

### Prerequisite/Requirement
FleXR runs on Linux (developed/tested on Ubutu 18.04/20.04).
We expect the running device of Linux and X11, e.g., Ubuntu 18.04 or 20.04.
FleXR is tested with x86-/ARM-based CPUs.

### FleXR Wiki
To ensure that everyone is on the same page and has access to all the necessary information, we kindly ask you to refer to [our GitHub Wiki page](https://github.com/gt-flexr/FleXR/wiki).
Here, you will find everything you need to know about the project, including its goals, requirements, tutorials and guidelines for contributing.
We believe that the Wiki page is an essential resource that will help you understand the project's objectives and how you can make valuable contributions.

_For someone who wants to experience **the FleXR demo** quickly_, we provide the docker image with the demo codes.
Please check [this instruction](https://github.com/gt-flexr/FleXR/wiki/Get-Started:-Proof-of-Concept) for quick demo.


## Paper
Heo, J., Bhardwaj, K., & Gavrilovska, A. (2023, June). [FleXR: A System Enabling Flexibly Distributed Extended Reality](https://dl.acm.org/doi/abs/10.1145/3587819.3590966). In Proceedings of the 14th Conference on ACM Multimedia Systems (pp. 1-13).

BibTex
```
@inproceedings{heo2023flexr,
  title={FleXR: A System Enabling Flexibly Distributed Extended Reality},
  author={Heo, Jin and Bhardwaj, Ketan and Gavrilovska, Ada},
  booktitle={Proceedings of the 14th Conference on ACM Multimedia Systems},
  pages={1--13},
  year={2023}
}
```


## Contact

If you have any question or discussion points, please use the issue in this repository or email to [Jin Heo](jheo33@gatech.edu).
