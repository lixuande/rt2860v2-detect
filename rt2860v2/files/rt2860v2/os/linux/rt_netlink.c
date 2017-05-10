/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

    Module Name:
    rt_netlink.c

    Abstract:
    Create and register netlink system for ralink device

    Revision History:
    Who         When            What
    --------    ----------      ----------------------------------------------
*/

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/sched.h>
#include <net/sock.h>
#include <linux/netlink.h>

#include "rt_config.h"
#include "ap.h"

#define NETLINK_TEST 17 

struct {
    __u32 pid;
}user_process;

static struct sock *netlinkfd = NULL;

int rt_netlink_init(void);
int rt_netlink_exit(void);

/********************************************************************
description:
	send the detect data to user netlink, e.x mac rssi snr

argu:
	mac: mac of sta
	detect: subtype,frametype,rssi snr.....

return:
	0:                       sucess
	other:                 fail

autor:
	evenli@gmail.com

date:
	10/08/2015

********************************************************************/
int send_detectdata_to_user(unsigned char *mac, wifi_detect detect);

int send_status_to_user(void);

int send_onestatus_to_user(unsigned char *mac, bool bestop);

int send_onestatus_to_user(unsigned char *mac, bool bestop) //
{
    int size;
    struct sk_buff *skb;
    unsigned char *old_tail;
    struct nlmsghdr *nlh; //

    int retval;
    
    unsigned char info[16];
    memset(info, 0, 16);
    if (bestop)
    {
    	info[0] = 9;
    }
    else
    {
    	info[0] = 4;
    	info[1] = 4;
    	memcpy(&info[2], mac, 6);
  	}
    
    info[8] = 1;
    info[9] = 0;
    info[10] = 3;
    info[11] = 15;
    info[12] = 20;
    info[13] = 0;
    info[14] = 0;

    size = NLMSG_SPACE(strlen(info)); //
    skb = alloc_skb(size, GFP_ATOMIC); //,GFP_ATOMIC>

    nlh = nlmsg_put(skb, 0, 0, 0, NLMSG_SPACE(strlen(info))-sizeof(struct nlmsghdr), 0); 
    old_tail = skb->tail;
	
    memcpy(NLMSG_DATA(nlh), info, strlen(info));
    nlh->nlmsg_len = skb->tail - old_tail; //

    NETLINK_CB(skb).dst_group = 1;

    retval = netlink_broadcast(netlinkfd, skb, 0, 1, GFP_ATOMIC);
    
    return 0;
}
int send_status_to_user(void)
{
	struct  net_device		*net_dev;
	RTMP_ADAPTER		    *pAd;
	unsigned char           info[16];
	MAC_TABLE_ENTRY         *pEntry = NULL;
	
	int retval;

	int size;
	int index=0;
  struct sk_buff *skb;
  unsigned char *old_tail;
  struct nlmsghdr *nlh; 

	#if 1
	extern struct net_device* getadapter(VOID);
	net_dev = getadapter();
	if (net_dev != NULL)
	{
		GET_PAD_FROM_NET_DEV(pAd, net_dev);
	}
	else
	{
		return -1;
	}
	
	for(index=0; index<HASH_TABLE_SIZE; index++)
	{
		pEntry = pAd->MacTab.Hash[index];
		while (pEntry && !IS_ENTRY_NONE(pEntry))
		{
		    send_onestatus_to_user(pEntry->Addr, false);
				pEntry = pEntry->pNext;
				
		}
	}
	#endif
	
	send_onestatus_to_user(NULL, true);
	
	return 0;
}


