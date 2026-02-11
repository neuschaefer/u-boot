/*****************************************************************************
*
* Kona generic clock framework
*
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
* Unless you and Broadcom execute a separate written software license
* agreement governing use of this software, this software is licensed to you
* under the terms of the GNU General Public License version 2, available at
* http://www.broadcom.com/licenses/GPLv2.php (the "GPL").
*
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a
* license other than the GPL, without Broadcom's express prior written
* consent.
*****************************************************************************/

#include <asm/io.h>

#include <asm/kona-common/clk.h>
#include <asm/kona-common/clock.h>

#include <common.h>

#include <asm/arch/brcm_rdb_kproc_clk_mgr_reg.h>

#define abs(value) (((value) < 0) ? ((value)*-1) : (value))

int clk_debug;

static struct clk_lookup* clk_tbl;
static unsigned int clk_tbl_array_size;

void init_clock_framework(void)
{
	get_clock_array(&clk_tbl, &clk_tbl_array_size);
}


int clk_enable(struct clk *clk)
{
	int ret = 0;

	if (!clk->ops || !clk->ops->enable)
		return -1;

	/* enable parent clock first */
	if (clk->parent)
		ret = clk_enable(clk->parent);

	if (ret)
		return ret;

	if (!clk->use_cnt) {
		clk->use_cnt++;
		ret = clk->ops->enable(clk, 1);
	}

	return ret;
}

void clk_disable(struct clk *clk)
{
	if (!clk->ops || !clk->ops->enable)
		return;

	if (clk->use_cnt) {
		clk->use_cnt--;
		clk->ops->enable(clk, 0);
	}

	/* disable parent */
	if (clk->parent)
		clk_disable(clk->parent);
}

unsigned long clk_get_rate(struct clk *clk)
{
	unsigned long rate;

	if (!clk || !clk->ops || !clk->ops->get_rate)
		return 0;

	rate = clk->ops->get_rate(clk);
	return rate;
}

long clk_round_rate(struct clk *clk, unsigned long rate)
{
	unsigned long actual;

	if (!clk || !clk->ops || !clk->ops->round_rate)
		return -1;

	if (clk->use_cnt)
		return -1;

	actual = clk->ops->round_rate(clk, rate);

	return actual;
}

int clk_set_rate(struct clk *clk, unsigned long rate)
{
	int ret;

	if (!clk || !clk->ops || !clk->ops->set_rate)
		return -1;

	if (clk->use_cnt)
		return -1;

	ret = clk->ops->set_rate(clk, rate);

	return 0;
}

struct clk *clk_get_parent(struct clk *clk)
{
	struct clk *parent;

	if (!clk)
		return 0;

	parent = clk->parent;
	return parent;
}

int clk_set_parent(struct clk *clk, struct clk *parent)
{
	struct clk *old_parent;

	if (!clk || !parent || !clk->ops || !clk->ops->set_parent)
		return -1;

	/* if more than one user, parent is not allowed */
	if (clk->use_cnt > 1)
		return -1;

	if (clk->parent == parent)
		return 0;

	old_parent = clk->parent;
	clk->ops->set_parent(clk, parent);

	/* if clock is active */
	if (clk->use_cnt != 0) {
		clk->use_cnt--;
		/* enable clock with the new parent */
		clk_enable(clk);
		/* disable the old parent */
		clk_disable(old_parent);
	}

	return 0;
}

static unsigned long common_get_rate(struct clk *c)
{
	if (c->parent && c->div)
		c->rate = c->parent->ops->get_rate(c->parent) / c->div;

	return c->rate;
}

static unsigned long common_round_rate(struct clk *c, unsigned long rate)
{
	int i, ind = 0;
	unsigned long diff;

	diff = rate;

	if (!c->parent)
		return c->rate;

	for (i=0; i<c->src->total;i++) {
		unsigned long new_rate, div;
		/* round to the new rate */
		div = c->src->parents[i]->rate / rate;
		if(div == 0)
			div = 1;
		new_rate = c->src->parents[i]->rate/div;
		/* get the min diff */
		if(abs(new_rate-rate) < diff) {
			diff = abs(new_rate-rate);
			ind = i;
		}
	}
	return c->src->parents[ind]->rate;
}

