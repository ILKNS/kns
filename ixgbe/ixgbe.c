#include <linux/pci.h>
#include <linux/module.h>
#include <linux/workqueue.h>
#include <linux/aer.h>
#include <linux/spinlock.h>

#include <kns/ixgbe_setup.h>
#include <kns/ethdev.h>
#include <kns/errno.h>
#include "ixgbe.h"
#include "ixgbe_api.h"
#include "ixgbe_dcb.h"
#include "ixgbe_common.h"
#include "ixgbe_type.h"
#include "ixgbe_ethdev.h"
#include "ixgbe_vf.h"

char ixgbe_driver_name[] = "ixgbe";
static const char ixgbe_driver_string[] =
			      "Intel(R) 10 Gigabit PCI Express Network Driver";
#ifdef IXGBE_FCOE
char ixgbe_default_device_descr[] =
			      "Intel(R) 10 Gigabit Network Connection";
#else
static char ixgbe_default_device_descr[] =
			      "Intel(R) 10 Gigabit Network Connection";
#endif
#define DRV_VERSION "5.1.0-k"
const char ixgbe_driver_version[] = DRV_VERSION;
static const char ixgbe_copyright[] =
				"Copyright (c) 1999-2016 Intel Corporation.";

static const char ixgbe_overheat_msg[] = "Network adapter has been stopped because it has over heated. Restart the computer. If the problem persists, power off the system and replace the adapter";

static const struct ixgbe_info *ixgbe_info_tbl[] = {
	[board_82598]		= &ixgbe_82598_info,
	[board_82599]		= &ixgbe_82599_info,
	[board_X540]		= &ixgbe_X540_info,
	[board_X550]		= &ixgbe_X550_info,
	[board_X550EM_x]	= &ixgbe_X550EM_x_info,
	[board_x550em_x_fw]	= &ixgbe_x550em_x_fw_info,
	[board_x550em_a]	= &ixgbe_x550em_a_info,
	[board_x550em_a_fw]	= &ixgbe_x550em_a_fw_info,
};

/* ixgbe_pci_tbl - PCI Device ID Table
 *
 * Wildcard entries (PCI_ANY_ID) should come last
 * Last entry must be all 0s
 *
 * { Vendor ID, Device ID, SubVendor ID, SubDevice ID,
 *   Class, Class Mask, private data (not used) }
 */
static const struct pci_device_id ixgbe_pci_tbl[] = {
	{PCI_VDEVICE(INTEL, IXGBE_DEV_ID_82598), board_82598 },
	{PCI_VDEVICE(INTEL, IXGBE_DEV_ID_82598AF_DUAL_PORT), board_82598 },
	{PCI_VDEVICE(INTEL, IXGBE_DEV_ID_82598AF_SINGLE_PORT), board_82598 },
	{PCI_VDEVICE(INTEL, IXGBE_DEV_ID_82598AT), board_82598 },
	{PCI_VDEVICE(INTEL, IXGBE_DEV_ID_82598AT2), board_82598 },
	{PCI_VDEVICE(INTEL, IXGBE_DEV_ID_82598EB_CX4), board_82598 },
	{PCI_VDEVICE(INTEL, IXGBE_DEV_ID_82598_CX4_DUAL_PORT), board_82598 },
	{PCI_VDEVICE(INTEL, IXGBE_DEV_ID_82598_DA_DUAL_PORT), board_82598 },
	{PCI_VDEVICE(INTEL, IXGBE_DEV_ID_82598_SR_DUAL_PORT_EM), board_82598 },
	{PCI_VDEVICE(INTEL, IXGBE_DEV_ID_82598EB_XF_LR), board_82598 },
	{PCI_VDEVICE(INTEL, IXGBE_DEV_ID_82598EB_SFP_LOM), board_82598 },
	{PCI_VDEVICE(INTEL, IXGBE_DEV_ID_82598_BX), board_82598 },
	{PCI_VDEVICE(INTEL, IXGBE_DEV_ID_82599_KX4), board_82599 },
	{PCI_VDEVICE(INTEL, IXGBE_DEV_ID_82599_XAUI_LOM), board_82599 },
	{PCI_VDEVICE(INTEL, IXGBE_DEV_ID_82599_KR), board_82599 },
	{PCI_VDEVICE(INTEL, IXGBE_DEV_ID_82599_SFP), board_82599 },
	{PCI_VDEVICE(INTEL, IXGBE_DEV_ID_82599_SFP_EM), board_82599 },
	{PCI_VDEVICE(INTEL, IXGBE_DEV_ID_82599_KX4_MEZZ), board_82599 },
	{PCI_VDEVICE(INTEL, IXGBE_DEV_ID_82599_CX4), board_82599 },
	{PCI_VDEVICE(INTEL, IXGBE_DEV_ID_82599_BACKPLANE_FCOE), board_82599 },
	{PCI_VDEVICE(INTEL, IXGBE_DEV_ID_82599_SFP_FCOE), board_82599 },
	{PCI_VDEVICE(INTEL, IXGBE_DEV_ID_82599_T3_LOM), board_82599 },
	{PCI_VDEVICE(INTEL, IXGBE_DEV_ID_82599_COMBO_BACKPLANE), board_82599 },
	{PCI_VDEVICE(INTEL, IXGBE_DEV_ID_X540T), board_X540 },
	{PCI_VDEVICE(INTEL, IXGBE_DEV_ID_82599_SFP_SF2), board_82599 },
	{PCI_VDEVICE(INTEL, IXGBE_DEV_ID_82599_LS), board_82599 },
	{PCI_VDEVICE(INTEL, IXGBE_DEV_ID_82599_QSFP_SF_QP), board_82599 },
	{PCI_VDEVICE(INTEL, IXGBE_DEV_ID_82599EN_SFP), board_82599 },
	{PCI_VDEVICE(INTEL, IXGBE_DEV_ID_82599_SFP_SF_QP), board_82599 },
	{PCI_VDEVICE(INTEL, IXGBE_DEV_ID_X540T1), board_X540 },
	{PCI_VDEVICE(INTEL, IXGBE_DEV_ID_X550T), board_X550},
	{PCI_VDEVICE(INTEL, IXGBE_DEV_ID_X550T1), board_X550},
	{PCI_VDEVICE(INTEL, IXGBE_DEV_ID_X550EM_X_KX4), board_X550EM_x},
	{PCI_VDEVICE(INTEL, IXGBE_DEV_ID_X550EM_X_XFI), board_X550EM_x},
	{PCI_VDEVICE(INTEL, IXGBE_DEV_ID_X550EM_X_KR), board_X550EM_x},
	{PCI_VDEVICE(INTEL, IXGBE_DEV_ID_X550EM_X_10G_T), board_X550EM_x},
	{PCI_VDEVICE(INTEL, IXGBE_DEV_ID_X550EM_X_SFP), board_X550EM_x},
	{PCI_VDEVICE(INTEL, IXGBE_DEV_ID_X550EM_X_1G_T), board_x550em_x_fw},
	{PCI_VDEVICE(INTEL, IXGBE_DEV_ID_X550EM_A_KR), board_x550em_a },
	{PCI_VDEVICE(INTEL, IXGBE_DEV_ID_X550EM_A_KR_L), board_x550em_a },
	{PCI_VDEVICE(INTEL, IXGBE_DEV_ID_X550EM_A_SFP_N), board_x550em_a },
	{PCI_VDEVICE(INTEL, IXGBE_DEV_ID_X550EM_A_SGMII), board_x550em_a },
	{PCI_VDEVICE(INTEL, IXGBE_DEV_ID_X550EM_A_SGMII_L), board_x550em_a },
	{PCI_VDEVICE(INTEL, IXGBE_DEV_ID_X550EM_A_10G_T), board_x550em_a},
	{PCI_VDEVICE(INTEL, IXGBE_DEV_ID_X550EM_A_SFP), board_x550em_a },
	{PCI_VDEVICE(INTEL, IXGBE_DEV_ID_X550EM_A_1G_T), board_x550em_a_fw },
	{PCI_VDEVICE(INTEL, IXGBE_DEV_ID_X550EM_A_1G_T_L), board_x550em_a_fw },
	/* required last entry */
	{0, }
};
MODULE_DEVICE_TABLE(pci, ixgbe_pci_tbl);

// #ifdef CONFIG_IXGBE_DCA
// static int ixgbe_notify_dca(struct notifier_block *, unsigned long event,
// 			    void *p);
// static struct notifier_block dca_notifier = {
// 	.notifier_call = ixgbe_notify_dca,
// 	.next          = NULL,
// 	.priority      = 0
// };
// #endif

#ifdef CONFIG_PCI_IOV
static unsigned int max_vfs;
module_param(max_vfs, uint, 0);
MODULE_PARM_DESC(max_vfs,
		 "Maximum number of virtual functions to allocate per physical function - default is zero and maximum value is 63. (Deprecated)");
#endif /* CONFIG_PCI_IOV */

// static unsigned int allow_unsupported_sfp;
// module_param(allow_unsupported_sfp, uint, 0);
// MODULE_PARM_DESC(allow_unsupported_sfp,
// 		 "Allow unsupported and untested SFP+ modules on 82599-based adapters");

// #define DEFAULT_MSG_ENABLE (NETIF_MSG_DRV|NETIF_MSG_PROBE|NETIF_MSG_LINK)
// static int debug = -1;
// module_param(debug, int, 0);
// MODULE_PARM_DESC(debug, "Debug level (0=none,...,16=all)");

// MODULE_AUTHOR("Intel Corporation, <linux.nics@intel.com>");
// MODULE_DESCRIPTION("Intel(R) 10 Gigabit PCI Express Network Driver");
// MODULE_LICENSE("GPL");
// MODULE_VERSION(DRV_VERSION);

static struct workqueue_struct *ixgbe_wq;

spinlock_t ixgbe_dev_lock;

// static bool ixgbe_check_cfg_remove(struct ixgbe_hw *hw, struct pci_dev *pdev);
// static void ixgbe_watchdog_link_is_down(struct ixgbe_adapter *);

/*
 * High threshold controlling when to start sending XOFF frames. Must be at
 * least 8 bytes less than receive packet buffer size. This value is in units
 * of 1024 bytes.
 */
#define IXGBE_FC_HI    0x80

/*
 * Low threshold controlling when to start sending XON frames. This value is
 * in units of 1024 bytes.
 */
#define IXGBE_FC_LO    0x40

/* Timer value included in XOFF frames. */
#define IXGBE_FC_PAUSE 0x680

#define IXGBE_LINK_DOWN_CHECK_TIMEOUT 4000 /* ms */
#define IXGBE_LINK_UP_CHECK_TIMEOUT   1000 /* ms */
#define IXGBE_VMDQ_NUM_UC_MAC         4096 /* Maximum nb. of UC MAC addr. */

#define IXGBE_QUEUE_STAT_COUNTERS (sizeof(hw_stats->qprc) / sizeof(hw_stats->qprc[0]))

static int  ixgbe_dev_configure(struct rte_eth_dev *dev);
static int  ixgbe_dev_start(struct rte_eth_dev *dev);
static void ixgbe_dev_stop(struct rte_eth_dev *dev);
static void ixgbe_dev_close(struct rte_eth_dev *dev);
static void ixgbe_dev_promiscuous_enable(struct rte_eth_dev *dev);
static void ixgbe_dev_promiscuous_disable(struct rte_eth_dev *dev);
static void ixgbe_dev_allmulticast_enable(struct rte_eth_dev *dev);
static void ixgbe_dev_allmulticast_disable(struct rte_eth_dev *dev);
static int ixgbe_dev_link_update(struct rte_eth_dev *dev,
				int wait_to_complete);
static void ixgbe_dev_stats_get(struct rte_eth_dev *dev,
				struct rte_eth_stats *stats);
static void ixgbe_dev_stats_reset(struct rte_eth_dev *dev);
static int ixgbe_dev_queue_stats_mapping_set(struct rte_eth_dev *eth_dev,
					     uint16_t queue_id,
					     uint8_t stat_idx,
					     uint8_t is_rx);
static void ixgbe_dev_info_get(struct rte_eth_dev *dev,
				struct rte_eth_dev_info *dev_info);
static int ixgbe_vlan_filter_set(struct rte_eth_dev *dev,
		uint16_t vlan_id, int on);
static void ixgbe_vlan_tpid_set(struct rte_eth_dev *dev, uint16_t tpid_id);
static void ixgbe_vlan_hw_strip_bitmap_set(struct rte_eth_dev *dev,
		uint16_t queue, bool on);
static void ixgbe_vlan_strip_queue_set(struct rte_eth_dev *dev, uint16_t queue,
		int on);
static void ixgbe_vlan_offload_set(struct rte_eth_dev *dev, int mask);
static void ixgbe_vlan_hw_strip_enable(struct rte_eth_dev *dev, uint16_t queue);
static void ixgbe_vlan_hw_strip_disable(struct rte_eth_dev *dev, uint16_t queue);
static void ixgbe_vlan_hw_extend_enable(struct rte_eth_dev *dev);
static void ixgbe_vlan_hw_extend_disable(struct rte_eth_dev *dev);

static int ixgbe_dev_led_on(struct rte_eth_dev *dev);
static int ixgbe_dev_led_off(struct rte_eth_dev *dev);
static int  ixgbe_flow_ctrl_set(struct rte_eth_dev *dev,
		struct rte_eth_fc_conf *fc_conf);
static int ixgbe_priority_flow_ctrl_set(struct rte_eth_dev *dev,
		struct rte_eth_pfc_conf *pfc_conf);
static int ixgbe_dev_rss_reta_update(struct rte_eth_dev *dev,
		struct rte_eth_rss_reta *reta_conf);
static int ixgbe_dev_rss_reta_query(struct rte_eth_dev *dev,
		struct rte_eth_rss_reta *reta_conf);
static void ixgbe_dev_link_status_print(struct rte_eth_dev *dev);
static int ixgbe_dev_lsc_interrupt_setup(struct rte_eth_dev *dev);
static void ixgbe_add_rar(struct rte_eth_dev *dev, struct eth_addr *mac_addr,
		uint32_t index, uint32_t pool);
static void ixgbe_remove_rar(struct rte_eth_dev *dev, uint32_t index);
//static void ixgbe_dcb_init(struct ixgbe_hw *hw,struct ixgbe_dcb_config *dcb_config);

/* For Eth VMDQ APIs support */
static int ixgbe_uc_hash_table_set(struct rte_eth_dev *dev, struct
		eth_addr* mac_addr,uint8_t on);
static int ixgbe_uc_all_hash_table_set(struct rte_eth_dev *dev,uint8_t on);
static int  ixgbe_set_pool_rx_mode(struct rte_eth_dev *dev,  uint16_t pool,
		uint16_t rx_mask, uint8_t on);
static int ixgbe_set_pool_rx(struct rte_eth_dev *dev,uint16_t pool,uint8_t on);
static int ixgbe_set_pool_tx(struct rte_eth_dev *dev,uint16_t pool,uint8_t on);
static int ixgbe_set_pool_vlan_filter(struct rte_eth_dev *dev, uint16_t vlan,
		uint64_t pool_mask,uint8_t vlan_on);
