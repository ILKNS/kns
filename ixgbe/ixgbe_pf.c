/*-
 *   BSD LICENSE
 * 
 *   Copyright(c) 2010-2013 Intel Corporation. All rights reserved.
 *   All rights reserved.
 * 
 *   Redistribution and use in source and binary forms, with or without
 *   modification, are permitted provided that the following conditions
 *   are met:
 * 
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in
 *       the documentation and/or other materials provided with the
 *       distribution.
 *     * Neither the name of Intel Corporation nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 * 
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *   OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include <kns/ethdev.h>

#include "ixgbe_common.h"
#include "ixgbe_ethdev.h"

#define IXGBE_MAX_VFTA     (128)

static inline 
int ixgbe_vf_perm_addr_gen(struct rte_eth_dev *dev, uint16_t vf_num)
{
	unsigned char vf_mac_addr[ETH_ADDR_LEN];
	struct ixgbe_vf_info *vfinfo = 
		*IXGBE_DEV_PRIVATE_TO_P_VFDATA(dev->data->dev_private);
	uint16_t vfn;

	for (vfn = 0; vfn < vf_num; vfn++) {
		//FIXME: eth_random_addr(vf_mac_addr);
		/* keep the random address as default */
		memcpy(vfinfo[vfn].vf_mac_addresses, vf_mac_addr, 
			   ETH_ADDR_LEN);
	}

	return 0;
}

static inline int
ixgbe_mb_intr_setup(struct rte_eth_dev *dev)
{
	struct ixgbe_interrupt *intr =
		IXGBE_DEV_PRIVATE_TO_INTR(dev->data->dev_private);

	intr->mask |= IXGBE_EICR_MAILBOX;

	return 0;
}

void ixgbe_pf_host_init(struct rte_eth_dev *eth_dev)
{
	struct ixgbe_vf_info **vfinfo = 
		IXGBE_DEV_PRIVATE_TO_P_VFDATA(eth_dev->data->dev_private);
	struct ixgbe_mirror_info *mirror_info =
        IXGBE_DEV_PRIVATE_TO_PFDATA(eth_dev->data->dev_private);
	struct ixgbe_uta_info *uta_info =
        IXGBE_DEV_PRIVATE_TO_UTA(eth_dev->data->dev_private);
	struct ixgbe_hw *hw = 
		IXGBE_DEV_PRIVATE_TO_HW(eth_dev->data->dev_private);
	uint16_t vf_num = hw->num_vfs;
	uint8_t nb_queue;

	RTE_ETH_DEV_SRIOV(eth_dev).active = 0;
	if (!vf_num)
		return;

	*vfinfo = kmalloc(sizeof(struct ixgbe_vf_info) * vf_num, GFP_KERNEL);
	if (*vfinfo == NULL)
		panic("Cannot allocate memory for private VF data\n");
	memset(vfinfo, 0, sizeof(struct ixgbe_vf_info) * vf_num);

	memset(mirror_info,0,sizeof(struct ixgbe_mirror_info));
	memset(uta_info,0,sizeof(struct ixgbe_uta_info));
	hw->mac.mc_filter_type = 0;

	if (vf_num >= ETH_32_POOLS) {
		nb_queue = 2;
		RTE_ETH_DEV_SRIOV(eth_dev).active = ETH_64_POOLS;
	} else if (vf_num >= ETH_16_POOLS) {
		nb_queue = 4;
		RTE_ETH_DEV_SRIOV(eth_dev).active = ETH_32_POOLS;
	} else {
		nb_queue = 8;
		RTE_ETH_DEV_SRIOV(eth_dev).active = ETH_16_POOLS;
	}

	RTE_ETH_DEV_SRIOV(eth_dev).nb_q_per_pool = nb_queue;
	RTE_ETH_DEV_SRIOV(eth_dev).def_vmdq_idx = vf_num;
	RTE_ETH_DEV_SRIOV(eth_dev).def_pool_q_idx = (uint16_t)(vf_num * nb_queue);

	ixgbe_vf_perm_addr_gen(eth_dev, vf_num);

	/* init_mailbox_params */
	hw->mbx.ops.init_params(hw);

	/* set mb interrupt mask */
	ixgbe_mb_intr_setup(eth_dev);

	return;
}

