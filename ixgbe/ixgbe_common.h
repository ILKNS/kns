#pragma once

#include "ixgbe_type.h"

//FIXME: The implement of original "read_reg" twisted too
// much with other functions, use ix's instead temperarily

// AND the addr of reg seems to be different

#define IXGBE_PCI_REG(reg) (*((volatile uint32_t *)(reg)))

static inline uint32_t ixgbe_read_addr(volatile void *addr)
{
	return IXGBE_PCI_REG(addr);
}

#define IXGBE_PCI_REG_WRITE(reg, value) do { \
	IXGBE_PCI_REG((reg)) = (value); \
} while(0)

static inline bool ixgbe_removed(void __iomem *addr)
{
	return unlikely(!addr);
}

static inline void ixgbe_write_reg(struct ixgbe_hw *hw, u32 reg, u32 value)
{
	u8 __iomem *reg_addr = ACCESS_ONCE(hw->hw_addr);

	if (ixgbe_removed(reg_addr))
		return;
	writel(value, reg_addr + reg);
}
#define IXGBE_WRITE_REG(a, reg, value) ixgbe_write_reg((a), (reg), (value))

#define IXGBE_PCI_REG_ADDR(hw, reg) \
	((volatile uint32_t *)((char *)(hw)->hw_addr + (reg)))

#define IXGBE_PCI_REG_ARRAY_ADDR(hw, reg, index) \
	IXGBE_PCI_REG_ADDR((hw), (reg) + ((index) << 2))

#define IXGBE_READ_REG(hw, reg) \
	ixgbe_read_addr(IXGBE_PCI_REG_ADDR((hw), (reg)))

#define IXGBE_WRITE_FLUSH(a) IXGBE_READ_REG((a), IXGBE_STATUS)

/* Not implemented !! */
#define IXGBE_READ_PCIE_WORD(hw, reg) 0	
#define IXGBE_WRITE_PCIE_WORD(hw, reg, value) do { } while(0)

// #ifndef writeq
// #define writeq writeq
// static inline void writeq(u64 val, void __iomem *addr)
// {
// 	writel((u32)val, addr);
// 	writel((u32)(val >> 32), addr + 4);
// }
// #endif

// static inline void ixgbe_write_reg64(struct ixgbe_hw *hw, u32 reg, u64 value)
// {
// 	u8 __iomem *reg_addr = ACCESS_ONCE(hw->hw_addr);

// 	if (ixgbe_removed(reg_addr))
// 		return;
// 	writeq(value, reg_addr + reg);
// }
// #define IXGBE_WRITE_REG64(a, reg, value) ixgbe_write_reg64((a), (reg), (value))

#define IXGBE_WRITE_REG_ARRAY(a, reg, offset, value) \
		ixgbe_write_reg((a), (reg) + ((offset) << 2), (value))

#define IXGBE_READ_REG_ARRAY(a, reg, offset) \
		IXGBE_PCI_REG(IXGBE_PCI_REG_ARRAY_ADDR((a), (reg), (offset)))

#define IXGBE_WRITE_REG64(hw, reg, value) \
	do { \
		IXGBE_WRITE_REG(hw, reg, (u32) value); \
		IXGBE_WRITE_REG(hw, reg + 4, (u32) (value >> 32)); \
	} while (0)
struct ixgbe_pba {
	u16 word[2];
	u16 *pba_block;
};

u16 ixgbe_get_pcie_msix_count_generic(struct ixgbe_hw *hw);
s32 ixgbe_init_ops_generic(struct ixgbe_hw *hw);
s32 ixgbe_init_hw_generic(struct ixgbe_hw *hw);
s32 ixgbe_start_hw_generic(struct ixgbe_hw *hw);
s32 ixgbe_start_hw_gen2(struct ixgbe_hw *hw);
s32 ixgbe_clear_hw_cntrs_generic(struct ixgbe_hw *hw);
s32 ixgbe_read_pba_num_generic(struct ixgbe_hw *hw, u32 *pba_num);
s32 ixgbe_read_pba_string_generic(struct ixgbe_hw *hw, u8 *pba_num,
				  u32 pba_num_size);
