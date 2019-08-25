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

<a id="strTyp"></a>
### Structures/Typedefs
<a id="lgpi"></a>
#### `LazyGaspiProcessInfo (struct)`

| Type | Member | Explanation |
| ---- | ------ | ----------- |
| `gaspi_rank_t`    | `id`               | The current rank, as outputted by `gaspi_proc_rank` |
| `gaspi_rank_t`    | `n`                | The total amount of ranks, as outputted by `gaspi_proc_num` |
| `lazygaspi_age_t` | `age`              | The age of the current process. This corresponds to how many times `lazygaspi_clock` has been called |
| `lazygaspi_id_t`  | `table_amount`     | The total amount of tables that have been distributed among all processes. Not the same as the amount of tables stored by the current rank|
| `lazygaspi_id_t`  | `table_size`       | The amount of rows in each table (same for all) |
| `gaspi_size_t`    | `row_size`         | The size of each row (same for all), in bytes |
| `gaspi_pointer_t` | `rows`             | A pointer to the rows segment (this shouldn't be used for reading/writing rows; use the library's procedures) |
| `lazygaspi_age_t` | `communicator`     | Member used for communication with servers | 
| `std::ostream*`   | `out`              | A pointer to the output stream for debugging. See [OutputCreator](#oc). |
| `bool`            | `offset_slack`     | `true` if accetable age range should be calculated from the previous age (iteration); `false` if it should be calculated from the current age (\*) |

(\*) For example, if current age is 7, slack is 2 and `offset_slack` is `true`, the minimum acceptable age for a read row is 7 - 2 - 1 = 4; if `offset_slack` is `false`, the minimum age is 7 - 2 = 5.

<a id="lgrd"></a>
#### `LazyGaspiRowData (struct)`
This is the metadata associated to a row stored by this process.

| Type | Member | Explanation |
| ---- | ------ | ----------- |
| `lazygaspi_age_t` | `age` | The amount of times `lazygaspi_clock` had been called before row was written to server |

<a id="sd"></a>
#### `SizeDeterminer (typedef)`
Can determine one of these: `LazyGaspiProcessInfo::table_amount`, `LazyGaspiProcessInfo::table_size` or `LazyGaspiProcessInfo::row_size`, which are henceforth considered "sizes".\
This function is called by `lazygaspi_init` after GASPI is initialized, `LazyGaspiProcessInfo::id` and `LazyGaspiProcessInfo::n` have been set and after output is created (see [OutputCreator](#oc)). This means that not only does a `SizeDeterminer` already know the rank of the current process and the total amount of ranks, but it can also output to the process's output stream.\
If a "size" is passed as a non-zero value to `lazygaspi_init` then its corresponding `SizeDeterminer` will never be called, even if it not a `nullptr` (see [`lazygaspi_init`](#fInit)), for example:
```
#define TABLE_AMOUNT 5
...
lazygaspi_init(TABLE_AMOUNT, TABLE_SIZE, ROW_SIZE, MyOutputCreator, MyCacheOptions, FREE_MEM, MyTableAmountDeterminer, ...);
//Since TABLE_AMOUNT != 0, MyTableAmountDeterminer will never be called.
```

Parameters:
- `gaspi_rank_t rank` - The rank of the current process, as outputted by `gaspi_proc_rank`.
- `gaspi_rank_t total` - The total amount of ranks, as outputted by `gaspi_proc_num`.
- `void * data` - A pointer to the data that will be used by the function. This is only used by the function itself.

\
Returns:
- The size of the corresponding member, as a `gaspi_size_t`.

<a id="oc"></a>
#### `OutputCreator (typedef)`
Responsible for creating the output stream and storing a pointer to it in this process's `LAZYGASPI_ID_INFO` segment.
This function is called by `lazygaspi_init` after GASPI is initialized, `LazyGaspiProcessInfo::id` and `LazyGaspiProcessInfo::n` are set. If `nullptr` is passed, the output stream is `std::cout`.

Parameters:
- `LazyGaspiProcessInfo*` - A pointer to the process's `LAZYGASPI_ID_INFO` segment.

<a id="fInit"></a>
#### `lazygaspi_init` 
Initializes the LazyGASPI library, including GASPI itself. This function must be called before any other LazyGASPI functions.\
Also initializes MPI (before GASPI) if library was compiled with MPI support. 

| Type | Parameter | Explanation |
| ---- | --------- | ----------- |
| `lazygaspi_id_t` | `table_amount` | The amount of tables to be allocated, or `0` if value is to be determined by `det_amount` |
| `lazygaspi_id_t` | `table_size` | The size of each table (\*), in amount of rows, or `0` if value is to be determined by `det_tablesize` |
| `gaspi_size_t` | `row_size` | The size of a single row, in bytes, or `0` if value is to be determined by `det_rowsize` |
| [`OutputCreator`](#oc) | `creator` | Used to create the process's output stream for debug messages. Use `nullptr` to indicate `std::cout` should be used |
| `gaspi_size_t` | `freeMemory` | The minimum amount of memory guaranteed to be left unallocated for client processes, in bytes; default is 1 MB |
| [`SizeDeterminer`](#sd) | `det_amount` | A `SizeDeterminer` for the amount of tables. Will only be called if `table_amount` is `0` |
| `void*` | `data_amount` | A pointer passed to `det_amount` when it is called |
| [`SizeDeterminer`](#sd) | `det_tablesize` | A `SizeDeterminer` for the amount of rows in a table. Will only be called if `table_size` is `0` |
| `void*` | `data_tablesize` | A pointer passed to `det_tablesize` when it is called |
| [`SizeDeterminer`](#sd) | `det_rowsize` | A `SizeDeterminer` for the size of a row, in bytes. Will only be called if `row_size` is `0` |
| `void*` | `data_rowsize` | A pointer passed to `det_rowsize` when it is called |

Returns:
- `GASPI_SUCCESS` on success
- `GASPI_ERROR` on unknown error thrown by GASPI (or another error code)
- `GASPI_TIMEOUT` on timeout
- `GASPI_ERR_INV_RANK` if GASPI failed to obtain the amount of ranks (returned 0), or if MPI is supported, if it assigned a different rank or determined a different amount of ranks from GASPI (can also be thrown by GASPI for other reasons)
- `GASPI_ERR_INV_NUM` if a "size" was `0` and its corresponding `SizeDeterminer` was a `nullptr` (can also be thrown by GASPI for other reasons)
- `GASPI_ERR_NULLPTR` if the OutputCreator failed to set a value for `LazyGaspiProcessInfo::out`

\
(\*) All tables have the same size.

<a id="fInfo"></a>
#### `lazygaspi_get_info`

Outputs a pointer to the [`LAZYGASPI_ID_INFO`](#idInfo) segment.

| Type | Parameter | Explanation |
| ---- | --------- | ----------- |
| [`LazyGaspiProcessinfo**`](#lgpi) | `info` |  The output parameter that will receive the pointer |

Returns:
- `GASPI_SUCCESS` on success;
- `GASPI_ERROR` on unknown error thrown by GASPI (or another error code);
- `GASPI_TIMEOUT` on timeout;
- `GASPI_ERR_NULLPTR` if a `nullptr` is passed as the value of `info`. 

<a id="fRead"></a>
#### `lazygaspi_read`

Reads a row. Row's age is guaranteed to be at least the current rank's age minus the slack (minus one if `LazyGaspiProcessInfo::offset_slack` is `true`).

| Type | Parameter | Explanation |
| ---- | --------- | ----------- |
| `lazygaspi_id_t` | `row_id` | The ID of the row to be read |
| `lazygaspi_id_t` | `table_id` | The ID of the row's table |
| `lazygaspi_slack_t` | `slack` | The amount of slack allowed for the returned row's age |
| `void*` | `row` | Output parameter for the row data. Will write `LazyGaspiProcessInfo::row_size` bytes |
| `LazyGaspiRowData*` | `data` |  Output parameter for the row's metadata (see [LazyGaspiRowData](#lgrd)), or `nullptr` to ignore |

Returns:
- `GASPI_SUCCESS` on success;
- `GASPI_ERROR` on unknown error thrown by GASPI (or another error code);
- `GASPI_TIMEOUT` on timeout;
- `GASPI_ERR_NULLPTR` if passed pointer was a `nullptr`;
- `GASPI_ERR_INV_NUM` if either `row_id` or `table_id` is not a valid ID (or thrown by GASPI for another reason).