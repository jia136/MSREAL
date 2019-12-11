#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/types.h>
#include <linux/cdev.h>
#include <linux/kdev_t.h>
#include <linux/uaccess.h>
#include <linux/errno.h>
#include <linux/device.h>
#include <linux/wait.h>
#include <linux/sched.h>
//#include <linux/semaphore.h>
#define BUFF_SIZE 80 
MODULE_LICENSE("Dual BSD/GPL");


dev_t my_dev_id;
static struct class *my_class;
static struct device *my_device;
static struct cdev *my_cdev;

DECLARE_WAIT_QUEUE_HEAD(readQ);
DECLARE_WAIT_QUEUE_HEAD(writeQ);
//struct semaphore sem;

int fifo[16];//bafer za upis dec brojeva
char fifo_str[16][16];//bafer za upis hex brojeva
int pos_r = 0;//pozicija za citanje
int pos_w = 0;//pozicija za upis
int endRead = 0;//brojac za citanje
int mod_rada=0;//odredjuje mod rada hex ili dec ispis
int n=1;//odredjuje koliko se brojeva istovremeno cita sa cat 

int fifo_open(struct inode *pinode, struct file *pfile);
int fifo_close(struct inode *pinode, struct file *pfile);
ssize_t fifo_read(struct file *pfile, char __user *buffer, size_t length, loff_t *offset);
ssize_t fifo_write(struct file *pfile, const char __user *buffer, size_t length, loff_t *offset);

struct file_operations my_fops =
{
	.owner = THIS_MODULE,
	.open = fifo_open,
	.read = fifo_read,
	.write = fifo_write,
	.release = fifo_close,
};


int fifo_open(struct inode *pinode, struct file *pfile) 
{
		printk(KERN_INFO "Succesfully opened fifo\n");
		return 0;
}

int fifo_close(struct inode *pinode, struct file *pfile) 
{
		printk(KERN_INFO "Succesfully closed fifo\n");
		return 0;
}

ssize_t fifo_read(struct file *pfile, char __user *buffer, size_t length, loff_t *offset) 
{
	int ret;
	char buff[BUFF_SIZE];
	long int len = 0;
	if(endRead==n){
		endRead = 0;
		return 0;
	}
	//if(down_interruptible(&sem))
	//	return -ERESTARTSYS;
	//while(pos_r==pos_w){//bafer prazan
	//	up(&sem);
	if(wait_event_interruptible(readQ,(pos_r!=pos_w)))
		return -ERESTARTSYS;
	//	if(down_interruptible(&sem))
	//		return -ERESTARTSYS;
	//}
	if(pos_r!= pos_w)
	{	
		if(mod_rada){
			len = scnprintf(buff, BUFF_SIZE, " %d ", fifo[pos_r]);//ispis dec
		}
		else{
			len = scnprintf(buff, BUFF_SIZE, " 0x%s ", fifo_str[pos_r]);//ispis hex
		}
		
		ret = copy_to_user(buffer, buff, len);
		if(ret)
			return -EFAULT;
		
		printk(KERN_INFO "Succesfully read\n");
		pos_r= (pos_r+1)%17;//pomeranje pozicije citanja za +1
		
	}
	else
	{
		printk(KERN_WARNING "Fifo is empty\n"); 
	}
	//up(&sem);
	wake_up_interruptible(&writeQ);
	endRead = endRead+1;
	return len;
}

