#  -*- mode: Outline; fill-column: 74; -*-
* Outline Commands cheat sheet (C-c C-s to see this)

        C-c C-t         Hide EVERYTHING in buffer
        C-c C-a         Show EVERYTHING in buffer

        C-c C-d         Hide THIS item and subitems (subtree)
        C-c C-s         Show THIS item and subitems (subtree)

        C-c C-c         Hide ONE item
        C-c C-e         Show ONE item

-------------------------------------------------------------------------


* Introduction

* Coding Rules

** Macro Usage

There are several 3rd party packages and opensource code in our F/W,
which include:

  - linux kernel [ path: /linux ]
  - broadcom bsp [ path: drivers/GPON, drivers/GPON_OMCI, driver/DSL ]

In order to easy to distinguish the sercomm code from original source
code, macros are needed.

The macros are defined as following:

  - CONFIG_SCM_SUPPORT

  This macro is used for all code changes which doesn't relate to modulized
  functions.


















* Appendix


-----------------------------------------------------------------------
Local Variables:
mode: outline
fill-column:72
outline-regexp:"[*\^L]+\\|^[[:digit:]][[:digit:]\.]+\\|[#\^L]+"
End:

-----------------------------------------------------------------------
Q: How to build GPL package?
A: 
    cd build/;
    run ./CreatGPL.sh;
    
    All the packages are put into build/${MODULE_NAME}_v${FW_VERSION}.tgz 
    (e.g. build/RFT630_RON_vf.2.47.tgz)
    
