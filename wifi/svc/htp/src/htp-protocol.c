
#include "htp-impl.h"
#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <net/inet_sock.h>

//MODULE_LICENCE("GPL");
#define HTP_PROTOCOL_NAME 	"HTP"
#define AF_HTP				AF_PHONET //use this protocol to not modify the kernel, in hope that the system doesn't have it yet

struct htp_sock {
	struct inet_sock isk;
	/* custom new-socket implementation*/
};

static struct proto htp_proto = {
	.close = htp_close,
	.connect = htp_connect,
	.disconnect = htp_disconnect,
	.accept = htp_accept,
	.ioctl = htp_ioctl,
	.init = htp_init_sock,
	.shutdown = htp_shutdown,
	.setsockopt = htp_setsockopt,
	.getsockopt = htp_getsockopt,
	.sendmsg = htp_sendmsg,
	.recvmsg = htp_recvmsg,
	.unhash = htp_unhash,
	.get_port = htp_get_port,
	//.enter_memory_pressure = htp_enter_memory_pressure,
	//.sockets_allocated = &sockets_allocated,
	//.memory_allocated = &memory_allocated,
	//.memory_pressure = &memory_pressure,
	//.orphan_count = &orphan_count,
	//.sysctl_mem = sysctl_tcp_mem,
	//.sysctl_wmem = sysctl_tcp_wmem,
	//.sysctl_rmem = sysctl_tcp_rmem,
	.max_header = 0,
	.obj_size = sizeof(struct htp_sock),
	.owner = THIS_MODULE,
	.name = HTP_PROTOCOL_NAME,
};

static struct proto_ops htp_proto_ops = {
	.family = PF_INET,
	.owner = THIS_MODULE,
	.release = htp_ops_release,
	.bind = htp_ops_bind,
	.connect = htp_ops_connect,
	.socketpair = sock_no_socketpair,
	.accept = htp_ops_accept,
	.getname = htp_ops_getname,
	.poll = htp_ops_poll,
	.ioctl = htp_ops_ioctl,
	//.listen = htp_inet_listen,
	//.shutdown = htp_ops_shutdown,
	//.setsockopt = htp_ops_setsockopt,
	//.getsockopt = htp_ops_getsockopt,
	//.sendmsg = htp_ops_sendmsg,
	//.recvmsg = htp_ops_recvmsg,
};

static struct net_proto_family htp_proto_family = {
	.family = AF_HTP, //by modifying 
	.create = htp_create_socket,
	.owner = THIS_MODULE,
};

static int proto_register_status;
static int sock_register_status;

static int __init htp_init(void){

	//register new protocol to kernel's protocol stack
	proto_register_status = proto_register(&htp_proto, 1);	
	if (proto_register_status!=0)
		goto register_fail;

	//register new socket to kernel

	sock_register_status = sock_register(&htp_proto_family);
	if (sock_register_status!=0)
		goto register_fail;
			
	//init 4 static message queue
	
	goto register_success;
	
	register_fail:
		printk(KERN_ALERT "HTP registering failed.\n");
		return proto_register_status | sock_register_status;
	
	register_success:
		printk(KERN_ALERT "HTP registered to kernel. Start listening.\n");
		return proto_register_status | sock_register_status;
}


static void __exit htp_exit(void){
	
	//unregister socket
	if (sock_register_status==0){
		sock_unregister(AF_HTP);
	}
	//unregister proto
	if (proto_register_status==0){
		proto_unregister(&htp_proto);
	}
	
	printk(KERN_ALERT "HTP unregistered. Module exit.\n");
}

module_init(htp_init);
module_exit(htp_exit);

