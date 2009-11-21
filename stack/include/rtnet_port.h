/* include/rtnet_port.h
 *
 * RTnet - real-time networking subsystem
 * Copyright (C) 2003      Wittawat Yamwong
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */
#ifndef __RTNET_PORT_H_
#define __RTNET_PORT_H_

#ifdef __KERNEL__

#include <linux/bitops.h>
#include <linux/moduleparam.h>
#include <linux/list.h>
#include <linux/netdevice.h>

#include <rtdev.h>
#include <rtdev_mgr.h>
#include <rtnet_sys.h>
#include <stack_mgr.h>
#include <ethernet/eth.h>


#ifndef compat_pci_register_driver
# if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0)
#  define compat_pci_register_driver(drv) \
	(pci_register_driver(drv) <= 0 ? -EINVAL : 0)
# else
#  define compat_pci_register_driver(drv) \
	pci_register_driver(drv)
# endif
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0)
# define pci_dma_sync_single_for_device     pci_dma_sync_single
# define pci_dma_sync_single_for_cpu        pci_dma_sync_single
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0)
# define kmem_cache                         kmem_cache_s
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0)
#include <linux/pci.h>
/* only matches directly on vendor and device ID */
static inline int pci_dev_present(const struct pci_device_id *ids)
{
	while (ids->vendor) {
		if (pci_find_device(ids->vendor, ids->device, NULL))
			return 1;
		ids++;
	}
	return 0;
}
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,8)
# define proc_dointvec(a, b, c, d, e, f)    proc_dointvec(a, b, c, d, e)
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,10)
# define compat_pci_restore_state(a, b)     pci_restore_state(a, b)
# define compat_pci_save_state(a, b)        pci_save_state(a, b)
# define compat_module_int_param_array(name, count) \
    MODULE_PARM(name, "1-" __MODULE_STRING(count) "i")
#else
# define compat_pci_restore_state(a, b)     pci_restore_state(a)
# define compat_pci_save_state(a, b)        pci_save_state(a)
# define compat_module_int_param_array(name, count) \
    module_param_array(name, int, NULL, 0444)
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0)
# define LIST_POISON1  ((void *) 0x00100100)
# define LIST_POISON2  ((void *) 0x00200200)

struct hlist_head {
	struct hlist_node *first;
};

struct hlist_node {
	struct hlist_node *next, **pprev;
};

# define INIT_HLIST_HEAD(ptr) ((ptr)->first = NULL)

static inline void __hlist_del(struct hlist_node *n)
{
	struct hlist_node *next = n->next;
	struct hlist_node **pprev = n->pprev;
	*pprev = next;
	if (next)
		next->pprev = pprev;
}

static inline void hlist_del(struct hlist_node *n)
{
	__hlist_del(n);
	n->next = LIST_POISON1;
	n->pprev = LIST_POISON2;
}

static inline void hlist_add_head(struct hlist_node *n, struct hlist_head *h)
{
	struct hlist_node *first = h->first;
	n->next = first;
	if (first)
		first->pprev = &n->next;
	h->first = n;
	n->pprev = &h->first;
}

# define hlist_entry(ptr, type, member) \
	((type *)((char *)(ptr) - (unsigned long)(&((type *)0)->member)))

# define hlist_for_each_entry(tpos, pos, head, member)			 \
	for (pos = (head)->first;					 \
	     pos && ({ prefetch(pos->next); 1;}) &&			 \
		({ tpos = hlist_entry(pos, typeof(*tpos), member); 1;}); \
	     pos = pos->next)
#endif

static inline void rtnetif_start_queue(struct rtnet_device *rtdev)
{
    clear_bit(__RTNET_LINK_STATE_XOFF, &rtdev->link_state);
}

static inline void rtnetif_wake_queue(struct rtnet_device *rtdev)
{
    if (test_and_clear_bit(__RTNET_LINK_STATE_XOFF, &rtdev->link_state))
    /*TODO __netif_schedule(dev); */ ;
}

static inline void rtnetif_stop_queue(struct rtnet_device *rtdev)
{
    set_bit(__RTNET_LINK_STATE_XOFF, &rtdev->link_state);
}

static inline int rtnetif_queue_stopped(struct rtnet_device *rtdev)
{
    return test_bit(__RTNET_LINK_STATE_XOFF, &rtdev->link_state);
}

static inline int rtnetif_running(struct rtnet_device *rtdev)
{
    return test_bit(__RTNET_LINK_STATE_START, &rtdev->link_state);
}

static inline int rtnetif_device_present(struct rtnet_device *rtdev)
{
    return test_bit(__RTNET_LINK_STATE_PRESENT, &rtdev->link_state);
}

static inline void rtnetif_carrier_on(struct rtnet_device *rtdev)
{
    clear_bit(__RTNET_LINK_STATE_NOCARRIER, &rtdev->link_state);
    /*
    if (netif_running(dev))
        __netdev_watchdog_up(dev);
    */
}

static inline void rtnetif_carrier_off(struct rtnet_device *rtdev)
{
    set_bit(__RTNET_LINK_STATE_NOCARRIER, &rtdev->link_state);
}

static inline int rtnetif_carrier_ok(struct rtnet_device *rtdev)
{
    return !test_bit(__RTNET_LINK_STATE_NOCARRIER, &rtdev->link_state);
}

#ifndef HAVE_NETDEV_PRIV
static inline void *netdev_priv(struct net_device *dev)
{
        return dev->priv;
}
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,24)
#define DMA_BIT_MASK(n)	(((n) == 64) ? ~0ULL : ((1ULL<<(n))-1))
#endif

#endif /* __KERNEL__ */

#endif /* __RTNET_PORT_H_ */