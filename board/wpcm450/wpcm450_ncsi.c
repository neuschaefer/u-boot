/*
 *  Copied from Linux Monitor (LiMon) - Networking.
 *
 *  Copyright 1994 - 2000 Neil Russell.
 *  (See License)
 *  Copyright 2000 Roland Borde
 *  Copyright 2000 Paolo Scaffardi
 *  Copyright 2000-2002 Wolfgang Denk, wd@denx.de
 */

/*
 * General Desription:
 *
 * The user interface supports commands for BOOTP, RARP, and TFTP.
 * Also, we support ARP internally. Depending on available data,
 * these interact as follows:
 *
 */


#include <common.h>
#include <watchdog.h>
#include <command.h>
#include <net.h>
#include "wpcm450_ncsi.h"


#if 0
#define NCSI_DEBUG   1
#endif


/* enable to print out message */
#ifdef NCSI_DEBUG
#define DEBUGP(format, args...) printf("NCSI: " format, ## args)
#else
#define DEBUGP(format, args...)
#endif


#define NCSI_RECEIVE_RES_PACKET_OK          0
#define NCSI_RECEIVE_RES_PACKET_INVALID     1


sNCSI_IF_Info ncsi_if;
u8 recv_res_cmd = 0;


#ifdef NCSI_DEBUG
static char *ncsi_cmd_str[] = 
{
    "CMD_CLEAR_INITIAL_STATE",
    "CMD_SELECT_PACKAGE",
    "CMD_DESELECT_PACKAGE",
    "CMD_ENABLE_CHANNEL",
    "CMD_DISABLE_CHANNEL",
    "CMD_RESET_CHANNEL",
    "CMD_ENABLE_CHANNEL_NETWORK_TX",
    "CMD_DISABLE_CHANNEL_NETWORK_TX",
    "CMD_AEN_ENABLE",
    "CMD_SET_LINK",
    "CMD_GET_LINK_STATUS",
    "CMD_SET_VLAN_FILTERS",
    "CMD_ENABLE_VLAN",
    "CMD_DISABLE_VLAN",
    "CMD_SET_MAC_ADDRESS",
    "CMD_RESERVED_0xF",
    "CMD_ENABLE_BROADCAST_FILTERING",
    "CMD_DISABLE_BROADCAST_FILTERING",
    "CMD_ENABLE_GLOBAL_MULTICAST_FILTERING",
    "CMD_DISABLE_GLOBAL_MULTICAST_FILTERING ",
    "CMD_SET_NCSI_FLOW_CONTROL",
    "CMD_GET_VERSION_ID",
    "CMD_GET_CAPABILITIES",
    "CMD_GET_PARAMETERS",
    "CMD_GET_CONTROLLER_PACKET_STATISTICS",
    "CMD_GET_NCSI_STATISTICS",
    "CMD_GET_NCSI_PASSTHROUGH_STATISTICS",
};
#endif

#if 0
static char *ncsi_res_code_str[] = 
{
    "NCSI_RES_CODE_CMD_OK",
    "NCSI_RES_CODE_CMD_FAILED",
    "NCSI_RES_CODE_CMD_INVALID",
    "NCSI_RES_CODE_CMD_UNSUPPORTED",
};

static char *ncsi_reason_code_str[] = 
{
    "NCSI_REASON_CODE_NO_ERROR",
    "NCSI_REASON_CODE_INTERFACE_INIT_REQUIRED",
    "NCSI_REASON_CODE_PARAMETER_INVALID",
    "NCSI_REASON_CODE_CHNL_NOT_READY",
    "NCSI_REASON_CODE_PKG_NOT_READY",
    "NCSI_REASON_CODE_UNKNOWN_CMD_TYPE",
    "NCSI_REASON_CODE_FEATURE_CAPABILITY_NOT_SUPPORTED",
};
#endif


int ncsi_send_packet (u8 cmd, u8 *payload, u16 payload_len);
static int ncsi_cmd_disable_channel_network_tx(void);
static int ncsi_cmd_clear_initial_state(void);
static int ncsi_send_and_wait(u8 cmd, u8 *payload, u16 payload_len);
static int ncsi_cmd_set_mac_address(void);
static int ncsi_cmd_enable_broadcast_filtering(void);
static int ncsi_cmd_enable_channel_network_tx(void);
static int ncsi_cmd_enable_channel(void);


/* NC-SI init */
int ncsi_init(void)
{
    DEBUGP("ncsi_initn");
    
    ncsi_if.curr_state = NCSI_STATE_NOT_READY;
    
    /* when NIC power up, the package will ready */
    ncsi_if.pkg_ready = 1;  
    ncsi_if.pkg_selected = 0;
    ncsi_if.chnl_ready = 0;
    
    /* when NIC started-up and initialized, the channel was in init state */
    ncsi_if.chnl_in_init = 1;
    
    ncsi_if.chnl_id = NCSI_CHNL_ID_UNAVAILABLE;
    
    /* the comand IID begins with 1 */
    ncsi_if.cmd_iid = 1;
    
    return 0;
}


