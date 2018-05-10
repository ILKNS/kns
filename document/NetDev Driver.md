# Ideas
In the newest IX, DPDK's driver is used to initialize NIC, however, I did not figure out a way to adapt it into kns.
Originally derived from old version of DPDK's driver, and the newest ixgbe driver is refered to as I believe the newer the better.

# Layers
Two layers: the lower to function in phy layer, and the upper layer to rx/tx pkt. The upper layer is deprived from ixgbe driver, kns 
read and write ring directly.

# Ixgbe Driver
In the original ixgbe driver, `ixgbe_main.c` is used to hold functions to registe and remove driver. In DPDK, it is substitute with 

- First, restore the `ixgbe_probe` function as what PCI drivers do.
  - Detect pci dev, and check its info, all the `pci_` set of driver functions are brought back.
  - `adapter` and `netdev` are rewrited. In the original Linux, `netdev_ops` is used to hold callback functions to visit rings. However, we do not use vfs of network anymore.
  - iommap and pci mem allocation are also put here.
  - all the other lines are moved out to `ixgbe_init_adapter`, but in Linux, `probe` function finish all the initialization process.
- Callback Functions in ixgbe driver are also devided into two set:
  - `mac` for the upper layer, and `phy` for lower layer.
  - and `_general` holds all the general functions, and each specific file like `ixgbe_82599` hold different functions.
- In Linux, softirq is called to let the kernel jump to visit rings. We disable irq here, kernel poll the rings continuously.
- Finally, I compared all the driver functions in old version of ix and newest ixgbe driver:
  - some parameter's type and number of parameter are changed
  - some macros are disgarded and i do not know why, so i add them back
- The way to visit register of ixgbe is slightly different in linux and dpdk.
- Another hash function for rss is impled

## Files
- `ixgbe`: hold funcitons to register PCI dev and some general hardware functions
- `ixgbe_api`: some callback functions, just to be used as template for different type of ixgbe
- `ixgbe_common`: general functions of the driver, ixgbe register visit is also put here
- `ixgbe_dcb`: datacenter functions to config and calculate traffic
- `ixgbe_mbx` `ixgbe_pf` `ixgbe_vf`: some NIC functions
- `ixgbe_type`: important structures and macros
- `ixgbe_rxtx`: the file to provide stream for upper layer, the most important file
  - rx/tx queues are defined here
  - important rx/tx functions: rx queue poll, tx procedures
  - RSS set up
  - dcb configurations: while here, combined with some other configurations, make up the configuration of ixgbe