static int common_set_rate(struct clk *c, unsigned long rate)
{
	int i, ind = 0;
	unsigned long diff;

	diff = rate;

	if (!c->parent) {
		return c->rate;
	}

	for (i=0; i<c->src->total;i++) {
		unsigned long new_rate, div;
		/* round to the new rate */
		div = c->src->parents[i]->rate / rate;
		new_rate = c->src->parents[i]->rate/div;
		/* get the min diff */
		if(new_rate-rate < diff) {
			diff = new_rate-rate;
			ind = i;
			c->div = div;
			c->rate = new_rate;
		}
	}
	return 0;
}

static int common_set_parent(struct clk *c, struct clk *parent)
{
	int i;

	if(!c->parent)
		return -1;

	for (i=0; i<c->src->total; i++) {
		if (is_same_clock(parent, c->src->parents[i]))
			return 0;
	}
	return -1;
}

static inline void __proc_clk_enable_access (void *base, int enable)
{
	if (enable)
		writel(CLK_WR_ACCESS_PASSWORD, base + KPROC_CLK_MGR_REG_WR_ACCESS_OFFSET);
	else
		writel(0, base + KPROC_CLK_MGR_REG_WR_ACCESS_OFFSET);
}

/* post divider for policy 6 and 7*/
static inline void __proc_clk_set_policy_div(void *base, int policy, int div)
{
	unsigned long offset;

	if (policy==6)
		offset = KPROC_CLK_MGR_REG_PLLARMC_OFFSET;
	else if( policy==7)
		offset = KPROC_CLK_MGR_REG_PLLARMCTRL5_OFFSET;

	clk_dbg ("policy %d div %d\n", policy, div);
	writel(div, base+offset);
	writel((div<<KPROC_CLK_MGR_REG_PLLARMC_PLLARM_MDIV_SHIFT)|
		KPROC_CLK_MGR_REG_PLLARMC_PLLARM_LOAD_EN_MASK,
		base+offset);
	writel(div, base+offset);
}

static inline void __proc_clk_dump_register (void *base)
{
	clk_dbg ("policy freq      0x%08x\n", readl(base+KPROC_CLK_MGR_REG_POLICY_FREQ_OFFSET));
	clk_dbg ("policy_ctrl      0x%08x\n", readl(base+KPROC_CLK_MGR_REG_POLICY_CTL_OFFSET));
	clk_dbg ("arma             0x%08x\n", readl(base+KPROC_CLK_MGR_REG_PLLARMA_OFFSET));
	clk_dbg ("armb             0x%08x\n", readl(base+KPROC_CLK_MGR_REG_PLLARMB_OFFSET));
	clk_dbg ("armc             0x%08x\n", readl(base+KPROC_CLK_MGR_REG_PLLARMC_OFFSET));
	clk_dbg ("armctrl3         0x%08x\n", readl(base+KPROC_CLK_MGR_REG_PLLARMCTRL3_OFFSET));
	clk_dbg ("armctrl5         0x%08x\n", readl(base+KPROC_CLK_MGR_REG_PLLARMCTRL5_OFFSET));
	clk_dbg ("lvm_en           0x%08x\n", readl(base+KPROC_CLK_MGR_REG_LVM_EN_OFFSET));
	clk_dbg ("arm_div          0x%08x\n", readl(base+KPROC_CLK_MGR_REG_ARM_DIV_OFFSET));
}

/* Proc clocks */
static int proc_clk_enable(struct clk *c, int enable)
{
	int ret=0;
	return ret;
}

