
/*
 * Copyright 2005 Sun Microsystems, Inc.  All rights reserved.
 * Use is subject to license terms.
 */

#ifndef __CMD_H__
#define __CMD_H__

extern int do_bootm (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);
extern int do_bootelf (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);
extern int do_go (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);
extern int do_jffs2_chpart (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);
extern int do_jffs2_fsload (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);

#endif