/********************************************************************
description:
	send the detect data to user netlink, e.x mac rssi snr

argu:
	mac: mac of sta
	detect: subtype,frametype,rssi snr.....

return:
	0:                       sucess
	other:                 fail

autor:
	evenli@gmail.com

date:
	10/08/2015

********************************************************************/
int send_detectdata_to_user(unsigned char *mac, wifi_detect detect)
{
	int size;
    struct sk_buff *skb;
    unsigned char *old_tail;
    struct nlmsghdr *nlh; //

    int retval;

    unsigned char info[16];
    memset(info, 0, 16);
    info[0] = detect.frametype;
    info[1] = detect.subtype;
    
    memcpy(&info[2], mac, 6);
    info[8] = detect.rssi0;
    info[9] = detect.rssi1;
    info[10] = detect.rssi2;
	
    info[11] = detect.snr0;
    info[12] = detect.snr1;
    info[13] = detect.snr2;

    size = NLMSG_SPACE(strlen(info)); //
    skb = alloc_skb(size, GFP_ATOMIC); //,GFP_ATOMIC>

    //netlink
    nlh = nlmsg_put(skb, 0, 0, 0, NLMSG_SPACE(strlen(info))-sizeof(struct nlmsghdr), 0); 
    old_tail = skb->tail;
    //memcpy(NLMSG_DATA(nlh), info, strlen(info)); //
    memcpy(NLMSG_DATA(nlh), info, strlen(info));
    nlh->nlmsg_len = skb->tail - old_tail; //

    //
    //NETLINK_CB(skb).pid = 0;
    NETLINK_CB(skb).dst_group = 1;

		//DBGPRINT(RT_DEBUG_ERROR, ("------SKB--------%02x:%02x:%02x:%02x:%02x:%02x------------\n", skb->data[2],skb->data[3],skb->data[4],skb->data[5],skb->data[6],skb->data[7]));
		//DBGPRINT(RT_DEBUG_ERROR, ("------SKB--------%04x:%04x:%04x:%04x:%04x:%04x------------\n", ((unsigned char *)NLMSG_DATA((struct nlmsghdr *)skb->data))[2],((char *)NLMSG_DATA((struct nlmsghdr *)skb->data))[3],((char *)NLMSG_DATA((struct nlmsghdr *)skb->data))[4],((char *)NLMSG_DATA((struct nlmsghdr *)skb->data))[5],((char *)NLMSG_DATA((struct nlmsghdr *)skb->data))[6],((char *)NLMSG_DATA((struct nlmsghdr *)skb->data))[7]));
    //printk(KERN_ERROR "[kernel space] skb->data:%s\n", (char *)NLMSG_DATA((struct nlmsghdr *)skb->data));

    //
    //DBGPRINT(RT_DEBUG_ERROR, ("------netlink--------%02x:%02x:%02x:%02x:%02x:%02x------------\n", info[2],info[3],info[4],info[5],info[6],info[7]));
    //retval = netlink_unicast(netlinkfd, skb, user_process.pid, MSG_DONTWAIT);
    retval = netlink_broadcast(netlinkfd, skb, 0, 1, GFP_ATOMIC);
    
    return 0;
}

void kernel_receive(struct sk_buff *__skb) 
{
    struct sk_buff *skb;
    struct nlmsghdr *nlh = NULL;

    char *data = "This is test message from kernel";

    skb = skb_get(__skb);

    if(skb->len >= sizeof(struct nlmsghdr)){
        nlh = (struct nlmsghdr *)skb->data;
        if((nlh->nlmsg_len >= sizeof(struct nlmsghdr))
            && (__skb->len >= nlh->nlmsg_len)){
            user_process.pid = nlh->nlmsg_pid;
            //printk(KERN_ERR "[kernel space]1 data receive from user are:%s\n", (char *)NLMSG_DATA(nlh));
            //send_to_user(data);
            if (strcmp((char *)NLMSG_DATA(nlh), "g") == 0)
            {
							send_status_to_user();
            }
        }
    }else{
        printk(KERN_ERR "[kernel space]2 data receive from user are:%s\n",(char *)NLMSG_DATA(nlmsg_hdr(__skb)));
        //send_to_user(data);
    }

    kfree_skb(skb);
}

int rt_netlink_init(void)
{
		struct netlink_kernel_cfg cfg = {
    .input = kernel_receive,
		};
		
    netlinkfd = netlink_kernel_create(&init_net, NETLINK_TEST,  &cfg);
    if(!netlinkfd){
        printk(KERN_ERR "can not create a netlink socket\n");
        return -1;
    }
    return 0;
}
int rt_netlink_exit(void)
{
    sock_release(netlinkfd->sk_socket);
    printk(KERN_DEBUG "test_netlink_exit!!\n");
}