static unsigned int __proc_clk_get_vco_rate(void *base)
{
	unsigned long xtal = clock_get_xtal();
	unsigned int ndiv_int, ndiv_frac, vco_rate;

	ndiv_int = (readl(base + KPROC_CLK_MGR_REG_PLLARMA_OFFSET)&KPROC_CLK_MGR_REG_PLLARMA_PLLARM_NDIV_INT_MASK)
		>> KPROC_CLK_MGR_REG_PLLARMA_PLLARM_NDIV_INT_SHIFT;
	ndiv_frac = (readl(base + KPROC_CLK_MGR_REG_PLLARMB_OFFSET) & KPROC_CLK_MGR_REG_PLLARMB_PLLARM_NDIV_FRAC_MASK)
		>> KPROC_CLK_MGR_REG_PLLARMB_PLLARM_NDIV_FRAC_SHIFT;

	vco_rate = ndiv_int * xtal;

	vco_rate += (unsigned long) (uint64_t) (((uint64_t)ndiv_frac * (uint64_t)xtal) >> 20);

	clk_dbg ("xtal %d, int %d, frac %d, vco %d\n", (int)xtal, ndiv_int, ndiv_frac, vco_rate);
	return vco_rate;
}

static unsigned int __proc_clk_get_rate(void *base)
{
	unsigned int vco_rate = __proc_clk_get_vco_rate (base);
	int div = (readl(base+KPROC_CLK_MGR_REG_PLLARMCTRL5_OFFSET)& KPROC_CLK_MGR_REG_PLLARMCTRL5_PLLARM_H_MDIV_MASK)
		>> KPROC_CLK_MGR_REG_PLLARMCTRL5_PLLARM_H_MDIV_SHIFT;

	return vco_rate /div;
}

static int proc_clk_set_rate(struct clk *c, unsigned long rate)
{
	struct proc_clock *proc_clk = to_proc_clk(c);
	unsigned long old_rate, vco_rate;
	void *base = (void*) proc_clk->proc_clk_mgr_base;
	int ret = 0, div = 2;

	/* enable  access */
	__proc_clk_enable_access (base, 1);


	old_rate = __proc_clk_get_rate(base);

	vco_rate = __proc_clk_get_vco_rate (base);

	/* to get minimium clock >= desired_rate */
	div = vco_rate/rate;
	div = min (max (2, div), 255);

	c->rate = vco_rate / div;

	/* switch to normal freq of policy 7 */
	__proc_clk_set_policy_div (base, 7, div);

	if (ret)
		goto err;

	__proc_clk_dump_register (base);

	__proc_clk_enable_access (base, 0);

	return ret;
err:
	__proc_clk_enable_access (base, 0);
	return ret;
}
static unsigned long proc_clk_get_rate(struct clk *c)
{
	unsigned int ret = 0;
	struct proc_clock *proc_clk = to_proc_clk(c);
	void *base = (void*) proc_clk->proc_clk_mgr_base;

	c->rate = __proc_clk_get_rate(base);
	ret = c->rate;
	return ret;
}

static unsigned long proc_clk_round_rate(struct clk *c, unsigned long rate)
{
	unsigned long ret = rate;
	return ret;
}

struct clk_ops proc_clk_ops = {
	.enable		=	proc_clk_enable,
	.set_rate	=	proc_clk_set_rate,
	.get_rate	=	proc_clk_get_rate,
	.round_rate	=	proc_clk_round_rate,
	.set_parent	=	common_set_parent,
};

static int peri_clk_enable(struct clk *c, int enable)
{
	int ret=0, reg;
	struct peri_clock *peri_clk = to_peri_clk(c);
	void *base = (void*) peri_clk->ccu_clk_mgr_base;

	/* enable access */
	writel(CLK_WR_ACCESS_PASSWORD, base + peri_clk->wr_access_offset);

	if(enable) {
		clk_dbg("%s %s set rate %lu div %lu ind %d parent %lu\n", __func__, c->name,
			c->rate, c->div, c->src->sel, c->parent->rate);

		/* clkgate */
		reg = readl(base + peri_clk->clkgate_offset);
		reg |= peri_clk->clk_en_mask;
		writel(reg, base + peri_clk->clkgate_offset);

		/* div and pll select */
		reg = ((c->div-1) << (peri_clk->div_shift + peri_clk->div_dithering));
		reg |= (c->src->sel << peri_clk->pll_select_shift);

		writel(reg, base + peri_clk->div_offset);

		/* trigger */
		writel(peri_clk->trigger_mask, base + peri_clk->div_trig_offset);
		while(readl(base + peri_clk->div_trig_offset) & peri_clk->trigger_mask);

		/* wait for running */
		while(! (readl(base + peri_clk->clkgate_offset) & peri_clk->stprsts_mask));
	}
	else {
		clk_dbg("%s disable clock %s\n", __func__, c->name);

		/* clkgate */
		reg = readl(base + peri_clk->clkgate_offset);
		reg &= ~peri_clk->clk_en_mask;
		writel(reg, base + peri_clk->clkgate_offset);

		/* wait for stop */
		while((readl(base + peri_clk->clkgate_offset) & peri_clk->stprsts_mask));
	}

	/* disable access */
	writel(0, base + peri_clk->wr_access_offset);

	return ret;
}

