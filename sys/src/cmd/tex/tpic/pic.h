#ifndef PI
#define PI 3.1415926535897932384626433832795028841971693993751
#endif

#define	MAXWID	8.5	/* default limits max picture to 8.5 x 11; */
#define	MAXHT	11	/* change to taste without peril */

#define	dprintf	if(dbg)printf

extern	char	errbuf[200];
#define	ERROR	sprintf(errbuf,
#define	FATAL	), yyerror(errbuf), exit(1)
#define	WARNING	), yyerror(errbuf)

#define	DEFAULT	0

#define	HEAD1	1
#define	HEAD2	2
#define	HEAD12	(HEAD1+HEAD2)
#define	INVIS	4
#define	CW_ARC	8	/* clockwise arc */
#define	DOTBIT	16	/* line styles */
#define	DASHBIT	32
#define	FILLBIT	64	/* gray-fill on boxes, etc. */

#define	CENTER	01	/* text attributes */
#define	LJUST	02
#define	RJUST	04
#define	ABOVE	010
#define	BELOW	020
#define	SPREAD	040

#define	SCALE	1.0	/* default scale: units/inch */
#define	WID	0.75	/* default width for boxes and ellipses */
#define	WID2	0.375
#define	HT	0.5	/* default height and line length */
#define	HT2	(HT/2)
#define	HT5	(HT/5)
#define	HT10	(HT/10)

/* these have to be like so, so that we can write */
/* things like R & V, etc. */
#define	H	0
#define	V	1
#define	R_DIR	0
#define	U_DIR	1
#define	L_DIR	2
#define	D_DIR	3
#define	ishor(n)	(((n) & V) == 0)
#define	isvert(n)	(((n) & V) != 0)
#define	isright(n)	((n) == R_DIR)
#define	isleft(n)	((n) == L_DIR)
#define	isdown(n)	((n) == D_DIR)
#define	isup(n)		((n) == U_DIR)

typedef	float	ofloat;	/* for o_val[] in obj;  could be double */

typedef struct obj {	/* stores various things in variable length */
	int	o_type;
	int	o_count;	/* number of things */
	int	o_nobj;		/* index in objlist */
	int	o_mode;		/* hor or vert */
	float	o_x;		/* coord of "center" */
	float	o_y;
	int	o_nt1;		/* 1st index in text[] for this object */
	int	o_nt2;		/* 2nd; difference is #text strings */
	int	o_attr;		/* HEAD, CW, INVIS, etc., go here */
	int	o_size;		/* linesize */
	int	o_nhead;	/* arrowhead style */
	struct symtab *o_symtab; /* symtab for [...] */
	float	o_ddval;	/* value of dot/dash expression */
	float	o_fillval;	/* gray scale value */
	ofloat	o_val[1];	/* actually this will be > 1 in general */
				/* type is not always FLOAT!!!! */
} obj;

typedef union {		/* the yacc stack type */
	int	i;
	char	*p;
	obj	*o;
	double	f;
	struct symtab *st;
} YYSTYPE;

extern	YYSTYPE	yylval, yyval;

struct symtab {
	char	*s_name;
	int	s_type;
	YYSTYPE	s_val;
	struct symtab *s_next;
};

typedef struct {	/* attribute of an object */
	int	a_type;
	int	a_sub;
	YYSTYPE	a_val;
} Attr;

typedef struct {
	int	t_type;		/* CENTER, LJUST, etc. */
	char	t_op;		/* optional sign for size changes */
	char	t_size;		/* size, abs or rel */
	char	*t_val;
} Text;

#define	String	01
#define	Macro	02
#define	File	04
#define	Char	010
#define	Thru	020
#define	Free	040

typedef struct {	/* input source */
	int	type;	/* Macro, String, File */
	char	*sp;	/* if String or Macro */
} Src;

extern	Src	src[], *srcp;	/* input source stack */

typedef struct {
	FILE	*fin;
	char	*fname;
	int	lineno;
} Infile;

extern	Infile	infile[], *curfile;

#define	MAXARGS	20
typedef struct {	/* argument stack */
	char	*argstk[MAXARGS];	/* pointers to args */
	char	*argval;	/* points to space containing args */
} Arg;

extern	int	dbg;
extern	obj	**objlist;
extern	int	nobj, nobjlist;
extern	Attr	*attr;
extern	int	nattr, nattrlist;
extern	Text	*text;
extern	int	ntextlist;
extern	int	ntext;
extern	int	ntext1;
extern	double	curx, cury;
extern	int	hvmode;
extern	int	codegen;
extern	int	PEseen;
extern	char	*malloc(), *realloc(), *tostring(), *grow();
extern	double	getfval(), getcomp(), getblkvar();
extern	YYSTYPE	getvar();
extern	struct	symtab *lookup(), *makevar();
extern	char	*ifstat(), *delimstr(), *sprintgen();

extern	double	deltx, delty;
extern	int	lineno;
extern	int	synerr;

extern	double	xmin, ymin, xmax, ymax;
extern	obj	*leftthing(), *boxgen(), *circgen(), *arcgen();
extern	obj	*linegen(), *splinegen(), *movegen(), *textgen(), *plotgen();
extern	obj	*troffgen(), *rightthing(), *blockgen();
extern	obj	*makenode(), *makepos(), *fixpos(), *addpos(), *subpos(), *makebetween();
extern	obj	*getpos(), *gethere(), *getfirst(), *getlast(), *getblock();

struct pushstack {
	double	p_x;
	double	p_y;
	int	p_hvmode;
	double	p_xmin;
	double	p_ymin;
	double	p_xmax;
	double	p_ymax;
	struct symtab *p_symtab;
};
extern	struct pushstack stack[];
extern	int	nstack;
extern	int	cw;

extern	double	errcheck();
#define	Log10(x) errcheck(log10(x), "log")
#define	Exp(x)	errcheck(exp(x), "exp")
#define	Sqrt(x)	errcheck(sqrt(x), "sqrt")
