#include "limbo.h"

static	void	disbig(long, Long);
static	void	disbyte(long, int);
static	void	disbytes(long, void*, int);
static	void	disdatum(long, Node*);
static	void	disflush(int, long, long);
static	void	disint(long, long);
static	void	disreal(long, Real);
static	void	disstring(long, Sym*);

static	uchar	*cache;
static	int	ncached;
static	int	ndatum;
static	int	startoff;
static	int	lastoff;
static	int	lastkind;
static	int	lencache;

void
discon(long val)
{
	if(val >= -64 && val <= 63) {
		Bputc(bout, val & ~0x80);
		return;
	}
	if(val >= -8192 && val <= 8191) {
		Bputc(bout, ((val>>8) & ~0xC0) | 0x80);
		Bputc(bout, val);
		return;
	}
	if(val < 0 && ((val >> 29) & 0x7) != 7
	|| val > 0 && (val >> 29) != 0)
		fatal("overflow in constant 0x%lux\n", val);
	Bputc(bout, (val>>24) | 0xC0);
	Bputc(bout, val>>16);
	Bputc(bout, val>>8);
	Bputc(bout, val);
}

void
disword(long w)
{
	Bputc(bout, w >> 24);
	Bputc(bout, w >> 16);
	Bputc(bout, w >> 8);
	Bputc(bout, w);
}

void
disdata(int kind, long n)
{
	if(n < DMAX && n != 0)
		Bputc(bout, DBYTE(kind, n));
	else{
		Bputc(bout, DBYTE(kind, 0));
		discon(n);
	}
}

void
dismod(Decl *m)
{
	char name[8*NAMELEN], *sp;
	ulong fileoff;

	fileoff = Boffset(bout);
	strncpy(name, m->sym->name, NAMELEN);
	name[NAMELEN-1] = '\0';
	Bwrite(bout, name, strlen(name)+1);
	for(m = m->ty->tof->ids; m != nil; m = m->next){
		switch(m->store){
		case Dglobal:
			discon(-1);
			discon(-1);
			disword(sign(m));
			Bprint(bout, ".mp");
			Bputc(bout, '\0');
			break;
		case Dfn:
			discon(m->pc->pc);
			discon(m->desc->id);
			disword(sign(m));
			if(m->dot->ty->kind == Tadt)
				Bprint(bout, "%s.", m->dot->sym->name);
			Bprint(bout, "%s", m->sym->name);
			Bputc(bout, '\0');
			break;
		default:
			fatal("unknown kind %K in dismod", m);
			break;
		}
	}
	if(debug['s'])
		print("%ld linkage bytes start %ld\n", Boffset(bout) - fileoff, fileoff);
	sp = srcpath(name, 8*NAMELEN);
	Bwrite(bout, sp, strlen(sp)+1);
}

void
disentry(Decl *e)
{
	if(e == nil){
		discon(-1);
		discon(-1);
		return;
	}
	discon(e->pc->pc);
	discon(e->desc->id);
}

void
disdesc(Desc *d)
{
	ulong fileoff;

	fileoff = Boffset(bout);
	for(; d != nil; d = d->next){
		discon(d->id);
		discon(d->size);
		discon(d->nmap);
		Bwrite(bout, d->map, d->nmap);
	}
	if(debug['s'])
		print("%ld type descriptor bytes start %ld\n", Boffset(bout) - fileoff, fileoff);
}

void
disvar(long size, Decl *d)
{
	ulong fileoff;

	fileoff = Boffset(bout);
	USED(size);

	lastkind = -1;
	ncached = 0;
	ndatum = 0;

	for(; d != nil; d = d->next)
		if(d->store == Dglobal && d->init != nil)
			disdatum(d->offset, d->init);

	disflush(-1, -1, 0);

	Bputc(bout, 0);

	if(debug['s'])
		print("%ld data bytes start %ld\n", Boffset(bout) - fileoff, fileoff);
}