static int ixgbe_mirror_rule_set(struct rte_eth_dev *dev,
		struct rte_eth_vmdq_mirror_conf *mirror_conf,
		uint8_t rule_id, uint8_t on);
static int ixgbe_mirror_rule_reset(struct rte_eth_dev *dev,
		uint8_t	rule_id);

#define IXGBE_SET_HWSTRIP(h, q) do{\
		uint32_t idx = (q) / (sizeof ((h)->bitmap[0]) * NBBY); \
		uint32_t bit = (q) % (sizeof ((h)->bitmap[0]) * NBBY); \
		(h)->bitmap[idx] |= 1 << bit;\
	}while(0)

#define IXGBE_CLEAR_HWSTRIP(h, q) do{\
		uint32_t idx = (q) / (sizeof ((h)->bitmap[0]) * NBBY); \
		uint32_t bit = (q) % (sizeof ((h)->bitmap[0]) * NBBY); \
		(h)->bitmap[idx] &= ~(1 << bit);\
	}while(0)

#define IXGBE_GET_HWSTRIP(h, q, r) do{\
		uint32_t idx = (q) / (sizeof ((h)->bitmap[0]) * NBBY); \
		uint32_t bit = (q) % (sizeof ((h)->bitmap[0]) * NBBY); \
		(r) = (h)->bitmap[idx] >> bit & 1;\
	}while(0)

static struct eth_dev_ops ixgbe_eth_dev_ops = {
	.dev_configure        = ixgbe_dev_configure,
	.dev_start            = ixgbe_dev_start,
	.dev_stop             = ixgbe_dev_stop,
	.dev_close            = ixgbe_dev_close,
	.promiscuous_enable   = ixgbe_dev_promiscuous_enable,
	.promiscuous_disable  = ixgbe_dev_promiscuous_disable,
	.allmulticast_enable  = ixgbe_dev_allmulticast_enable,
	.allmulticast_disable = ixgbe_dev_allmulticast_disable,
	.link_update          = ixgbe_dev_link_update,
	.stats_get            = ixgbe_dev_stats_get,
	.stats_reset          = ixgbe_dev_stats_reset,
	.queue_stats_mapping_set = ixgbe_dev_queue_stats_mapping_set,
	.dev_infos_get        = ixgbe_dev_info_get,
	.vlan_filter_set      = ixgbe_vlan_filter_set,
	.vlan_tpid_set        = ixgbe_vlan_tpid_set,
	.vlan_offload_set     = ixgbe_vlan_offload_set,
	.vlan_strip_queue_set = ixgbe_vlan_strip_queue_set,
	.rx_queue_setup       = ixgbe_dev_rx_queue_setup,
	.rx_queue_release     = ixgbe_dev_rx_queue_release,
	.tx_queue_setup       = ixgbe_dev_tx_queue_setup,
	.tx_queue_release     = ixgbe_dev_tx_queue_release,
	.dev_led_on           = ixgbe_dev_led_on,
	.dev_led_off          = ixgbe_dev_led_off,
	.flow_ctrl_set        = ixgbe_flow_ctrl_set,
	.priority_flow_ctrl_set = ixgbe_priority_flow_ctrl_set,
	.mac_addr_add         = ixgbe_add_rar,
	.mac_addr_remove      = ixgbe_remove_rar,
	.uc_hash_table_set    = ixgbe_uc_hash_table_set,
	.uc_all_hash_table_set  = ixgbe_uc_all_hash_table_set,
	.mirror_rule_set   	= ixgbe_mirror_rule_set,
	.mirror_rule_reset 	= ixgbe_mirror_rule_reset,
	.set_vf_rx_mode       = ixgbe_set_pool_rx_mode,
	.set_vf_rx            = ixgbe_set_pool_rx,
	.set_vf_tx            = ixgbe_set_pool_tx,
	.set_vf_vlan_filter   = ixgbe_set_pool_vlan_filter,
	.fdir_add_signature_filter    = ixgbe_fdir_add_signature_filter,
	.fdir_update_signature_filter = ixgbe_fdir_update_signature_filter,
	.fdir_remove_signature_filter = ixgbe_fdir_remove_signature_filter,
	.fdir_infos_get               = ixgbe_fdir_info_get,
	.fdir_add_perfect_filter      = ixgbe_fdir_add_perfect_filter,
	.fdir_update_perfect_filter   = ixgbe_fdir_update_perfect_filter,
	.fdir_remove_perfect_filter   = ixgbe_fdir_remove_perfect_filter,
	.fdir_set_masks               = ixgbe_fdir_set_masks,
	.reta_update          = ixgbe_dev_rss_reta_update,
	.reta_query           = ixgbe_dev_rss_reta_query,
};

/*
 * This function is the same as ixgbe_is_sfp() in ixgbe/ixgbe.h.
 */
static inline int
ixgbe_is_sfp(struct ixgbe_hw *hw)
{
	switch (hw->phy.type) {
	case ixgbe_phy_sfp_avago:
	case ixgbe_phy_sfp_ftl:
	case ixgbe_phy_sfp_intel:
	case ixgbe_phy_sfp_unknown:
	case ixgbe_phy_sfp_passive_tyco:
	case ixgbe_phy_sfp_passive_unknown:
		return 1;
	default:
		return 0;
	}
}

static inline int32_t
ixgbe_pf_reset_hw(struct ixgbe_hw *hw)
{
	uint32_t ctrl_ext;
	int32_t status;

	status = ixgbe_reset_hw(hw);

	ctrl_ext = IXGBE_READ_REG(hw, IXGBE_CTRL_EXT);
	/* Set PF Reset Done bit so PF/VF Mail Ops can work */
	ctrl_ext |= IXGBE_CTRL_EXT_PFRSTD;
	IXGBE_WRITE_REG(hw, IXGBE_CTRL_EXT, ctrl_ext);
	IXGBE_WRITE_FLUSH(hw);

	return status;
}

static inline void
ixgbe_enable_intr(struct rte_eth_dev *dev)
{
	struct ixgbe_interrupt *intr =
		IXGBE_DEV_PRIVATE_TO_INTR(dev->data->dev_private);
	struct ixgbe_hw *hw =
		IXGBE_DEV_PRIVATE_TO_HW(dev->data->dev_private);

	IXGBE_WRITE_REG(hw, IXGBE_EIMS, intr->mask);
	IXGBE_WRITE_FLUSH(hw);
}

static int
ixgbe_dev_queue_stats_mapping_set(struct rte_eth_dev *eth_dev,
				  uint16_t queue_id,
				  uint8_t stat_idx,
				  uint8_t is_rx)
{
#define QSM_REG_NB_BITS_PER_QMAP_FIELD 8
#define NB_QMAP_FIELDS_PER_QSM_REG 4
#define QMAP_FIELD_RESERVED_BITS_MASK 0x0f

	struct ixgbe_hw *hw = IXGBE_DEV_PRIVATE_TO_HW(eth_dev->data->dev_private);
	struct ixgbe_stat_mapping_registers *stat_mappings =
		IXGBE_DEV_PRIVATE_TO_STAT_MAPPINGS(eth_dev->data->dev_private);
	uint32_t qsmr_mask = 0;
	uint32_t clearing_mask = QMAP_FIELD_RESERVED_BITS_MASK;
	uint32_t q_map;
	uint8_t n, offset;

	if ((hw->mac.type != ixgbe_mac_82599EB) && (hw->mac.type != ixgbe_mac_X540))
		return -ENOSYS;

	pr_info("ixgbe: Setting port %d, %s queue_id %d to stat index %d\n",
		     (int)(eth_dev->data->port_id), is_rx ? "RX" : "TX", queue_id, stat_idx);

	n = (uint8_t)(queue_id / NB_QMAP_FIELDS_PER_QSM_REG);
	if (n >= IXGBE_NB_STAT_MAPPING_REGS) {
		pr_info("ixgbe: Nb of stat mapping registers exceeded\n");
		return -EIO;
	}
	offset = (uint8_t)(queue_id % NB_QMAP_FIELDS_PER_QSM_REG);

	/* Now clear any previous stat_idx set */
	clearing_mask <<= (QSM_REG_NB_BITS_PER_QMAP_FIELD * offset);
	if (!is_rx)
		stat_mappings->tqsm[n] &= ~clearing_mask;
	else
		stat_mappings->rqsmr[n] &= ~clearing_mask;

	q_map = (uint32_t)stat_idx;
	q_map &= QMAP_FIELD_RESERVED_BITS_MASK;
	qsmr_mask = q_map << (QSM_REG_NB_BITS_PER_QMAP_FIELD * offset);
	if (!is_rx)
		stat_mappings->tqsm[n] |= qsmr_mask;
	else
		stat_mappings->rqsmr[n] |= qsmr_mask;

	pr_info("ixgbe: Set port %d, %s queue_id %d to stat index %d\n"
		     "%s[%d] = 0x%08x\n",
		     (int)(eth_dev->data->port_id), is_rx ? "RX" : "TX", queue_id, stat_idx,
		     is_rx ? "RQSMR" : "TQSM",n, is_rx ? stat_mappings->rqsmr[n] : stat_mappings->tqsm[n]);

	/* Now write the mapping in the appropriate register */
	if (is_rx) {
		pr_info("ixgbe: Write 0x%x to RX IXGBE stat mapping reg:%d\n",
			     stat_mappings->rqsmr[n], n);
		IXGBE_WRITE_REG(hw, IXGBE_RQSMR(n), stat_mappings->rqsmr[n]);
	}
	else {
		pr_info("ixgbe: Write 0x%x to TX IXGBE stat mapping reg:%d\n",
			     stat_mappings->tqsm[n], n);
		IXGBE_WRITE_REG(hw, IXGBE_TQSM(n), stat_mappings->tqsm[n]);
	}
	return 0;
}


/*
 * This function is based on ixgbe_disable_intr() in ixgbe/ixgbe.h.
 */
static void
ixgbe_disable_intr(struct ixgbe_hw *hw)
{
	if (hw->mac.type == ixgbe_mac_82598EB) {
		IXGBE_WRITE_REG(hw, IXGBE_EIMC, ~0);
	} else {
		IXGBE_WRITE_REG(hw, IXGBE_EIMC, 0xFFFF0000);
		IXGBE_WRITE_REG(hw, IXGBE_EIMC_EX(0), ~0);
		IXGBE_WRITE_REG(hw, IXGBE_EIMC_EX(1), ~0);
	}
	IXGBE_WRITE_FLUSH(hw);
}

static void
ixgbe_restore_statistics_mapping(struct rte_eth_dev * dev)
{
	struct ixgbe_stat_mapping_registers *stat_mappings =
		IXGBE_DEV_PRIVATE_TO_STAT_MAPPINGS(dev->data->dev_private);
	struct ixgbe_hw *hw = IXGBE_DEV_PRIVATE_TO_HW(dev->data->dev_private);
	int i;

	/* write whatever was in stat mapping table to the NIC */
	for (i = 0; i < IXGBE_NB_STAT_MAPPING_REGS; i++) {
		/* rx */
		IXGBE_WRITE_REG(hw, IXGBE_RQSMR(i), stat_mappings->rqsmr[i]);

		/* tx */
		IXGBE_WRITE_REG(hw, IXGBE_TQSM(i), stat_mappings->tqsm[i]);
	}
}

static int
ixgbe_vlan_filter_set(struct rte_eth_dev *dev, uint16_t vlan_id, int on)
{
	struct ixgbe_hw *hw =
		IXGBE_DEV_PRIVATE_TO_HW(dev->data->dev_private);
	struct ixgbe_vfta * shadow_vfta =
		IXGBE_DEV_PRIVATE_TO_VFTA(dev->data->dev_private);
	uint32_t vfta;
	uint32_t vid_idx;
	uint32_t vid_bit;

	vid_idx = (uint32_t) ((vlan_id >> 5) & 0x7F);
	vid_bit = (uint32_t) (1 << (vlan_id & 0x1F));
	vfta = IXGBE_READ_REG(hw, IXGBE_VFTA(vid_idx));
	if (on)
		vfta |= vid_bit;
	else
		vfta &= ~vid_bit;
	IXGBE_WRITE_REG(hw, IXGBE_VFTA(vid_idx), vfta);

	/* update local VFTA copy */
	shadow_vfta->vfta[vid_idx] = vfta;

	return 0;
}

static void
ixgbe_vlan_strip_queue_set(struct rte_eth_dev *dev, uint16_t queue, int on)
{
	if (on)
		ixgbe_vlan_hw_strip_enable(dev, queue);
	else
		ixgbe_vlan_hw_strip_disable(dev, queue);
}

static void
ixgbe_vlan_tpid_set(struct rte_eth_dev *dev, uint16_t tpid)
{
	struct ixgbe_hw *hw =
		IXGBE_DEV_PRIVATE_TO_HW(dev->data->dev_private);

	/* Only the high 16-bits is valid */
	IXGBE_WRITE_REG(hw, IXGBE_EXVET, tpid << 16);
}

void
ixgbe_vlan_hw_filter_disable(struct rte_eth_dev *dev)
{
	struct ixgbe_hw *hw =
		IXGBE_DEV_PRIVATE_TO_HW(dev->data->dev_private);
	uint32_t vlnctrl;

	/* Filter Table Disable */
	vlnctrl = IXGBE_READ_REG(hw, IXGBE_VLNCTRL);
	vlnctrl &= ~IXGBE_VLNCTRL_VFE;

	IXGBE_WRITE_REG(hw, IXGBE_VLNCTRL, vlnctrl);
}

void
ixgbe_vlan_hw_filter_enable(struct rte_eth_dev *dev)
{
	struct ixgbe_hw *hw =
		IXGBE_DEV_PRIVATE_TO_HW(dev->data->dev_private);
	struct ixgbe_vfta * shadow_vfta =
		IXGBE_DEV_PRIVATE_TO_VFTA(dev->data->dev_private);
	uint32_t vlnctrl;
	uint16_t i;

	/* Filter Table Enable */
	vlnctrl = IXGBE_READ_REG(hw, IXGBE_VLNCTRL);
	vlnctrl &= ~IXGBE_VLNCTRL_CFIEN;
	vlnctrl |= IXGBE_VLNCTRL_VFE;

	IXGBE_WRITE_REG(hw, IXGBE_VLNCTRL, vlnctrl);

	/* write whatever is in local vfta copy */
	for (i = 0; i < IXGBE_VFTA_SIZE; i++)
		IXGBE_WRITE_REG(hw, IXGBE_VFTA(i), shadow_vfta->vfta[i]);
}