int ixgbe_pf_host_configure(struct rte_eth_dev *eth_dev)
{
	uint32_t vtctl, fcrth;
	uint32_t vfre_slot, vfre_offset;
	const uint8_t VFRE_SHIFT = 5;  /* VFRE 32 bits per slot */
	const uint8_t VFRE_MASK = (uint8_t)((1U << VFRE_SHIFT) - 1);
	struct ixgbe_hw *hw = IXGBE_DEV_PRIVATE_TO_HW(eth_dev->data->dev_private);
	uint16_t vf_num = hw->num_vfs;
	uint32_t gpie, gcr_ext;
	uint32_t vlanctrl;
	int i;

	if (!vf_num)
		return -1;

	/* enable VMDq and set the default pool for PF */
	vtctl = IXGBE_READ_REG(hw, IXGBE_VT_CTL);
	vtctl |= IXGBE_VMD_CTL_VMDQ_EN;
	vtctl &= ~IXGBE_VT_CTL_POOL_MASK;
	vtctl |= RTE_ETH_DEV_SRIOV(eth_dev).def_vmdq_idx 
		<< IXGBE_VT_CTL_POOL_SHIFT;
	vtctl |= IXGBE_VT_CTL_REPLEN;
	IXGBE_WRITE_REG(hw, IXGBE_VT_CTL, vtctl);

	vfre_offset = vf_num & VFRE_MASK; 
	vfre_slot = (vf_num >> VFRE_SHIFT) > 0 ? 1 : 0;
	
	/* Enable pools reserved to PF only */
	IXGBE_WRITE_REG(hw, IXGBE_VFRE(vfre_slot), (~0) << vfre_offset);
	IXGBE_WRITE_REG(hw, IXGBE_VFRE(vfre_slot ^ 1), vfre_slot - 1);
	IXGBE_WRITE_REG(hw, IXGBE_VFTE(vfre_slot), (~0) << vfre_offset);
	IXGBE_WRITE_REG(hw, IXGBE_VFTE(vfre_slot ^ 1), vfre_slot - 1);

	/* PFDMA Tx General Switch Control Enables VMDQ loopback */
	IXGBE_WRITE_REG(hw, IXGBE_PFDTXGSWC, IXGBE_PFDTXGSWC_VT_LBEN);

	/* clear VMDq map to perment rar 0 */
	hw->mac.ops.clear_vmdq(hw, 0, IXGBE_CLEAR_VMDQ_ALL);

	/* clear VMDq map to scan rar 127 */
	IXGBE_WRITE_REG(hw, IXGBE_MPSAR_LO(hw->mac.num_rar_entries), 0);
	IXGBE_WRITE_REG(hw, IXGBE_MPSAR_HI(hw->mac.num_rar_entries), 0);

	/* set VMDq map to default PF pool */
	hw->mac.ops.set_vmdq(hw, 0, RTE_ETH_DEV_SRIOV(eth_dev).def_vmdq_idx);

	/*
	 * SW msut set GCR_EXT.VT_Mode the same as GPIE.VT_Mode
	 */
	gcr_ext = IXGBE_READ_REG(hw, IXGBE_GCR_EXT);
	gcr_ext &= ~IXGBE_GCR_EXT_VT_MODE_MASK;
	
	gpie = IXGBE_READ_REG(hw, IXGBE_GPIE);
	gpie &= ~IXGBE_GPIE_VTMODE_MASK;
	gpie |= IXGBE_GPIE_MSIX_MODE;
	
	switch (RTE_ETH_DEV_SRIOV(eth_dev).active) {
	case ETH_64_POOLS:
		gcr_ext |= IXGBE_GCR_EXT_VT_MODE_64;
		gpie |= IXGBE_GPIE_VTMODE_64;
		break;
	case ETH_32_POOLS:
		gcr_ext |= IXGBE_GCR_EXT_VT_MODE_32;
		gpie |= IXGBE_GPIE_VTMODE_32;
		break;
	case ETH_16_POOLS:
		gcr_ext |= IXGBE_GCR_EXT_VT_MODE_16;
		gpie |= IXGBE_GPIE_VTMODE_16;
		break;
	}

	IXGBE_WRITE_REG(hw, IXGBE_GCR_EXT, gcr_ext);
        IXGBE_WRITE_REG(hw, IXGBE_GPIE, gpie);

        /* 
	 * enable vlan filtering and allow all vlan tags through 
	 */
        vlanctrl = IXGBE_READ_REG(hw, IXGBE_VLNCTRL);
        vlanctrl |= IXGBE_VLNCTRL_VFE ; /* enable vlan filters */
        IXGBE_WRITE_REG(hw, IXGBE_VLNCTRL, vlanctrl);

        /* VFTA - enable all vlan filters */
        for (i = 0; i < IXGBE_MAX_VFTA; i++) {
                IXGBE_WRITE_REG(hw, IXGBE_VFTA(i), 0xFFFFFFFF);
        }
	
	/* Enable MAC Anti-Spoofing */
	hw->mac.ops.set_mac_anti_spoofing(hw, false, vf_num);

	/* set flow control threshold to max to avoid tx switch hang */	
	for (i = 0; i < MAX_TRAFFIC_CLASS; i++) {
		IXGBE_WRITE_REG(hw, IXGBE_FCRTL_82599(i), 0);
		fcrth = IXGBE_READ_REG(hw, IXGBE_RXPBSIZE(i)) - 32;
		IXGBE_WRITE_REG(hw, IXGBE_FCRTH_82599(i), fcrth);
	}

	return 0;
}

