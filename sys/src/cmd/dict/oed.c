#include <u.h>
#include <libc.h>
#include <bio.h>
#include "dict.h"

enum {
	Buflen=1000,
	Maxaux=5,
};

/* Possible tags */
enum {
	A,		/* author in quote (small caps) */
	B,		/* bold */
	Ba,		/* author inside bib */
	Bch,		/* builtup chem component */
	Bib,		/* surrounds word 'in' for bibliographic ref */
	Bl,		/* bold */
	Bo,		/* bond over */
	Bu,		/* bond under */
	Cb,		/* ? block of stuff (indent) */
	Cf,		/* cross ref to another entry (italics) */
	Chem,		/* chemistry formula */
	Co,		/* over (preceding sum, integral, etc.) */
	Col,		/* column of table (aux just may be r) */
	Cu,		/* under (preceding sum, integral, etc.) */
	Dat,		/* date */
	Db,		/* def block? indent */
	Dn,		/* denominator of fraction */
	E,		/* main entry */
	Ed,		/* editor's comments (in [...]) */
	Etym,		/* etymology (in [...]) */
	Fq,		/* frequency count (superscript) */
	Form,		/* formula */
	Fr,		/* fraction (contains <nu>, then <dn>) */
	Gk,		/* greek (transliteration) */
	Gr,		/* grammar? (e.g., around 'pa.' in 'pa. pple.') */
	Hg,		/* headword group */
	Hm,		/* homonym (superscript) */
	Hw,		/* headword (bold) */
	I,		/* italics */
	Il,		/* italic list? */
	In,		/* inferior (subscript) */
	L,		/* row of col of table */
	La,		/* status or usage label (italic) */
	Lc,		/* chapter/verse sort of thing for works */
	N,		/* note (smaller type) */
	Nu,		/* numerator of fraction */
	Ov,		/* needs overline */
	P,		/* paragraph (indent) */
	Ph,		/* pronunciation (transliteration) */
	Pi,		/* pile (frac without line) */
	Pqp,		/* subblock of quote */
	Pr,		/* pronunciation (in (...)) */
	Ps,		/* position (e.g., adv.) (italic) */
	Pt,		/* part (in lc) */
	Q,		/* quote in quote block */
	Qd,		/* quote date (bold) */
	Qig,		/* quote number (greek) */
	Qla,		/* status or usage label in quote (italic) */
	Qp,		/* quote block (small type, indent) */
	Qsn,		/* quote number */
	Qt,		/* quote words */
	R,		/* roman type style */
	Rx,		/* relative cross reference (e.g., next) */
	S,		/* another form? (italic) */
	S0,		/* sense (sometimes surrounds several sx's) */
	S1,		/* sense (aux num: indented bold letter) */
	S2,		/* sense (aux num: indented bold capital rom num) */
	S3,		/* sense (aux num: indented number of asterisks) */
	S4,		/* sense (aux num: indented bold number) */
	S5,		/* sense (aux num: indented number of asterisks) */
	S6,		/* subsense (aux num: bold letter) */
	S7a,		/* subsense (aux num: letter) */
	S7n,		/* subsense (aux num: roman numeral) */
	Sc,		/* small caps */
	Sgk,		/* subsense (aux num: transliterated greek) */
	Sn,		/* sense of subdefinition (aux num: roman letter) */
	Ss,		/* sans serif */
	Ssb,		/* sans serif bold */
	Ssi,		/* sans serif italic */
	Su,		/* superior (superscript) */
	Sub,		/* subdefinition */
	Table,		/* table (aux cols=number of columns) */
	Tt,		/* title? (italics) */
	Vd,		/* numeric label for variant form */
	Ve,		/* variant entry */
	Vf,		/* variant form (light bold) */
	Vfl,		/* list of vf's (starts with Also or Forms) */
	W,		/* work (e.g., Beowulf) (italics) */
	X,		/* cross reference to main word (small caps) */
	Xd,		/* cross reference to quotation by date */
	Xi,		/* internal cross reference ? (italic) */
	Xid,		/* cross reference identifer, in quote ? */
	Xs,		/* cross reference sense (lower number) */
	Xr,		/* list of x's */
	Ntag		/* end of tags */
};

/* Assoc tables must be sorted on first field */

static Assoc tagtab[] = {
	{"a",		A},
	{"b",		B},
	{"ba",		Ba},
	{"bch",		Bch},
	{"bib",		Bib},
	{"bl",		Bl},
	{"bo",		Bo},
	{"bu",		Bu},
	{"cb",		Cb},
	{"cf",		Cf},
	{"chem",	Chem},
	{"co",		Co},
	{"col",		Col},
	{"cu",		Cu},
	{"dat",		Dat},
	{"db",		Db},
	{"dn",		Dn},
	{"e",		E},
	{"ed",		Ed},
	{"et",		Etym},
	{"etym",	Etym},
	{"form",	Form},
	{"fq",		Fq},
	{"fr",		Fr},
	{"frac",	Fr},
	{"gk",		Gk},
	{"gr",		Gr},
	{"hg",		Hg},
	{"hm",		Hm},
	{"hw",		Hw},
	{"i",		I},
	{"il",		Il},
	{"in",		In},
	{"l",		L},
	{"la",		La},
	{"lc",		Lc},
	{"n",		N},
	{"nu",		Nu},
	{"ov",		Ov},
	{"p",		P},
	{"ph",		Ph},
	{"pi",		Pi},
	{"pqp",		Pqp},
	{"pr",		Pr},
	{"ps",		Ps},
	{"pt",		Pt},
	{"q",		Q},
	{"qd",		Qd},
	{"qig",		Qig},
	{"qla",		Qla},
	{"qp",		Qp},
	{"qsn",		Qsn},
	{"qt",		Qt},
	{"r",		R},
	{"rx",		Rx},
	{"s",		S},
	{"s0",		S0},
	{"s1",		S1},
	{"s2",		S2},
	{"s3",		S3},
	{"s4",		S4},
	{"s5",		S5},
	{"s6",		S6},
	{"s7a",		S7a},
	{"s7n",		S7n},
	{"sc",		Sc},
	{"sgk",		Sgk},
	{"sn",		Sn},
	{"ss",		Ss,},
	{"ssb",		Ssb},
	{"ssi",		Ssi},
	{"su",		Su},
	{"sub",		Sub},
	{"table",	Table},
	{"tt",		Tt},
	{"vd",		Vd},
	{"ve",		Ve},
	{"vf",		Vf},
	{"vfl",		Vfl},
	{"w",		W},
	{"x",		X},
	{"xd",		Xd},
	{"xi",		Xi},
	{"xid",		Xid},
	{"xr",		Xr},
	{"xs",		Xs},
};

/* Possible tag auxilliary info */
enum {
	Cols,		/* number of columns in a table */
	Num,		/* letter or number, for a sense */
	St,		/* status (e.g., obs) */
	Naux
};

static Assoc auxtab[] = {
	{"cols",	Cols},
	{"num",		Num},
	{"st",		St}
};

