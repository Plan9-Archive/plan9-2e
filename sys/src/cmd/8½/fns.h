int	alnum(int);
void	bitclose(Window*);
void	bitopen(Window*);
int	bitread(Window*, int, int*);
int	bitwrite(Window*, char*, int, int*);
int	bitwinread(Window*, ulong, int);
void	bitwclose(Window*);
void	checkcursor(int);
void	current(Window*);
void	doubleclick(Window*, long*, long*);
void*	emalloc(ulong);
void*	erealloc(void*, ulong);
void	error(char*);
int	fork2(ulong);
long	getclock(void);
void	getmouse(void);
void	hidemenu(Window*, int);
void	io(void);
int	lmenuhit(int, Menu*);
long	max(long, long);
long	min(long, long);
Proc*	newproc(void (*)(void), void*);
int	newterm(Rectangle, int, int);
int	okrect(Rectangle);
void	run(Proc*);
void	sched(void);
void	scrdraw(Window*);
void	scroll(Window*, int, Mouse*);
void	scrorigin(Window*, int, long);
void	sendread(int, int, int, char*, int);
void	sendwrite(int, int, int, int);
int	spawn(int);
void	termborder(Window*);
void	termcut(Window*, int);
void	termdelete(Window*);
void	termfill(Window*);
void	termflush(int);
void	termhighlight(Window*, long, long);
void	termhold(Window*, int);
void	termmouseread(Window*, int, int, int);
void	termnote(Window*, char*);
void	termpaste(Window*);
void	termraw(Window*);
void	termread(Window*, int, int, int);
void	termreshapeall(void);
void	termrects(Window*, Rectangle);
void	termscroll(Window*);
void	termselect(Window*, Mouse*);
void	termsend(Window*);
void	termshow(Window*, ulong, int, int);
void	termsnarf(Window*);
Window*	termtop(Point);
void	termunraw(Window*);
Window*	termwhich(Point);
void	termwrite(Window*, int, int, char*, int);
int	termwrune(Window*);
void	textdelete(Text*, long, long);
void	textinsert(Window*, Text*, Rune*, int, ulong, int);
Rune*	strrune(Rune*, Rune);
