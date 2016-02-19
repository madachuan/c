struct ll {
        char key;
        struct ll *next;
};

struct ll *llcreate(void);
struct ll *lltail(struct ll *p);
struct ll *llmax(struct ll *p);
struct ll *llmin(struct ll *p);
struct ll *llfind(struct ll *p, char key);
struct ll *llget(struct ll *p, char key);
struct ll *llstrip(struct ll *p, struct ll *node);
struct ll *llbehead(struct ll *p);
int lllen(struct ll *p);
int llscan(struct ll *p, char key);
int lldelscan(struct ll *p, char key);
int llrep(struct ll *p, char key, char new);
void llins(struct ll *p, char key);
void llinstail(struct ll *p, char key);
void lladd(struct ll *p, struct ll *node);
void lladdtail(struct ll *p, struct ll *node);
void lldel(struct ll *p);
void lldeltail(struct ll *p);
void lldelfind(struct ll *p, char key);
void llbuild(struct ll *p, int n, char key);
void llinit(struct ll *p, int n, char *buf);
void llcpy(char *buf, int n, struct ll *p);
void llclean(struct ll *p);
void lldestroy(struct ll *p);
void llrev(struct ll *p); 
void llwrap(struct ll *p);