static int peri_clk_set_rate(struct clk *c, unsigned long rate)
{
	int ret = 0;
	int i, ind = 0;
	unsigned long diff;
	unsigned long new_rate=0, div=1;

	diff = rate;

	for (i=0; i<c->src->total;i++) {
		/* round to the new rate */
		div = c->src->parents[i]->rate / rate;
		if(div==0)
			div = 1;
		new_rate = c->src->parents[i]->rate/div;

		/* get the min diff */
		if(abs(new_rate-rate) < diff) {
			diff = abs(new_rate-rate);
			c->src->sel = i;
			c->parent = c->src->parents[i];
			c->rate = new_rate;
			c->div = div;
			ind = i;
		}
	}

	clk_dbg("%s %s set rate %lu div %lu ind %d parent %lu\n", __func__, c->name,
		c->rate, c->div, ind, c->parent->rate);
	return ret;
}

static unsigned long peri_clk_get_rate(struct clk *c)
{
	struct peri_clock *peri_clk = to_peri_clk(c);
	void *base = (void*) peri_clk->ccu_clk_mgr_base;
	int sel, div;

	sel = (readl(base + peri_clk->div_offset) & peri_clk->pll_select_mask)
		>> peri_clk->pll_select_shift;

	div = ((readl(base + peri_clk->div_offset) & peri_clk->div_mask)
		>> peri_clk->div_shift);
	div = (div >> peri_clk->div_dithering) + 1;

	c->src->sel = sel;
	c->parent = c->src->parents[sel];
	c->div = div;
	c->rate = c->parent->rate / c->div;
	clk_dbg("%s src %lu sel %d div %d rate %lu\n",__func__, c->parent->rate, sel, div, c->rate);

	return c->rate;
}

struct clk_ops peri_clk_ops = {
	.enable		=	peri_clk_enable,
	.set_rate	=	peri_clk_set_rate,
	.get_rate	=	peri_clk_get_rate,
	.round_rate	=	common_round_rate,
	.set_parent	=	common_set_parent,
};

static int ccu_clk_enable(struct clk *c, int enable)
{
	struct ccu_clock *ccu_clk = to_ccu_clk(c);
	void *base = (void*) ccu_clk->ccu_clk_mgr_base;
	int reg, ret = 0;

	if (!enable)
		return -1;	/* CCU clock cannot shutdown */

	/* enable access */
	writel(CLK_WR_ACCESS_PASSWORD, base + ccu_clk->wr_access_offset);

	/* config enable for policy engine */
	writel(1, base + ccu_clk->lvm_en_offset);
	while (readl(base + ccu_clk->lvm_en_offset) & 1);

	/* freq ID */
	if (!ccu_clk->freq_bit_shift)
		ccu_clk->freq_bit_shift = 8;
	reg = ccu_clk->freq_id |
		(ccu_clk->freq_id << (ccu_clk->freq_bit_shift)) |
		(ccu_clk->freq_id << (ccu_clk->freq_bit_shift * 2)) |
		(ccu_clk->freq_id << (ccu_clk->freq_bit_shift * 3));
	writel(reg, base + ccu_clk->policy_freq_offset);

	/* enable all clock mask */
	writel(0x7fffffff, base + ccu_clk->policy0_mask_offset);
	writel(0x7fffffff, base + ccu_clk->policy1_mask_offset);
	writel(0x7fffffff, base + ccu_clk->policy2_mask_offset);
	writel(0x7fffffff, base + ccu_clk->policy3_mask_offset);

	if ( ccu_clk->num_policy_masks > 1 )
	{
		writel(0x7fffffff, base + ccu_clk->policy0_mask1_offset);
		writel(0x7fffffff, base + ccu_clk->policy1_mask1_offset);
		writel(0x7fffffff, base + ccu_clk->policy2_mask1_offset);
		writel(0x7fffffff, base + ccu_clk->policy3_mask1_offset);
	}

	/* start policy engine */
	reg = readl(base + ccu_clk->policy_ctl_offset);
	reg |= 5;
	writel(reg, base + ccu_clk->policy_ctl_offset);
	while (readl(base + ccu_clk->policy_ctl_offset) & 1);

	/* disable access */
	writel(0, base + ccu_clk->wr_access_offset);

	return ret;
}

