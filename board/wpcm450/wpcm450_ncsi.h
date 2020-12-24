/*
 * $RCSfile$
 * $Revision$
 * $Date$
 * $Author$
 *
 * WPCM450 NC-SI driver.
 *  
 * Copyright (C) 2007 Avocent Corp.
 *
 * This file is subject to the terms and conditions of the GNU 
 * General Public License. This program is distributed in the hope 
 * that it will be useful, but WITHOUT ANY WARRANTY; without even 
 * the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
 * PURPOSE.  See the GNU General Public License for more details.
 */


#ifndef __NCSI_H__
#define __NCSI_H__

/* -----------------------
   NC-SI setting 
   ----------------------- */

#define NCSI_ETHER_TYPE                             0x88F8

#define ETH_HLEN                                    14
#define PKTMINSIZE                                  60

#define NCSI_MC_ID                                  0x0
#define NCSI_HEADER_REVISION                        0x1

#define NCSI_CHNL_ID_UNAVAILABLE                    0xFF

/* one sending command + two retry command*/
#define MAX_CMD_RETRY_COUNT                         3

#define CHNL_ID(pkgid, internel_chnlid)             (((pkgid & 0x7) << 5) \
                                                     | (internel_chnlid & 0x1F))

#define PKG_ID(chnl_id)                             ((chnl_id & 0xE0) >> 5)

#define INTERNAL_CHNL_ID(chnl_id)                   (chnl_id & 0x1F)

#define NCSI_RES_PACKET_MASK                        0x80

/* NCSI command */
#define CMD_CLEAR_INITIAL_STATE                     0x00 
#define CMD_SELECT_PACKAGE                          0x01 
#define CMD_DESELECT_PACKAGE                        0x02 
#define CMD_ENABLE_CHANNEL                          0x03 
#define CMD_DISABLE_CHANNEL                         0x04 
#define CMD_RESET_CHANNEL                           0x05 
#define CMD_ENABLE_CHANNEL_NETWORK_TX               0x06 
#define CMD_DISABLE_CHANNEL_NETWORK_TX              0x07 
#define CMD_AEN_ENABLE                              0x08 
#define CMD_SET_LINK                                0x09 
#define CMD_GET_LINK_STATUS                         0x0A 
#define CMD_SET_VLAN_FILTERS                        0x0B 
#define CMD_ENABLE_VLAN                             0x0C 
#define CMD_DISABLE_VLAN                            0x0D 
#define CMD_SET_MAC_ADDRESS                         0x0E 
#define CMD_ENABLE_BROADCAST_FILTERING              0x10 
#define CMD_DISABLE_BROADCAST_FILTERING             0x11 
#define CMD_ENABLE_GLOBAL_MULTICAST_FILTERING       0x12 
#define CMD_DISABLE_GLOBAL_MULTICAST_FILTERING      0x13 
#define CMD_SET_NCSI_FLOW_CONTROL                   0x14 
#define CMD_GET_VERSION_ID                          0x15 
#define CMD_GET_CAPABILITIES                        0x16 
#define CMD_GET_PARAMETERS                          0x17 
#define CMD_GET_CONTROLLER_PACKET_STATISTICS        0x18 
#define CMD_GET_NCSI_STATISTICS                     0x19 
#define CMD_GET_NCSI_PASSTHROUGH_STATISTICS         0x1A 
#define CMD_OEM_COMMAND                             0x50 

