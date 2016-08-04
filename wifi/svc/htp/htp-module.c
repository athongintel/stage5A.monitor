#include <linux/module.h>
#include <linux/init.h>

#include <netinet/in.h>

MODULE_LICENCE("GPL")

static void __init htp_init(void){
	
}


static void __exit htp_exit(void){

}


module_init(htp_init);
module_exit(htp_exit);

