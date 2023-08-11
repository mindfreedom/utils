/***************************************************************************************
* 支持的平台：
*	CentOS Linux release 7.9.2009  - 3.10.0-1160.83.1.el7.x86_64
****************************************************************************************/

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/err.h>
#include <linux/time.h>
#include <linux/skbuff.h>
#include <net/tcp.h>
#include <net/inet_common.h>
#include <linux/uaccess.h>
#include <linux/netdevice.h>
#include <net/net_namespace.h>
#include <linux/fs.h>
#include <linux/sysctl.h>
#include <linux/proc_fs.h>
#include <linux/kallsyms.h>
#include <net/ipv6.h>
#include <net/transp_v6.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>

#define TCPOPT_TOA  254
#define TCPOPTLEN_TOA	8
#define SOL_TOA		100
#define SO_TOA		200

/*
struct toa_data {
	__u8 opcode;	// TCPOPT_TOA，或254
	__u8 opsize;	// TCPOPTLEN_TOA, 或8
	__u16 port;		// 待透传的端口，必须是网络字节序
	__u32 ip;		// 待透传的ip地址，必须是网络字节序
};

// 在应用层填充TCP的TOA选项时，执行如下语句：
setsockopt(fd, SOL_TOA, SO_TOA, &toa_val, TCPOPTLEN_TOA)
其中toa_val是指向toa_data结构的指针，其中包含需要透传的ip地址和端口
*/

/* 设置address所有页框读写属性 */
int _make_rw(unsigned long address, int rw_flag)
{
	unsigned int level;
	pte_t* pte = lookup_address(address, &level);
	if (NULL == pte) {
		printk("lookup_address 0x%lx failed.\n", address);
		return 1;
	}
	if (!(pte->pte & _PAGE_PRESENT)) {
		printk("address 0x%lx not present\n", address);
		return 1;
	}
	if (rw_flag == 1) {
		if (pte->pte & ~_PAGE_RW)
			pte->pte |= _PAGE_RW;
	}
	else {
		pte->pte &= ~_PAGE_RW;
	}
	return 0;
}
#define make_rw(address) _make_rw((address), 1)
#define make_ro(address) _make_rw((address), 0)

/* local_out钩子函数 */
static unsigned int local_out_okfn(const struct nf_hook_ops *ops, 
	struct sk_buff *skb, 
	const struct net_device *in, const struct net_device *out,
#ifndef __GENKSYMS__
	const struct nf_hook_state* state
#else
	int (*okfn)(struct sk_buff *)
#endif
	)
{
	struct sock* sk = skb->sk;
	unsigned char* p, * q;
	struct iphdr* iph_old, * iph_new;
	struct tcphdr* tcph_old, * tcph_new;
	int i;
	uint32_t mtu;
	uint16_t tcpoff;
	struct dst_entry* dst;
	int ihlen, tcphlen;

	if (!sk)
		goto local_out_okfn_end;

	if (!(tcph_old = tcp_hdr(skb)))
		goto local_out_okfn_end;

	if (tcph_old->ack != 1 || tcph_old->psh == 1 || tcph_old->rst == 1 || tcph_old->fin == 1)
		goto local_out_okfn_end;

	if (sk->sk_user_data) {	/* 说明有数据，默认为TOA数据 */
		printk("start to fill toa option into tcp header\n");
		/* check mtu */
		if (!(dst = skb_dst(skb))) {
			printk("find dst error.\n");
			goto local_out_okfn_end;
		}
		mtu = dst_mtu(dst);
		if (skb->len > mtu - TCPOPTLEN_TOA)	/* 当无法容纳时，则不添加TOA选项 */
			goto local_out_okfn_end;

		iph_old = ip_hdr(skb);
		tcph_old = tcp_hdr(skb);
		ihlen = ip_hdrlen(skb);
		tcphlen = tcp_hdrlen(skb);

		if (tcph_old->doff > 13) {	/* 当tcp头部没有额外空间添加TOA选项时，由不添加 */
			printk("cannot add toa option.\n");
			goto local_out_okfn_end;
		}

		/* 在skb前面开辟TOA选项空间 */
		skb_push(skb, TCPOPTLEN_TOA);
		skb->network_header -= TCPOPTLEN_TOA;	/* 将ip头部指针往前移 */
		skb->transport_header -= TCPOPTLEN_TOA; /* 将tcp头部指针往前移 */
		iph_new = ip_hdr(skb);	
		tcph_new = tcp_hdr(skb);

		/* 移动IP头部 */
		p = (unsigned char*)iph_new;
		q = (unsigned char*)iph_old;
		for (i = 0; i < ihlen; i++)
			*p++ = *q++;

		/* 修改IP头部 */
		iph_new->tot_len = htons(skb->len);	
		ip_send_check(iph_new);				/* 重新计算检验和 */

		/* 移动TCP头部原来的值 */
		p = (unsigned char*)tcph_new;
		q = (unsigned char*)tcph_old;
		for (i = 0; i < tcphlen; i++)
			*p++ = *q++;

		/* 添加TOA选项 */
		memcpy(p, &sk->sk_user_data, sizeof(sk->sk_user_data));

		/* 修改TCP头部 */
		tcph_new->doff += sizeof(sk->sk_user_data) / 4;
		/* 重新计算检验和 */
		tcph_new->check = 0;
		tcpoff = ip_hdrlen(skb);
		skb->csum = skb_checksum(skb, tcpoff, skb->len - tcpoff, 0);
		tcph_new->check = csum_tcpudp_magic(iph_new->saddr, iph_new->daddr, skb->len - tcpoff, iph_new->protocol, skb->csum);
		if (skb->ip_summed == CHECKSUM_PARTIAL) {
			skb->ip_summed = CHECKSUM_COMPLETE;
			skb_shinfo(skb)->gso_size = 0;
		}
	}

local_out_okfn_end:
	return NF_ACCEPT;
}