s32 ixgbe_read_pba_raw(struct ixgbe_hw *hw, u16 *eeprom_buf,
		       u32 eeprom_buf_size, u16 max_pba_block_size,
		       struct ixgbe_pba *pba);
s32 ixgbe_write_pba_raw(struct ixgbe_hw *hw, u16 *eeprom_buf,
			u32 eeprom_buf_size, struct ixgbe_pba *pba);
s32 ixgbe_get_pba_block_size(struct ixgbe_hw *hw, u16 *eeprom_buf,
			     u32 eeprom_buf_size, u16 *pba_block_size);
s32 ixgbe_get_mac_addr_generic(struct ixgbe_hw *hw, u8 *mac_addr);
s32 ixgbe_get_bus_info_generic(struct ixgbe_hw *hw);
void ixgbe_set_lan_id_multi_port_pcie(struct ixgbe_hw *hw);
s32 ixgbe_stop_adapter_generic(struct ixgbe_hw *hw);

s32 ixgbe_led_on_generic(struct ixgbe_hw *hw, u32 index);
s32 ixgbe_led_off_generic(struct ixgbe_hw *hw, u32 index);

s32 ixgbe_init_eeprom_params_generic(struct ixgbe_hw *hw);
s32 ixgbe_write_eeprom_generic(struct ixgbe_hw *hw, u16 offset, u16 data);
s32 ixgbe_write_eeprom_buffer_bit_bang_generic(struct ixgbe_hw *hw, u16 offset,
					       u16 words, u16 *data);
s32 ixgbe_read_eerd_generic(struct ixgbe_hw *hw, u16 offset, u16 *data);
s32 ixgbe_read_eerd_buffer_generic(struct ixgbe_hw *hw, u16 offset,
				   u16 words, u16 *data);
s32 ixgbe_write_eewr_generic(struct ixgbe_hw *hw, u16 offset, u16 data);
s32 ixgbe_write_eewr_buffer_generic(struct ixgbe_hw *hw, u16 offset,
				    u16 words, u16 *data);
s32 ixgbe_read_eeprom_bit_bang_generic(struct ixgbe_hw *hw, u16 offset,
				       u16 *data);
s32 ixgbe_read_eeprom_buffer_bit_bang_generic(struct ixgbe_hw *hw, u16 offset,
					      u16 words, u16 *data);
s32 ixgbe_calc_eeprom_checksum_generic(struct ixgbe_hw *hw);
s32 ixgbe_validate_eeprom_checksum_generic(struct ixgbe_hw *hw,
					   u16 *checksum_val);
s32 ixgbe_update_eeprom_checksum_generic(struct ixgbe_hw *hw);
s32 ixgbe_poll_eerd_eewr_done(struct ixgbe_hw *hw, u32 ee_reg);

s32 ixgbe_set_rar_generic(struct ixgbe_hw *hw, u32 index, u8 *addr, u32 vmdq,
			  u32 enable_addr);
s32 ixgbe_clear_rar_generic(struct ixgbe_hw *hw, u32 index);
s32 ixgbe_init_rx_addrs_generic(struct ixgbe_hw *hw);
s32 ixgbe_update_mc_addr_list_generic(struct ixgbe_hw *hw, u8 *mc_addr_list,
				      u32 mc_addr_count,
				      ixgbe_mc_addr_itr func, bool clear);
s32 ixgbe_update_uc_addr_list_generic(struct ixgbe_hw *hw, u8 *addr_list,
				      u32 addr_count, ixgbe_mc_addr_itr func);
