#include "../port/portfns.h"
#define bpenumenv(n)	((char*)0)
void	aamloop(int);
void	archinit(void);
void	bootargs(ulong);
int	cistrcmp(char*, char*);
int	cistrncmp(char*, char*, int);
#define	clearmmucache()				/* x86 doesn't have one */
void	clockintr(Ureg*, void*);
void	cpuid(char*, int*, int*);
int	cpuidentify(void);
void	cpuidprint(void);
void	delay(int);
int	dmacount(int);
int	dmadone(int);
void	dmaend(int);
void	dmainit(int);
long	dmasetup(int, void*, long, int);
void	dumpregs(Ureg*);
#define	evenaddr(x)				/* x86 doesn't care */
void	fpinit(void);
void	fpoff(void);
void	fprestore(FPU*);
void	fpsave(FPU*);
ulong	fpstatus(void);
ulong	getcr0(void);
ulong	getcr2(void);
ulong	getcr3(void);
ulong	getcr4(void);
char*	getconf(char*);
int	i8042auxcmd(int);
int	i8042auxcmdval(int);
void	i8042auxenable(void (*)(int, int));
int i8042auxdetect(void);
void	i8042reset(void);
void	i8253init(int);
void	i8253enable(void);
void	i8259init(void);
int	i8259enable(int, int, Irqctl*);
void	idle(void);
#define	idlehands()			/* nothing to do in the runproc */
int	inb(int);
void	insb(int, void*, int);
ushort	ins(int);
void	inss(int, void*, int);
ulong	inl(int);
void	insl(int, void*, int);
void	intrenable(int, void (*)(Ureg*, void*), void*, int);
int	ioalloc(int, int, int, char*);
void	iofree(int);
#define	ioalloc(a,b,c,d) 0
#define	iofree(v)
int	iprint(char *fmt, ...);
int	isaconfig(char*, int, ISAConf*);
int	isvalid_pc(ulong);
int	isvalid_va(void*);
void	kbdinit(void);
void	lgdt(ushort[3]);
void	lidt(ushort[3]);
void	links(void);
void	ltr(ulong);
void	mathinit(void);
void	meminit(ulong);
#define mmuflushtlb(pdb) putcr3(pdb)
void	mmuinit(void);
ulong	mmukmap(ulong, ulong, int);
int	mmukmapsync(ulong);
ulong*	mmuwalk(ulong*, ulong, int, int);
void	ns16552install(void);
void	ns16552special(int, int, Queue**, Queue**, int (*)(Queue*, int));
uchar	nvramread(int);
void	nvramwrite(int, uchar);
void	outb(int, int);
void	outsb(int, void*, int);
void	outs(int, ushort);
void	outss(int, void*, int);
void	outl(int, ulong);
void	outsl(int, void*, int);
int	pcicfgr8(Pcidev*, int);
int	pcicfgr16(Pcidev*, int);
int	pcicfgr32(Pcidev*, int);
void	pcicfgw8(Pcidev*, int, int);
void	pcicfgw16(Pcidev*, int, int);
void	pcicfgw32(Pcidev*, int, int);
void	pcihinv(Pcidev*);
Pcidev* pcimatch(Pcidev*, int, int);
Pcidev* pcimatchtbdf(int);
void	pcireset(void);
void	pcisetbme(Pcidev*);
int	pcmspecial(char*, ISAConf*);
void	pcmspecialclose(int);
void	poolinit(void);
void	poolsizeinit(void);
void	procsave(Proc*);
void	procsetup(Proc*);
void	putcr3(ulong);
void	putcr4(ulong);
void	rdmsr(int, ulong*, ulong*);
void	rdtsc(vlong *);
ulong rdtsc32(void);
void	screeninit(void);
int	screenprint(char*, ...);			/* debugging */
void	screenputs(char*, int);
int	scsibio(Target*, char, int, void*, long, long, long);
int	scsicap(Target*, char, ulong*, ulong*);
int	scsiexec(Target*, int, uchar*, int, void*, int*);
int	scsiinquiry(Target*, char, void*, int*);
int	scsiinv(int, int*, Target**, uchar**, char*);
int	scsireqsense(Target*, char, void*, int*, int);
int	scsistart(Target*, char, int);
int	scsitest(Target*, char);
void	trapinit(void);
int	tas(void*);
ulong	umbmalloc(ulong, int, int);
void	umbfree(ulong, int);
ulong	umbrwmalloc(ulong, int, int);
void	umbrwfree(ulong, int);
ulong	upamalloc(ulong, int, int);
void	upafree(ulong, int);
void	vectortable(void);
void	wrmsr(int, ulong, ulong);
void	wbflush(void);
int	xchgw(ushort*, int);
ulong	kzeromap(ulong, ulong, int);
void	nmiscreen(void);
int	kbdinready(void);
void	(*coherence)(void);

#define	waserror()	(up->nerrlab++, setlabel(&up->errlab[up->nerrlab-1]))
#define getcallerpc(x)	(((ulong*)(x))[-1])
#define KADDR(a)	((void*)((ulong)(a)|KZERO))
#define PADDR(a)	((ulong)(a)&~KZERO)

#define	dcflush(a, b)
#define	clockcheck();
#define 	dumplongs(x, y, z)
#define 	setpanic()