static void
ixgbe_vlan_hw_strip_bitmap_set(struct rte_eth_dev *dev, uint16_t queue, bool on)
{
	struct ixgbe_hwstrip *hwstrip =
		IXGBE_DEV_PRIVATE_TO_HWSTRIP_BITMAP(dev->data->dev_private);

	if(queue >= IXGBE_MAX_RX_QUEUE_NUM)
		return;

	if (on)
		IXGBE_SET_HWSTRIP(hwstrip, queue);
	else
		IXGBE_CLEAR_HWSTRIP(hwstrip, queue);
}

static void
ixgbe_vlan_hw_strip_disable(struct rte_eth_dev *dev, uint16_t queue)
{
	struct ixgbe_hw *hw =
		IXGBE_DEV_PRIVATE_TO_HW(dev->data->dev_private);
	uint32_t ctrl;

	if (hw->mac.type == ixgbe_mac_82598EB) {
		/* No queue level support */
		pr_info("ixgbe: 82598EB does not support queue level hw strip");
		return;
	}
	else {
		/* Other 10G NIC, the VLAN strip can be setup per queue in RXDCTL */
		ctrl = IXGBE_READ_REG(hw, IXGBE_RXDCTL(queue));
		ctrl &= ~IXGBE_RXDCTL_VME;
		IXGBE_WRITE_REG(hw, IXGBE_RXDCTL(queue), ctrl);
	}
	/* record those setting for HW strip per queue */
	ixgbe_vlan_hw_strip_bitmap_set(dev, queue, 0);
}

static void
ixgbe_vlan_hw_strip_enable(struct rte_eth_dev *dev, uint16_t queue)
{
	struct ixgbe_hw *hw =
		IXGBE_DEV_PRIVATE_TO_HW(dev->data->dev_private);
	uint32_t ctrl;

	if (hw->mac.type == ixgbe_mac_82598EB) {
		/* No queue level supported */
		pr_info("ixgbe: 82598EB not support queue level hw strip");
		return;
	}
	else {
		/* Other 10G NIC, the VLAN strip can be setup per queue in RXDCTL */
		ctrl = IXGBE_READ_REG(hw, IXGBE_RXDCTL(queue));
		ctrl |= IXGBE_RXDCTL_VME;
		IXGBE_WRITE_REG(hw, IXGBE_RXDCTL(queue), ctrl);
	}
	/* record those setting for HW strip per queue */
	ixgbe_vlan_hw_strip_bitmap_set(dev, queue, 1);
}

void
ixgbe_vlan_hw_strip_disable_all(struct rte_eth_dev *dev)
{
	struct ixgbe_hw *hw =
		IXGBE_DEV_PRIVATE_TO_HW(dev->data->dev_private);
	uint32_t ctrl;
	uint16_t i;

	if (hw->mac.type == ixgbe_mac_82598EB) {
		ctrl = IXGBE_READ_REG(hw, IXGBE_VLNCTRL);
		ctrl &= ~IXGBE_VLNCTRL_VME;
		IXGBE_WRITE_REG(hw, IXGBE_VLNCTRL, ctrl);
	}
	else {
		/* Other 10G NIC, the VLAN strip can be setup per queue in RXDCTL */
		for (i = 0; i < dev->data->nb_rx_queues; i++) {
			ctrl = IXGBE_READ_REG(hw, IXGBE_RXDCTL(i));
			ctrl &= ~IXGBE_RXDCTL_VME;
			IXGBE_WRITE_REG(hw, IXGBE_RXDCTL(i), ctrl);

			/* record those setting for HW strip per queue */
			ixgbe_vlan_hw_strip_bitmap_set(dev, i, 0);
		}
	}
}

void
ixgbe_vlan_hw_strip_enable_all(struct rte_eth_dev *dev)
{
	struct ixgbe_hw *hw =
		IXGBE_DEV_PRIVATE_TO_HW(dev->data->dev_private);
	uint32_t ctrl;
	uint16_t i;

	if (hw->mac.type == ixgbe_mac_82598EB) {
		ctrl = IXGBE_READ_REG(hw, IXGBE_VLNCTRL);
		ctrl |= IXGBE_VLNCTRL_VME;
		IXGBE_WRITE_REG(hw, IXGBE_VLNCTRL, ctrl);
	}
	else {
		/* Other 10G NIC, the VLAN strip can be setup per queue in RXDCTL */
		for (i = 0; i < dev->data->nb_rx_queues; i++) {
			ctrl = IXGBE_READ_REG(hw, IXGBE_RXDCTL(i));
			ctrl |= IXGBE_RXDCTL_VME;
			IXGBE_WRITE_REG(hw, IXGBE_RXDCTL(i), ctrl);

			/* record those setting for HW strip per queue */
			ixgbe_vlan_hw_strip_bitmap_set(dev, i, 1);
		}
	}
}

static void
ixgbe_vlan_hw_extend_disable(struct rte_eth_dev *dev)
{
	struct ixgbe_hw *hw =
		IXGBE_DEV_PRIVATE_TO_HW(dev->data->dev_private);
	uint32_t ctrl;

	/* DMATXCTRL: Geric Double VLAN Disable */
	ctrl = IXGBE_READ_REG(hw, IXGBE_DMATXCTL);
	ctrl &= ~IXGBE_DMATXCTL_GDV;
	IXGBE_WRITE_REG(hw, IXGBE_DMATXCTL, ctrl);

	/* CTRL_EXT: Global Double VLAN Disable */
	ctrl = IXGBE_READ_REG(hw, IXGBE_CTRL_EXT);
	ctrl &= ~IXGBE_EXTENDED_VLAN;
	IXGBE_WRITE_REG(hw, IXGBE_CTRL_EXT, ctrl);

}

static void
ixgbe_vlan_hw_extend_enable(struct rte_eth_dev *dev)
{
	struct ixgbe_hw *hw =
		IXGBE_DEV_PRIVATE_TO_HW(dev->data->dev_private);
	uint32_t ctrl;

	/* DMATXCTRL: Geric Double VLAN Enable */
	ctrl  = IXGBE_READ_REG(hw, IXGBE_DMATXCTL);
	ctrl |= IXGBE_DMATXCTL_GDV;
	IXGBE_WRITE_REG(hw, IXGBE_DMATXCTL, ctrl);

	/* CTRL_EXT: Global Double VLAN Enable */
	ctrl  = IXGBE_READ_REG(hw, IXGBE_CTRL_EXT);
	ctrl |= IXGBE_EXTENDED_VLAN;
	IXGBE_WRITE_REG(hw, IXGBE_CTRL_EXT, ctrl);

	/*
	 * VET EXT field in the EXVET register = 0x8100 by default
	 * So no need to change. Same to VT field of DMATXCTL register
	 */
}

static void
ixgbe_vlan_offload_set(struct rte_eth_dev *dev, int mask)
{
	if(mask & ETH_VLAN_STRIP_MASK){
		if (dev->data->dev_conf.rxmode.hw_vlan_strip)
			ixgbe_vlan_hw_strip_enable_all(dev);
		else
			ixgbe_vlan_hw_strip_disable_all(dev);
	}

	if(mask & ETH_VLAN_FILTER_MASK){
		if (dev->data->dev_conf.rxmode.hw_vlan_filter)
			ixgbe_vlan_hw_filter_enable(dev);
		else
			ixgbe_vlan_hw_filter_disable(dev);
	}

	if(mask & ETH_VLAN_EXTEND_MASK){
		if (dev->data->dev_conf.rxmode.hw_vlan_extend)
			ixgbe_vlan_hw_extend_enable(dev);
		else
			ixgbe_vlan_hw_extend_disable(dev);
	}
}


static void
ixgbe_vmdq_vlan_hw_filter_enable(struct rte_eth_dev *dev)
{
	struct ixgbe_hw *hw =
		IXGBE_DEV_PRIVATE_TO_HW(dev->data->dev_private);
	/* VLNCTRL: enable vlan filtering and allow all vlan tags through */
	uint32_t vlanctrl = IXGBE_READ_REG(hw, IXGBE_VLNCTRL);
	vlanctrl |= IXGBE_VLNCTRL_VFE ; /* enable vlan filters */
	IXGBE_WRITE_REG(hw, IXGBE_VLNCTRL, vlanctrl);
}

static int
ixgbe_dev_configure(struct rte_eth_dev *dev)
{
	struct ixgbe_interrupt *intr =
		IXGBE_DEV_PRIVATE_TO_INTR(dev->data->dev_private);

	/* set flag to update link status after init */
	intr->flags |= IXGBE_FLAG_NEED_LINK_UPDATE;

	return 0;
}

/*
 * Configure device link speed and setup link.
 * It returns 0 on success.
 */
static int
ixgbe_dev_start(struct rte_eth_dev *dev)
{
	struct ixgbe_hw *hw =
		IXGBE_DEV_PRIVATE_TO_HW(dev->data->dev_private);
	int err;
	uint32_t speed = 0;
	int mask = 0;
	int status;
	bool link_up = 0, negotiate = 0;

	/* IXGBE devices don't support half duplex */
	if ((dev->data->dev_conf.link_duplex != ETH_LINK_AUTONEG_DUPLEX) &&
			(dev->data->dev_conf.link_duplex != ETH_LINK_FULL_DUPLEX)) {
		printk(KERN_ERR "ixgbe: Invalid link_duplex (%u) for port %u\n",
			dev->data->dev_conf.link_duplex, dev->data->port_id);
		return -EINVAL;
	}

	/* stop adapter */
	hw->adapter_stopped = 0;
	ixgbe_stop_adapter(hw);

	/* reinitialize adapter
	 * this calls reset and start */
	status = ixgbe_pf_reset_hw(hw);
	if (status != 0)
		return -1;
	hw->mac.ops.start_hw(hw);

	/* configure PF module if SRIOV enabled */
	ixgbe_pf_host_configure(dev);

	/* initialize transmission unit */
	ixgbe_dev_tx_init(dev);

	/* This can fail when allocating mbufs for descriptor rings */
	err = ixgbe_dev_rx_init(dev);
	if (err) {
		printk(KERN_ERR "ixgbe: Unable to initialize RX hardware\n");
		goto error;
	}

	ixgbe_dev_rxtx_start(dev);

	if (ixgbe_is_sfp(hw) && hw->phy.multispeed_fiber) {
		err = hw->mac.ops.setup_sfp(hw);
		if (err)
			goto error;
	}

	/* Turn on the laser */
	ixgbe_enable_tx_laser(hw);

	/* Skip link setup if loopback mode is enabled for 82599. */
	if (hw->mac.type == ixgbe_mac_82599EB &&
			dev->data->dev_conf.lpbk_mode == IXGBE_LPBK_82599_TX_RX)
		goto skip_link_setup;

	err = ixgbe_check_link(hw, &speed, &link_up, 0);
	if (err)
		goto error;
	err = ixgbe_get_link_capabilities(hw, &speed, &negotiate);
	if (err)
		goto error;

	switch(dev->data->dev_conf.link_speed) {
	case ETH_LINK_SPEED_AUTONEG:
		speed = (hw->mac.type != ixgbe_mac_82598EB) ?
				IXGBE_LINK_SPEED_82599_AUTONEG :
				IXGBE_LINK_SPEED_82598_AUTONEG;
		break;
	case ETH_LINK_SPEED_100:
		/*
		 * Invalid for 82598 but error will be detected by
		 * ixgbe_setup_link()
		 */
		speed = IXGBE_LINK_SPEED_100_FULL;
		break;
	case ETH_LINK_SPEED_1000:
		speed = IXGBE_LINK_SPEED_1GB_FULL;
		break;
	case ETH_LINK_SPEED_10000:
		speed = IXGBE_LINK_SPEED_10GB_FULL;
		break;
	default:
		printk(KERN_ERR "ixgbe: Invalid link_speed (%u) for port %u\n",
			dev->data->dev_conf.link_speed, dev->data->port_id);
		goto error;
	}

	err = ixgbe_setup_link(hw, speed, negotiate, 0);
	if (err)
		goto error;

skip_link_setup:

	/* check if lsc interrupt is enabled */
	if (dev->data->dev_conf.intr_conf.lsc != 0)
		ixgbe_dev_lsc_interrupt_setup(dev);

	/* resume enabled intr since hw reset */
	ixgbe_enable_intr(dev);

	mask = ETH_VLAN_STRIP_MASK | ETH_VLAN_FILTER_MASK | \
		ETH_VLAN_EXTEND_MASK;
	ixgbe_vlan_offload_set(dev, mask);

	if (dev->data->dev_conf.rxmode.mq_mode == ETH_MQ_RX_VMDQ_ONLY) {
		/* Enable vlan filtering for VMDq */
		ixgbe_vmdq_vlan_hw_filter_enable(dev);
	}

	/* Configure DCB hw */
	ixgbe_configure_dcb(dev);

	if (dev->data->dev_conf.fdir_conf.mode != RTE_FDIR_MODE_NONE) {
		err = ixgbe_fdir_configure(dev);
		if (err)
			goto error;
	}

	ixgbe_restore_statistics_mapping(dev);

	return (0);

error:
	printk(KERN_ERR "ixgbe: failure in ixgbe_dev_start(): %d", err);
	ixgbe_dev_clear_queues(dev);
	return -EIO;
}

/*
 * Stop device: disable rx and tx functions to allow for reconfiguring.
 */
static void
ixgbe_dev_stop(struct rte_eth_dev *dev)
{
	struct rte_eth_link link;
	struct ixgbe_hw *hw =
		IXGBE_DEV_PRIVATE_TO_HW(dev->data->dev_private);

	/* disable interrupts */
	ixgbe_disable_intr(hw);

	/* reset the NIC */
	ixgbe_pf_reset_hw(hw);
	hw->adapter_stopped = 0;

	/* stop adapter */
	ixgbe_stop_adapter(hw);

	/* Turn off the laser */
	ixgbe_disable_tx_laser(hw);

	ixgbe_dev_clear_queues(dev);

	/* Clear recorded link status */
	memset(&link, 0, sizeof(link));
	dev->data->dev_link = link;
}

/*
 * Reest and stop device.
 */
static void
ixgbe_dev_close(struct rte_eth_dev *dev)
{
	struct ixgbe_hw *hw =
		IXGBE_DEV_PRIVATE_TO_HW(dev->data->dev_private);

	ixgbe_pf_reset_hw(hw);

	ixgbe_dev_stop(dev);
	hw->adapter_stopped = 1;

	ixgbe_disable_pcie_master(hw);

	/* reprogram the RAR[0] in case user changed it. */
	ixgbe_set_rar(hw, 0, hw->mac.addr, 0, IXGBE_RAH_AV);
}

/*
 * This function is based on ixgbe_update_stats_counters() in ixgbe/ixgbe.c
 */