s32 ixgbe_enable_mc_generic(struct ixgbe_hw *hw);
s32 ixgbe_disable_mc_generic(struct ixgbe_hw *hw);
s32 ixgbe_enable_rx_dma_generic(struct ixgbe_hw *hw, u32 regval);
s32 ixgbe_disable_sec_rx_path_generic(struct ixgbe_hw *hw);
s32 ixgbe_enable_sec_rx_path_generic(struct ixgbe_hw *hw);

s32 ixgbe_fc_enable_generic(struct ixgbe_hw *hw);
s32 ixgbe_device_supports_autoneg_fc(struct ixgbe_hw *hw);
void ixgbe_fc_autoneg(struct ixgbe_hw *hw);

s32 ixgbe_validate_mac_addr(u8 *mac_addr);
s32 ixgbe_acquire_swfw_sync(struct ixgbe_hw *hw, u32 mask);
void ixgbe_release_swfw_sync(struct ixgbe_hw *hw, u32 mask);
s32 ixgbe_disable_pcie_master(struct ixgbe_hw *hw);

s32 ixgbe_blink_led_start_generic(struct ixgbe_hw *hw, u32 index);
s32 ixgbe_blink_led_stop_generic(struct ixgbe_hw *hw, u32 index);

s32 ixgbe_get_san_mac_addr_generic(struct ixgbe_hw *hw, u8 *san_mac_addr);
s32 ixgbe_set_san_mac_addr_generic(struct ixgbe_hw *hw, u8 *san_mac_addr);

s32 ixgbe_set_vmdq_generic(struct ixgbe_hw *hw, u32 rar, u32 vmdq);
s32 ixgbe_set_vmdq_san_mac_generic(struct ixgbe_hw *hw, u32 vmdq);
s32 ixgbe_clear_vmdq_generic(struct ixgbe_hw *hw, u32 rar, u32 vmdq);
s32 ixgbe_insert_mac_addr_generic(struct ixgbe_hw *hw, u8 *addr, u32 vmdq);
s32 ixgbe_init_uta_tables_generic(struct ixgbe_hw *hw);
s32 ixgbe_set_vfta_generic(struct ixgbe_hw *hw, u32 vlan,
			 u32 vind, bool vlan_on);
s32 ixgbe_set_vlvf_generic(struct ixgbe_hw *hw, u32 vlan, u32 vind,
			   bool vlan_on, bool *vfta_changed);
s32 ixgbe_clear_vfta_generic(struct ixgbe_hw *hw);
s32 ixgbe_find_vlvf_slot(struct ixgbe_hw *hw, u32 vlan);

s32 ixgbe_check_mac_link_generic(struct ixgbe_hw *hw,
			       ixgbe_link_speed *speed,
			       bool *link_up, bool link_up_wait_to_complete);

s32 ixgbe_get_wwn_prefix_generic(struct ixgbe_hw *hw, u16 *wwnn_prefix,
				 u16 *wwpn_prefix);

s32 ixgbe_get_fcoe_boot_status_generic(struct ixgbe_hw *hw, u16 *bs);
void ixgbe_set_mac_anti_spoofing(struct ixgbe_hw *hw, bool enable, int pf);
void ixgbe_set_vlan_anti_spoofing(struct ixgbe_hw *hw, bool enable, int vf);
s32 ixgbe_get_device_caps_generic(struct ixgbe_hw *hw, u16 *device_caps);
void ixgbe_set_rxpba_generic(struct ixgbe_hw *hw, int num_pb, u32 headroom,
			     int strategy);
void ixgbe_enable_relaxed_ordering_gen2(struct ixgbe_hw *hw);
s32 ixgbe_set_fw_drv_ver_generic(struct ixgbe_hw *hw, u8 maj, u8 min,
				 u8 build, u8 ver, u16 len, const char *str);
void ixgbe_clear_tx_pending(struct ixgbe_hw *hw);

extern s32 ixgbe_reset_pipeline_82599(struct ixgbe_hw *hw);

extern const u32 ixgbe_mvals_8259X[IXGBE_MVALS_IDX_LIMIT];