static Assoc spectab[] = {
	{"3on4",	L'??'},
	{"Aacu",	L'??'},
	{"Aang",	L'??'},
	{"Abarab",	L'??'},
	{"Acirc",	L'??'},
	{"Ae",		L'??'},
	{"Agrave",	L'??'},
	{"Alpha",	L'??'},
	{"Amac",	L'??'},
	{"Asg",		L'??'},		/* Unicyle. Cf "Sake" */
	{"Auml",	L'??'},
	{"Beta",	L'??'},
	{"Cced",	L'??'},
	{"Chacek",	L'??'},
	{"Chi",		L'??'},
	{"Chirho",	L'???'},		/* Chi Rho U+2627 */
	{"Csigma",	L'??'},
	{"Delta",	L'??'},
	{"Eacu",	L'??'},
	{"Ecirc",	L'??'},
	{"Edh",		L'??'},
	{"Epsilon",	L'??'},
	{"Eta",		L'??'},
	{"Gamma",	L'??'},
	{"Iacu",	L'??'},
	{"Icirc",	L'??'},
	{"Imac",	L'??'},
	{"Integ",	L'???'},
	{"Iota",	L'??'},
	{"Kappa",	L'??'},
	{"Koppa",	L'??'},
	{"Lambda",	L'??'},
	{"Lbar",	L'??'},
	{"Mu",		L'??'},
	{"Naira",	L'N'},		/* should have bar through */
	{"Nplus",	L'N'},		/* should have plus above */
	{"Ntilde",	L'??'},
	{"Nu",		L'??'},
	{"Oacu",	L'??'},
	{"Obar",	L'??'},
	{"Ocirc",	L'??'},
	{"Oe",		L'??'},
	{"Omega",	L'??'},
	{"Omicron",	L'??'},
	{"Ouml",	L'??'},
	{"Phi",		L'??'},
	{"Pi",		L'??'},
	{"Psi",		L'??'},
	{"Rho",		L'??'},
	{"Sacu",	L'??'},
	{"Sigma",	L'??'},
	{"Summ",	L'???'},
	{"Tau",		L'??'},
	{"Th",		L'??'},
	{"Theta",	L'??'},
	{"Tse",		L'??'},
	{"Uacu",	L'??'},
	{"Ucirc",	L'??'},
	{"Upsilon",	L'??'},
	{"Uuml",	L'??'},
	{"Wyn",		L'??'},		/* wynn U+01BF */
	{"Xi",		L'??'},
	{"Ygh",		L'??'},		/* Yogh	U+01B7 */
	{"Zeta",	L'??'},
	{"Zh",		L'??'},		/* looks like Yogh. Cf "Sake" */
	{"a",		L'a'},		/* ante */
	{"aacu",	L'??'},
	{"aang",	L'??'},
	{"aasper",	MAAS},
	{"abreve",	L'??'},
	{"acirc",	L'??'},
	{"acu",		LACU},
	{"ae",		L'??'},
	{"agrave",	L'??'},
	{"ahook",	L'??'},
	{"alenis",	MALN},
	{"alpha",	L'??'},
	{"amac",	L'??'},
	{"amp",		L'&'},
	{"and",		MAND},
	{"ang",		LRNG},
	{"angle",	L'???'},
	{"ankh",	L'???'},		/* ankh U+2625 */
	{"ante",	L'a'},		/* before (year) */
	{"aonq",	MAOQ},
	{"appreq",	L'???'},
	{"aquar",	L'???'},
	{"arDadfull",	L'??'},		/* Dad U+0636 */
	{"arHa",	L'??'},		/* haa U+062D */
	{"arTa",	L'??'},		/* taa U+062A */
	{"arain",	L'??'},		/* ain U+0639 */
	{"arainfull",	L'??'},		/* ain U+0639 */
	{"aralif",	L'??'},		/* alef U+0627 */
	{"arba",	L'??'},		/* baa U+0628 */
	{"arha",	L'??'},		/* ha U+0647 */
	{"aries",	L'???'},
	{"arnun",	L'??'},		/* noon U+0646 */
	{"arnunfull",	L'??'},		/* noon U+0646 */
	{"arpa",	L'??'},		/* ha U+0647 */
	{"arqoph",	L'??'},		/* qaf U+0642 */
	{"arshinfull",	L'??'},		/* sheen U+0634 */
	{"arta",	L'??'},		/* taa U+062A */
	{"artafull",	L'??'},		/* taa U+062A */
	{"artha",	L'??'},		/* thaa U+062B */
	{"arwaw",	L'??'},		/* waw U+0648 */
	{"arya",	L'??'},		/* ya U+064A */
	{"aryafull",	L'??'},		/* ya U+064A */
	{"arzero",	L'??'},		/* indic zero U+0660 */
	{"asg",		L'??'},		/* unicycle character. Cf "hallow" */
	{"asper",	LASP},
	{"assert",	L'???'},
	{"astm",	L'???'},		/* asterism: should be upside down */
	{"at",		L'@'},
	{"atilde",	L'??'},
	{"auml",	L'??'},
	{"ayin",	L'??'},		/* arabic ain U+0639 */
	{"b1",		L'-'},		/* single bond */
	{"b2",		L'='},		/* double bond */
	{"b3",		L'???'},		/* triple bond */
	{"bbar",	L'??'},		/* b with bar U+0180 */
	{"beta",	L'??'},
	{"bigobl",	L'/'},
	{"blC",		L'C'},		/* should be black letter */
	{"blJ",		L'J'},		/* should be black letter */
	{"blU",		L'U'},		/* should be black letter */
	{"blb",		L'b'},		/* should be black letter */
	{"blozenge",	L'???'},		/* U+25CA; should be black */
	{"bly",		L'y'},		/* should be black letter */
	{"bra",		MBRA},
	{"brbl",	LBRB},
	{"breve",	LBRV},
	{"bslash",	L'\\'},
	{"bsquare",	L'???'},		/* black square U+25A0 */
	{"btril",	L'???'},		/* U+25C0 */
	{"btrir",	L'???'},		/* U+25B6 */
	{"c",		L'c'},		/* circa */
	{"cab",		L'???'},
	{"cacu",	L'??'},
	{"canc",	L'???'},
	{"capr",	L'???'},
	{"caret",	L'^'},
	{"cb",		L'}'},
	{"cbigb",	L'}'},
	{"cbigpren",	L')'},
	{"cbigsb",	L']'},
	{"cced",	L'??'},
	{"cdil",	LCED},
	{"cdsb",	L'???'},		/* ]] U+301b */
	{"cent",	L'??'},
	{"chacek",	L'??'},
	{"chi",		L'??'},
	{"circ",	LRNG},
	{"circa",	L'c'},		/* about (year) */
	{"circbl",	L'??'},		/* ring below accent U+0325 */
	{"circle",	L'???'},		/* U+25CB */
	{"circledot",	L'???'},
	{"click",	L'??'},
	{"club",	L'???'},
	{"comtime",	L'C'},
	{"conj",	L'???'},
	{"cprt",	L'??'},
	{"cq",		L'\''},
	{"cqq",		L'???'},
	{"cross",	L'???'},		/* maltese cross U+2720 */
	{"crotchet",	L'???'},
	{"csb",		L']'},
	{"ctilde",	L'c'},		/* +tilde */
	{"ctlig",	MLCT},
	{"cyra",	L'??'},
	{"cyre",	L'??'},
	{"cyrhard",	L'??'},
	{"cyrjat",	L'??'},
	{"cyrm",	L'??'},
	{"cyrn",	L'??'},
	{"cyrr",	L'??'},
	{"cyrsoft",	L'??'},
	{"cyrt",	L'??'},
	{"cyry",	L'??'},
	{"dag",		L'???'},
	{"dbar",	L'??'},
	{"dblar",	L'???'},
	{"dblgt",	L'???'},
	{"dbllt",	L'???'},
	{"dced",	L'd'},		/* +cedilla */
	{"dd",		MDD},
	{"ddag",	L'???'},
	{"ddd",		MDDD},
	{"decr",	L'???'},
	{"deg",		L'??'},
	{"dele",	L'd'},		/* should be dele */
	{"delta",	L'??'},
	{"descnode",	L'???'},		/* descending node U+260B */
	{"diamond",	L'???'},
	{"digamma",	L'??'},
	{"div",		L'??'},
	{"dlessi",	L'??'},
	{"dlessj1",	L'j'},		/* should be dotless */
	{"dlessj2",	L'j'},		/* should be dotless */
	{"dlessj3",	L'j'},		/* should be dotless */
	{"dollar",	L'$'},
	{"dotab",	LDOT},
	{"dotbl",	LDTB},
	{"drachm",	L'??'},
	{"dubh",	L'-'},
	{"eacu",	L'??'},
	{"earth",	L'???'},
	{"easper",	MEAS},
	{"ebreve",	L'??'},
	{"ecirc",	L'??'},
	{"edh",		L'??'},
	{"egrave",	L'??'},
	{"ehacek",	L'??'},
	{"ehook",	L'??'},
	{"elem",	L'???'},
	{"elenis",	MELN},
	{"em",		L'???'},
	{"emac",	L'??'},
	{"emem",	MEMM},
	{"en",		L'???'},
	{"epsilon",	L'??'},
	{"equil",	L'???'},
	{"ergo",	L'???'},
	{"es",		MES},
	{"eszett",	L'??'},
	{"eta",		L'??'},
	{"eth",		L'??'},
	{"euml",	L'??'},
	{"expon",	L'???'},
	{"fact",	L'!'},
	{"fata",	L'??'},
	{"fatpara",	L'??'},		/* should have fatter, filled in bowl */
	{"female",	L'???'},
	{"ffilig",	MLFFI},
	{"fflig",	MLFF},
	{"ffllig",	MLFFL},
	{"filig",	MLFI},
	{"flat",	L'???'},
	{"fllig",	MLFL},
	{"frE",		L'E'},		/* should be curly */
	{"frL",		L'L'},		/* should be curly */
	{"frR",		L'R'},		/* should be curly */
	{"frakB",	L'B'},		/* should have fraktur style */
	{"frakG",	L'G'},
	{"frakH",	L'H'},
	{"frakI",	L'I'},
	{"frakM",	L'M'},
	{"frakU",	L'U'},
	{"frakX",	L'X'},
	{"frakY",	L'Y'},
	{"frakh",	L'h'},
	{"frbl",	LFRB},
	{"frown",	LFRN},
	{"fs",		L' '},
	{"fsigma",	L'??'},
	{"gAacu",	L'??'},		/* should be ??+acute */
	{"gaacu",	L'??'},		/* +acute */
	{"gabreve",	L'??'},		/* +breve */
	{"gafrown",	L'??'},		/* +frown */
	{"gagrave",	L'??'},		/* +grave */
	{"gamac",	L'??'},		/* +macron */
	{"gamma",	L'??'},
	{"gauml",	L'??'},		/* +umlaut */
	{"ge",		L'???'},
	{"geacu",	L'??'},		/* +acute */
	{"gegrave",	L'??'},		/* +grave */
	{"ghacu",	L'??'},		/* +acute */
	{"ghfrown",	L'??'},		/* +frown */
	{"ghgrave",	L'??'},		/* +grave */
	{"ghmac",	L'??'},		/* +macron */
	{"giacu",	L'??'},		/* +acute */
	{"gibreve",	L'??'},		/* +breve */
	{"gifrown",	L'??'},		/* +frown */
	{"gigrave",	L'??'},		/* +grave */
	{"gimac",	L'??'},		/* +macron */
	{"giuml",	L'??'},		/* +umlaut */
	{"glagjat",	L'??'},
	{"glots",	L'??'},
	{"goacu",	L'??'},		/* +acute */
	{"gobreve",	L'??'},		/* +breve */
	{"grave",	LGRV},
	{"gt",		L'>'},
	{"guacu",	L'??'},		/* +acute */
	{"gufrown",	L'??'},		/* +frown */
	{"gugrave",	L'??'},		/* +grave */
	{"gumac",	L'??'},		/* +macron */
	{"guuml",	L'??'},		/* +umlaut */
	{"gwacu",	L'??'},		/* +acute */
	{"gwfrown",	L'??'},		/* +frown */
	{"gwgrave",	L'??'},		/* +grave */
	{"hacek",	LHCK},
	{"halft",	L'???'},
	{"hash",	L'#'},
	{"hasper",	MHAS},
	{"hatpath",	L'??'},		/* hataf patah U+05B2 */
	{"hatqam",	L'??'},		/* hataf qamats U+05B3 */
	{"hatseg",	L'??'},		/* hataf segol U+05B1 */
	{"hbar",	L'??'},
	{"heart",	L'???'},
	{"hebaleph",	L'??'},		/* aleph U+05D0 */
	{"hebayin",	L'??'},		/* ayin U+05E2 */
	{"hebbet",	L'??'},		/* bet U+05D1 */
	{"hebbeth",	L'??'},		/* bet U+05D1 */
	{"hebcheth",	L'??'},		/* bet U+05D7 */
	{"hebdaleth",	L'??'},		/* dalet U+05D3 */
	{"hebgimel",	L'??'},		/* gimel U+05D2 */
	{"hebhe",	L'??'},		/* he U+05D4 */
	{"hebkaph",	L'??'},		/* kaf U+05DB */
	{"heblamed",	L'??'},		/* lamed U+05DC */
	{"hebmem",	L'??'},		/* mem U+05DE */
	{"hebnun",	L'??'},		/* nun U+05E0 */
	{"hebnunfin",	L'??'},		/* final nun U+05DF */
	{"hebpe",	L'??'},		/* pe U+05E4 */
	{"hebpedag",	L'??'},		/* final pe? U+05E3 */
	{"hebqoph",	L'??'},		/* qof U+05E7 */
	{"hebresh",	L'??'},		/* resh U+05E8 */
	{"hebshin",	L'??'},		/* shin U+05E9 */
	{"hebtav",	L'??'},		/* tav U+05EA */
	{"hebtsade",	L'??'},		/* tsadi U+05E6 */
	{"hebwaw",	L'??'},		/* vav? U+05D5 */
	{"hebyod",	L'??'},		/* yod U+05D9 */
	{"hebzayin",	L'??'},		/* zayin U+05D6 */
	{"hgz",		L'??'},		/* ??? Cf "alet" */
	{"hireq",	L'??'},		/* U+05B4 */
	{"hlenis",	MHLN},
	{"hook",	LOGO},
	{"horizE",	L'E'},		/* should be on side */
	{"horizP",	L'P'},		/* should be on side */
	{"horizS",	L'???'},
	{"horizT",	L'???'},
	{"horizb",	L'{'},		/* should be underbrace */
	{"ia",		L'??'},
	{"iacu",	L'??'},
	{"iasper",	MIAS},
	{"ib",		L'??'},
	{"ibar",	L'??'},
	{"ibreve",	L'??'},
	{"icirc",	L'??'},
	{"id",		L'??'},
	{"ident",	L'???'},
	{"ie",		L'??'},
	{"ifilig",	MLFI},
	{"ifflig",	MLFF},
	{"ig",		L'??'},
	{"igrave",	L'??'},
	{"ih",		L'??'},
	{"ii",		L'??'},
	{"ik",		L'??'},
	{"ilenis",	MILN},
	{"imac",	L'??'},
	{"implies",	L'???'},
	{"index",	L'???'},
	{"infin",	L'???'},
	{"integ",	L'???'},
	{"intsec",	L'???'},
	{"invpri",	L'??'},
	{"iota",	L'??'},
	{"iq",		L'??'},
	{"istlig",	MLST},
	{"isub",	L'??'},		/* iota below accent */
	{"iuml",	L'??'},
	{"iz",		L'??'},
	{"jup",		L'???'},
	{"kappa",	L'??'},
	{"koppa",	L'??'},
	{"lambda",	L'??'},
	{"lar",		L'???'},
	{"lbar",	L'??'},
	{"le",		L'???'},
	{"lenis",	LLEN},
	{"leo",		L'???'},
	{"lhalfbr",	L'???'},
	{"lhshoe",	L'???'},
	{"libra",	L'???'},
	{"llswing",	MLLS},
	{"lm",		L'??'},
	{"logicand",	L'???'},
	{"logicor",	L'???'},
	{"longs",	L'??'},
	{"lrar",	L'???'},
	{"lt",		L'<'},
	{"ltappr",	L'???'},
	{"ltflat",	L'???'},
	{"lumlbl",	L'l'},		/* +umlaut below */
	{"mac",		LMAC},
	{"male",	L'???'},
	{"mc",		L'c'},		/* should be raised */
	{"merc",	L'???'},		/* mercury U+263F */
	{"min",		L'???'},
	{"moonfq",	L'???'},		/* first quarter moon U+263D */
	{"moonlq",	L'???'},		/* last quarter moon U+263E */
	{"msylab",	L'm'},		/* +sylab (??) */
	{"mu",		L'??'},
	{"nacu",	L'??'},
	{"natural",	L'???'},
	{"neq",		L'???'},
	{"nfacu",	L'???'},
	{"nfasper",	L'??'},
	{"nfbreve",	L'??'},
	{"nfced",	L'??'},
	{"nfcirc",	L'??'},
	{"nffrown",	L'???'},
	{"nfgra",	L'??'},
	{"nfhacek",	L'??'},
	{"nfmac",	L'??'},
	{"nftilde",	L'??'},
	{"nfuml",	L'??'},
	{"ng",		L'??'},
	{"not",		L'??'},
	{"notelem",	L'???'},
	{"ntilde",	L'??'},
	{"nu",		L'??'},
	{"oab",		L'???'},
	{"oacu",	L'??'},
	{"oasper",	MOAS},
	{"ob",		L'{'},
	{"obar",	L'??'},
	{"obigb",	L'{'},		/* should be big */
	{"obigpren",	L'('},
	{"obigsb",	L'['},		/* should be big */
	{"obreve",	L'??'},
	{"ocirc",	L'??'},
	{"odsb",	L'???'},		/* [[ U+301A */
	{"oe",		L'??'},
	{"oeamp",	L'&'},
	{"ograve",	L'??'},
	{"ohook",	L'o'},		/* +hook */
	{"olenis",	MOLN},
	{"omac",	L'??'},
	{"omega",	L'??'},
	{"omicron",	L'??'},
	{"ope",		L'??'},
	{"opp",		L'???'},
	{"oq",		L'`'},
	{"oqq",		L'???'},
	{"or",		MOR},
	{"osb",		L'['},
	{"otilde",	L'??'},
	{"ouml",	L'??'},
	{"ounce",	L'???'},		/* ounce U+2125 */
	{"ovparen",	L'???'},		/* should be sideways ( */
	{"p",		L'???'},
	{"pa",		L'???'},
	{"page",	L'P'},
	{"pall",	L'??'},
	{"paln",	L'??'},
	{"par",		PAR},
	{"para",	L'??'},
	{"pbar",	L'p'},		/* +bar */
	{"per",		L'???'},		/* per U+2118 */
	{"phi",		L'??'},
	{"phi2",	L'??'},
	{"pi",		L'??'},
	{"pisces",	L'???'},
	{"planck",	L'??'},
	{"plantinJ",	L'J'},		/* should be script */
	{"pm",		L'??'},
	{"pmil",	L'???'},
	{"pp",		L'???'},
	{"ppp",		L'???'},
	{"prop",	L'???'},
	{"psi",		L'??'},
	{"pstlg",	L'??'},
	{"q",		L'?'},		/* should be raised */
	{"qamets",	L'??'},		/* U+05B3 */
	{"quaver",	L'???'},
	{"rar",		L'???'},
	{"rasper",	MRAS},
	{"rdot",	L'??'},
	{"recipe",	L'???'},		/* U+211E */
	{"reg",		L'??'},
	{"revC",	L'??'},		/* open O U+0186 */
	{"reva",	L'??'},
	{"revc",	L'??'},
	{"revope",	L'??'},
	{"revr",	L'??'},
	{"revsc",	L'??'},		/* upside-down semicolon */
	{"revv",	L'??'},
	{"rfa",		L'o'},		/* +hook (Cf "goal") */
	{"rhacek",	L'??'},
	{"rhalfbr",	L'???'},
	{"rho",		L'??'},
	{"rhshoe",	L'???'},
	{"rlenis",	MRLN},
	{"rsylab",	L'r'},		/* +sylab */
	{"runash",	L'F'},		/* should be runic 'ash' */
	{"rvow",	L'??'},
	{"sacu",	L'??'},
	{"sagit",	L'???'},
	{"sampi",	L'??'},
	{"saturn",	L'???'},
	{"sced",	L'??'},
	{"schwa",	L'??'},
	{"scorpio",	L'???'},
	{"scrA",	L'A'},		/* should be script */
	{"scrC",	L'C'},
	{"scrE",	L'E'},
	{"scrF",	L'F'},
	{"scrI",	L'I'},
	{"scrJ",	L'J'},
	{"scrL",	L'L'},
	{"scrO",	L'O'},
	{"scrP",	L'P'},
	{"scrQ",	L'Q'},
	{"scrS",	L'S'},
	{"scrT",	L'T'},
	{"scrb",	L'b'},
	{"scrd",	L'd'},
	{"scrh",	L'h'},
	{"scrl",	L'l'},
	{"scruple",	L'???'},		/* U+2108 */
	{"sdd",		L'??'},
	{"sect",	L'??'},
	{"semE",	L'???'},
	{"sh",		L'??'},
	{"shacek",	L'??'},
	{"sharp",	L'???'},
	{"sheva",	L'??'},		/* U+05B0 */
	{"shti",	L'??'},
	{"shtsyll",	L'???'},
	{"shtu",	L'??'},
	{"sidetri",	L'???'},
	{"sigma",	L'??'},
	{"since",	L'???'},
	{"slge",	L'???'},		/* should have slanted line under */
	{"slle",	L'???'},		/* should have slanted line under */
	{"sm",		L'??'},
	{"smm",		L'??'},
	{"spade",	L'???'},
	{"sqrt",	L'???'},
	{"square",	L'???'},		/* U+25A1 */
	{"ssChi",	L'??'},		/* should be sans serif */
	{"ssIota",	L'??'},
	{"ssOmicron",	L'??'},
	{"ssPi",	L'??'},
	{"ssRho",	L'??'},
	{"ssSigma",	L'??'},
	{"ssTau",	L'??'},
	{"star",	L'*'},
	{"stlig",	MLST},
	{"sup2",	L'???'},
	{"supgt",	L'??'},
	{"suplt",	L'??'},
	{"sur",		L'??'},
	{"swing",	L'???'},
	{"tau",		L'??'},
	{"taur",	L'???'},
	{"th",		L'??'},
	{"thbar",	L'??'},		/* +bar */
	{"theta",	L'??'},
	{"thinqm",	L'?'},		/* should be thinner */
	{"tilde",	LTIL},
	{"times",	L'??'},
	{"tri",		L'???'},
	{"trli",	L'???'},
	{"ts",		L'???'},
	{"uacu",	L'??'},
	{"uasper",	MUAS},
	{"ubar",	L'u'},		/* +bar */
	{"ubreve",	L'??'},
	{"ucirc",	L'??'},
	{"udA",		L'???'},
	{"udT",		L'???'},
	{"uda",		L'??'},
	{"udh",		L'??'},
	{"udqm",	L'??'},
	{"udpsi",	L'???'},
	{"udtr",	L'???'},
	{"ugrave",	L'??'},
	{"ulenis",	MULN},
	{"umac",	L'??'},
	{"uml",		LUML},
	{"undl",	L'??'},		/* underline accent */
	{"union",	L'???'},
	{"upsilon",	L'??'},
	{"uuml",	L'??'},
	{"vavpath",	L'??'},		/* vav U+05D5 (+patah) */
	{"vavsheva",	L'??'},		/* vav U+05D5 (+sheva) */
	{"vb",		L'|'},
	{"vddd",	L'???'},
	{"versicle2",	L'???'},		/* U+2123 */
	{"vinc",	L'??'},
	{"virgo",	L'???'},
	{"vpal",	L'??'},
	{"vvf",		L'??'},
	{"wasper",	MWAS},
	{"wavyeq",	L'???'},
	{"wlenis",	MWLN},
	{"wyn",		L'??'},		/* wynn U+01BF */
	{"xi",		L'??'},
	{"yacu",	L'??'},
	{"ycirc",	L'??'},
	{"ygh",		L'??'},
	{"ymac",	L'y'},		/* +macron */
	{"yuml",	L'??'},
	{"zced",	L'z'},		/* +cedilla */
	{"zeta",	L'??'},
	{"zh",		L'??'},
	{"zhacek",	L'??'},
};
/*
   The following special characters don't have close enough
   equivalents in Unicode, so aren't in the above table.
	22n		2^(2^n) Cf Fermat
	2on4		2/4
	3on8		3/8
	Bantuo		Bantu O. Cf Otshi-herero
	Car		C with circular arrow on top
	albrtime 	cut-time: C with vertical line
	ardal		Cf dental
	bantuo		Bantu o. Cf Otshi-herero
	bbc1		single chem bond below
	bbc2		double chem bond below
	bbl1		chem bond like /
	bbl2		chem bond like //
	bbr1		chem bond like \
	bbr2		chem bond \\
	bcop1		copper symbol. Cf copper
	bcop2		copper symbol. Cf copper
	benchm		Cf benchmark
	btc1		single chem bond above
	btc2		double chem bond above
	btl1		chem bond like \
	btl2		chem bond like \\
	btr1		chem bond like /
	btr2		chem bond line //
	burman		Cf Burman
	devph		sanskrit letter. Cf ph
	devrfls		sanskrit letter. Cf cerebral
	duplong[12]	musical note
	egchi		early form of chi
	eggamma[12]	early form of gamma
	egiota		early form of iota
	egkappa		early form of kappa
	eglambda	early form of lambda
	egmu[12]	early form of mu
	egnu[12]	early form of nu
	egpi[123]	early form of pi
	egrho[12]	early form of rho
	egsampi		early form of sampi
	egsan		early form of san
	egsigma[12]	early form of sigma
	egxi[123]	early form of xi
	elatS		early form of S
	elatc[12]	early form of C
	elatg[12]	early form of G
	glagjeri	Slavonic Glagolitic jeri
	glagjeru	Slavonic Glagolitic jeru
	hypolem		hypolemisk (line with underdot)
	lhrbr		lower half }
	longmord	long mordent
	mbwvow		backwards scretched C. Cf retract.
	mord		music symbol.  Cf mordent
	mostra		Cf direct
	ohgcirc		old form of circumflex
	oldbeta		old form of ??. Cf perturbate
	oldsemibr[12]	old forms of semibreve. Cf prolation
	ormg		old form of g. Cf G
	para[12345]	form of ??
	pauseo		musical pause sign
	pauseu		musical pause sign
	pharyng		Cf pharyngal
	ragr		Black letter ragged r
	repetn		musical repeat. Cf retort
	segno		musical segno sign
	semain[12]	semitic ain
	semhe		semitic he
	semheth		semitic heth
	semkaph		semitic kaph
	semlamed[12]	semitic lamed
	semmem		semitic mem
	semnum		semitic nun
	sempe		semitic pe
	semqoph[123]	semitic qoph
	semresh		semitic resh
	semtav[1234]	semitic tav
	semyod		semitic yod
	semzayin[123]	semitic zayin
	shtlong[12]	U with underbar. Cf glyconic
	sigmatau	??,?? combination
	squaver		sixteenth note
	sqbreve		square musical breve note
	swast		swastika
	uhrbr		upper half of big }
	versicle1		Cf versicle
 */


