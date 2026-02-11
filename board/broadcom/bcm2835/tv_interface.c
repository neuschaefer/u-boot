#include <common.h>
#include <asm/arch/vcio.h>
#include <asm/arch/ipc.h>
#include <asm/io.h>

#include "tv_interface.h"

/*******************************************************************/
/* tv interface -- control tv output */
/*******************************************************************/
static uint32_t tvif_id = 0;
static uint32_t tvif_base = 0;

union fourcc_union
{
	uint32_t fourcc_num;
	char fourcc_char[4];
};

int tvif_init(void) {
    // Check VC TV Interface IPC.
    union fourcc_union fourcc;

    if (tvif_base == 0) {
        memcpy(fourcc.fourcc_char, "TVIF", 4);
        tvif_base = ipc_get_service_addr(fourcc.fourcc_num, &tvif_id);
        printf("Starting tv interface, base = %x id=%d\n", tvif_base, tvif_id);
        if (!tvif_base) {
            printf("Error: Failed to get tvif base addr\n");
            return -1;
        }
    }
    return 0;
}

void tvif_shutdown(void) {
    tvif_id = 0;
    tvif_base = 0;
}

// Switch between HDMI and SDTV every 10 seconds
int tvif_set_output(TV_INTF_CTRL_T mode)
{
    int change = 0;
    switch (mode)
    {
        case TV_INTF_CTRL_HDMI:
        case TV_INTF_CTRL_SDTV:
		case TV_INTF_CTRL_AUTO:
        case TV_INTF_CTRL_OFF:
            writel(mode, tvif_base + TV_INTF_OUTPUT_CTRL_OFFSET);
            change = 1;
            break;

        default:
            printf("TV Interface: invalid mode.\n");
            break;
    }

    if (change) {
        //tv_intf_vc_arm_lock();
        writel(TV_INTF_OUTPUT_CHANGE, tvif_base + TV_INTF_CTRL_CHANGE_OFFSET);
        //tv_intf_vc_arm_unlock();
        ipc_notify_vc_event(tvif_id);
    }
    return change ? 0 : 1;
}

int tvif_set_hdmi_resolution(HDMI_CEA_RES_CODE_T mode)
{
    uint32_t changes = 0;
    uint32_t status = 0;

    if (HDMI_RES_GROUP_CEA != readl(tvif_base + TV_INTF_HDMI_RES_GROUP_CTRL_OFFSET))
    {
        writel(HDMI_RES_GROUP_CEA, tvif_base + TV_INTF_HDMI_RES_GROUP_CTRL_OFFSET);
        changes |= TV_INTF_HDMI_RES_GROUP_CHANGE;
    }

    if(mode >= HDMI_CEA_VGA && mode <= HDMI_CEA_480i240H)
    {
        writel(mode, tvif_base + TV_INTF_HDMI_RES_CODE_CTRL_OFFSET);
        changes |= TV_INTF_HDMI_RES_CODE_CHANGE;

        //test_tv_intf_vc_arm_lock();
        writel(changes, tvif_base + TV_INTF_CTRL_CHANGE_OFFSET);
        //test_tv_intf_vc_arm_unlock();
        ipc_notify_vc_event(tvif_id);
    } else {
        printf("TV Interface: resolution not supported.\n");
        status = 1;
    }
    return status;
}