static void
disdatum(long offset, Node *n)
{
	Node *elem, *wild;
	Case *c;
	Label *lab;
	Decl *id;
	Sym *s;
	long e, last, esz;
	int i;

	switch(n->ty->kind){
	case Tbyte:
		disbyte(offset, n->val);
		break;
	case Tint:
		disint(offset, n->val);
		break;
	case Tbig:
		disbig(offset, n->val);
		break;
	case Tstring:
		disstring(offset, n->decl->sym);
		break;
	case Treal:
		disreal(offset, n->rval);
		break;
	case Tadt:
	case Tadtpick:
	case Ttuple:
		id = n->ty->ids;
		for(n = n->left; n != nil; n = n->right){
			disdatum(offset + id->offset, n->left);
			id = id->next;
		}
		break;
	case Tany:
		break;
	case Tcase:
		c = n->ty->cse;
		disint(offset, c->nlab);
		offset += IBY2WD;
		for(i = 0; i < c->nlab; i++){
			lab = &c->labs[i];
			disint(offset, lab->start->val);
			offset += IBY2WD;
			disint(offset, lab->stop->val+1);
			offset += IBY2WD;
			disint(offset, lab->inst->pc);
			offset += IBY2WD;
		}
		disint(offset, c->iwild->pc);
		break;
	case Tcasec:
		c = n->ty->cse;
		disint(offset, c->nlab);
		offset += IBY2WD;
		for(i = 0; i < c->nlab; i++){
			lab = &c->labs[i];
			disstring(offset, lab->start->decl->sym);
			offset += IBY2WD;
			if(lab->stop != lab->start)
				disstring(offset, lab->stop->decl->sym);
			offset += IBY2WD;
			disint(offset, lab->inst->pc);
			offset += IBY2WD;
		}
		disint(offset, c->iwild->pc);
		break;
	case Tgoto:
		c = n->ty->cse;
		disint(offset, n->ty->size/IBY2WD-1);
		offset += IBY2WD;
		for(i = 0; i < c->nlab; i++){
			disint(offset, c->labs[i].inst->pc);
			offset += IBY2WD;
		}
		if(c->iwild != nil)
			disint(offset, c->iwild->pc);
		break;
	case Tarray:
		disflush(-1, -1, 0);
		disdata(DEFA, 1);		/* 1 is ignored */
		discon(offset);
		disword(n->ty->tof->decl->desc->id);
		disword(n->left->val);

		if(n->right == nil)
			break;

		disdata(DIND, 1);		/* 1 is ignored */
		discon(offset);
		disword(0);

		c = n->right->ty->cse;
		wild = nil;
		if(c->wild != nil)
			wild = c->wild->right;
		last = 0;
		esz = n->ty->tof->size;
		for(i = 0; i < c->nlab; i++){
			e = c->labs[i].start->val;
			if(wild != nil){
				for(; last < e; last++)
					disdatum(esz * last, wild);
			}
			last = e;
			e = c->labs[i].stop->val;
			elem = c->labs[i].node->right;
			for(; last <= e; last++)
				disdatum(esz * last, elem);
		}
		if(wild != nil)
			for(e = n->left->val; last < e; last++)
				disdatum(esz * last, wild);

		disflush(-1, -1, 0);
		disdata(DAPOP, 1);		/* 1 is ignored */
		discon(0);

		break;
	case Tiface:
		disint(offset, n->val);
		offset += IBY2WD;
		for(id = n->decl->ty->ids; id != nil; id = id->next){
			offset = align(offset, IBY2WD);
			disint(offset, sign(id));
			offset += IBY2WD;

			if(id->dot->ty->kind == Tadt){
				s = id->dot->sym;
				disbytes(offset, s->name, s->len);
				offset += s->len;
				disbyte(offset, '.');
				offset++;
			}
			s = id->sym;
			disbytes(offset, s->name, s->len);
			offset += s->len;
			disbyte(offset, 0);
			offset++;
		}
		break;
	default:
		nerror(n, "can't dis global %n", n);
		break;
	}
}

static void
disbyte(long off, int v)
{
	disflush(DEFB, off, 1);
	cache[ncached++] = v;
	ndatum++;
}

static void
disbytes(long off, void *v, int n)
{
	disflush(DEFB, off, n);
	memmove(&cache[ncached], v, n);
	ncached += n;
	ndatum += n;
}

static void
disint(long off, long v)
{
	disflush(DEFW, off, IBY2WD);
	cache[ncached++] = v >> 24;
	cache[ncached++] = v >> 16;
	cache[ncached++] = v >> 8;
	cache[ncached++] = v;
	ndatum++;
}

static void
disbig(long off, Long v)
{
	ulong iv;

	disflush(DEFL, off, IBY2LG);
	iv = v >> 32;
	cache[ncached++] = iv >> 24;
	cache[ncached++] = iv >> 16;
	cache[ncached++] = iv >> 8;
	cache[ncached++] = iv;
	iv = v;
	cache[ncached++] = iv >> 24;
	cache[ncached++] = iv >> 16;
	cache[ncached++] = iv >> 8;
	cache[ncached++] = iv;
	ndatum++;
}