static unsigned long ccu_clk_get_rate(struct clk *c)
{
	struct ccu_clock *ccu_clk = to_ccu_clk(c);
	c->rate = ccu_clk->freq_tbl[ccu_clk->freq_id];
	return 	c->rate;
}

struct clk_ops ccu_clk_ops = {
	.enable		=	ccu_clk_enable,
	.get_rate	=	ccu_clk_get_rate,
};

/* bus clocks */
static int bus_clk_enable(struct clk *c, int enable)
{
	struct bus_clock *bus_clk = to_bus_clk(c);
	void *base = (void*) bus_clk->ccu_clk_mgr_base;
	int reg, ret = 0;

	/* enable access */
	writel(CLK_WR_ACCESS_PASSWORD, base + bus_clk->wr_access_offset);

	/* enable gating */
	reg = readl(base + bus_clk->clkgate_offset);
	if (!!(reg&bus_clk->stprsts_mask) == !!enable)
		clk_dbg ("%s already %s\n", c->name, enable?"enabled":"disabled");
	else if (enable) {
		reg |= bus_clk->hw_sw_gating_mask;
		reg |= bus_clk->clk_en_mask;
		writel(reg, base + bus_clk->clkgate_offset);
		while(! (readl(base + bus_clk->clkgate_offset) & bus_clk->stprsts_mask));
	}
	else {
		reg |= bus_clk->hw_sw_gating_mask;
		reg &= ~bus_clk->clk_en_mask;
		writel(reg, base + bus_clk->clkgate_offset);
		while(readl(base + bus_clk->clkgate_offset) & bus_clk->stprsts_mask);
	}

	/* disable access */
	writel(0, base + bus_clk->wr_access_offset);

	return ret;
}

static unsigned long bus_clk_get_rate(struct clk *c)
{
	struct bus_clock *bus_clk = to_bus_clk(c);
	struct ccu_clock *ccu_clk;

	ccu_clk= to_ccu_clk(c->parent);

	c->rate = bus_clk->freq_tbl[ccu_clk->freq_id];
	c->div = ccu_clk->freq_tbl[ccu_clk->freq_id]/c->rate;
	return c->rate;
}

struct clk_ops bus_clk_ops = {
	.enable		=	bus_clk_enable,
	.get_rate	=	bus_clk_get_rate,
};

/* reference clocks */
static int ref_clk_enable(struct clk *c, int enable)
{
	return 0;
}

struct clk_ops ref_clk_ops = {
	.enable		=	ref_clk_enable,
	.set_rate	=	common_set_rate,
	.get_rate	=	common_get_rate,
	.round_rate	=	common_round_rate,
	.set_parent	=	common_set_parent,
};


struct clk *clk_get(const char *con_id)
{
	int i;

	for (i=0; i<clk_tbl_array_size; i++) {
		struct clk_lookup *p = &clk_tbl[i];
		
		if (p->con_id) {
			if (!con_id || strcmp(p->con_id, con_id))
				continue;
			return p->clk;
		}

	}

	return NULL;
}

