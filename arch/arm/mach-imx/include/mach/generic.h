
int imx_silicon_revision(void);
#define IMX27_CHIP_REVISION_1_0   0
#define IMX27_CHIP_REVISION_2_0   1

#ifdef CONFIG_ARCH_IMX1
#define cpu_is_mx1()	(1)
#else
#define cpu_is_mx1()	(0)
#endif

#ifdef CONFIG_ARCH_IMX21
#define cpu_is_mx21()	(1)
#else
#define cpu_is_mx21()	(0)
#endif

#ifdef CONFIG_ARCH_IMX25
#define cpu_is_mx25()	(1)
#else
#define cpu_is_mx25()	(0)
#endif

#ifdef CONFIG_ARCH_IMX27
#define cpu_is_mx27()	(1)
#else
#define cpu_is_mx27()	(0)
#endif

#ifdef CONFIG_ARCH_IMX31
#define cpu_is_mx31()	(1)
#else
#define cpu_is_mx31()	(0)
#endif

#ifdef CONFIG_ARCH_IMX35
#define cpu_is_mx35()	(1)
#else
#define cpu_is_mx35()	(0)
#endif