// static void 
// set_rx_mode(struct rte_eth_dev *dev)
// {
// 	struct rte_eth_dev_data *dev_data = 
// 		(struct rte_eth_dev_data*)dev->data->dev_private;
// 	struct ixgbe_hw *hw = IXGBE_DEV_PRIVATE_TO_HW(dev->data->dev_private);
// 	u32 fctrl, vmolr = IXGBE_VMOLR_BAM | IXGBE_VMOLR_AUPE;
// 	uint16_t vfn = hw->num_vfs;

// 	/* Check for Promiscuous and All Multicast modes */
// 	fctrl = IXGBE_READ_REG(hw, IXGBE_FCTRL);

// 	/* set all bits that we expect to always be set */
// 	fctrl &= ~IXGBE_FCTRL_SBP; /* disable store-bad-packets */
// 	fctrl |= IXGBE_FCTRL_BAM;

// 	/* clear the bits we are changing the status of */
// 	fctrl &= ~(IXGBE_FCTRL_UPE | IXGBE_FCTRL_MPE);

// 	if (dev_data->promiscuous) {
// 		fctrl |= (IXGBE_FCTRL_UPE | IXGBE_FCTRL_MPE);
// 		vmolr |= (IXGBE_VMOLR_ROPE | IXGBE_VMOLR_MPE);
// 	} else {
// 		if (dev_data->all_multicast) {
// 			fctrl |= IXGBE_FCTRL_MPE;
// 			vmolr |= IXGBE_VMOLR_MPE;
// 		} else {
// 			vmolr |= IXGBE_VMOLR_ROMPE;
// 		}
// 	}

// 	if (hw->mac.type != ixgbe_mac_82598EB) {
// 		vmolr |= IXGBE_READ_REG(hw, IXGBE_VMOLR(vfn)) &
// 			 ~(IXGBE_VMOLR_MPE | IXGBE_VMOLR_ROMPE |
// 			   IXGBE_VMOLR_ROPE);
// 		IXGBE_WRITE_REG(hw, IXGBE_VMOLR(vfn), vmolr);
// 	}

// 	IXGBE_WRITE_REG(hw, IXGBE_FCTRL, fctrl);

// 	if (dev->data->dev_conf.rxmode.hw_vlan_strip)
// 		ixgbe_vlan_hw_strip_enable_all(dev);
// 	else
// 		ixgbe_vlan_hw_strip_disable_all(dev);
// }

// static inline void 
// ixgbe_vf_reset_event(struct rte_eth_dev *dev, uint16_t vf)
// {
// 	struct ixgbe_hw *hw = 
// 		IXGBE_DEV_PRIVATE_TO_HW(dev->data->dev_private);
// 	struct ixgbe_vf_info *vfinfo = 
// 		*(IXGBE_DEV_PRIVATE_TO_P_VFDATA(dev->data->dev_private));
// 	int rar_entry = hw->mac.num_rar_entries - (vf + 1);
// 	uint32_t vmolr = IXGBE_READ_REG(hw, IXGBE_VMOLR(vf));

// 	vmolr |= (IXGBE_VMOLR_ROPE | IXGBE_VMOLR_ROMPE | 
// 			IXGBE_VMOLR_BAM | IXGBE_VMOLR_AUPE);
// 	IXGBE_WRITE_REG(hw, IXGBE_VMOLR(vf), vmolr);