static Rune normtab[128] = {
	/*0*/	/*1*/	/*2*/	/*3*/	/*4*/	/*5*/	/*6*/	/*7*/
/*00*/	NONE,	NONE,	NONE,	NONE,	NONE,	NONE,	NONE,	NONE,
	NONE,	NONE,	NONE,	NONE,	NONE,	NONE,	NONE,	NONE,
/*10*/	NONE,	NONE,	NONE,	NONE,	NONE,	NONE,	NONE,	NONE,
	NONE,	NONE,	NONE,	NONE,	NONE,	NONE,	NONE,	NONE,
/*20*/	L' ',	L'!',	L'"',	L'#',	L'$',	L'%',	SPCS,	L'\'',
	L'(',	L')',	L'*',	L'+',	L',',	L'-',	L'.',	L'/',
/*30*/  L'0',	L'1',	L'2',	L'3',	L'4',	L'5',	L'6',	L'7',
	L'8',	L'9',	L':',	L';',	TAGS,	L'=',	TAGE,	L'?',
/*40*/  L'@',	L'A',	L'B',	L'C',	L'D',	L'E',	L'F',	L'G',
	L'H',	L'I',	L'J',	L'K',	L'L',	L'M',	L'N',	L'O',
/*50*/	L'P',	L'Q',	L'R',	L'S',	L'T',	L'U',	L'V',	L'W',
	L'X',	L'Y',	L'Z',	L'[',	L'\\',	L']',	L'^',	L'_',
/*60*/	L'`',	L'a',	L'b',	L'c',	L'd',	L'e',	L'f',	L'g',
	L'h',	L'i',	L'j',	L'k',	L'l',	L'm',	L'n',	L'o',
/*70*/	L'p',	L'q',	L'r',	L's',	L't',	L'u',	L'v',	L'w',
	L'x',	L'y',	L'z',	L'{',	L'|',	L'}',	L'~',	NONE,
};
static Rune phtab[128] = {
	/*0*/	/*1*/	/*2*/	/*3*/	/*4*/	/*5*/	/*6*/	/*7*/
/*00*/	NONE,	NONE,	NONE,	NONE,	NONE,	NONE,	NONE,	NONE,
	NONE,	NONE,	NONE,	NONE,	NONE,	NONE,	NONE,	NONE,
/*10*/	NONE,	NONE,	NONE,	NONE,	NONE,	NONE,	NONE,	NONE,
	NONE,	NONE,	NONE,	NONE,	NONE,	NONE,	NONE,	NONE,
/*20*/	L' ',	L'!',	L'??',	L'#',	L'$',	L'??',	L'??',	L'\'',
	L'(',	L')',	L'*',	L'+',	L',',	L'-',	L'.',	L'/',
/*30*/  L'0',	L'1',	L'2',	L'??',	L'4',	L'5',	L'6',	L'7',
	L'8',	L'??',	L'??',	L';',	TAGS,	L'=',	TAGE,	L'?',
/*40*/  L'??',	L'??',	L'B',	L'C',	L'??',	L'??',	L'F',	L'G',
	L'H',	L'??',	L'J',	L'K',	L'L',	L'M',	L'??',	L'??',
/*50*/	L'P',	L'??',	L'R',	L'??',	L'??',	L'??',	L'??',	L'W',
	L'X',	L'Y',	L'??',	L'[',	L'\\',	L']',	L'^',	L'_',
/*60*/	L'`',	L'a',	L'b',	L'c',	L'd',	L'e',	L'f',	L'g',
	L'h',	L'i',	L'j',	L'k',	L'l',	L'm',	L'n',	L'o',
/*70*/	L'p',	L'q',	L'r',	L's',	L't',	L'u',	L'v',	L'w',
	L'x',	L'y',	L'z',	L'{',	L'|',	L'}',	L'~',	NONE,
};
static Rune grtab[128] = {
	/*0*/	/*1*/	/*2*/	/*3*/	/*4*/	/*5*/	/*6*/	/*7*/
/*00*/	NONE,	NONE,	NONE,	NONE,	NONE,	NONE,	NONE,	NONE,
	NONE,	NONE,	NONE,	NONE,	NONE,	NONE,	NONE,	NONE,
/*10*/	NONE,	NONE,	NONE,	NONE,	NONE,	NONE,	NONE,	NONE,
	NONE,	NONE,	NONE,	NONE,	NONE,	NONE,	NONE,	NONE,
/*20*/	L' ',	L'!',	L'"',	L'#',	L'$',	L'%',	SPCS,	L'\'',
	L'(',	L')',	L'*',	L'+',	L',',	L'-',	L'.',	L'/',
/*30*/  L'0',	L'1',	L'2',	L'3',	L'4',	L'5',	L'6',	L'7',
	L'8',	L'9',	L':',	L';',	TAGS,	L'=',	TAGE,	L'?',
/*40*/  L'@',	L'??',	L'??',	L'??',	L'??',	L'??',	L'??',	L'??',
	L'??',	L'??',	L'??',	L'??',	L'??',	L'??',	L'??',	L'??',
/*50*/	L'??',	L'??',	L'??',	L'??',	L'??',	L'??',	L'V',	L'??',
	L'??',	L'??',	L'??',	L'[',	L'\\',	L']',	L'^',	L'_',
/*60*/	L'`',	L'??',	L'??',	L'??',	L'??',	L'??',	L'??',	L'??',
	L'??',	L'??',	L'??',	L'??',	L'??',	L'??',	L'??',	L'??',
/*70*/	L'??',	L'??',	L'??',	L'??',	L'??',	L'??',	L'v',	L'??',
	L'??',	L'??',	L'??',	L'{',	L'|',	L'}',	L'~',	NONE,
};
static Rune subtab[128] = {
	/*0*/	/*1*/	/*2*/	/*3*/	/*4*/	/*5*/	/*6*/	/*7*/
/*00*/	NONE,	NONE,	NONE,	NONE,	NONE,	NONE,	NONE,	NONE,
	NONE,	NONE,	NONE,	NONE,	NONE,	NONE,	NONE,	NONE,
/*10*/	NONE,	NONE,	NONE,	NONE,	NONE,	NONE,	NONE,	NONE,
	NONE,	NONE,	NONE,	NONE,	NONE,	NONE,	NONE,	NONE,
/*20*/	L' ',	L'!',	L'"',	L'#',	L'$',	L'%',	SPCS,	L'\'',
	L'???',	L'???',	L'*',	L'???',	L',',	L'???',	L'.',	L'/',
/*30*/  L'???',	L'???',	L'???',	L'???',	L'???',	L'???',	L'???',	L'???',
	L'???',	L'???',	L':',	L';',	TAGS,	L'???',	TAGE,	L'?',
/*40*/  L'@',	L'A',	L'B',	L'C',	L'D',	L'E',	L'F',	L'G',
	L'H',	L'I',	L'J',	L'K',	L'L',	L'M',	L'N',	L'O',
/*50*/	L'P',	L'Q',	L'R',	L'S',	L'T',	L'U',	L'V',	L'W',
	L'X',	L'Y',	L'Z',	L'[',	L'\\',	L']',	L'^',	L'_',
/*60*/	L'`',	L'a',	L'b',	L'c',	L'd',	L'e',	L'f',	L'g',
	L'h',	L'i',	L'j',	L'k',	L'l',	L'm',	L'n',	L'o',
/*70*/	L'p',	L'q',	L'r',	L's',	L't',	L'u',	L'v',	L'w',
	L'x',	L'y',	L'z',	L'{',	L'|',	L'}',	L'~',	NONE,
};
static Rune suptab[128] = {
	/*0*/	/*1*/	/*2*/	/*3*/	/*4*/	/*5*/	/*6*/	/*7*/
/*00*/	NONE,	NONE,	NONE,	NONE,	NONE,	NONE,	NONE,	NONE,
	NONE,	NONE,	NONE,	NONE,	NONE,	NONE,	NONE,	NONE,
/*10*/	NONE,	NONE,	NONE,	NONE,	NONE,	NONE,	NONE,	NONE,
	NONE,	NONE,	NONE,	NONE,	NONE,	NONE,	NONE,	NONE,
/*20*/	L' ',	L'!',	L'"',	L'#',	L'$',	L'%',	SPCS,	L'\'',
	L'???',	L'???',	L'*',	L'???',	L',',	L'???',	L'.',	L'/',
/*30*/  L'???',	L'???',	L'???',	L'???',	L'???',	L'???',	L'???',	L'???',
	L'???',	L'???',	L':',	L';',	TAGS,	L'???',	TAGE,	L'?',
/*40*/  L'@',	L'A',	L'B',	L'C',	L'D',	L'E',	L'F',	L'G',
	L'H',	L'I',	L'J',	L'K',	L'L',	L'M',	L'N',	L'O',
/*50*/	L'P',	L'Q',	L'R',	L'S',	L'T',	L'U',	L'V',	L'W',
	L'X',	L'Y',	L'Z',	L'[',	L'\\',	L']',	L'^',	L'_',
/*60*/	L'`',	L'a',	L'b',	L'c',	L'd',	L'e',	L'f',	L'g',
	L'h',	L'i',	L'j',	L'k',	L'l',	L'm',	L'n',	L'o',
/*70*/	L'p',	L'q',	L'r',	L's',	L't',	L'u',	L'v',	L'w',
	L'x',	L'y',	L'z',	L'{',	L'|',	L'}',	L'~',	NONE,
};