/* NCSI command payload length */
#define CMD_CLEAR_INITIAL_STATE_PAYLOAD_LEN                     0
#define CMD_SELECT_PACKAGE_PAYLOAD_LEN                          4
#define CMD_DESELECT_PACKAGE_PAYLOAD_LEN                        0
#define CMD_ENABLE_CHANNEL_PAYLOAD_LEN                          0
#define CMD_DISABLE_CHANNEL_PAYLOAD_LEN                         4
#define CMD_RESET_CHANNEL_PAYLOAD_LEN                           0
#define CMD_ENABLE_CHANNEL_NETWORK_TX_PAYLOAD_LEN               0
#define CMD_DISABLE_CHANNEL_NETWORK_TX_PAYLOAD_LEN              0
#define CMD_AEN_ENABLE_PAYLOAD_LEN                              8
#define CMD_SET_LINK_PAYLOAD_LEN                                4
#define CMD_GET_LINK_STATUS_PAYLOAD_LEN                         0
#define CMD_SET_VLAN_FILTERS_PAYLOAD_LEN                        8
#define CMD_ENABLE_VLAN_PAYLOAD_LEN                             4
#define CMD_DISABLE_VLAN_PAYLOAD_LEN                            0
#define CMD_SET_MAC_ADDRESS_PAYLOAD_LEN                         8
#define CMD_ENABLE_BROADCAST_FILTERING_PAYLOAD_LEN              4
#define CMD_DISABLE_BROADCAST_FILTERING_PAYLOAD_LEN             0
#define CMD_ENABLE_GLOBAL_MULTICAST_FILTERING_PAYLOAD_LEN       0
#define CMD_DISABLE_GLOBAL_MULTICAST_FILTERING_PAYLOAD_LEN      0
#define CMD_SET_NCSI_FLOW_CONTROL_PAYLOAD_LEN                   4
#define CMD_GET_VERSION_ID_PAYLOAD_LEN                          0
#define CMD_GET_CAPABILITIES_PAYLOAD_LEN                        0
#define CMD_GET_PARAMETERS_PAYLOAD_LEN                          0
#define CMD_GET_CONTROLLER_PACKET_STATISTICS_PAYLOAD_LEN        0
#define CMD_GET_NCSI_STATISTICS_PAYLOAD_LEN                     0
#define CMD_GET_NCSI_PASSTHROUGH_STATISTICS_PAYLOAD_LEN         0

/* NCSI command payload data */
/* SELECT_PACKAGE command payload */
#define CMD_SELECT_PACKAGE_PAYLOAD_HW_ARB_ENABLE                0x00
#define CMD_SELECT_PACKAGE_PAYLOAD_HW_ARB_DISABLE               0x01

/* DISABLE_CHANNEL command payload */
#define CMD_DISABLE_CHANNEL_PAYLOAD_KEEP_LINK_UP                0x0
#define CMD_DISABLE_CHANNEL_PAYLOAD_ALLOW_LINK_DOWN             0x1

/* SET_LINK command payload */
#define CMD_SET_LINK_PAYLOAD_AUTO_NEGO_DISABLE                  0x0
#define CMD_SET_LINK_PAYLOAD_AUTO_NEGO_ENABLE                   0x1
#define CMD_SET_LINK_PAYLOAD_LINK_SPEED_10M                     0x01
#define CMD_SET_LINK_PAYLOAD_LINK_SPEED_100M                    0x02
#define CMD_SET_LINK_PAYLOAD_LINK_SPEED_1000M                   0x04
#define CMD_SET_LINK_PAYLOAD_LINK_SPEED_10G                     0x08
#define CMD_SET_LINK_PAYLOAD_FULL_DUPLEX                        0x0
#define CMD_SET_LINK_PAYLOAD_HALF_DUPLEX                        0x1
#define CMD_SET_LINK_PAYLOAD_PAUSE_CAPS_DISABLE                 0x0
#define CMD_SET_LINK_PAYLOAD_PAUSE_CAPS_ENABLE                  0x1
#define CMD_SET_LINK_PAYLOAD_ASYMMETRIC_PAUSE_CAPS_DISABLE      0x0
#define CMD_SET_LINK_PAYLOAD_ASYMMETRIC_PAUSE_CAPS_ENABLE       0x1
#define CMD_SET_LINK_PAYLOAD_OEM_SETTINGS_DISABLE               0x0
#define CMD_SET_LINK_PAYLOAD_OEM_SETTINGS_ENABLE                0x1