ssize_t fifo_write(struct file *pfile, const char __user *buffer, size_t length, loff_t *offset) 
{
	char buff[BUFF_SIZE];
	char value[50];//niz
	char temp[50];//privremeni niz
	char tempn[10];
	int ret;
	int i=0;//brojac za for petlju
	int j=0;//brojac za temp
	int k=0;//brojac za for petlju
	int l=4;//brojac za for petlju
	int broj=0;// vrednost koja se upisuje u fifo dec
	int duzina=0;//duzina ulaznog stinga
	int a = 0;
	ret = copy_from_user(buff, buffer, length);
	if(ret)
		return -EFAULT;
	buff[length-1] = '\0';	
	ret = sscanf(buff,"%s",value);
	duzina = strlen(value);

	//proverava da li je korisnik uneo hex i menja mod rada ako jeste
	if(value[0]=='h' && value[1]=='e' && value[2]=='x'){
		mod_rada=0;
		printk(KERN_INFO "Mod rada: hex\n");			
	}
	//proverava da li je korisnik uneo dec i menja mod rada ako jeste
	else if(value[0]=='d' && value[1]=='e' && value[2]=='c'){
		mod_rada=1;
		printk(KERN_INFO "Mod rada: dec\n");
	}
	//proverava da li je korisnik uneo num= n ako jeste uzima novu vrenost za n  
	else if(value[0]=='n' && value[1]=='u' && value[2]=='m'&& value[3]=='='){
		for(l=4;l<duzina;l++){
			tempn[a]=value[l];
			a++;
		}
		tempn[a]='\0';
		a=0;
		kstrtoint(tempn,10,&n);
		if(n>16){
			n=16;
		}
		if(n<0){
			n=0;
		}
		printk(KERN_INFO "Ispis n=%d vrednosti",n);
	}
	else{
		for(i=0; i<=duzina; i++){
			//if(down_interruptible(&sem))
			//	return -ERESTARTSYS;
			//while((pos_w+1)%17 == pos_r){//bafer je pun
			//	up(&sem);
			if(wait_event_interruptible(writeQ,((pos_w+1)%17 != pos_r)))
                 	return -ERESTARTSYS;
			//	if(down_interruptible(&sem))
			//		return -ERESTARTSYS;
			//}
			if(value[i]!=',' && value[i]!='\0')
			{
				temp[j]=value[i];
				j=j+1;
			}
			else
			{
				temp[j] = '\0';
				j = 0;
				if(strlen(temp)>=3){//ogranicava unos na broj do 255
					printk(KERN_INFO "Pogresan unos-prekoracenje opsega\n");
					return length;
				}
				kstrtoint(temp,16,&broj);
				if((pos_w+1)%17!=pos_r){
					fifo[pos_w] = broj;//upis dec broja u bafer
					for(k=0;k<strlen(temp); k++){
						fifo_str[pos_w][k]=temp[k];//upis hex broja u bafer
					}
					fifo_str[pos_w][k]='\0';
					printk(KERN_INFO "Succesfully wrote value %d", broj);
					pos_w = (pos_w+1)%17;//pomeranje pozicije za upis za +1
				}
				else{
					printk(KERN_INFO "Fifo is full\n");
					i=duzina+1;//da izadje iz for petlje
				}
			}
		}
	}
	//up(&sem);
	wake_up_interruptible(&readQ);
	return length;
}

static int __init fifo_init(void)
{
	   int ret = 0;
		int i=0;
		int j=0;

		//Initialize array
		for (i=0; i<16; i++){
			fifo[i] = 0;
			for(j=0; j<16; j++)
				fifo_str[i][j] = '\0';
		}
	   //sema_init(&sem,1);

	   ret = alloc_chrdev_region(&my_dev_id, 0, 1, "fifo");
	   if (ret){
		  printk(KERN_ERR "failed to register char device\n");
		  return ret;
	   }
	   printk(KERN_INFO "char device region allocated\n");

	   my_class = class_create(THIS_MODULE, "fifo_class");
	   if (my_class == NULL){
		  printk(KERN_ERR "failed to create class\n");
		  goto fail_0;
	   }
	   printk(KERN_INFO "class created\n");
	   
	   my_device = device_create(my_class, NULL, my_dev_id, NULL, "fifo");
	   if (my_device == NULL){
		  printk(KERN_ERR "failed to create device\n");
		  goto fail_1;
	   }
	   printk(KERN_INFO "device created\n");

		my_cdev = cdev_alloc();	
		my_cdev->ops = &my_fops;
		my_cdev->owner = THIS_MODULE;
		ret = cdev_add(my_cdev, my_dev_id, 1);
		if (ret)
		{
		  printk(KERN_ERR "failed to add cdev\n");
			goto fail_2;
		}
	   printk(KERN_INFO "cdev added\n");
	   printk(KERN_INFO "Hello world\n");

	   return 0;

	   fail_2:
		  device_destroy(my_class, my_dev_id);
	   fail_1:
		  class_destroy(my_class);
	   fail_0:
		  unregister_chrdev_region(my_dev_id, 1);
	   return -1;
}

static void __exit fifo_exit(void)
{
	   cdev_del(my_cdev);
	   device_destroy(my_class, my_dev_id);
	   class_destroy(my_class);
	   unregister_chrdev_region(my_dev_id,1);
	   printk(KERN_INFO "Goodbye world\n");
}


module_init(fifo_init);
module_exit(fifo_exit);