static int	tagstarts;
static char	tag[Buflen];
static int	naux;
static char	auxname[Maxaux][Buflen];
static char	auxval[Maxaux][Buflen];
static char	spec[Buflen];
static char	*auxstate[Naux];	/* vals for most recent tag */
static Entry	curentry;
#define cursize (curentry.end-curentry.start)

static char	*getspec(char *, char *);
static char	*gettag(char *, char *);
static void	dostatus(void);

/*
 * cmd is one of:
 *    'p': normal print
 *    'h': just print headwords
 *    'P': print raw
 */
void
oedprintentry(Entry e, int cmd)
{
	char *p, *pe;
	int t, a, i;
	long r, rprev, rlig;
	Rune *transtab;

	p = e.start;
	pe = e.end;
	transtab = normtab;
	rprev = NONE;
	changett(0, 0, 0);
	curentry = e;
	if(cmd == 'h')
		outinhibit = 1;
	while(p < pe) {
		if(cmd == 'r') {
			outchar(*p++);
			continue;
		}
		r = transtab[(*p++)&0x7F];
		if(r < NONE) {
			/* Emit the rune, but buffer in case of ligature */
			if(rprev != NONE)
				outrune(rprev);
			rprev = r;
		} else if(r == SPCS) {
			/* Start of special character name */
			p = getspec(p, pe);
			r = lookassoc(spectab, asize(spectab), spec);
			if(r == -1) {
				if(debug)
					err("spec %ld %d %s",
						e.doff, cursize, spec);
				r = L'???';
			}
			if(r >= LIGS && r < LIGE) {
				/* handle possible ligature */
				rlig = liglookup(r, rprev);
				if(rlig != NONE)
					rprev = rlig;	/* overwrite rprev */
				else {
					/* could print accent, but let's not */
					if(rprev != NONE) outrune(rprev);
					rprev = NONE;
				}
			} else if(r >= MULTI && r < MULTIE) {
				if(rprev != NONE) {
					outrune(rprev);
					rprev = NONE;
				}
				outrunes(multitab[r-MULTI]);
			} else if(r == PAR) {
				if(rprev != NONE) {
					outrune(rprev);
					rprev = NONE;
				}
				outnl(1);
			} else {
				if(rprev != NONE) outrune(rprev);
				rprev = r;
			}
		} else if(r == TAGS) {
			/* Start of tag name */
			if(rprev != NONE) {
				outrune(rprev);
				rprev = NONE;
			}
			p = gettag(p, pe);
			t = lookassoc(tagtab, asize(tagtab), tag);
			if(t == -1) {
				if(debug)
					err("tag %ld %d %s",
						e.doff, cursize, tag);
				continue;
			}
			for(i = 0; i < Naux; i++)
				auxstate[i] = 0;
			for(i = 0; i < naux; i++) {
				a = lookassoc(auxtab, asize(auxtab), auxname[i]);
				if(a == -1) {
					if(debug)
						err("aux %ld %d %s",
							e.doff, cursize, auxname[i]);
				} else
					auxstate[a] = auxval[i];
			}
			switch(t){
			case E:
			case Ve:
				outnl(0);
				if(tagstarts)
					dostatus();
				break;
			case Ed:
			case Etym:
				outchar(tagstarts? '[' : ']');
				break;
			case Pr:
				outchar(tagstarts? '(' : ')');
				break;
			case In:
				transtab = changett(transtab, subtab, tagstarts);
				break;
			case Hm:
			case Su:
			case Fq:
				transtab = changett(transtab, suptab, tagstarts);
				break;
			case Gk:
				transtab = changett(transtab, grtab, tagstarts);
				break;
			case Ph:
				transtab = changett(transtab, phtab, tagstarts);
				break;
			case Hw:
				if(cmd == 'h') {
					if(!tagstarts)
						outchar(' ');
					outinhibit = !tagstarts;
				}
				break;
			case S0:
			case S1:
			case S2:
			case S3:
			case S4:
			case S5:
			case S6:
			case S7a:
			case S7n:
			case Sn:
			case Sgk:
				if(tagstarts) {
					outnl(2);
					dostatus();
					if(auxstate[Num]) {
						if(t == S3 || t == S5) {
							i = atoi(auxstate[Num]);
							while(i--)
								outchar('*');
							outchars("  ");
						} else if(t == S7a || t == S7n || t == Sn) {
							outchar('(');
							outchars(auxstate[Num]);
							outchars(") ");
						} else if(t == Sgk) {
							i = grtab[auxstate[Num][0]];
							if(i != NONE)
								outrune(i);
							outchars(".  ");
						} else {
							outchars(auxstate[Num]);
							outchars(".  ");
						}
					}
				}
				break;
			case Cb:
			case Db:
			case Qp:
			case P:
				if(tagstarts)
					outnl(1);
				break;
			case Table:
				/*
				 * Todo: gather columns, justify them, etc.
				 * For now, just let colums come out as rows
				 */
				if(!tagstarts)
					outnl(0);
				break;
			case Col:
				if(tagstarts)
					outnl(0);
				break;
			case Dn:
				if(tagstarts)
					outchar('/');
				break;
			}
		}
	}
	if(cmd == 'h') {
		outinhibit = 0;
		outnl(0);
	}
}