static void
ixgbe_dev_stats_get(struct rte_eth_dev *dev, struct rte_eth_stats *stats)
{
	struct ixgbe_hw *hw =
			IXGBE_DEV_PRIVATE_TO_HW(dev->data->dev_private);
	struct ixgbe_hw_stats *hw_stats =
			IXGBE_DEV_PRIVATE_TO_STATS(dev->data->dev_private);
	uint32_t bprc, lxon, lxoff, total;
	uint64_t total_missed_rx, total_qbrc, total_qprc;
	unsigned i;

	total_missed_rx = 0;
	total_qbrc = 0;
	total_qprc = 0;

	hw_stats->crcerrs += IXGBE_READ_REG(hw, IXGBE_CRCERRS);
	hw_stats->illerrc += IXGBE_READ_REG(hw, IXGBE_ILLERRC);
	hw_stats->errbc += IXGBE_READ_REG(hw, IXGBE_ERRBC);
	hw_stats->mspdc += IXGBE_READ_REG(hw, IXGBE_MSPDC);

	for (i = 0; i < 8; i++) {
		uint32_t mp;
		mp = IXGBE_READ_REG(hw, IXGBE_MPC(i));
		/* global total per queue */
		hw_stats->mpc[i] += mp;
		/* Running comprehensive total for stats display */
		total_missed_rx += hw_stats->mpc[i];
		if (hw->mac.type == ixgbe_mac_82598EB)
			hw_stats->rnbc[i] +=
			    IXGBE_READ_REG(hw, IXGBE_RNBC(i));
		hw_stats->pxontxc[i] +=
		    IXGBE_READ_REG(hw, IXGBE_PXONTXC(i));
		hw_stats->pxonrxc[i] +=
		    IXGBE_READ_REG(hw, IXGBE_PXONRXC(i));
		hw_stats->pxofftxc[i] +=
		    IXGBE_READ_REG(hw, IXGBE_PXOFFTXC(i));
		hw_stats->pxoffrxc[i] +=
		    IXGBE_READ_REG(hw, IXGBE_PXOFFRXC(i));
		hw_stats->pxon2offc[i] +=
		    IXGBE_READ_REG(hw, IXGBE_PXON2OFFCNT(i));
	}
	for (i = 0; i < IXGBE_QUEUE_STAT_COUNTERS; i++) {
		hw_stats->qprc[i] += IXGBE_READ_REG(hw, IXGBE_QPRC(i));
		hw_stats->qptc[i] += IXGBE_READ_REG(hw, IXGBE_QPTC(i));
		hw_stats->qbrc[i] += IXGBE_READ_REG(hw, IXGBE_QBRC_L(i));
		hw_stats->qbrc[i] +=
		    ((uint64_t)IXGBE_READ_REG(hw, IXGBE_QBRC_H(i)) << 32);
		hw_stats->qbtc[i] += IXGBE_READ_REG(hw, IXGBE_QBTC_L(i));
		hw_stats->qbtc[i] +=
		    ((uint64_t)IXGBE_READ_REG(hw, IXGBE_QBTC_H(i)) << 32);
		hw_stats->qprdc[i] += IXGBE_READ_REG(hw, IXGBE_QPRDC(i));

		total_qprc += hw_stats->qprc[i];
		total_qbrc += hw_stats->qbrc[i];
	}
	hw_stats->mlfc += IXGBE_READ_REG(hw, IXGBE_MLFC);
	hw_stats->mrfc += IXGBE_READ_REG(hw, IXGBE_MRFC);
	hw_stats->rlec += IXGBE_READ_REG(hw, IXGBE_RLEC);

	/* Note that gprc counts missed packets */
	hw_stats->gprc += IXGBE_READ_REG(hw, IXGBE_GPRC);

	if (hw->mac.type != ixgbe_mac_82598EB) {
		hw_stats->gorc += IXGBE_READ_REG(hw, IXGBE_GORCL);
		hw_stats->gorc += ((u64)IXGBE_READ_REG(hw, IXGBE_GORCH) << 32);
		hw_stats->gotc += IXGBE_READ_REG(hw, IXGBE_GOTCL);
		hw_stats->gotc += ((u64)IXGBE_READ_REG(hw, IXGBE_GOTCH) << 32);
		hw_stats->tor += IXGBE_READ_REG(hw, IXGBE_TORL);
		hw_stats->tor += ((u64)IXGBE_READ_REG(hw, IXGBE_TORH) << 32);
		hw_stats->lxonrxc += IXGBE_READ_REG(hw, IXGBE_LXONRXCNT);
		hw_stats->lxoffrxc += IXGBE_READ_REG(hw, IXGBE_LXOFFRXCNT);
	} else {
		hw_stats->lxonrxc += IXGBE_READ_REG(hw, IXGBE_LXONRXC);
		hw_stats->lxoffrxc += IXGBE_READ_REG(hw, IXGBE_LXOFFRXC);
		/* 82598 only has a counter in the high register */
		hw_stats->gorc += IXGBE_READ_REG(hw, IXGBE_GORCH);
		hw_stats->gotc += IXGBE_READ_REG(hw, IXGBE_GOTCH);
		hw_stats->tor += IXGBE_READ_REG(hw, IXGBE_TORH);
	}

	/*
	 * Workaround: mprc hardware is incorrectly counting
	 * broadcasts, so for now we subtract those.
	 */
	bprc = IXGBE_READ_REG(hw, IXGBE_BPRC);
	hw_stats->bprc += bprc;
	hw_stats->mprc += IXGBE_READ_REG(hw, IXGBE_MPRC);
	if (hw->mac.type == ixgbe_mac_82598EB)
		hw_stats->mprc -= bprc;

	hw_stats->prc64 += IXGBE_READ_REG(hw, IXGBE_PRC64);
	hw_stats->prc127 += IXGBE_READ_REG(hw, IXGBE_PRC127);
	hw_stats->prc255 += IXGBE_READ_REG(hw, IXGBE_PRC255);
	hw_stats->prc511 += IXGBE_READ_REG(hw, IXGBE_PRC511);
	hw_stats->prc1023 += IXGBE_READ_REG(hw, IXGBE_PRC1023);
	hw_stats->prc1522 += IXGBE_READ_REG(hw, IXGBE_PRC1522);

	lxon = IXGBE_READ_REG(hw, IXGBE_LXONTXC);
	hw_stats->lxontxc += lxon;
	lxoff = IXGBE_READ_REG(hw, IXGBE_LXOFFTXC);
	hw_stats->lxofftxc += lxoff;
	total = lxon + lxoff;

	hw_stats->gptc += IXGBE_READ_REG(hw, IXGBE_GPTC);
	hw_stats->mptc += IXGBE_READ_REG(hw, IXGBE_MPTC);
	hw_stats->ptc64 += IXGBE_READ_REG(hw, IXGBE_PTC64);
	hw_stats->gptc -= total;
	hw_stats->mptc -= total;
	hw_stats->ptc64 -= total;
	hw_stats->gotc -= total * ETH_MIN_LEN;

	hw_stats->ruc += IXGBE_READ_REG(hw, IXGBE_RUC);
	hw_stats->rfc += IXGBE_READ_REG(hw, IXGBE_RFC);
	hw_stats->roc += IXGBE_READ_REG(hw, IXGBE_ROC);
	hw_stats->rjc += IXGBE_READ_REG(hw, IXGBE_RJC);
	hw_stats->mngprc += IXGBE_READ_REG(hw, IXGBE_MNGPRC);
	hw_stats->mngpdc += IXGBE_READ_REG(hw, IXGBE_MNGPDC);
	hw_stats->mngptc += IXGBE_READ_REG(hw, IXGBE_MNGPTC);
	hw_stats->tpr += IXGBE_READ_REG(hw, IXGBE_TPR);
	hw_stats->tpt += IXGBE_READ_REG(hw, IXGBE_TPT);
	hw_stats->ptc127 += IXGBE_READ_REG(hw, IXGBE_PTC127);
	hw_stats->ptc255 += IXGBE_READ_REG(hw, IXGBE_PTC255);
	hw_stats->ptc511 += IXGBE_READ_REG(hw, IXGBE_PTC511);
	hw_stats->ptc1023 += IXGBE_READ_REG(hw, IXGBE_PTC1023);
	hw_stats->ptc1522 += IXGBE_READ_REG(hw, IXGBE_PTC1522);
	hw_stats->bptc += IXGBE_READ_REG(hw, IXGBE_BPTC);
	hw_stats->xec += IXGBE_READ_REG(hw, IXGBE_XEC);
	hw_stats->fccrc += IXGBE_READ_REG(hw, IXGBE_FCCRC);
	hw_stats->fclast += IXGBE_READ_REG(hw, IXGBE_FCLAST);
	/* Only read FCOE on 82599 */
	if (hw->mac.type != ixgbe_mac_82598EB) {
		hw_stats->fcoerpdc += IXGBE_READ_REG(hw, IXGBE_FCOERPDC);
		hw_stats->fcoeprc += IXGBE_READ_REG(hw, IXGBE_FCOEPRC);
		hw_stats->fcoeptc += IXGBE_READ_REG(hw, IXGBE_FCOEPTC);
		hw_stats->fcoedwrc += IXGBE_READ_REG(hw, IXGBE_FCOEDWRC);
		hw_stats->fcoedwtc += IXGBE_READ_REG(hw, IXGBE_FCOEDWTC);
	}

	if (stats == NULL)
		return;

	/* Fill out the rte_eth_stats statistics structure */
	stats->ipackets = total_qprc;
	stats->ibytes = total_qbrc;
	stats->opackets = hw_stats->gptc;
	stats->obytes = hw_stats->gotc;
	stats->imcasts = hw_stats->mprc;

	for (i = 0; i < IXGBE_QUEUE_STAT_COUNTERS; i++) {
		stats->q_ipackets[i] = hw_stats->qprc[i];
		stats->q_opackets[i] = hw_stats->qptc[i];
		stats->q_ibytes[i] = hw_stats->qbrc[i];
		stats->q_obytes[i] = hw_stats->qbtc[i];
		stats->q_errors[i] = hw_stats->qprdc[i];
	}

	/* Rx Errors */
	stats->ierrors = total_missed_rx + hw_stats->crcerrs +
		hw_stats->rlec;

	stats->oerrors  = 0;

	/* XON/XOFF pause frames */
	stats->tx_pause_xon  = hw_stats->lxontxc;
	stats->rx_pause_xon  = hw_stats->lxonrxc;
	stats->tx_pause_xoff = hw_stats->lxofftxc;
	stats->rx_pause_xoff = hw_stats->lxoffrxc;

	/* Flow Director Stats registers */
	hw_stats->fdirmatch += IXGBE_READ_REG(hw, IXGBE_FDIRMATCH);
	hw_stats->fdirmiss += IXGBE_READ_REG(hw, IXGBE_FDIRMISS);
	stats->fdirmatch = hw_stats->fdirmatch;
	stats->fdirmiss = hw_stats->fdirmiss;
}

static void
ixgbe_dev_stats_reset(struct rte_eth_dev *dev)
{
	struct ixgbe_hw_stats *stats =
			IXGBE_DEV_PRIVATE_TO_STATS(dev->data->dev_private);

	/* HW registers are cleared on read */
	ixgbe_dev_stats_get(dev, NULL);

	/* Reset software totals */
	memset(stats, 0, sizeof(*stats));
}

static void
ixgbe_dev_info_get(struct rte_eth_dev *dev, struct rte_eth_dev_info *dev_info)
{
	struct ixgbe_hw *hw = IXGBE_DEV_PRIVATE_TO_HW(dev->data->dev_private);

	dev_info->max_rx_queues = (uint16_t)hw->mac.max_rx_queues;
	dev_info->max_tx_queues = (uint16_t)hw->mac.max_tx_queues;
	dev_info->nb_rx_fgs = 128;
	dev_info->min_rx_bufsize = 1024; /* cf BSIZEPACKET in SRRCTL register */
	dev_info->max_rx_pktlen = 15872; /* includes CRC, cf MAXFRS register */
	dev_info->max_mac_addrs = hw->mac.num_rar_entries;
	dev_info->max_hash_mac_addrs = IXGBE_VMDQ_NUM_UC_MAC;
	//dev_info->max_vfs = dev->pci_dev->max_vfs;
	//dumplicated para?
	if (hw->mac.type == ixgbe_mac_82598EB)
		dev_info->max_vmdq_pools = ETH_16_POOLS;
	else
		dev_info->max_vmdq_pools = ETH_64_POOLS;
	dev_info->rx_offload_capa =
		DEV_RX_OFFLOAD_VLAN_STRIP |
		DEV_RX_OFFLOAD_IPV4_CKSUM |
		DEV_RX_OFFLOAD_UDP_CKSUM  |
		DEV_RX_OFFLOAD_TCP_CKSUM;
	dev_info->tx_offload_capa =
		DEV_TX_OFFLOAD_VLAN_INSERT |
		DEV_TX_OFFLOAD_IPV4_CKSUM  |
		DEV_TX_OFFLOAD_UDP_CKSUM   |
		DEV_TX_OFFLOAD_TCP_CKSUM   |
		DEV_TX_OFFLOAD_SCTP_CKSUM;
}

/* return 0 means link status changed, -1 means not changed */
static int
ixgbe_dev_link_update(struct rte_eth_dev *dev, int wait_to_complete)
{
	struct ixgbe_hw *hw = IXGBE_DEV_PRIVATE_TO_HW(dev->data->dev_private);
	struct rte_eth_link link, old;
	ixgbe_link_speed link_speed;
	bool link_up;
	int diag;

	link.link_status = 0;
	link.link_speed = 0;
	link.link_duplex = 0;
	memset(&old, 0, sizeof(old));
	old = dev->data->dev_link;

	/* check if it needs to wait to complete, if lsc interrupt is enabled */
	if (wait_to_complete == 0 || dev->data->dev_conf.intr_conf.lsc != 0)
		diag = ixgbe_check_link(hw, &link_speed, &link_up, 0);
	else
		diag = ixgbe_check_link(hw, &link_speed, &link_up, 1);
	if (diag != 0) {
		link.link_speed = ETH_LINK_SPEED_100;
		link.link_duplex = ETH_LINK_HALF_DUPLEX;
		dev->data->dev_link = link;
		if (link.link_status == old.link_status)
			return -1;
		return 0;
	}

	if (link_up == 0) {
		dev->data->dev_link = link;
		if (link.link_status == old.link_status)
			return -1;
		return 0;
	}
	link.link_status = 1;
	link.link_duplex = ETH_LINK_FULL_DUPLEX;

	switch (link_speed) {
	default:
	case IXGBE_LINK_SPEED_UNKNOWN:
		link.link_duplex = ETH_LINK_HALF_DUPLEX;
		link.link_speed = ETH_LINK_SPEED_100;
		break;

	case IXGBE_LINK_SPEED_100_FULL:
		link.link_speed = ETH_LINK_SPEED_100;
		break;

	case IXGBE_LINK_SPEED_1GB_FULL:
		link.link_speed = ETH_LINK_SPEED_1000;
		break;

	case IXGBE_LINK_SPEED_10GB_FULL:
		link.link_speed = ETH_LINK_SPEED_10000;
		break;
	}

	dev->data->dev_link = link;

	if (link.link_status == old.link_status)
		return -1;

	return 0;
}