// 	IXGBE_WRITE_REG(hw, IXGBE_VMVIR(vf), 0);
	
// 	/* reset multicast table array for vf */
// 	vfinfo[vf].num_vf_mc_hashes = 0;

// 	/* reset rx mode */
// 	set_rx_mode(dev);
	
// 	hw->mac.ops.clear_rar(hw, rar_entry);
// }

// static inline void 
// ixgbe_vf_reset_msg(struct rte_eth_dev *dev, uint16_t vf)
// {
// 	struct ixgbe_hw *hw = IXGBE_DEV_PRIVATE_TO_HW(dev->data->dev_private);
// 	uint32_t reg;
// 	uint32_t reg_offset, vf_shift;
// 	const uint8_t VFRE_SHIFT = 5;  /* VFRE 32 bits per slot */
// 	const uint8_t VFRE_MASK = (uint8_t)((1U << VFRE_SHIFT) - 1);

// 	vf_shift = vf & VFRE_MASK;
// 	reg_offset = (vf >> VFRE_SHIFT) > 0 ? 1 : 0;

// 	/* enable transmit and receive for vf */
// 	reg = IXGBE_READ_REG(hw, IXGBE_VFTE(reg_offset));
// 	reg |= (reg | (1 << vf_shift));
// 	IXGBE_WRITE_REG(hw, IXGBE_VFTE(reg_offset), reg);

// 	reg = IXGBE_READ_REG(hw, IXGBE_VFRE(reg_offset));
// 	reg |= (reg | (1 << vf_shift));
// 	IXGBE_WRITE_REG(hw, IXGBE_VFRE(reg_offset), reg);

// 	/* Enable counting of spoofed packets in the SSVPC register */
// 	reg = IXGBE_READ_REG(hw, IXGBE_VMECM(reg_offset));
// 	reg |= (1 << vf_shift);
// 	IXGBE_WRITE_REG(hw, IXGBE_VMECM(reg_offset), reg);

// 	ixgbe_vf_reset_event(dev, vf);
// }

// static int
// ixgbe_vf_reset(struct rte_eth_dev *dev, uint16_t vf, uint32_t *msgbuf)
// {
// 	struct ixgbe_hw *hw = IXGBE_DEV_PRIVATE_TO_HW(dev->data->dev_private);
// 	struct ixgbe_vf_info *vfinfo = 
// 		*(IXGBE_DEV_PRIVATE_TO_P_VFDATA(dev->data->dev_private));
// 	unsigned char *vf_mac = vfinfo[vf].vf_mac_addresses;
// 	int rar_entry = hw->mac.num_rar_entries - (vf + 1);
// 	uint8_t *new_mac = (uint8_t *)(&msgbuf[1]);

// 	ixgbe_vf_reset_msg(dev, vf);

// 	hw->mac.ops.set_rar(hw, rar_entry, vf_mac, vf, IXGBE_RAH_AV);

// 	/* reply to reset with ack and vf mac address */
// 	msgbuf[0] = IXGBE_VF_RESET | IXGBE_VT_MSGTYPE_ACK;
// 	memcpy(new_mac, vf_mac, ETH_ADDR_LEN);
// 	/*
// 	 * Piggyback the multicast filter type so VF can compute the
// 	 * correct vectors
// 	 */
// 	msgbuf[3] = hw->mac.mc_filter_type;
// 	ixgbe_write_mbx(hw, msgbuf, IXGBE_VF_PERMADDR_MSG_LEN, vf);

// 	return 0;
// }

// static int
// ixgbe_vf_set_mac_addr(struct rte_eth_dev *dev, uint32_t vf, uint32_t *msgbuf)
// {
// 	struct ixgbe_hw *hw = IXGBE_DEV_PRIVATE_TO_HW(dev->data->dev_private);
// 	struct ixgbe_vf_info *vfinfo = 
// 		*(IXGBE_DEV_PRIVATE_TO_P_VFDATA(dev->data->dev_private));
// 	int rar_entry = hw->mac.num_rar_entries - (vf + 1);
// 	uint8_t *new_mac = (uint8_t *)(&msgbuf[1]);
// 	struct eth_addr *addr = (struct eth_addr *) new_mac;

