#ifndef __LIBSAIO_GMA_H
#define __LIBSAIO_GMA_H

bool setup_gma_devprop(pci_dt_t *gma_dev);

struct gma_gpu_t {
	unsigned device;
	char *name;
};

#define REG8(reg)  ((volatile uint8_t *)regs)[(reg)]
#define REG16(reg)  ((volatile uint16_t *)regs)[(reg) >> 1]
#define REG32(reg)  ((volatile uint32_t *)regs)[(reg) >> 2]


#endif /* !__LIBSAIO_GMA_H */
