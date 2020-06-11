#ifndef __AMI_CMD_R2C_H__
#define __AMI_CMD_R2C_H__

/* U-boot's cmd function enter remote recovery console session */
extern int  do_r2c (cmd_tbl_t *, int, int, char *[]);

U_BOOT_CMD(				
	r2c,	1,	0,	do_r2c,				
	"r2c     - Enter AMI's Remote Recovery Console Session\n",	
	"    - Enter AMI's Remote Recovery Console Session\n"		
);


/* Actual function implementing r2c protocol*/
extern int  rconsole(void);	

#endif	