/* 注册与注销LOCAL_OUT钩子 */
struct nf_hook_ops local_out_hook = {
	.list = {&local_out_hook.list, &local_out_hook.list},
	.hook = local_out_okfn,
	.pf = NFPROTO_IPV4,
	.hooknum = NF_INET_LOCAL_OUT,
	.priority = NF_IP_PRI_FIRST,
	.owner = THIS_MODULE
};
static int register_or_unregister_local_out_hook(int flag)
{
	if (flag == 1)
		return nf_register_hook(&local_out_hook);
	else {
		nf_unregister_hook(&local_out_hook);
		return 0;
	}
}
#define register_hook()	register_or_unregister_local_out_hook(1)
#define unregister_hook()	register_or_unregister_local_out_hook(0)

int setsockopt_toa(struct socket* sock, int level, int optname, char __user* optval, unsigned int optlen)
{
	struct sock* sk = sock->sk;
	if (level == SOL_TOA && optname == SO_TOA && optlen == TCPOPTLEN_TOA) {
		if (copy_from_user(&sk->sk_user_data, optval, sizeof(sk->sk_user_data)) != 0) {
			printk("copy from user failed.\n");
			return 1;
		}
		return 0;
	}
	return sock_common_setsockopt(sock, level, optname, optval, optlen);
}

static int __init toa_init(void)
{
	/* 注册setsockopt钩子 */
	struct proto_ops* inet_stream_ops_p = (struct proto_ops*)&inet_stream_ops;
	if (make_rw((unsigned long)inet_stream_ops_p)) {
		printk("toa init failed: cannot set inet_stream_ops\n");
		return 1;
	}
	inet_stream_ops_p->setsockopt = setsockopt_toa;
	/*
	if (make_ro((unsigned long)inet_stream_ops_p)) {
		printk("toa init failed: set protect of page which contains inet_stream_ops\n");
		return 1;
	}
	*/

	/* 注册LOCAL_OUT钩子 */
	register_hook();

	return 0;
}

static void __exit toa_exit(void)
{
	/* 注销setsockopt钩子 */
	struct proto_ops* inet_stream_ops_p = (struct proto_ops*)&inet_stream_ops;
	if (make_rw((unsigned long)inet_stream_ops_p)) {
		printk("toa exit failed at set address rw\n");
		return;
	}
	inet_stream_ops_p->setsockopt = sock_common_setsockopt;
	
	/*
	if (make_ro((unsigned long)inet_stream_ops_p)) {
		printk("toa exit failed at set address rdonly\n");
		return;
	}
	*/

	/* 注册LOCALL_OUT钩子 */
	unregister_hook();
}


module_init(toa_init);
module_exit(toa_exit);

MODULE_LICENSE("GPL v2");