/* GET_CAPABILITIES command payload */
/* 
#define CMD_GET_CAPABILITIES_PAYLOAD_HW_ARB_NOT_SUPPORT                 0x0
#define CMD_GET_CAPABILITIES_PAYLOAD_HW_ARB_SUPPORT                     0x1
#define CMD_GET_CAPABILITIES_PAYLOAD_OS_PRES_NOT_SUPPORT                0x0
#define CMD_GET_CAPABILITIES_PAYLOAD_OS_PRES_SUPPORT                    0x1
#define CMD_GET_CAPABILITIES_PAYLOAD_TX_FLOW_CTRL_NOT_SUPPORT           0x0
#define CMD_GET_CAPABILITIES_PAYLOAD_TX_FLOW_CTRL_SUPPORT               0x1
#define CMD_GET_CAPABILITIES_PAYLOAD_RX_FLOW_CTRL_NOT_SUPPORT           0x0
#define CMD_GET_CAPABILITIES_PAYLOAD_RX_FLOW_CTRL_SUPPORT               0x1
#define CMD_GET_CAPABILITIES_PAYLOAD_ALL_MCAST_ADDR_NOT_SUPPORT         0x0
#define CMD_GET_CAPABILITIES_PAYLOAD_ALL_MCAST_ADDR_SUPPORT             0x1

#define CMD_GET_CAPABILITIES_PAYLOAD_BCASE_ARP_SUPPORT                  0x1
#define CMD_GET_CAPABILITIES_PAYLOAD_BCASE_DHCP_SUPPORT                 0x2
#define CMD_GET_CAPABILITIES_PAYLOAD_BCASE_NETBIOS_SUPPORT              0x1

#define CMD_GET_CAPABILITIES_PAYLOAD_AEN_LINK_STATUS_CHANGE_NOT_SUPPORT     0x0
#define CMD_GET_CAPABILITIES_PAYLOAD_AEN_LINK_STATUS_CHANGE_SUPPORT         0x1
#define CMD_GET_CAPABILITIES_PAYLOAD_AEN_RECONFIG_REQUIRED_NOT_SUPPORT      0x0
#define CMD_GET_CAPABILITIES_PAYLOAD_AEN_RECONFIG_REQUIRED_SUPPORT          0x1
#define CMD_GET_CAPABILITIES_PAYLOAD_AEN_NIC_OS_DRIVER_CHANGE_NOT_SUPPORT   0x0
#define CMD_GET_CAPABILITIES_PAYLOAD_AEN_NIC_OS_DRIVER_CHANGE_SUPPORT       0x1
*/


/* SET_MAC_ADDRESS command payload */
#define CMD_SET_MAC_ADDRESS_PAYLOAD_ADDR_TYPE_UNICAST           0x00
#define CMD_SET_MAC_ADDRESS_PAYLOAD_ADDR_TYPE_MULTICAST         0x80
#define CMD_SET_MAC_ADDRESS_PAYLOAD_DISABLE_MAC_ADDR_FILTER     0x00
#define CMD_SET_MAC_ADDRESS_PAYLOAD_ENABLE_MAC_ADDR_FILTER      0x01

/* ENABLE_BROADCAST command payload */
#define CMD_ENABLE_BROADCAST_PAYLOAD_ARP_ENABLE                 0x1
#define CMD_ENABLE_BROADCAST_PAYLOAD_ARP_DISABLE                0x0
#define CMD_ENABLE_BROADCAST_PAYLOAD_DHCP_ENABLE                0x1
#define CMD_ENABLE_BROADCAST_PAYLOAD_DHCP_DISABLE               0x0
#define CMD_ENABLE_BROADCAST_PAYLOAD_NETBIOS_ENABLE             0x1
#define CMD_ENABLE_BROADCAST_PAYLOAD_NETBIOS_DISABLE            0x0

/* SET_VLAN_FILTERS command payload */
#define CMD_SET_VLAN_FILTERS_PAYLOAD_DISABLE_FILTER             0x0
#define CMD_SET_VLAN_FILTERS_PAYLOAD_ENABLE_FILTER              0x1

/* ENABLE_VLAN command payload */
#define CMD_ENABLE_VLAN_PAYLOAD_VLAN_ONLY                       0x1
#define CMD_ENABLE_VLAN_PAYLOAD_VLAN_NONVLAN                    0x2
#define CMD_ENABLE_VLAN_PAYLOAD_ANYVLAN_NONVLAN                 0x3

/* SET_NCSI_FLOW_CONTROL command payload */
#define CMD_SET_NCSI_FLOW_CONTROL_DISABLE                       0x0
#define CMD_SET_NCSI_FLOW_CONTROL_TX_ENABLE                     0x1
#define CMD_SET_NCSI_FLOW_CONTROL_RX_ENABLE                     0x2
#define CMD_SET_NCSI_FLOW_CONTROL_TX_RX_ENABLE                  0x3

