# Ideas
In the newest IX, DPDK's driver is used to initialize NIC, however, I did not figure out a way to adapt it into kns.
Originally derived from old version of DPDK's driver, and the newest ixgbe driver is refered to as I believe the newer the better.

# Layers
Two layers: the lower to function in phy layer, and the upper layer to rx/tx pkt. The upper layer is deprived from ixgbe driver, kns 
read and write ring directly.

# Ixgbe Driver
In the original ixgbe driver, `ixgbe_main.c` is used to hold functions to registe and remove driver. In DPDK, it is substitute with 

- First, restore the `ixgbe_probe` function as what PCI drivers do.
  - Detect pci dev, and check its info.
  - `adapter` and `netdev` are rewrited. In the original Linux, `netdev_ops` is used to hold callback functions to visit rings.
  - iommap is also put here.
  - all the other lines are moved out to `adapter_init`.
- In Linux, softirq is called to let the kernel jump to visit rings. We disable irq here, kernel poll the rings continuously.
- The way to visit register of ixgbe is slightly different in linux and dpdk.
- Another hash function for rss is impled