static void
disreal(long off, Real v)
{
	ulong bv[2];
	ulong iv;

	disflush(DEFF, off, IBY2LG);
	dtocanon(v, bv);
	iv = bv[0];
	cache[ncached++] = iv >> 24;
	cache[ncached++] = iv >> 16;
	cache[ncached++] = iv >> 8;
	cache[ncached++] = iv;
	iv = bv[1];
	cache[ncached++] = iv >> 24;
	cache[ncached++] = iv >> 16;
	cache[ncached++] = iv >> 8;
	cache[ncached++] = iv;
	ndatum++;
}

static void
disstring(long offset, Sym *sym)
{
	disflush(-1, -1, 0);
	disdata(DEFS, sym->len);
	discon(offset);
	Bwrite(bout, sym->name, sym->len);
}

static void
disflush(int kind, long off, long size)
{
	if(kind != lastkind || off != lastoff){
		if(lastkind != -1 && ncached){
			disdata(lastkind, ndatum);
			discon(startoff);
			Bwrite(bout, cache, ncached);
		}
		startoff = off;
		lastkind = kind;
		ncached = 0;
		ndatum = 0;
	}
	lastoff = off + size;
	while(kind >= 0 && ncached + size >= lencache){
		lencache = ncached+1024;
		cache = reallocmem(cache, lencache);
	}
}

static int dismode[Aend] = {
	/* Aimm */	AIMM,
	/* Amp */	AMP,
	/* Ampind */	AMP|AIND,
	/* Afp */	AFP,
	/* Afpind */	AFP|AIND,
	/* Apc */	AIMM,
	/* Adesc */	AIMM,
	/* Aoff */	AIMM,
	/* Aerr */	AXXX,
	/* Anone */	AXXX,
};

static int disregmode[Aend] = {
	/* Aimm */	AXIMM,
	/* Amp */	AXINM,
	/* Ampind */	AXNON,
	/* Afp */	AXINF,
	/* Afpind */	AXNON,
	/* Apc */	AXIMM,
	/* Adesc */	AXIMM,
	/* Aoff */	AXIMM,
	/* Aerr */	AXNON,
	/* Anone */	AXNON,
};

enum
{
	MAXCON	= 4,
	MAXADDR	= 2*MAXCON,
	MAXINST	= 3*MAXADDR+2,
	NIBUF	= 1024
};

static	uchar	*ibuf;
static	int	nibuf;

void
disinst(Inst *in)
{
	ulong fileoff;

	fileoff = Boffset(bout);
	ibuf = allocmem(NIBUF);
	nibuf = 0;
	for(; in != nil; in = in->next){
		if(nibuf >= NIBUF-MAXINST){
			Bwrite(bout, ibuf, nibuf);
			nibuf = 0;
		}
		ibuf[nibuf++] = in->op;
		ibuf[nibuf++] = SRC(dismode[in->sm]) | DST(dismode[in->dm]) | disregmode[in->mm];
		if(in->mm != Anone)
			disaddr(in->mm, &in->m);
		if(in->sm != Anone)
			disaddr(in->sm, &in->s);
		if(in->dm != Anone)
			disaddr(in->dm, &in->d);
	}
	if(nibuf > 0)
		Bwrite(bout, ibuf, nibuf);
	free(ibuf);
	ibuf = nil;

	if(debug['s'])
		print("%ld instruction bytes start %ld\n", Boffset(bout) - fileoff, fileoff);
}

void
disaddr(int m, Addr *a)
{
	long val;

	val = 0;
	switch(m){
	case Anone:
	case Aerr:
	default:
		break;
	case Aimm:
	case Apc:
	case Adesc:
		val = a->offset;
		break;
	case Aoff:
		val = a->decl->iface->offset;
		break;
	case Afp:
	case Amp:
		val = a->reg;
		break;
	case Afpind:
	case Ampind:
		disbcon(a->reg);
		val = a->offset;
		break;
	}
	disbcon(val);
}

void
disbcon(long val)
{
	if(val >= -64 && val <= 63){
		ibuf[nibuf++] = val & ~0x80;
		return;
	}
	if(val >= -8192 && val <= 8191){
		ibuf[nibuf++] = val>>8 & ~0xC0 | 0x80;
		ibuf[nibuf++] = val;
		return;
	}
	if(val < 0 && ((val >> 29) & 7) != 7
	|| val > 0 && (val >> 29) != 0)
		fatal("overflow in constant 16r%lux", val);
	ibuf[nibuf++] = val>>24 | 0xC0;
	ibuf[nibuf++] = val>>16;
	ibuf[nibuf++] = val>>8;
	ibuf[nibuf++] = val;
}