static void
ixgbe_dev_promiscuous_enable(struct rte_eth_dev *dev)
{
	struct ixgbe_hw *hw = IXGBE_DEV_PRIVATE_TO_HW(dev->data->dev_private);
	uint32_t fctrl;

	fctrl = IXGBE_READ_REG(hw, IXGBE_FCTRL);
	fctrl |= (IXGBE_FCTRL_UPE | IXGBE_FCTRL_MPE);
	IXGBE_WRITE_REG(hw, IXGBE_FCTRL, fctrl);
}

static void
ixgbe_dev_promiscuous_disable(struct rte_eth_dev *dev)
{
	struct ixgbe_hw *hw = IXGBE_DEV_PRIVATE_TO_HW(dev->data->dev_private);
	uint32_t fctrl;

	fctrl = IXGBE_READ_REG(hw, IXGBE_FCTRL);
	fctrl &= (~IXGBE_FCTRL_UPE);
	if (dev->data->all_multicast == 1)
		fctrl |= IXGBE_FCTRL_MPE;
	else
		fctrl &= (~IXGBE_FCTRL_MPE);
	IXGBE_WRITE_REG(hw, IXGBE_FCTRL, fctrl);
}

static void
ixgbe_dev_allmulticast_enable(struct rte_eth_dev *dev)
{
	struct ixgbe_hw *hw = IXGBE_DEV_PRIVATE_TO_HW(dev->data->dev_private);
	uint32_t fctrl;

	fctrl = IXGBE_READ_REG(hw, IXGBE_FCTRL);
	fctrl |= IXGBE_FCTRL_MPE;
	IXGBE_WRITE_REG(hw, IXGBE_FCTRL, fctrl);
}

static void
ixgbe_dev_allmulticast_disable(struct rte_eth_dev *dev)
{
	struct ixgbe_hw *hw = IXGBE_DEV_PRIVATE_TO_HW(dev->data->dev_private);
	uint32_t fctrl;

	if (dev->data->promiscuous == 1)
		return; /* must remain in all_multicast mode */

	fctrl = IXGBE_READ_REG(hw, IXGBE_FCTRL);
	fctrl &= (~IXGBE_FCTRL_MPE);
	IXGBE_WRITE_REG(hw, IXGBE_FCTRL, fctrl);
}


/**
 * It clears the interrupt causes and enables the interrupt.
 * It will be called once only during nic initialized.
 *
 * @param dev
 *  Pointer to struct rte_eth_dev.
 *
 * @return
 *  - On success, zero.
 *  - On failure, a negative value.
 */
static int
ixgbe_dev_lsc_interrupt_setup(struct rte_eth_dev *dev)
{
	struct ixgbe_interrupt *intr =
		IXGBE_DEV_PRIVATE_TO_INTR(dev->data->dev_private);

	ixgbe_dev_link_status_print(dev);
	intr->mask |= IXGBE_EICR_LSC;

	return 0;
}

/**
 * It gets and then prints the link status.
 *
 * @param dev
 *  Pointer to struct rte_eth_dev.
 *
 * @return
 *  - On success, zero.
 *  - On failure, a negative value.
 */
static void
ixgbe_dev_link_status_print(struct rte_eth_dev *dev)
{
	struct rte_eth_link link;

	memset(&link, 0, sizeof(link));
	link = dev->data->dev_link;
	if (link.link_status) {
		pr_info("ixgbe: Port %d: Link Up - speed %u Mbps - %s",
			 (int)(dev->data->port_id), (unsigned)link.link_speed,
			 link.link_duplex == ETH_LINK_FULL_DUPLEX ?
					"full-duplex" : "half-duplex");
	} else {
		pr_info("ixgbe: Port %d: Link Down", (int)(dev->data->port_id));
	}
}

static int
ixgbe_dev_led_on(struct rte_eth_dev *dev)
{
	struct ixgbe_hw *hw;

	hw = IXGBE_DEV_PRIVATE_TO_HW(dev->data->dev_private);
	return (ixgbe_led_on(hw, 0) == IXGBE_SUCCESS ? 0 : -ENOTSUP);
}

static int
ixgbe_dev_led_off(struct rte_eth_dev *dev)
{
	struct ixgbe_hw *hw;

	hw = IXGBE_DEV_PRIVATE_TO_HW(dev->data->dev_private);
	return (ixgbe_led_off(hw, 0) == IXGBE_SUCCESS ? 0 : -ENOTSUP);
}

static int
ixgbe_flow_ctrl_set(struct rte_eth_dev *dev, struct rte_eth_fc_conf *fc_conf)
{
	struct ixgbe_hw *hw;
	int err;
	uint32_t rx_buf_size;
	uint32_t max_high_water;
	enum ixgbe_fc_mode rte_fcmode_2_ixgbe_fcmode[] = {
		ixgbe_fc_none,
		ixgbe_fc_rx_pause,
		ixgbe_fc_tx_pause,
		ixgbe_fc_full
	};

	hw = IXGBE_DEV_PRIVATE_TO_HW(dev->data->dev_private);
	rx_buf_size = IXGBE_READ_REG(hw, IXGBE_RXPBSIZE(0));
	printk(KERN_DEBUG"ixgbe: Rx packet buffer size = 0x%x \n", rx_buf_size);

	/*
	 * At least reserve one Ethernet frame for watermark
	 * high_water/low_water in kilo bytes for ixgbe
	 */
	max_high_water = (rx_buf_size - ETH_MAX_LEN) >> IXGBE_RXPBSIZE_SHIFT;
	if ((fc_conf->high_water > max_high_water) ||
		(fc_conf->high_water < fc_conf->low_water)) {
		pr_info("ixgbe: Invalid high/low water setup value in KB\n");
		pr_info("ixgbe: High_water must <=  0x%x\n", max_high_water);
		return (-EINVAL);
	}

	hw->fc.requested_mode = rte_fcmode_2_ixgbe_fcmode[fc_conf->mode];
	hw->fc.pause_time     = fc_conf->pause_time;
	hw->fc.high_water[0]  = fc_conf->high_water;
	hw->fc.low_water[0]   = fc_conf->low_water;
	hw->fc.send_xon       = fc_conf->send_xon;

	err = ixgbe_fc_enable(hw);
	/* Not negotiated is not an error case */
	if ((err == IXGBE_SUCCESS) || (err == IXGBE_ERR_FC_NOT_NEGOTIATED)) {
		return 0;
	}

	printk(KERN_ERR"ixgbe: ixgbe_fc_enable = 0x%x \n", err);
	return -EIO;
}

/**
 *  ixgbe_pfc_enable_generic - Enable flow control
 *  @hw: pointer to hardware structure
 *  @tc_num: traffic class number
 *  Enable flow control according to the current settings.
 */
static int
ixgbe_dcb_pfc_enable_generic(struct ixgbe_hw *hw,uint8_t tc_num)
{
	int ret_val = 0;
	uint32_t mflcn_reg, fccfg_reg;
	uint32_t reg;
	uint32_t fcrtl, fcrth;
	uint8_t i;
	uint8_t nb_rx_en;

	/* Validate the water mark configuration */
	if (!hw->fc.pause_time) {
		ret_val = IXGBE_ERR_INVALID_LINK_SETTINGS;
		goto out;
	}

	/* Low water mark of zero causes XOFF floods */
	if (hw->fc.current_mode & ixgbe_fc_tx_pause) {
		 /* High/Low water can not be 0 */
		if( (!hw->fc.high_water[tc_num])|| (!hw->fc.low_water[tc_num])) {
			printk(KERN_ERR"ixgbe: Invalid water mark configuration\n");
			ret_val = IXGBE_ERR_INVALID_LINK_SETTINGS;
			goto out;
		}

		if(hw->fc.low_water[tc_num] >= hw->fc.high_water[tc_num]) {
			printk(KERN_ERR"ixgbe: Invalid water mark configuration\n");
			ret_val = IXGBE_ERR_INVALID_LINK_SETTINGS;
			goto out;
		}
	}
	/* Negotiate the fc mode to use */
	ixgbe_fc_autoneg(hw);

	/* Disable any previous flow control settings */
	mflcn_reg = IXGBE_READ_REG(hw, IXGBE_MFLCN);
	mflcn_reg &= ~(IXGBE_MFLCN_RPFCE_SHIFT | IXGBE_MFLCN_RFCE|IXGBE_MFLCN_RPFCE);

	fccfg_reg = IXGBE_READ_REG(hw, IXGBE_FCCFG);
	fccfg_reg &= ~(IXGBE_FCCFG_TFCE_802_3X | IXGBE_FCCFG_TFCE_PRIORITY);

	switch (hw->fc.current_mode) {
	case ixgbe_fc_none:
		/*
		 * If the count of enabled RX Priority Flow control >1,
		 * and the TX pause can not be disabled
		 */
		nb_rx_en = 0;
		for (i =0; i < MAX_TRAFFIC_CLASS; i++) {
			reg = IXGBE_READ_REG(hw, IXGBE_FCRTH_82599(i));
			if (reg & IXGBE_FCRTH_FCEN)
				nb_rx_en++;
		}
		if (nb_rx_en > 1)
			fccfg_reg |=IXGBE_FCCFG_TFCE_PRIORITY;
		break;
	case ixgbe_fc_rx_pause:
		/*
		 * Rx Flow control is enabled and Tx Flow control is
		 * disabled by software override. Since there really
		 * isn't a way to advertise that we are capable of RX
		 * Pause ONLY, we will advertise that we support both
		 * symmetric and asymmetric Rx PAUSE.  Later, we will
		 * disable the adapter's ability to send PAUSE frames.
		 */
		mflcn_reg |= IXGBE_MFLCN_RPFCE;
		/*
		 * If the count of enabled RX Priority Flow control >1,
		 * and the TX pause can not be disabled
		 */
		nb_rx_en = 0;
		for (i =0; i < MAX_TRAFFIC_CLASS; i++) {
			reg = IXGBE_READ_REG(hw, IXGBE_FCRTH_82599(i));
			if (reg & IXGBE_FCRTH_FCEN)
				nb_rx_en++;
		}
		if (nb_rx_en > 1)
			fccfg_reg |=IXGBE_FCCFG_TFCE_PRIORITY;
		break;
	case ixgbe_fc_tx_pause:
		/*
		 * Tx Flow control is enabled, and Rx Flow control is
		 * disabled by software override.
		 */
		fccfg_reg |=IXGBE_FCCFG_TFCE_PRIORITY;
		break;
	case ixgbe_fc_full:
		/* Flow control (both Rx and Tx) is enabled by SW override. */
		mflcn_reg |= IXGBE_MFLCN_RPFCE;
		fccfg_reg |= IXGBE_FCCFG_TFCE_PRIORITY;
		break;
	default:
		printk(KERN_DEBUG "Flow control param set incorrectly\n");
		ret_val = IXGBE_ERR_CONFIG;
		goto out;
		break;
	}

	/* Set 802.3x based flow control settings. */
	mflcn_reg |= IXGBE_MFLCN_DPF;
	IXGBE_WRITE_REG(hw, IXGBE_MFLCN, mflcn_reg);
	IXGBE_WRITE_REG(hw, IXGBE_FCCFG, fccfg_reg);

	/* Set up and enable Rx high/low water mark thresholds, enable XON. */
	if ((hw->fc.current_mode & ixgbe_fc_tx_pause) &&
		hw->fc.high_water[tc_num]) {
		fcrtl = (hw->fc.low_water[tc_num] << 10) | IXGBE_FCRTL_XONE;
		IXGBE_WRITE_REG(hw, IXGBE_FCRTL_82599(tc_num), fcrtl);
		fcrth = (hw->fc.high_water[tc_num] << 10) | IXGBE_FCRTH_FCEN;
	} else {
		IXGBE_WRITE_REG(hw, IXGBE_FCRTL_82599(tc_num), 0);
		/*
		 * In order to prevent Tx hangs when the internal Tx
		 * switch is enabled we must set the high water mark
		 * to the maximum FCRTH value.  This allows the Tx
		 * switch to function even under heavy Rx workloads.
		 */
		fcrth = IXGBE_READ_REG(hw, IXGBE_RXPBSIZE(tc_num)) - 32;
	}
	IXGBE_WRITE_REG(hw, IXGBE_FCRTH_82599(tc_num), fcrth);

	/* Configure pause time (2 TCs per register) */
	reg = hw->fc.pause_time * 0x00010001;
	for (i = 0; i < (MAX_TRAFFIC_CLASS / 2); i++)
		IXGBE_WRITE_REG(hw, IXGBE_FCTTV(i), reg);

	/* Configure flow control refresh threshold value */
	IXGBE_WRITE_REG(hw, IXGBE_FCRTV, hw->fc.pause_time / 2);

out:
	return ret_val;
}


static int
ixgbe_dcb_pfc_enable(struct rte_eth_dev *dev,uint8_t tc_num)
{
	struct ixgbe_hw *hw = IXGBE_DEV_PRIVATE_TO_HW(dev->data->dev_private);
	int32_t ret_val = IXGBE_NOT_IMPLEMENTED;

	if(hw->mac.type != ixgbe_mac_82598EB) {
		ret_val = ixgbe_dcb_pfc_enable_generic(hw,tc_num);
	}
	return ret_val;
}

static int
ixgbe_priority_flow_ctrl_set(struct rte_eth_dev *dev, struct rte_eth_pfc_conf *pfc_conf)
{
	int err;
	uint32_t rx_buf_size;
	uint32_t max_high_water;
	uint8_t tc_num;
	uint8_t  map[IXGBE_DCB_MAX_USER_PRIORITY] = { 0 };
	struct ixgbe_hw *hw =
                IXGBE_DEV_PRIVATE_TO_HW(dev->data->dev_private);
	struct ixgbe_dcb_config *dcb_config =
                IXGBE_DEV_PRIVATE_TO_DCB_CFG(dev->data->dev_private);

	enum ixgbe_fc_mode rte_fcmode_2_ixgbe_fcmode[] = {
		ixgbe_fc_none,
		ixgbe_fc_rx_pause,
		ixgbe_fc_tx_pause,
		ixgbe_fc_full
	};

	ixgbe_dcb_unpack_map_cee(dcb_config, IXGBE_DCB_RX_CONFIG, map);
	tc_num = map[pfc_conf->priority];
	rx_buf_size = IXGBE_READ_REG(hw, IXGBE_RXPBSIZE(tc_num));
	printk(KERN_DEBUG"ixgbe: Rx packet buffer size = 0x%x \n", rx_buf_size);
	/*
	 * At least reserve one Ethernet frame for watermark
	 * high_water/low_water in kilo bytes for ixgbe
	 */
	max_high_water = (rx_buf_size - ETH_MAX_LEN) >> IXGBE_RXPBSIZE_SHIFT;
	if ((pfc_conf->fc.high_water > max_high_water) ||
		(pfc_conf->fc.high_water <= pfc_conf->fc.low_water)) {
		printk(KERN_ERR"ixgbe: Invalid high/low water setup value in KB\n");
		printk(KERN_ERR"ixgbe: High_water must <=  0x%x\n", max_high_water);
		return (-EINVAL);
	}

	hw->fc.requested_mode = rte_fcmode_2_ixgbe_fcmode[pfc_conf->fc.mode];
	hw->fc.pause_time = pfc_conf->fc.pause_time;
	hw->fc.send_xon = pfc_conf->fc.send_xon;
	hw->fc.low_water[tc_num] =  pfc_conf->fc.low_water;
	hw->fc.high_water[tc_num] = pfc_conf->fc.high_water;

	err = ixgbe_dcb_pfc_enable(dev,tc_num);

	/* Not negotiated is not an error case */
	if ((err == IXGBE_SUCCESS) || (err == IXGBE_ERR_FC_NOT_NEGOTIATED))
		return 0;

	printk(KERN_ERR "ixgbe: ixgbe_dcb_pfc_enable = 0x%x \n", err);
	return -EIO;
}