int ncsi_open(u8 chnl_id, bd_t* bis)
{
    int err;
    
    DEBUGP("ncsi_open, chnl_id=%x\n", chnl_id);
    
    ncsi_if.chnl_id = chnl_id;  
    
    /* 2. Discover and Get Capabilities for each Channel in the Package */
    /* Use the real channel id to clear initial state */
    err = ncsi_cmd_clear_initial_state();
    if (err != 0)
    {
        DEBUGP("send ncsi_cmd_clear_initial_state command Fail\n");
        return err;
    }
    
    /* 3. Initialize each Channel in the Package */
    /* TBD: support only one MAC addr filter now. Maybe can expand */
    memcpy((u8 *)ncsi_if.ncsi_mac_info.mac_addr, (u8 *)bis->bi_enetaddr, 6);
    
    ncsi_if.ncsi_mac_info.mac_num = 1;
    
    ncsi_if.ncsi_mac_info.addr_type_enable 
    = CMD_SET_MAC_ADDRESS_PAYLOAD_ADDR_TYPE_UNICAST 
      | CMD_SET_MAC_ADDRESS_PAYLOAD_ENABLE_MAC_ADDR_FILTER;
    
    err = ncsi_cmd_set_mac_address();
    
    if (err != 0)
    {
        DEBUGP("send ncsi_cmd_set_mac_address command Fail\n");
        return err;
    }  
    
    ncsi_if.ncsi_chnl_caps.bcast_filter_flag = 0xf;
    ncsi_if.ncsi_chnl_parameters.bcast_filter_flag = 0xf;
    err = ncsi_cmd_enable_broadcast_filtering();
    
    if (err != 0)
    {
        DEBUGP("send ncsi_cmd_enable_broadcast_filtering command Fail\n");
        return err;
    }
    
    err = ncsi_cmd_enable_channel_network_tx();
    if (err != 0)
    {
        DEBUGP("send ncsi_cmd_enable_channel_network_tx command Fail\n");
        return err;
    }
    
    /* 5. Start Pass-through packet and AEN operation on the channels */
    err = ncsi_cmd_enable_channel();
    if (err != 0)
    {
        DEBUGP("send ncsi_cmd_enable_channel command Fail\n");
        return err;
    }
    
    return 0;
}


int ncsi_close(u8 chnl_id)
{
    int res;
    
    DEBUGP("ncsi_close, chnl_id=%x\n", chnl_id);
    
    ncsi_if.chnl_id = chnl_id;
    
    res = ncsi_cmd_disable_channel_network_tx();
    
    return res;
}


static int ncsi_cmd_disable_channel_network_tx(void)
{
    int res;
    
    /* Issue the command */
    res = ncsi_send_and_wait(CMD_DISABLE_CHANNEL_NETWORK_TX, 
                             NULL, 
                             CMD_DISABLE_CHANNEL_NETWORK_TX_PAYLOAD_LEN);
    
    return res;
}


static int ncsi_cmd_clear_initial_state(void)
{
    int res;

    /* Issue the command */
    res = ncsi_send_and_wait(CMD_CLEAR_INITIAL_STATE, 
                             NULL, 
                             CMD_CLEAR_INITIAL_STATE_PAYLOAD_LEN);
    
    if (res == NCSI_RECEIVE_RES_PACKET_OK)
    {
        /* Once Select Package success, change the state to the Initial state */
        ncsi_if.curr_state = NCSI_STATE_INITIAL;
        ncsi_if.chnl_in_init = 0;
        ncsi_if.chnl_ready = 1;
    }
    
    return res;
}


static int ncsi_cmd_set_mac_address(void)
{
    int res, i;
    u8 payload[CMD_SET_MAC_ADDRESS_PAYLOAD_LEN];
    
    for(i = 0; i < 6; i++) 
    {
        payload[i] = ncsi_if.ncsi_mac_info.mac_addr[i];
    }
    
    payload[6] = ncsi_if.ncsi_mac_info.mac_num;
    payload[7] = ncsi_if.ncsi_mac_info.addr_type_enable;
    
    /* Issue the command */
    res = ncsi_send_and_wait(CMD_SET_MAC_ADDRESS, 
                             payload, 
                             CMD_SET_MAC_ADDRESS_PAYLOAD_LEN);
    
    return res;
}


static int ncsi_cmd_enable_broadcast_filtering(void)
{
    int res;
    u32 payload;
    
    payload = htonl(ncsi_if.ncsi_chnl_caps.bcast_filter_flag);
    
    /* Issue the command */
    res = ncsi_send_and_wait(CMD_ENABLE_BROADCAST_FILTERING, 
                             (u8 *) &payload, 
                             CMD_ENABLE_BROADCAST_FILTERING_PAYLOAD_LEN);
    
    return res;
}