// 	if (!eth_addr_is_zero(addr) && !eth_addr_is_multicast(addr)) {
// 		memcpy(vfinfo[vf].vf_mac_addresses, new_mac, 6);
// 		return hw->mac.ops.set_rar(hw, rar_entry, new_mac, vf, IXGBE_RAH_AV);
// 	}
// 	return -1;
// }

// static int
// ixgbe_vf_set_multicast(struct rte_eth_dev *dev, __notused uint32_t vf, uint32_t *msgbuf)
// {
// 	struct ixgbe_hw *hw = IXGBE_DEV_PRIVATE_TO_HW(dev->data->dev_private);
// 	struct ixgbe_vf_info *vfinfo = 
// 		*(IXGBE_DEV_PRIVATE_TO_P_VFDATA(dev->data->dev_private));
// 	int nb_entries = (msgbuf[0] & IXGBE_VT_MSGINFO_MASK) >> 
// 		IXGBE_VT_MSGINFO_SHIFT;
// 	uint16_t *hash_list = (uint16_t *)&msgbuf[1];
// 	uint32_t mta_idx;
// 	uint32_t mta_shift;
// 	const uint32_t IXGBE_MTA_INDEX_MASK = 0x7F;
// 	const uint32_t IXGBE_MTA_BIT_SHIFT = 5;
// 	const uint32_t IXGBE_MTA_BIT_MASK = (0x1 << IXGBE_MTA_BIT_SHIFT) - 1;
// 	uint32_t reg_val;
// 	int i;
		
// 	/* only so many hash values supported */
// 	nb_entries = min(nb_entries, IXGBE_MAX_VF_MC_ENTRIES);

// 	/* store the mc entries  */
// 	vfinfo->num_vf_mc_hashes = (uint16_t)nb_entries;
// 	for (i = 0; i < nb_entries; i++) {
// 		vfinfo->vf_mc_hashes[i] = hash_list[i];
// 	}

// 	for (i = 0; i < vfinfo->num_vf_mc_hashes; i++) {
// 		mta_idx = (vfinfo->vf_mc_hashes[i] >> IXGBE_MTA_BIT_SHIFT) 
// 				& IXGBE_MTA_INDEX_MASK;
// 		mta_shift = vfinfo->vf_mc_hashes[i] & IXGBE_MTA_BIT_MASK;
// 		reg_val = IXGBE_READ_REG(hw, IXGBE_MTA(mta_idx));
// 		reg_val |= (1 << mta_shift);
// 		IXGBE_WRITE_REG(hw, IXGBE_MTA(mta_idx), reg_val);
// 	}

// 	return 0;
// }

// static int
// ixgbe_vf_set_vlan(struct rte_eth_dev *dev, uint32_t vf, uint32_t *msgbuf)
// {
// 	int add, vid;
// 	struct ixgbe_hw *hw = IXGBE_DEV_PRIVATE_TO_HW(dev->data->dev_private);
// 	struct ixgbe_vf_info *vfinfo = 
// 		*(IXGBE_DEV_PRIVATE_TO_P_VFDATA(dev->data->dev_private));

// 	add = (msgbuf[0] & IXGBE_VT_MSGINFO_MASK)
// 		>> IXGBE_VT_MSGINFO_SHIFT;
// 	vid = (msgbuf[1] & IXGBE_VLVF_VLANID_MASK);

// 	if (add)
// 		vfinfo[vf].vlan_count++;
// 	else if (vfinfo[vf].vlan_count)
// 		vfinfo[vf].vlan_count--;
// 	return hw->mac.ops.set_vfta(hw, vid, vf, (bool)add);
// }

// static int 
// ixgbe_set_vf_lpe(struct rte_eth_dev *dev, __notused uint32_t vf, uint32_t *msgbuf)
// {
// 	struct ixgbe_hw *hw = IXGBE_DEV_PRIVATE_TO_HW(dev->data->dev_private);
// 	uint32_t new_mtu = msgbuf[1];
// 	uint32_t max_frs;
// 	int max_frame = new_mtu + ETH_HDR_LEN + ETH_CRC_LEN;

// 	/* Only X540 supports jumbo frames in IOV mode */
// 	if (hw->mac.type != ixgbe_mac_X540)
// 		return -1;

// 	if ((max_frame < ETH_MIN_LEN) || (max_frame > ETH_MAX_LEN_JUMBO)) 
// 		return -1;