static int
ixgbe_dev_rss_reta_update(struct rte_eth_dev *dev,
				struct rte_eth_rss_reta *reta_conf)
{
	uint8_t i,j,mask;
	uint32_t reta;
	struct ixgbe_hw *hw =
			IXGBE_DEV_PRIVATE_TO_HW(dev->data->dev_private);

	spin_lock(&ixgbe_dev_lock);

	/*
	* Update Redirection Table RETA[n],n=0...31,The redirection table has
	* 128-entries in 32 registers
	 */
	for(i = 0; i < ETH_RSS_RETA_NUM_ENTRIES; i += 4) {
		if (i < ETH_RSS_RETA_NUM_ENTRIES/2)
			mask = (uint8_t)((reta_conf->mask_lo >> i) & 0xF);
		else
			mask = (uint8_t)((reta_conf->mask_hi >>
				(i - ETH_RSS_RETA_NUM_ENTRIES/2)) & 0xF);
		if (mask != 0) {
			reta = 0;
			if (mask != 0xF)
				reta = IXGBE_READ_REG(hw,IXGBE_RETA(i >> 2));

			for (j = 0; j < 4; j++) {
				if (mask & (0x1 << j)) {
					if (mask != 0xF)
						reta &= ~(0xFF << 8 * j);
					reta |= reta_conf->reta[i + j] << 8*j;
				}
			}
			IXGBE_WRITE_REG(hw, IXGBE_RETA(i >> 2),reta);
		}
	}

	spin_unlock(&ixgbe_dev_lock);

	return 0;
}


static int
ixgbe_dev_rss_reta_query(struct rte_eth_dev *dev,
				struct rte_eth_rss_reta *reta_conf)
{
	uint8_t i,j,mask;
	uint32_t reta;
	struct ixgbe_hw *hw =
			IXGBE_DEV_PRIVATE_TO_HW(dev->data->dev_private);

	/*
	 * Read Redirection Table RETA[n],n=0...31,The redirection table has
	 * 128-entries in 32 registers
	 */
	for(i = 0; i < ETH_RSS_RETA_NUM_ENTRIES; i += 4) {
		if (i < ETH_RSS_RETA_NUM_ENTRIES/2)
			mask = (uint8_t)((reta_conf->mask_lo >> i) & 0xF);
		else
			mask = (uint8_t)((reta_conf->mask_hi >>
				(i - ETH_RSS_RETA_NUM_ENTRIES/2)) & 0xF);

		if (mask != 0) {
			reta = IXGBE_READ_REG(hw,IXGBE_RETA(i >> 2));
			for (j = 0; j < 4; j++) {
				if (mask & (0x1 << j))
					reta_conf->reta[i + j] =
						(uint8_t)((reta >> 8 * j) & 0xFF);
			}
		}
	}

	return 0;
}



static void
ixgbe_add_rar(struct rte_eth_dev *dev, struct eth_addr *mac_addr,
				uint32_t index, uint32_t pool)
{
	struct ixgbe_hw *hw = IXGBE_DEV_PRIVATE_TO_HW(dev->data->dev_private);
	uint32_t enable_addr = 1;

	ixgbe_set_rar(hw, index, mac_addr->addr, pool, enable_addr);
}

static void
ixgbe_remove_rar(struct rte_eth_dev *dev, uint32_t index)
{
	struct ixgbe_hw *hw = IXGBE_DEV_PRIVATE_TO_HW(dev->data->dev_private);

	ixgbe_clear_rar(hw, index);
}

static int
ixgbe_vmdq_mode_check(struct ixgbe_hw *hw)
{
	uint32_t reg_val;

	/* we only need to do this if VMDq is enabled */
	reg_val = IXGBE_READ_REG(hw, IXGBE_VT_CTL);
	if (!(reg_val & IXGBE_VT_CTL_VT_ENABLE)) {
		printk(KERN_ERR"ixgbe: VMDq must be enabled for this setting\n");
		return (-1);
	}

	return 0;
}

static uint32_t
ixgbe_uta_vector(struct ixgbe_hw *hw, struct eth_addr* uc_addr)
{
	uint32_t vector = 0;
	switch (hw->mac.mc_filter_type) {
	case 0:   /* use bits [47:36] of the address */
		vector = ((uc_addr->addr[4] >> 4) |
			(((uint16_t)uc_addr->addr[5]) << 4));
		break;
	case 1:   /* use bits [46:35] of the address */
		vector = ((uc_addr->addr[4] >> 3) |
			(((uint16_t)uc_addr->addr[5]) << 5));
		break;
	case 2:   /* use bits [45:34] of the address */
		vector = ((uc_addr->addr[4] >> 2) |
			(((uint16_t)uc_addr->addr[5]) << 6));
		break;
	case 3:   /* use bits [43:32] of the address */
		vector = ((uc_addr->addr[4]) |
			(((uint16_t)uc_addr->addr[5]) << 8));
		break;
	default:  /* Invalid mc_filter_type */
		break;
	}

	/* vector can only be 12-bits or boundary will be exceeded */
	vector &= 0xFFF;
	return vector;
}

static int
ixgbe_uc_hash_table_set(struct rte_eth_dev *dev,struct eth_addr* mac_addr,
			       uint8_t on)
{
	uint32_t vector;
	uint32_t uta_idx;
	uint32_t reg_val;
	uint32_t uta_shift;
	uint32_t rc;
	const uint32_t ixgbe_uta_idx_mask = 0x7F;
	const uint32_t ixgbe_uta_bit_shift = 5;
	const uint32_t ixgbe_uta_bit_mask = (0x1 << ixgbe_uta_bit_shift) - 1;
	const uint32_t bit1 = 0x1;

	struct ixgbe_hw *hw =
		IXGBE_DEV_PRIVATE_TO_HW(dev->data->dev_private);
	struct ixgbe_uta_info *uta_info =
		IXGBE_DEV_PRIVATE_TO_UTA(dev->data->dev_private);

	/* The UTA table only exists on 82599 hardware and newer */
	if (hw->mac.type < ixgbe_mac_82599EB)
		return (-ENOTSUP);

	vector = ixgbe_uta_vector(hw, mac_addr);
	uta_idx = (vector >> ixgbe_uta_bit_shift) & ixgbe_uta_idx_mask;
	uta_shift = vector & ixgbe_uta_bit_mask;

	rc = ((uta_info->uta_shadow[uta_idx] >> uta_shift & bit1) != 0);
	if(rc == on)
		return 0;

	reg_val = IXGBE_READ_REG(hw, IXGBE_UTA(uta_idx));
	if (on) {
		uta_info->uta_in_use++;
		reg_val |= (bit1 << uta_shift);
		uta_info->uta_shadow[uta_idx] |= (bit1 << uta_shift);
	} else {
		uta_info->uta_in_use--;
		reg_val &= ~(bit1 << uta_shift);
		uta_info->uta_shadow[uta_idx] &= ~(bit1 << uta_shift);
	}

	IXGBE_WRITE_REG(hw, IXGBE_UTA(uta_idx), reg_val);

	if (uta_info->uta_in_use > 0)
		IXGBE_WRITE_REG(hw, IXGBE_MCSTCTRL,
				IXGBE_MCSTCTRL_MFE | hw->mac.mc_filter_type);
	else
		IXGBE_WRITE_REG(hw, IXGBE_MCSTCTRL,hw->mac.mc_filter_type);

	return 0;
}


static int
ixgbe_uc_all_hash_table_set(struct rte_eth_dev *dev, uint8_t on)
{
	int i;
	struct ixgbe_hw *hw =
		IXGBE_DEV_PRIVATE_TO_HW(dev->data->dev_private);
	struct ixgbe_uta_info *uta_info =
		IXGBE_DEV_PRIVATE_TO_UTA(dev->data->dev_private);

	/* The UTA table only exists on 82599 hardware and newer */
	if (hw->mac.type < ixgbe_mac_82599EB)
		return (-ENOTSUP);

	if(on) {
		for (i = 0; i < ETH_VMDQ_NUM_UC_HASH_ARRAY; i++) {
			uta_info->uta_shadow[i] = ~0;
			IXGBE_WRITE_REG(hw, IXGBE_UTA(i), ~0);
		}
	} else {
		for (i = 0; i < ETH_VMDQ_NUM_UC_HASH_ARRAY; i++) {
			uta_info->uta_shadow[i] = 0;
			IXGBE_WRITE_REG(hw, IXGBE_UTA(i), 0);
		}
	}
	return 0;
}

static int
ixgbe_set_pool_rx_mode(struct rte_eth_dev *dev, uint16_t pool,
			       uint16_t rx_mask, uint8_t on)
{
	int val = 0;

	struct ixgbe_hw *hw =
		IXGBE_DEV_PRIVATE_TO_HW(dev->data->dev_private);
	uint32_t vmolr = IXGBE_READ_REG(hw, IXGBE_VMOLR(pool));

	if (hw->mac.type == ixgbe_mac_82598EB) {
		printk(KERN_ERR "ixgbe: setting VF receive mode set should be done"
			" on 82599 hardware and newer\n");
		return (-ENOTSUP);
	}
	if (ixgbe_vmdq_mode_check(hw) < 0)
		return (-ENOTSUP);

	if (rx_mask & ETH_VMDQ_ACCEPT_UNTAG )
		val |= IXGBE_VMOLR_AUPE;
	if (rx_mask & ETH_VMDQ_ACCEPT_HASH_MC )
		val |= IXGBE_VMOLR_ROMPE;
	if (rx_mask & ETH_VMDQ_ACCEPT_HASH_UC)
		val |= IXGBE_VMOLR_ROPE;
	if (rx_mask & ETH_VMDQ_ACCEPT_BROADCAST)
		val |= IXGBE_VMOLR_BAM;
	if (rx_mask & ETH_VMDQ_ACCEPT_MULTICAST)
		val |= IXGBE_VMOLR_MPE;

	if (on)
		vmolr |= val;
	else
		vmolr &= ~val;

	IXGBE_WRITE_REG(hw, IXGBE_VMOLR(pool), vmolr);

	return 0;
}

static int
ixgbe_set_pool_rx(struct rte_eth_dev *dev, uint16_t pool, uint8_t on)
{
	uint32_t reg,addr;
	uint32_t val;
	const uint8_t bit1 = 0x1;

	struct ixgbe_hw *hw =
		IXGBE_DEV_PRIVATE_TO_HW(dev->data->dev_private);

	if (ixgbe_vmdq_mode_check(hw) < 0)
		return (-ENOTSUP);

	addr = IXGBE_VFRE(pool >= ETH_64_POOLS/2);
	reg = IXGBE_READ_REG(hw, addr);
	val = bit1 << pool;

	if (on)
		reg |= val;
	else
		reg &= ~val;

	IXGBE_WRITE_REG(hw, addr,reg);

	return 0;
}

static int
ixgbe_set_pool_tx(struct rte_eth_dev *dev, uint16_t pool, uint8_t on)
{
	uint32_t reg,addr;
	uint32_t val;
	const uint8_t bit1 = 0x1;

	struct ixgbe_hw *hw =
		IXGBE_DEV_PRIVATE_TO_HW(dev->data->dev_private);

	if (ixgbe_vmdq_mode_check(hw) < 0)
		return (-ENOTSUP);

	addr = IXGBE_VFTE(pool >= ETH_64_POOLS/2);
	reg = IXGBE_READ_REG(hw, addr);
	val = bit1 << pool;

	if (on)
		reg |= val;
	else
		reg &= ~val;

	IXGBE_WRITE_REG(hw, addr,reg);

	return 0;
}

static int
ixgbe_set_pool_vlan_filter(struct rte_eth_dev *dev, uint16_t vlan,
			uint64_t pool_mask, uint8_t vlan_on)
{
	int ret = 0;
	uint16_t pool_idx;
	struct ixgbe_hw *hw =
		IXGBE_DEV_PRIVATE_TO_HW(dev->data->dev_private);

	if (ixgbe_vmdq_mode_check(hw) < 0)
		return (-ENOTSUP);
	for (pool_idx = 0; pool_idx < ETH_64_POOLS; pool_idx++) {
		if (pool_mask & ((uint64_t)(1ULL << pool_idx)))
			ret = hw->mac.ops.set_vfta(hw,vlan,pool_idx,vlan_on);
			if (ret < 0)
				return ret;
	}

	return ret;
}

