| Peripheral                      | Typical Base Address (Octal) | Registers Used | Notes                                       |
|---------------------------------|------------------------------|----------------|---------------------------------------------|
| **DL11 / DLV11** (Async Serial) | `177560`, `176500`           | 4              | RX Status, RX Buffer, TX Status, TX Buffer  |
| **RK11** (RK05 Disk)            | `177400`                     | 8              | Disk word count, bus address, control, etc. |
| **RL11** (RL01/02 Disk)         | `174400`                     | 8              | Similar to RK11; DMA-based controller       |
| **RX11** (RX01/02 Floppy)       | `177170`                     | 4              | Control and status registers                |
| **LP11** (Line Printer)         | `177514`                     | 2              | Control and buffer                          |
| **CR11** (Card Reader)          | `177760`                     | 2              | Control and buffer                          |
| **KW11-L** (Line Time Clock)    | `177546`                     | 2              | Time base interrupt timer                   |
| **Console DL11**                | `177560`                     | 4              | Same as standard DL11; often hardwired      |