/*
 * Return offset into bdict where next oed entry after fromoff starts.
 * Oed entries start with <e>, <ve>, <e st=...>, or <ve st=...>
 */
long
oednextoff(long fromoff)
{
	long a, n;
	int c;

	a = Bseek(bdict, fromoff, 0);
	if(a < 0)
		return -1;
	n = 0;
	for(;;) {
		c = Bgetc(bdict);
		if(c < 0)
			break;
		if(c == '<') {
			c = Bgetc(bdict);
			if(c == 'e') {
				c = Bgetc(bdict);
				if(c == '>' || c == ' ')
					n = 3;
			} else if(c == 'v' && Bgetc(bdict) == 'e') {
				c = Bgetc(bdict);
				if(c == '>' || c == ' ')
					n = 4;
			}
			if(n)
				break;
		}
	}
	return (Boffset(bdict)-n);
}

static char *prkey =
"KEY TO THE PRONUNCIATION\n"
"\n"
"I. CONSONANTS\n"
"b, d, f, k, l, m, n, p, t, v, z: usual English values\n"
"\n"
"g as in go (g????)\n"
"h  ...  ho! (h????)\n"
"r  ...  run (r??n), terrier (??t??ri??(r))\n"
"(r)...  her (h????(r))\n"
"s  ...  see (si??), success (s??k??s??s)\n"
"w  ...  wear (w????(r))\n"
"hw ...  when (hw??n)\n"
"j  ...  yes (j??s)\n"
"??  ...  thin (??in), bath (b??????)\n"
"??  ...  then (????n), bathe (be????)\n"
"??  ...  shop (????p), dish (d????)\n"
"t?? ...  chop (t????p), ditch (d??t??)\n"
"??  ...  vision (??v??????n), d??jeuner (de????ne)\n"
"d?? ...  judge (d????d??)\n"
"??  ...  singing (??s????????), think (??i??k)\n"
"??g ...  finger (??fi??g??(r))\n"
"\n"
"Foreign\n"
"?? as in It. seraglio (ser??ra??o)\n"
"??  ...  Fr. cognac (k????ak)\n"
"x  ...  Ger. ach (ax), Sc. loch (l??x)\n"
"??  ...  Ger. ich (????), Sc. nicht (n????t)\n"
"??  ...  North Ger. sagen (??za??????n)\n"
"c  ...  Afrikaans baardmannetjie (??ba??rtman??ci)\n"
"??  ...  Fr. cuisine (k??izin)\n"
"\n"
"II. VOWELS AND DIPTHONGS\n"
"\n"
"Short\n"
"?? as in pit (p??t), -ness (-n??s)\n"
"??  ...  pet (p??t), Fr. sept (s??t)\n"
"??  ...  pat (p??t)\n"
"??  ...  putt (p??t)\n"
"??  ...  pot (p??t)\n"
"??  ...  put (p??t)\n"
"??  ...  another (????n??????(r))\n"
"(??)...  beaten (??bi??t(??)n)\n"
"i  ...  Fr. si (si)\n"
"e  ...  Fr. b??b?? (bebe)\n"
"a  ...  Fr. mari (mari)\n"
"??  ...  Fr. b??timent (b??tim??)\n"
"??  ...  Fr. homme (??m)\n"
"o  ...  Fr. eau (o)\n"
"??  ...  Fr. peu (p??)\n"
"??  ...  Fr. boeuf (b??f), coeur (k??r)\n"
"u  ...  Fr. douce (dus)\n"
"??  ...  Ger. M??ller (??m??l??r)\n"
"y  ...  Fr. du (dy)\n"
"\n"
"Long\n"
"i?? as in bean (bi??n)\n"
"???? ...  barn (b????n)\n"
"???? ...  born (b????n)\n"
"u?? ...  boon (bu??n)\n"
"???? ...  burn (b????n)\n"
"e?? ...  Ger. Schnee (??ne??)\n"
"???? ...  Ger. F??hre (??f????r??)\n"
"a?? ...  Ger. Tag (ta??k)\n"
"o?? ...  Ger. Sohn (zo??n)\n"
"???? ...  Ger. Goethe (g????t??)\n"
"y?? ...  Ger. gr??n (gry??n)\n"
"\n"
"Nasal\n"
"????, ???? as in Fr. fin (f????, f????)\n"
"??  ...  Fr. franc (fr??)\n"
"???? ...  Fr. bon (b????n)\n"
"???? ...  Fr. un (????)\n"
"\n"
"Dipthongs, etc.\n"
"e?? as in bay (be??)\n"
"a?? ...  buy (ba??)\n"
"???? ...  boy (b????)\n"
"???? ...  no (n????)\n"
"a?? ...  now (na??)\n"
"???? ...  peer (p????(r))\n"
"???? ...  pair (p????(r))\n"
"???? ...  tour (t????(r))\n"
"???? ...  boar (b????(r))\n"
"\n"
"III. STRESS\n"
"\n"
"Main stress: ?? preceding stressed syllable\n"
"Secondary stress: ?? preceding stressed syllable\n"
"\n"
"E.g.: pronunciation (pr????n??ns????e????(??)n)\n";
/* TODO: find transcriptions of foreign consonents, ??, ??, nasals */