static int
ixgbe_mirror_rule_set(struct rte_eth_dev *dev,
			struct rte_eth_vmdq_mirror_conf *mirror_conf,
			uint8_t rule_id, uint8_t on)
{
	uint32_t mr_ctl,vlvf;
	uint32_t mp_lsb = 0;
	uint32_t mv_msb = 0;
	uint32_t mv_lsb = 0;
	uint32_t mp_msb = 0;
	uint8_t i = 0;
	int reg_index = 0;
	uint64_t vlan_mask = 0;

	const uint8_t pool_mask_offset = 32;
	const uint8_t vlan_mask_offset = 32;
	const uint8_t dst_pool_offset = 8;
	const uint8_t rule_mr_offset  = 4;
	const uint8_t mirror_rule_mask= 0x0F;

	struct ixgbe_mirror_info *mr_info =
			(IXGBE_DEV_PRIVATE_TO_PFDATA(dev->data->dev_private));
	struct ixgbe_hw *hw =
		IXGBE_DEV_PRIVATE_TO_HW(dev->data->dev_private);

	if (ixgbe_vmdq_mode_check(hw) < 0)
		return (-ENOTSUP);

	/* Check if vlan mask is valid */
	if ((mirror_conf->rule_type_mask & ETH_VMDQ_VLAN_MIRROR) && (on)) {
		if (mirror_conf->vlan.vlan_mask == 0)
			return (-EINVAL);
	}

	/* Check if vlan id is valid and find conresponding VLAN ID index in VLVF */
	if (mirror_conf->rule_type_mask & ETH_VMDQ_VLAN_MIRROR) {
		for (i = 0;i < IXGBE_VLVF_ENTRIES; i++) {
			if (mirror_conf->vlan.vlan_mask & (1ULL << i)) {
				/* search vlan id related pool vlan filter index */
				reg_index = ixgbe_find_vlvf_slot(hw,
						mirror_conf->vlan.vlan_id[i]);
				if(reg_index < 0)
					return (-EINVAL);
				vlvf = IXGBE_READ_REG(hw, IXGBE_VLVF(reg_index));
				if ((vlvf & IXGBE_VLVF_VIEN) &&
					((vlvf & IXGBE_VLVF_VLANID_MASK)
						== mirror_conf->vlan.vlan_id[i]))
					vlan_mask |= (1ULL << reg_index);
				else
					return (-EINVAL);
			}
		}

		if (on) {
			mv_lsb = vlan_mask & 0xFFFFFFFF;
			mv_msb = vlan_mask >> vlan_mask_offset;

			mr_info->mr_conf[rule_id].vlan.vlan_mask =
						mirror_conf->vlan.vlan_mask;
			for(i = 0 ;i < ETH_VMDQ_MAX_VLAN_FILTERS; i++) {
				if(mirror_conf->vlan.vlan_mask & (1ULL << i))
					mr_info->mr_conf[rule_id].vlan.vlan_id[i] =
						mirror_conf->vlan.vlan_id[i];
			}
		} else {
			mv_lsb = 0;
			mv_msb = 0;
			mr_info->mr_conf[rule_id].vlan.vlan_mask = 0;
			for(i = 0 ;i < ETH_VMDQ_MAX_VLAN_FILTERS; i++)
				mr_info->mr_conf[rule_id].vlan.vlan_id[i] = 0;
		}
	}

	/*
	 * if enable pool mirror, write related pool mask register,if disable
	 * pool mirror, clear PFMRVM register
	 */
	if (mirror_conf->rule_type_mask & ETH_VMDQ_POOL_MIRROR) {
		if (on) {
			mp_lsb = mirror_conf->pool_mask & 0xFFFFFFFF;
			mp_msb = mirror_conf->pool_mask >> pool_mask_offset;
			mr_info->mr_conf[rule_id].pool_mask =
					mirror_conf->pool_mask;

		} else {
			mp_lsb = 0;
			mp_msb = 0;
			mr_info->mr_conf[rule_id].pool_mask = 0;
		}
	}

	/* read  mirror control register and recalculate it */
	mr_ctl = IXGBE_READ_REG(hw,IXGBE_MRCTL(rule_id));

	if (on) {
		mr_ctl |= mirror_conf->rule_type_mask;
		mr_ctl &= mirror_rule_mask;
		mr_ctl |= mirror_conf->dst_pool << dst_pool_offset;
	} else
		mr_ctl &= ~(mirror_conf->rule_type_mask & mirror_rule_mask);

	mr_info->mr_conf[rule_id].rule_type_mask = (uint8_t)(mr_ctl & mirror_rule_mask);
	mr_info->mr_conf[rule_id].dst_pool = mirror_conf->dst_pool;

	/* write mirrror control  register */
	IXGBE_WRITE_REG(hw, IXGBE_MRCTL(rule_id), mr_ctl);

        /* write pool mirrror control  register */
	if (mirror_conf->rule_type_mask & ETH_VMDQ_POOL_MIRROR) {
		IXGBE_WRITE_REG(hw, IXGBE_VMRVM(rule_id), mp_lsb);
		IXGBE_WRITE_REG(hw, IXGBE_VMRVM(rule_id + rule_mr_offset),
				mp_msb);
	}
	/* write VLAN mirrror control  register */
	if (mirror_conf->rule_type_mask & ETH_VMDQ_VLAN_MIRROR) {
		IXGBE_WRITE_REG(hw, IXGBE_VMRVLAN(rule_id), mv_lsb);
		IXGBE_WRITE_REG(hw, IXGBE_VMRVLAN(rule_id + rule_mr_offset),
				mv_msb);
	}

	return 0;
}

static int
ixgbe_mirror_rule_reset(struct rte_eth_dev *dev, uint8_t rule_id)
{
	int mr_ctl = 0;
	uint32_t lsb_val = 0;
	uint32_t msb_val = 0;
	const uint8_t rule_mr_offset = 4;

	struct ixgbe_hw *hw =
		IXGBE_DEV_PRIVATE_TO_HW(dev->data->dev_private);
	struct ixgbe_mirror_info *mr_info =
		(IXGBE_DEV_PRIVATE_TO_PFDATA(dev->data->dev_private));

	if (ixgbe_vmdq_mode_check(hw) < 0)
		return (-ENOTSUP);

	memset(&mr_info->mr_conf[rule_id], 0,
		sizeof(struct rte_eth_vmdq_mirror_conf));

	/* clear PFVMCTL register */
	IXGBE_WRITE_REG(hw, IXGBE_MRCTL(rule_id), mr_ctl);

	/* clear pool mask register */
	IXGBE_WRITE_REG(hw, IXGBE_VMRVM(rule_id), lsb_val);
	IXGBE_WRITE_REG(hw, IXGBE_VMRVM(rule_id + rule_mr_offset), msb_val);

	/* clear vlan mask register */
	IXGBE_WRITE_REG(hw, IXGBE_VMRVLAN(rule_id), lsb_val);
	IXGBE_WRITE_REG(hw, IXGBE_VMRVLAN(rule_id + rule_mr_offset), msb_val);

	return 0;
}

/**
 * ixgbe_probe - Device Initialization Routine
 * @pdev: PCI device information struct
 * @ent: entry in ixgbe_pci_tbl
 *
 * Returns 0 on success, negative on failure
 *
 * ixgbe_probe initializes an adapter identified by a pci_dev structure.
 * The OS initialization, configuring of the adapter private structure,
 * and a hardware reset occur.
 **/
static int ixgbe_probe(struct pci_dev *pdev, const struct pci_device_id *ent)
{
	struct rte_eth_dev *dev;
	struct ixgbe_adapter *adapter = NULL;
	struct ixgbe_hw *hw;
	const struct ixgbe_info *ii = ixgbe_info_tbl[ent->driver_data];
	int i, err, pci_using_dac, expected_gts;
	unsigned int indices = MAX_TX_QUEUES;
	u8 part_str[IXGBE_PBANUM_LENGTH];
	bool disable_dev = false;
#ifdef IXGBE_FCOE
	u16 device_caps;
#endif
	u32 eec;

	/* Catch broken hardware that put the wrong VF device ID in
	 * the PCIe SR-IOV capability.
	 */
	if (pdev->is_virtfn) {
		WARN(1, KERN_ERR "%s (%hx:%hx) should not be a VF!\n",
		     pci_name(pdev), pdev->vendor, pdev->device);
		return -EINVAL;
	}

	err = pci_enable_device_mem(pdev);
	if (err)
		return err;

	if (!dma_set_mask_and_coherent(&pdev->dev, DMA_BIT_MASK(64))) {
		pci_using_dac = 1;
	} else {
		err = dma_set_mask_and_coherent(&pdev->dev, DMA_BIT_MASK(32));
		if (err) {
			dev_err(&pdev->dev,
				"No usable DMA configuration, aborting\n");
			goto err_dma;
		}
		pci_using_dac = 0;
	}

	err = pci_request_mem_regions(pdev, ixgbe_driver_name);
	if (err) {
		dev_err(&pdev->dev,
			"pci_request_selected_regions failed 0x%x\n", err);
		goto err_pci_reg;
	}

	pci_enable_pcie_error_reporting(pdev);

	pci_set_master(pdev);
	pci_save_state(pdev);

	if (ii->mac == ixgbe_mac_82598EB) {
#ifdef CONFIG_IXGBE_DCB
		/* 8 TC w/ 4 queues per TC */
		indices = 4 * MAX_TRAFFIC_CLASS;
#else
		indices = IXGBE_MAX_RSS_INDICES;
#endif
	}

	dev = eth_dev_alloc(sizeof(struct ixgbe_adapter));
	if (!dev){
		err = -ENOMEM;
		goto err_alloc_etherdev;
	}

	hw = IXGBE_DEV_PRIVATE_TO_HW(dev->data->dev_private);

	dev->pci_dev = pdev;
	dev->dev_ops = &ixgbe_eth_dev_ops;
	hw->device_id = pdev->device;
	hw->vendor_id = pdev->vendor;
	hw->num_vfs = max_vfs;//TO BE FIXED

	hw->hw_addr = ioremap(pci_resource_start(pdev, 0),
			      pci_resource_len(pdev, 0));
	if (!hw->hw_addr) {
		err = -EIO;
		goto err_ioremap;
	}

// 	netdev->netdev_ops = &ixgbe_netdev_ops;
// 	ixgbe_set_ethtool_ops(netdev);
// 	netdev->watchdog_timeo = 5 * HZ;
// 	strlcpy(netdev->name, pci_name(pdev), sizeof(netdev->name));

// 	/* Setup hw api */
// 	hw->mac.ops   = *ii->mac_ops;
// 	hw->mac.type  = ii->mac;
// 	hw->mvals     = ii->mvals;
// 	if (ii->link_ops)
// 		hw->link.ops  = *ii->link_ops;

// 	/* EEPROM */
// 	hw->eeprom.ops = *ii->eeprom_ops;
// 	eec = IXGBE_READ_REG(hw, IXGBE_EEC(hw));
// 	if (ixgbe_removed(hw->hw_addr)) {
// 		err = -EIO;
// 		goto err_ioremap;
// 	}
// 	/* If EEPROM is valid (bit 8 = 1), use default otherwise use bit bang */
// 	if (!(eec & BIT(8)))
// 		hw->eeprom.ops.read = &ixgbe_read_eeprom_bit_bang_generic;

// 	/* PHY */
// 	hw->phy.ops = *ii->phy_ops;
// 	hw->phy.sfp_type = ixgbe_sfp_type_unknown;
// 	/* ixgbe_identify_phy_generic will set prtad and mmds properly */
// 	hw->phy.mdio.prtad = MDIO_PRTAD_NONE;
// 	hw->phy.mdio.mmds = 0;
// 	hw->phy.mdio.mode_support = MDIO_SUPPORTS_C45 | MDIO_EMULATE_C22;
// 	hw->phy.mdio.dev = netdev;
// 	hw->phy.mdio.mdio_read = ixgbe_mdio_read;
// 	hw->phy.mdio.mdio_write = ixgbe_mdio_write;

// 	/* setup the private structure */
// 	err = ixgbe_sw_init(adapter, ii);
// 	if (err)
// 		goto err_sw_init;

// 	/* Make sure the SWFW semaphore is in a valid state */
// 	if (hw->mac.ops.init_swfw_sync)
// 		hw->mac.ops.init_swfw_sync(hw);

// 	/* Make it possible the adapter to be woken up via WOL */
// 	switch (adapter->hw.mac.type) {
// 	case ixgbe_mac_82599EB:
// 	case ixgbe_mac_X540:
// 	case ixgbe_mac_X550:
// 	case ixgbe_mac_X550EM_x:
// 	case ixgbe_mac_x550em_a:
// 		IXGBE_WRITE_REG(&adapter->hw, IXGBE_WUS, ~0);
// 		break;
// 	default:
// 		break;
// 	}

// 	/*
// 	 * If there is a fan on this device and it has failed log the
// 	 * failure.
// 	 */
// 	if (adapter->flags & IXGBE_FLAG_FAN_FAIL_CAPABLE) {
// 		u32 esdp = IXGBE_READ_REG(hw, IXGBE_ESDP);
// 		if (esdp & IXGBE_ESDP_SDP1)
// 			e_crit(probe, "Fan has stopped, replace the adapter\n");
// 	}

// 	if (allow_unsupported_sfp)
// 		hw->allow_unsupported_sfp = allow_unsupported_sfp;

// 	/* reset_hw fills in the perm_addr as well */
// 	hw->phy.reset_if_overtemp = true;
// 	err = hw->mac.ops.reset_hw(hw);
// 	hw->phy.reset_if_overtemp = false;
// 	ixgbe_set_eee_capable(adapter);
// 	if (err == IXGBE_ERR_SFP_NOT_PRESENT) {
// 		err = 0;
// 	} else if (err == IXGBE_ERR_SFP_NOT_SUPPORTED) {
// 		e_dev_err("failed to load because an unsupported SFP+ or QSFP module type was detected.\n");
// 		e_dev_err("Reload the driver after installing a supported module.\n");
// 		goto err_sw_init;
// 	} else if (err) {
// 		e_dev_err("HW Init failed: %d\n", err);
// 		goto err_sw_init;
// 	}

// #ifdef CONFIG_PCI_IOV
// 	/* SR-IOV not supported on the 82598 */
// 	if (adapter->hw.mac.type == ixgbe_mac_82598EB)
// 		goto skip_sriov;
// 	/* Mailbox */
// 	ixgbe_init_mbx_params_pf(hw);
// 	hw->mbx.ops = ii->mbx_ops;
// 	pci_sriov_set_totalvfs(pdev, IXGBE_MAX_VFS_DRV_LIMIT);
// 	ixgbe_enable_sriov(adapter, max_vfs);
// skip_sriov:

// #endif
// 	netdev->features = NETIF_F_SG |
// 			   NETIF_F_TSO |
// 			   NETIF_F_TSO6 |
// 			   NETIF_F_RXHASH |
// 			   NETIF_F_RXCSUM |
// 			   NETIF_F_HW_CSUM;

#define IXGBE_GSO_PARTIAL_FEATURES (NETIF_F_GSO_GRE | \
				    NETIF_F_GSO_GRE_CSUM | \
				    NETIF_F_GSO_IPXIP4 | \
				    NETIF_F_GSO_IPXIP6 | \
				    NETIF_F_GSO_UDP_TUNNEL | \
				    NETIF_F_GSO_UDP_TUNNEL_CSUM)

// 	netdev->gso_partial_features = IXGBE_GSO_PARTIAL_FEATURES;
// 	netdev->features |= NETIF_F_GSO_PARTIAL |
// 			    IXGBE_GSO_PARTIAL_FEATURES;

// 	if (hw->mac.type >= ixgbe_mac_82599EB)
// 		netdev->features |= NETIF_F_SCTP_CRC;

// 	/* copy netdev features into list of user selectable features */
// 	netdev->hw_features |= netdev->features |
// 			       NETIF_F_HW_VLAN_CTAG_FILTER |
// 			       NETIF_F_HW_VLAN_CTAG_RX |
// 			       NETIF_F_HW_VLAN_CTAG_TX |
// 			       NETIF_F_RXALL |
// 			       NETIF_F_HW_L2FW_DOFFLOAD;

// 	if (hw->mac.type >= ixgbe_mac_82599EB)
// 		netdev->hw_features |= NETIF_F_NTUPLE |
// 				       NETIF_F_HW_TC;

// 	if (pci_using_dac)
// 		netdev->features |= NETIF_F_HIGHDMA;

