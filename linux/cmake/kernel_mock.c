#include "linux/gfp.h"
#include "linux/types.h"
#include "linux/jiffies.h"
#include "linux/fs.h"
#include "linux/module.h"
#include "linux/idr.h"
#include "linux/rwsem.h"
#include "linux/moduleparam.h"
#include "linux/tpm.h"
#include "linux/spi/spi.h"
#include "linux/interrupt.h"
#include "linux/freezer.h"
#include "kernel_mock.h"

unsigned long volatile __cacheline_aligned_in_smp __jiffy_arch_data jiffies;
struct module __this_module;
const struct kernel_param_ops param_ops_uint;
const struct file_operations tpm_fops;
const struct file_operations tpmrm_fops;
atomic_t system_freezing_cnt;

void *__kmalloc(size_t size, gfp_t flags)
{
    void *ptr = malloc(size);
    memset(ptr, 0, size);
    return ptr;
}

void kfree(const void *objp)
{ 
    free((void *)objp);
}

void *devm_kmalloc(struct device *dev, size_t size, gfp_t gfp)
{
    void *ptr = malloc(size);
    memset(ptr, 0, size);
    return ptr;
}

int devm_add_action(struct device *dev, void (*action)(void *), void *data) { return 0; }
extern int __must_check
devm_request_threaded_irq(struct device *dev, unsigned int irq,
			  irq_handler_t handler, irq_handler_t thread_fn,
			  unsigned long irqflags, const char *devname,
			  void *dev_id) { return 0; }
extern void devm_free_irq(struct device *dev, unsigned int irq, void *dev_id) {}

/*int init_module(void) { return 0; }
void cleanup_module(void) { }*/

void _dev_err(const struct device *dev, const char *fmt, ...)
{
//    printf(fmt);
}

void _dev_warn(const struct device *dev, const char *fmt, ...)
{
//    printf(fmt);
}

void _dev_info(const struct device *dev, const char *fmt, ...)
{
//    printf(fmt);
}

extern struct class * __must_check __class_create(struct module *owner,
						  const char *name,
						  struct lock_class_key *key) { return NULL; }
extern void class_destroy(struct class *cls) {}

int printk(const char *fmt, ...)
{
    printf(fmt);
    return 0;
}

void usleep_range(unsigned long min, unsigned long max) {}

int alloc_chrdev_region(dev_t *dev, unsigned baseminor, unsigned count,
			const char *name) { return 0; }
void unregister_chrdev_region(dev_t from, unsigned count) {}

int tpm_dev_common_init(void) {}

void idr_destroy(struct idr *idr) {}

void __exit tpm_dev_common_exit(void) {}

unsigned long __get_free_pages(gfp_t gfp_mask, unsigned int order)
{
    return (unsigned long)malloc(PAGE_SIZE);
}

void free_pages(unsigned long addr, unsigned int order)
{
    free((void *)addr);
}

void warn_slowpath_fmt(const char *file, int line, unsigned taint,
		       const char *fmt, ...) {}

// to-be-fixed
unsigned long __msecs_to_jiffies(const unsigned int m) { return 1000; }
unsigned int jiffies_to_usecs(const unsigned long j) { return j; }
unsigned int jiffies_to_msecs(const unsigned long j) { return j; }
unsigned long __usecs_to_jiffies(const unsigned int u) { return u; }

struct device *get_device(struct device *dev) { return NULL; }
void put_device(struct device *dev) {}

extern void down_read(struct rw_semaphore *sem) {}
extern void down_write(struct rw_semaphore *sem) {}
extern void up_read(struct rw_semaphore *sem) {}
extern void up_write(struct rw_semaphore *sem) {}

extern void __init_rwsem(struct rw_semaphore *sem, const char *name,
			 struct lock_class_key *key) {}

void
__mutex_init(struct mutex *lock, const char *name, struct lock_class_key *key) {}
extern void mutex_lock(struct mutex *lock) {}
extern void mutex_unlock(struct mutex *lock) {}

int idr_alloc(struct idr *idr, void *ptr, int start, int end, gfp_t gfp) { return 0; }
void *idr_get_next(struct idr *idr, int *nextid) { return NULL; }
void *idr_remove(struct idr *idr, unsigned long id) { return NULL; }
void *idr_replace(struct idr *idr, void *ptr, unsigned long id) { return NULL; }

void device_initialize(struct device *dev) {}
int dev_set_name(struct device *dev, const char *name, ...) { return 0; }
void cdev_init(struct cdev *cdev, const struct file_operations *fops) {}
int cdev_device_add(struct cdev *cdev, struct device *dev) { return 0; }
void cdev_device_del(struct cdev *cdev, struct device *dev) {}

void tpm_sysfs_add_device(struct tpm_chip *chip) {}
void sysfs_remove_link(struct kobject *kobj, const char *name) {}
int compat_only_sysfs_link_entry_to_kobj(struct kobject *kobj,
					 struct kobject *target_kobj,
					 const char *target_name,
					 const char *symlink_name) { return 0; }

extern int hwrng_register(struct hwrng *rng) { return 0; }
extern void hwrng_unregister(struct hwrng *rng) {}

void tpm_bios_log_setup(struct tpm_chip *chip) {}
void tpm_bios_log_teardown(struct tpm_chip *chip) {}

extern void __init_swait_queue_head(struct swait_queue_head *q, const char *name,
				    struct lock_class_key *key) {}

extern void driver_unregister(struct device_driver *drv) {}

extern int __spi_register_driver(struct module *owner, struct spi_driver *sdrv) { return 0; }
extern int spi_bus_lock(struct spi_controller *ctlr) { return 0; }
extern int spi_bus_unlock(struct spi_controller *ctlr) { return 0; }
extern const struct spi_device_id *
spi_get_device_id(const struct spi_device *sdev) { return NULL; }

extern const void *of_device_get_match_data(const struct device *dev) { return NULL; }

extern void _clear_bit(int nr, volatile unsigned long * p) { }
extern int _test_and_set_bit(int nr, volatile unsigned long * p) { return 0; }

extern bool freezing_slow_path(struct task_struct *p) { return false; }

extern int _cond_resched(void) { return 0; }

extern void init_wait_entry(struct wait_queue_entry *wq_entry, int flags) {}
long prepare_to_wait_event(struct wait_queue_head *wq_head, struct wait_queue_entry *wq_entry, int state) { return 0; }
void finish_wait(struct wait_queue_head *wq_head, struct wait_queue_entry *wq_entry) {}
void __wake_up(struct wait_queue_head *wq_head, unsigned int mode, int nr, void *key) {}
void __init_waitqueue_head(struct wait_queue_head *wq_head, const char *name, struct lock_class_key *key) {}

extern long schedule_timeout(long timeout) { return 0; }

extern void dump_stack(void) {}

void iounmap(volatile void __iomem *iomem_cookie) {}
void __iomem *ioremap(resource_size_t res_cookie, size_t size) { return NULL; }
