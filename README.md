# LazyGASPI-H

This is an implementation of the LazyGASPI library where all processes have the same role (**homogeneous**) and each process stores all rows.\
LazyGASPI is the implementation of bounded staleness (https://www.usenix.org/system/files/conference/atc14/atc14-paper-cui.pdf) using GASPI (http://www.gaspi.de/).
For installation instructions, see INSTALL.

## Table of Contents

## How it works

Since all processes store all rows locally, row reads are local.\
When a process writes a row, it writes it to all other processes as well. 

<a id="idsMacStrTypFunc"></a>
## ID's/Macros, Structures/Typedefs and Functions
<a id="idsMac"></a>
### ID's/Macros
| Segment ID | Explanation |
| ---------- | ----------- |
| <a id="idInfo"></a>`LAZYGASPI_ID_INFO = 0` | Stores the [`LazyGaspiProcessInfo`](#lgpi) of the current rank | 
| <a id="idRows"></a>`LAZYGASPI_ID_ROWS = 1` | Stores the rows assigned to the current rank |
| <a id="idAvail"></a>`LAZYGASPI_ID_AVAIL = 2` | The first available segment ID for allocation (not an actual segment) |