// 	netdev->vlan_features |= netdev->features | NETIF_F_TSO_MANGLEID;
// 	netdev->hw_enc_features |= netdev->vlan_features;
// 	netdev->mpls_features |= NETIF_F_SG |
// 				 NETIF_F_TSO |
// 				 NETIF_F_TSO6 |
// 				 NETIF_F_HW_CSUM;
// 	netdev->mpls_features |= IXGBE_GSO_PARTIAL_FEATURES;

// 	/* set this bit last since it cannot be part of vlan_features */
// 	netdev->features |= NETIF_F_HW_VLAN_CTAG_FILTER |
// 			    NETIF_F_HW_VLAN_CTAG_RX |
// 			    NETIF_F_HW_VLAN_CTAG_TX;

// 	netdev->priv_flags |= IFF_UNICAST_FLT;
// 	netdev->priv_flags |= IFF_SUPP_NOFCS;

// 	/* MTU range: 68 - 9710 */
// 	netdev->min_mtu = ETH_MIN_MTU;
// 	netdev->max_mtu = IXGBE_MAX_JUMBO_FRAME_SIZE - (ETH_HLEN + ETH_FCS_LEN);

// #ifdef CONFIG_IXGBE_DCB
// 	if (adapter->flags & IXGBE_FLAG_DCB_CAPABLE)
// 		netdev->dcbnl_ops = &ixgbe_dcbnl_ops;
// #endif

// #ifdef IXGBE_FCOE
// 	if (adapter->flags & IXGBE_FLAG_FCOE_CAPABLE) {
// 		unsigned int fcoe_l;

// 		if (hw->mac.ops.get_device_caps) {
// 			hw->mac.ops.get_device_caps(hw, &device_caps);
// 			if (device_caps & IXGBE_DEVICE_CAPS_FCOE_OFFLOADS)
// 				adapter->flags &= ~IXGBE_FLAG_FCOE_CAPABLE;
// 		}


// 		fcoe_l = min_t(int, IXGBE_FCRETA_SIZE, num_online_cpus());
// 		adapter->ring_feature[RING_F_FCOE].limit = fcoe_l;

// 		netdev->features |= NETIF_F_FSO |
// 				    NETIF_F_FCOE_CRC;

// 		netdev->vlan_features |= NETIF_F_FSO |
// 					 NETIF_F_FCOE_CRC |
// 					 NETIF_F_FCOE_MTU;
// 	}
// #endif /* IXGBE_FCOE */

// 	if (adapter->flags2 & IXGBE_FLAG2_RSC_CAPABLE)
// 		netdev->hw_features |= NETIF_F_LRO;
// 	if (adapter->flags2 & IXGBE_FLAG2_RSC_ENABLED)
// 		netdev->features |= NETIF_F_LRO;

// 	/* make sure the EEPROM is good */
// 	if (hw->eeprom.ops.validate_checksum(hw, NULL) < 0) {
// 		e_dev_err("The EEPROM Checksum Is Not Valid\n");
// 		err = -EIO;
// 		goto err_sw_init;
// 	}

// 	eth_platform_get_mac_address(&adapter->pdev->dev,
// 				     adapter->hw.mac.perm_addr);

// 	memcpy(netdev->dev_addr, hw->mac.perm_addr, netdev->addr_len);

// 	if (!is_valid_ether_addr(netdev->dev_addr)) {
// 		e_dev_err("invalid MAC address\n");
// 		err = -EIO;
// 		goto err_sw_init;
// 	}

// 	/* Set hw->mac.addr to permanent MAC address */
// 	ether_addr_copy(hw->mac.addr, hw->mac.perm_addr);
// 	ixgbe_mac_set_default_filter(adapter);

// 	setup_timer(&adapter->service_timer, &ixgbe_service_timer,
// 		    (unsigned long) adapter);

// 	if (ixgbe_removed(hw->hw_addr)) {
// 		err = -EIO;
// 		goto err_sw_init;
// 	}
// 	INIT_WORK(&adapter->service_task, ixgbe_service_task);
// 	set_bit(__IXGBE_SERVICE_INITED, &adapter->state);
// 	clear_bit(__IXGBE_SERVICE_SCHED, &adapter->state);

// 	err = ixgbe_init_interrupt_scheme(adapter);
// 	if (err)
// 		goto err_sw_init;

// 	for (i = 0; i < adapter->num_rx_queues; i++)
// 		u64_stats_init(&adapter->rx_ring[i]->syncp);
// 	for (i = 0; i < adapter->num_tx_queues; i++)
// 		u64_stats_init(&adapter->tx_ring[i]->syncp);
// 	for (i = 0; i < adapter->num_xdp_queues; i++)
// 		u64_stats_init(&adapter->xdp_ring[i]->syncp);

// 	/* WOL not supported for all devices */
// 	adapter->wol = 0;
// 	hw->eeprom.ops.read(hw, 0x2c, &adapter->eeprom_cap);
// 	hw->wol_enabled = ixgbe_wol_supported(adapter, pdev->device,
// 						pdev->subsystem_device);
// 	if (hw->wol_enabled)
// 		adapter->wol = IXGBE_WUFC_MAG;

// 	device_set_wakeup_enable(&adapter->pdev->dev, adapter->wol);

// 	/* save off EEPROM version number */
// 	hw->eeprom.ops.read(hw, 0x2e, &adapter->eeprom_verh);
// 	hw->eeprom.ops.read(hw, 0x2d, &adapter->eeprom_verl);

// 	/* pick up the PCI bus settings for reporting later */
// 	if (ixgbe_pcie_from_parent(hw))
// 		ixgbe_get_parent_bus_info(adapter);
// 	else
// 		 hw->mac.ops.get_bus_info(hw);

// 	/* calculate the expected PCIe bandwidth required for optimal
// 	 * performance. Note that some older parts will never have enough
// 	 * bandwidth due to being older generation PCIe parts. We clamp these
// 	 * parts to ensure no warning is displayed if it can't be fixed.
// 	 */
// 	switch (hw->mac.type) {
// 	case ixgbe_mac_82598EB:
// 		expected_gts = min(ixgbe_enumerate_functions(adapter) * 10, 16);
// 		break;
// 	default:
// 		expected_gts = ixgbe_enumerate_functions(adapter) * 10;
// 		break;
// 	}

// 	/* don't check link if we failed to enumerate functions */
// 	if (expected_gts > 0)
// 		ixgbe_check_minimum_link(adapter, expected_gts);

// 	err = ixgbe_read_pba_string_generic(hw, part_str, sizeof(part_str));
// 	if (err)
// 		strlcpy(part_str, "Unknown", sizeof(part_str));
// 	if (ixgbe_is_sfp(hw) && hw->phy.sfp_type != ixgbe_sfp_type_not_present)
// 		e_dev_info("MAC: %d, PHY: %d, SFP+: %d, PBA No: %s\n",
// 			   hw->mac.type, hw->phy.type, hw->phy.sfp_type,
// 			   part_str);
// 	else
// 		e_dev_info("MAC: %d, PHY: %d, PBA No: %s\n",
// 			   hw->mac.type, hw->phy.type, part_str);

// 	e_dev_info("%pM\n", netdev->dev_addr);

// 	/* reset the hardware with the new settings */
// 	err = hw->mac.ops.start_hw(hw);
// 	if (err == IXGBE_ERR_EEPROM_VERSION) {
// 		/* We are running on a pre-production device, log a warning */
// 		e_dev_warn("This device is a pre-production adapter/LOM. "
// 			   "Please be aware there may be issues associated "
// 			   "with your hardware.  If you are experiencing "
// 			   "problems please contact your Intel or hardware "
// 			   "representative who provided you with this "
// 			   "hardware.\n");
// 	}
// 	strcpy(netdev->name, "eth%d");
// 	pci_set_drvdata(pdev, adapter);
// 	err = register_netdev(netdev);
// 	if (err)
// 		goto err_register;


// 	/* power down the optics for 82599 SFP+ fiber */
// 	if (hw->mac.ops.disable_tx_laser)
// 		hw->mac.ops.disable_tx_laser(hw);

// 	/* carrier off reporting is important to ethtool even BEFORE open */
// 	netif_carrier_off(netdev);

// #ifdef CONFIG_IXGBE_DCA
// 	if (dca_add_requester(&pdev->dev) == 0) {
// 		adapter->flags |= IXGBE_FLAG_DCA_ENABLED;
// 		ixgbe_setup_dca(adapter);
// 	}
// #endif
// 	if (adapter->flags & IXGBE_FLAG_SRIOV_ENABLED) {
// 		e_info(probe, "IOV is enabled with %d VFs\n", adapter->num_vfs);
// 		for (i = 0; i < adapter->num_vfs; i++)
// 			ixgbe_vf_configuration(pdev, (i | 0x10000000));
// 	}

// 	/* firmware requires driver version to be 0xFFFFFFFF
// 	 * since os does not support feature
// 	 */
// 	if (hw->mac.ops.set_fw_drv_ver)
// 		hw->mac.ops.set_fw_drv_ver(hw, 0xFF, 0xFF, 0xFF, 0xFF,
// 					   sizeof(ixgbe_driver_version) - 1,
// 					   ixgbe_driver_version);

// 	/* add san mac addr to netdev */
// 	ixgbe_add_sanmac_netdev(netdev);

// 	e_dev_info("%s\n", ixgbe_default_device_descr);

// #ifdef CONFIG_IXGBE_HWMON
// 	if (ixgbe_sysfs_init(adapter))
// 		e_err(probe, "failed to allocate sysfs resources\n");
// #endif /* CONFIG_IXGBE_HWMON */

// 	ixgbe_dbg_adapter_init(adapter);

// 	/* setup link for SFP devices with MNG FW, else wait for IXGBE_UP */
// 	if (ixgbe_mng_enabled(hw) && ixgbe_is_sfp(hw) && hw->mac.ops.setup_link)
// 		hw->mac.ops.setup_link(hw,
// 			IXGBE_LINK_SPEED_10GB_FULL | IXGBE_LINK_SPEED_1GB_FULL,
// 			true);

	return 0;

//err_register:
//	ixgbe_release_hw_control(adapter);
//	ixgbe_clear_interrupt_scheme(adapter);
// err_sw_init:
// 	ixgbe_disable_sriov(adapter);
// 	adapter->flags2 &= ~IXGBE_FLAG2_SEARCH_FOR_SFP;
// 	iounmap(adapter->io_addr);
// 	kfree(adapter->jump_tables[0]);
// 	kfree(adapter->mac_table);
// 	kfree(adapter->rss_key);
err_ioremap:
// 	disable_dev = !test_and_set_bit(__IXGBE_DISABLED, &adapter->state);
// 	free(dev);
err_alloc_etherdev:
 	pci_release_mem_regions(pdev);
err_pci_reg:
err_dma:
	if (!adapter || disable_dev)
		pci_disable_device(pdev);
	return err;
}

/**
 * ixgbe_remove - Device Removal Routine
 * @pdev: PCI device information struct
 *
 * ixgbe_remove is called by the PCI subsystem to alert the driver
 * that it should release a PCI device.  The could be caused by a
 * Hot-Plug event, or because the driver is going to be removed from
 * memory.
 **/
static void ixgbe_remove(struct pci_dev *pdev)
{
// 	struct ixgbe_adapter *adapter = pci_get_drvdata(pdev);
// 	struct net_device *netdev;
// 	bool disable_dev;
// 	int i;

// 	/* if !adapter then we already cleaned up in probe */
// 	if (!adapter)
// 		return;

// 	netdev  = adapter->netdev;
// 	ixgbe_dbg_adapter_exit(adapter);

// 	set_bit(__IXGBE_REMOVING, &adapter->state);
// 	cancel_work_sync(&adapter->service_task);


// #ifdef CONFIG_IXGBE_DCA
// 	if (adapter->flags & IXGBE_FLAG_DCA_ENABLED) {
// 		adapter->flags &= ~IXGBE_FLAG_DCA_ENABLED;
// 		dca_remove_requester(&pdev->dev);
// 		IXGBE_WRITE_REG(&adapter->hw, IXGBE_DCA_CTRL,
// 				IXGBE_DCA_CTRL_DCA_DISABLE);
// 	}

// #endif
// #ifdef CONFIG_IXGBE_HWMON
// 	ixgbe_sysfs_exit(adapter);
// #endif /* CONFIG_IXGBE_HWMON */

// 	/* remove the added san mac */
// 	ixgbe_del_sanmac_netdev(netdev);

// #ifdef CONFIG_PCI_IOV
// 	ixgbe_disable_sriov(adapter);
// #endif
// 	if (netdev->reg_state == NETREG_REGISTERED)
// 		unregister_netdev(netdev);

// 	ixgbe_clear_interrupt_scheme(adapter);

// 	ixgbe_release_hw_control(adapter);

// #ifdef CONFIG_DCB
// 	kfree(adapter->ixgbe_ieee_pfc);
// 	kfree(adapter->ixgbe_ieee_ets);

// #endif
// 	iounmap(adapter->io_addr);
	pci_release_mem_regions(pdev);

	// e_dev_info("complete\n");

	// for (i = 0; i < IXGBE_MAX_LINK_HANDLE; i++) {
	// 	if (adapter->jump_tables[i]) {
	// 		kfree(adapter->jump_tables[i]->input);
	// 		kfree(adapter->jump_tables[i]->mask);
	// 	}
	// 	kfree(adapter->jump_tables[i]);
	// }

	// kfree(adapter->mac_table);
	// kfree(adapter->rss_key);
	// disable_dev = !test_and_set_bit(__IXGBE_DISABLED, &adapter->state);
	// free_netdev(netdev);

	// pci_disable_pcie_error_reporting(pdev);

	// if (disable_dev)
		pci_disable_device(pdev);
}


static struct pci_driver ixgbe_driver = {
	.name     = ixgbe_driver_name,
	.id_table = ixgbe_pci_tbl,
	.probe    = ixgbe_probe,
	.remove   = ixgbe_remove,
/*#ifdef CONFIG_PM
	.suspend  = ixgbe_suspend,
	.resume   = ixgbe_resume,
#endif
	.shutdown = ixgbe_shutdown,
	.sriov_configure = ixgbe_pci_sriov_configure,
	.err_handler = &ixgbe_err_handler*/
};

int ixgbe_init(){
	int ret;
	pr_info("%s - version %s\n", ixgbe_driver_string, ixgbe_driver_version);
	pr_info("%s\n", ixgbe_copyright);

	ixgbe_wq = create_singlethread_workqueue(ixgbe_driver_name);
	if (!ixgbe_wq) {
		pr_err("%s: Failed to create workqueue\n", ixgbe_driver_name);
		return -ENOMEM;
	}

	//ixgbe_dbg_init();

	ret = pci_register_driver(&ixgbe_driver);
	if (ret) {
		destroy_workqueue(ixgbe_wq);
	//	ixgbe_dbg_exit();
		return ret;
	}

//#ifdef CONFIG_IXGBE_DCA
//	dca_register_notify(&dca_notifier);
//#endif

	return 0;
}
int ixgbe_exit(){
	pr_info("ixgbe driver removed");
	pci_unregister_driver(&ixgbe_driver);
	return 0;
}