static int ncsi_cmd_enable_channel_network_tx(void)
{
    int res;
    
    /* Issue the command */
    res = ncsi_send_and_wait(CMD_ENABLE_CHANNEL_NETWORK_TX, 
                             NULL, 
                             CMD_ENABLE_CHANNEL_NETWORK_TX_PAYLOAD_LEN);
    
    return res;
}


static int ncsi_cmd_enable_channel(void)
{
    int res;

    /* Issue the command */
    res = ncsi_send_and_wait(CMD_ENABLE_CHANNEL, 
                             NULL, 
                             CMD_ENABLE_CHANNEL_PAYLOAD_LEN);

    if (res == NCSI_RECEIVE_RES_PACKET_OK)
    {
        ncsi_if.curr_state = NCSI_STATE_OPERATIONAL;
    }
    return res;
}


static int ncsi_send_and_wait(u8 cmd, u8 *payload, u16 payload_len)
{
    int res;
    u8 retrycnt = 0;
    u8 res_cmd = (cmd | NCSI_RES_PACKET_MASK);
    
    do
    {
        /* Issue the command */
        res = ncsi_send_packet(cmd, payload, payload_len);
        
        if (res <= 0)
        {
            /* proces the command */
            DEBUGP("%s: send command fail \n",
            ncsi_cmd_str[CMD_CLEAR_INITIAL_STATE]);
            
            return res;
        }
        
        udelay(50000);
        
        do
        {
            if (eth_rx() > 0)
            {
                if (res_cmd == recv_res_cmd)
                {
                    DEBUGP("IS_VALID_NCSI_RES_PACKET\n");
                    
                    /* add the IID */
                    ncsi_if.cmd_iid++;
                    
                    if (0xFF == ncsi_if.cmd_iid)
                    {
                        ncsi_if.cmd_iid = 1;
                    }
                    
                    return (NCSI_RECEIVE_RES_PACKET_OK);
                }
                else
                {
                    DEBUGP("NCSI_RECEIVE_RES_PACKET_INVALID\n");
                }
            }
        }
        while (++retrycnt < MAX_CMD_RETRY_COUNT);   /* 2 retry */
    }
    while (++retrycnt < MAX_CMD_RETRY_COUNT);       /* 2 retry */
    
    DEBUGP("eth_rx() timeout : can't get response \n");
    
    return 0;
}


int ncsi_send_packet (u8 cmd, u8 *payload, u16 payload_len)
{
    volatile uchar *pkt;
    struct sNCSI_HDR *ncsi_hdr;
    u8 *ncsi_payload_start;
    unsigned int pkt_len ;  
    
#define PAYLOAD_PAD(len)    (4 - (len % 4))
    
    DEBUGP ("NCSI request command 0x%x\n", cmd);
    
    if (!NetTxPacket) 
    {
        DEBUGP ("NetTxPacket == NULL\n");
        return -1;
    }
    
    pkt_len = ETH_HLEN + NCSI_HLEN + 
              payload_len + PAYLOAD_PAD(payload_len) + NCSI_CHECKSUM_LEN;
    
    if (pkt_len < PKTMINSIZE)
    {
        pkt_len = PKTMINSIZE;
    }
    
    pkt = NetTxPacket;
    
    memset((void*) pkt, 0, pkt_len);
    
    pkt += NetSetEther (pkt, NetBcastAddr, NCSI_ETHER_TYPE);
    
    /* build NCSI header */
    ncsi_hdr = (struct sNCSI_HDR *) pkt;
    
    ncsi_hdr->mc_id = NCSI_MC_ID;
    ncsi_hdr->hdr_rev = NCSI_HEADER_REVISION;
    ncsi_hdr->cmd_iid = ncsi_if.cmd_iid;
    ncsi_hdr->cmd = cmd;
    ncsi_hdr->chnl_id = ncsi_if.chnl_id;
    ncsi_hdr->payload_len = payload_len;
    
    /* build NCSI payload */
    ncsi_payload_start = (u8 *) ((u32)ncsi_hdr + NCSI_HLEN);
    
    if (0 != ncsi_hdr->payload_len)
    {
        /* the payload is always in big-endian byte order */
        memcpy(ncsi_payload_start, payload, ncsi_hdr->payload_len);
    }
    
    return eth_send (NetTxPacket, pkt_len);
}


int ncsi_receive(volatile uchar * inpkt, int len)
{
    /* need to check rx packet content */
    
    recv_res_cmd = inpkt[ETH_HLEN + 5 - 1];
    
    DEBUGP ("NCSI receive response code =  0x%x\n", recv_res_cmd);
    
    return 0;   
}