void
oedprintkey(void)
{
	Bprint(bout, "%s", prkey);
}

/*
 * f points just after a '&', fe points at end of entry.
 * Accumulate the special name, starting after the &
 * and continuing until the next '.', in spec[].
 * Return pointer to char after '.'.
 */
static char *
getspec(char *f, char *fe)
{
	char *t;
	int c, i;

	t = spec;
	i = sizeof spec;
	while(--i > 0) {
		c = *f++;
		if(c == '.' || f == fe)
			break;
		*t++ = c;
	}
	*t = 0;
	return f;
}

/*
 * f points just after '<'; fe points at end of entry.
 * Expect next characters from bin to match:
 *  [/][^ >]+( [^>=]+=[^ >]+)*>
 *      tag   auxname auxval
 * Accumulate the tag and its auxilliary information in
 * tag[], auxname[][] and auxval[][].
 * Set tagstarts=1 if the tag is 'starting' (has no '/'), else 0.
 * Set naux to the number of aux pairs found.
 * Return pointer to after final '>'.
 */
static char *
gettag(char *f, char *fe)
{
	char *t;
	int c, i;

	t = tag;
	c = *f++;
	if(c == '/')
		tagstarts = 0;
	else {
		tagstarts = 1;
		*t++ = c;
	}
	i = Buflen;
	naux = 0;
	while(--i > 0) {
		c = *f++;
		if(c == '>' || f == fe)
			break;
		if(c == ' ') {
			*t = 0;
			t = auxname[naux];
			i = Buflen;
			if(naux < Maxaux-1)
				naux++;
		} else if(naux && c == '=') {
			*t = 0;
			t = auxval[naux-1];
			i = Buflen;
		} else
			*t++ = c;
	}
	*t = 0;
	return f;
}

static void
dostatus(void)
{
	char *s;

	s = auxstate[St];
	if(s) {
		if(strcmp(s, "obs") == 0)
			outrune(L'???');
		else if(strcmp(s, "ali") == 0)
			outrune(L'???');
		else if(strcmp(s, "err") == 0 || strcmp(s, "spu") == 0)
			outrune(L'??');
		else if(strcmp(s, "xref") == 0)
			/* nothing */;
		else if(debug)
			err("status %ld %d %s", curentry.doff, cursize, s);
	}
}
