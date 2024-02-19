#include <linux/module.h>    // Necesario para modulos
#include <linux/kernel.h>    // Necesario para KERN_INFO 
#include <linux/init.h>      // Necesario para macros
#include <linux/proc_fs.h>   // Para montarse en proc
#include <asm/uaccess.h>	 // Para copiar datos de capa de usuario
#include <linux/seq_file.h>  // Header para usar la lib seq_file y manejar el archivo

#define PROCFS_MAX_SIZE 1024

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Sistemas Operativos 2 - Clase 3");
MODULE_AUTHOR("Derek Esquivel Diaz");

static char proc_buffer[PROCFS_MAX_SIZE] = "Hola clase de sistemas operativos 2\n";


static ssize_t write_file(struct file *file, const char __user *buff, size_t count, loff_t *offset){

    ssize_t retval;

    if(*offset + count > sizeof(proc_buffer)){
        return -EINVAL;
    }

    memset(proc_buffer, '\0', sizeof(proc_buffer));

    retval = copy_from_user(proc_buffer, buff, count);
    
    if(retval){
        return -EINVAL;
    }

    *offset += count;

    return count;

}

static ssize_t read_file(struct file *file, char __user *buff, size_t count, loff_t *offset){

    ssize_t retval;

    if(*offset >= strlen(proc_buffer)){
        return 0;
    }

    retval = simple_read_from_buffer(buff, count, offset, proc_buffer, strlen(proc_buffer));

    return retval;
}


static struct proc_ops proc_operations =
{
    .proc_read = read_file,
    .proc_write = write_file,
};

static int _insert(void){
    proc_create("sopes2_clase3", 0, NULL, &proc_operations);
    printk(KERN_INFO "Modulo montado correctamente\n");
    return 0;
}

static void _remove(void){
    remove_proc_entry("sopes2_clase3", NULL);
    printk(KERN_INFO "Modulo desmontado correctamente\n");
}

module_init(_insert);
module_exit(_remove);