// 	max_frs = (IXGBE_READ_REG(hw, IXGBE_MAXFRS) &
// 		   IXGBE_MHADD_MFS_MASK) >> IXGBE_MHADD_MFS_SHIFT;
// 	if (max_frs < new_mtu) {
// 		max_frs = new_mtu << IXGBE_MHADD_MFS_SHIFT;
// 		IXGBE_WRITE_REG(hw, IXGBE_MAXFRS, max_frs);
// 	}

// 	return 0;
// }

// static int 
// ixgbe_rcv_msg_from_vf(struct rte_eth_dev *dev, uint16_t vf)
// {
// 	uint16_t mbx_size = IXGBE_VFMAILBOX_SIZE;
// 	uint32_t msgbuf[IXGBE_VFMAILBOX_SIZE];
// 	int32_t retval;
// 	struct ixgbe_hw *hw = IXGBE_DEV_PRIVATE_TO_HW(dev->data->dev_private);

// 	retval = ixgbe_read_mbx(hw, msgbuf, mbx_size, vf);
// 	if (retval) {
// 		log_err("ixgbe: Error mbx recv msg from VF %d\n", vf);
// 		return retval;
// 	}

// 	/* do nothing with the message already been processed */
// 	if (msgbuf[0] & (IXGBE_VT_MSGTYPE_ACK | IXGBE_VT_MSGTYPE_NACK))
// 		return retval;

// 	/* flush the ack before we write any messages back */
// 	IXGBE_WRITE_FLUSH(hw);

// 	/* perform VF reset */
// 	if (msgbuf[0] == IXGBE_VF_RESET) {
// 		return ixgbe_vf_reset(dev, vf, msgbuf);
// 	}

// 	/* check & process VF to PF mailbox message */
// 	switch ((msgbuf[0] & 0xFFFF)) {
// 	case IXGBE_VF_SET_MAC_ADDR:
// 		retval = ixgbe_vf_set_mac_addr(dev, vf, msgbuf);
// 		break;
// 	case IXGBE_VF_SET_MULTICAST:
// 		retval = ixgbe_vf_set_multicast(dev, vf, msgbuf);
// 		break;
// 	case IXGBE_VF_SET_LPE:
// 		retval = ixgbe_set_vf_lpe(dev, vf, msgbuf);
// 		break;
// 	case IXGBE_VF_SET_VLAN:
// 		retval = ixgbe_vf_set_vlan(dev, vf, msgbuf);
// 		break;
// 	default:
// 		log_debug("ixgbe: Unhandled Msg %8.8x\n", (unsigned)  msgbuf[0]);
// 		retval = IXGBE_ERR_MBX;
// 		break;
// 	}

// 	/* response the VF according to the message process result */
// 	if (retval)
// 		msgbuf[0] |= IXGBE_VT_MSGTYPE_NACK;
// 	else
// 		msgbuf[0] |= IXGBE_VT_MSGTYPE_ACK;

// 	msgbuf[0] |= IXGBE_VT_MSGTYPE_CTS;

// 	ixgbe_write_mbx(hw, msgbuf, 1, vf);

// 	return retval;
// }

// static inline void 
// ixgbe_rcv_ack_from_vf(struct rte_eth_dev *dev, uint16_t vf)
// {
// 	uint32_t msg = IXGBE_VT_MSGTYPE_NACK;
// 	struct ixgbe_hw *hw = 
// 		IXGBE_DEV_PRIVATE_TO_HW(dev->data->dev_private);

// 	ixgbe_write_mbx(hw, &msg, 1, vf);
// }

// void ixgbe_pf_mbx_process(struct rte_eth_dev *eth_dev)
// {
// 	uint16_t vf;
// 	struct ixgbe_hw *hw = 
// 		IXGBE_DEV_PRIVATE_TO_HW(eth_dev->data->dev_private);

// 	for (vf = 0; vf < hw->num_vfs; vf++) {
// 		/* check & process vf function level reset */
// 		if (!ixgbe_check_for_rst(hw, vf))
// 			ixgbe_vf_reset_event(eth_dev, vf);

// 		/* check & process vf mailbox messages */
// 		if (!ixgbe_check_for_msg(hw, vf))
// 			ixgbe_rcv_msg_from_vf(eth_dev, vf);

// 		/* check & process acks from vf */
// 		if (!ixgbe_check_for_ack(hw, vf))
// 			ixgbe_rcv_ack_from_vf(eth_dev, vf);
// 	}
// }