/* AEN_ENABLE command payload */
#define CMD_AEN_ENABLE_PAYLOAD_LINK_STATUS_CHANGE_DISABLE       0x0
#define CMD_AEN_ENABLE_PAYLOAD_LINK_STATUS_CHANGE_ENABLE        0x1
#define CMD_AEN_ENABLE_PAYLOAD_RECONFIG_REQUIRED_DISABLE        0x0
#define CMD_AEN_ENABLE_PAYLOAD_RECONFIG_REQUIRED_ENABLE         0x1
#define CMD_AEN_ENABLE_PAYLOAD_OS_DRIVER_CHANGE_DISABLE         0x0
#define CMD_AEN_ENABLE_PAYLOAD_OS_DRIVER_CHANGE_ENABLE          0x1

/* TBD: GET_LINK_STATUS command payload */


/* NCSI standard response codes */
#define NCSI_RES_CODE_CMD_OK                                    0x00
#define NCSI_RES_CODE_CMD_FAILED                                0x01
#define NCSI_RES_CODE_CMD_INVALID                               0x02
#define NCSI_RES_CODE_CMD_UNSUPPORTED                           0x03

/* NCSI standard reason codes */
#define NCSI_REASON_CODE_NO_ERROR                               0x00
#define NCSI_REASON_CODE_INTERFACE_INIT_REQUIRED                0x01
#define NCSI_REASON_CODE_PARAMETER_INVALID                      0x02
#define NCSI_REASON_CODE_CHNL_NOT_READY                         0x03
#define NCSI_REASON_CODE_PKG_NOT_READY                          0x04
#define NCSI_REASON_CODE_UNKNOWN_CMD_TYPE                       0x05
#define NCSI_REASON_CODE_FEATURE_CAPABILITY_NOT_SUPPORTED       0x0B

/* NCSI Specific reason code */
#define NCSI_SET_MAC_ADDRESS_REASON_CODE_MAC_ADDR_IS_ZERO               0x04
#define NCSI_SET_VLAN_FILTERS_REASON_CODE_VLAN_TAG_IS_INVAILD           0x04
#define NCSI_SET_LINK_REASON_CODE_SET_LINK_HOST_OS_DRIVER_CONFLICT      0x05
#define NCSI_SET_LINK_REASON_CODE_SET_LINK_MEDIA_CONFLICT               0x06
#define NCSI_SET_LINK_REASON_CODE_SET_LINK_PARAMETER_CONFLICT           0x07
#define NCSI_SET_LINK_REASON_CODE_SET_LINK_POWER_MODE_CONFLICT          0x08
#define NCSI_SET_LINK_REASON_CODE_SET_LINK_LINK_SPEED_CONFLICT          0x09
#define NCSI_SET_LINK_REASON_CODE_SET_LINK_CMD_FAIL_HW_ACCESS_ERR       0x0A
#define NCSI_GET_LINK_STATUS_REASON_CODE_LINK_CMD_FAIL_HW_ACCESS_ERR    0x0A


/* The type of ump packet queue state by UMP send command */
enum ncsi_queue_type {
    NCSI_RES_QUEUE,         /* NCSI response packet queue */
    NCSI_AEN_QUEUE,         /* AEN packet queue */
};

/* NCSI software model states */
enum ncsi_states {
    NCSI_STATE_NOT_READY ,
    NCSI_STATE_INITIAL ,
    NCSI_STATE_OPERATIONAL ,
    NCSI_STATE_ERROR,
};

/* NCSI packet header definition */
struct sNCSI_HDR {
    u8 mc_id;
    u8 hdr_rev;
    u8 reserved0;
    u8 cmd_iid;
    u8 cmd;
    u8 chnl_id;
    u8 payload_len_h_nibble;    /* the lower 4 bits is the high nibble of the \
                                   payload length */
    u8 payload_len;             /* should be 12 bits,changed by sonata */
    u32 reserved2;
    u32 reserved3;
    /* The payload start here. */
};


#define NCSI_HLEN           sizeof(struct sNCSI_HDR)        /* 16 bytes */
#define NCSI_CHECKSUM_LEN   (4)


/* NCSI packet response header definition */
struct sNCSI_RES_HDR {
    struct sNCSI_HDR ncsi_hdr;
    u16 response_code;
    u16 reason_code;

    /* Payload data start pointer. Express the checksum if no payload data */
    u8 payload_data;            
};

/* NCSI AEN packet header definition */
struct sNCSI_AEN_HDR {
    struct sNCSI_HDR ncsi_hdr;
    u16 reserved1;
    u8 reserved2;
    u8 aen_type;
};

/* NCSI MAC Information: refer to "Set MAC Address Command" Packet Format */
struct sNCSI_MAC_INFO {
    u8 mac_addr[6];
    u8 mac_num;
    u8 addr_type_enable;
};

/* NCSI Set Link definition: refer to "Set Link" Command Packet Format */
struct sNCSI_SET_LINK {
    u32 link_settings;
    u32 oem_link_settings;
};

/* NCSI Channel Parameter definition: Get Parameters Response Packet Format */
struct sNCSI_CHNL_PARAMETER {
    u32 mac_addr_cnt_flag;
    u32 vlan_tag_cnt_flag;

    struct sNCSI_SET_LINK link_setting;
    u32 bcast_filter_flag;
    u32 config_flag;
    u8 vlan_mode;
    u8 flow_control_enable;
    u16 reserved;
    u32 aen_ctrl;
    /* MAC address Entry */
    /* VLAN entry */
};

/* NCSI Channel Capabilities definition */
struct sNCSI_CHNL_CAPABILITIES {
    u32 capabilities_flags;
    u32 bcast_filter_flag;
    u32 mcast_filter_flag;
    u32 buf_cap;
    u32 aen_ctrl;
    u8  vlan_filter_cnt;
    u8  mixed_filter_cnt;
    u8  mcast_filter_cnt;
    u8  ucast_filter_cnt;
    u16 reserved2;
    u8  vlan_mode;
    u8  chnl_cnt;
};

typedef struct 
{
    u8 chnl_id;                     /* NCSI package id/channel id */
    u8 cmd_iid;                     /* NCSI command Instance ID (1-255) */

    /* NCSI state */
    enum ncsi_states curr_state;

    /* NCSI internal pkg/chnl status */
    u8 pkg_ready;
    u8 pkg_selected;
    u8 chnl_ready;
    u8 chnl_in_init;
    
#if 0
    u8 reserved;                            /* for alignment */
    
    struct skb_queue ncsi_res_quene;        /* NCSI response packet queue */
    struct semaphore ncsi_cmd_send_sem;     /* Ensure only one command \
                                               can send out at a  time */
    struct skb_queue ncsi_aen_quene;        /* NCSI AEN packet queue  */

    /* for AEN thread */
    pid_t aen_pid;  
    
    /* TBD: when user close the NIC, the AEN thread will be kill. \
            But there are some timing issue before kill the AEN thread, \
            so create this flag to disable scan the AEN packet */
    int aen_exit_flag;  
    
    /* use for eth close */
    struct completion aen_exited;   
    
    struct net_device *dev;
    
    NCSI_TX_FUNC ncsi_tx_func;  /* NCSI response packet transmit function */
    
    struct sNCSI_VERSION ncsi_version;
#endif
    
    struct sNCSI_CHNL_CAPABILITIES ncsi_chnl_caps;
    struct sNCSI_MAC_INFO ncsi_mac_info;

#if 0
    u32 ncsi_vlan_mode;
    u32 ncsi_flow_ctrl;

    u32 ncsi_hardware_arb;

    struct sNCSI_VLAN_INFO ncsi_vlan_info;
    u32 ncsi_aen_ctrl;
    struct sNCSI_GET_LINK_STATUS ncsi_link_status;
#endif
    
    struct sNCSI_CHNL_PARAMETER ncsi_chnl_parameters;
} sNCSI_IF_Info;


/* NC-SI init */
int ncsi_init(void);
int ncsi_open(u8 chnl_id, bd_t* bis);
int ncsi_close(u8 chnl_id);
int ncsi_receive(volatile uchar * inpkt, int len);


#endif /* __NCSI_H__ */
