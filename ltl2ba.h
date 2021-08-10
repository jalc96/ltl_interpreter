/***** ltl2ba : ltl2ba.h *****/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


typedef struct Symbol {
char		*name;
	struct Symbol	*next;	/* linked list, symbol table */
} Symbol;

typedef struct Node {
	short		ntyp;	/* node type */
	struct Symbol	*sym;
	struct Node	*lft;	/* tree */
	struct Node	*rgt;	/* tree */
	struct Node	*nxt;	/* if linked list */
} Node;

typedef struct Graph {
	Symbol		*name;
	Symbol		*incoming;
	Symbol		*outgoing;
	Symbol		*oldstring;
	Symbol		*nxtstring;
	Node		*New;
	Node		*Old;
	Node		*Other;
	Node		*Next;
	unsigned char	isred[64], isgrn[64];
	unsigned char	redcnt, grncnt;
	unsigned char	reachable;
	struct Graph	*nxt;
} Graph;

typedef struct Mapping {
	char	*from;
	Graph	*to;
	struct Mapping	*nxt;
} Mapping;

typedef struct ATrans {
  int *to;
  int *pos;
  int *neg;
  struct ATrans *nxt;
} ATrans;

typedef struct AProd {
  int astate;
  struct ATrans *prod;
  struct ATrans *trans;
  struct AProd *nxt;
  struct AProd *prv;
} AProd;


typedef struct GTrans {
  int *pos;
  int *neg;
  struct GState *to;
  int *final;
  struct GTrans *nxt;
} GTrans;

typedef struct GState {
  int id;
  int incoming;
  int *nodes_set;
  struct GTrans *trans;
  struct GState *nxt;
  struct GState *prv;
} GState;

typedef struct BTrans {
  struct BState *to;
  int *pos;
  int *neg;
  struct BTrans *nxt;
} BTrans;

typedef struct BState {
  struct GState *gstate;
  int id;
  int incoming;
  int final;
  struct BTrans *trans;
  struct BState *nxt;
  struct BState *prv;
} BState;

typedef struct GScc {
  struct GState *gstate;
  int rank;
  int theta;
  struct GScc *nxt;
} GScc;

typedef struct BScc {
  struct BState *bstate;
  int rank;
  int theta;
  struct BScc *nxt;
} BScc;

enum {
	ALWAYS=257,
	AND,		/* 258 */
	EQUIV,		/* 259 */
	EVENTUALLY,	/* 260 */
	FALSE,		/* 261 */
	IMPLIES,	/* 262 */
	NOT,		/* 263 */
	OR,		/* 264 */
	PREDICATE,	/* 265 */
	TRUE,		/* 266 */
	U_OPER,		/* 267 */
	V_OPER		/* 268 */
#ifdef NXT
	, NEXT		/* 269 */
#endif
};

Node	*Canonical(Node *);
Node	*canonical(Node *);
Node	*cached(Node *);
Node	*dupnode(Node *);
Node	*getnode(Node *);
Node	*in_cache(Node *);
Node	*push_negation(Node *);
Node	*right_linked(Node *);
Node	*tl_nn(int, Node *, Node *);

Symbol	*tl_lookup(char *);
Symbol	*getsym(Symbol *);
Symbol	*DoDump(Node *);

char	*emalloc(int);	

int	anywhere(int, Node *, Node *);
int	dump_cond(Node *, Node *, int);
int	isequal(Node *, Node *);
int	tl_Getchar(void);

void	*tl_emalloc(int);
ATrans  *emalloc_atrans();
void    free_atrans(ATrans *, int);
void    free_all_atrans();
GTrans  *emalloc_gtrans();
void    free_gtrans(GTrans *, GTrans *, int);
BTrans  *emalloc_btrans();
void    free_btrans(BTrans *, BTrans *, int);
void	a_stats(void);
void	addtrans(Graph *, char *, Node *, char *);
void	cache_stats(void);
void	dump(Node *);
void	Fatal(const char *);
void	fatal(const char *);
void	fsm_print(void);
void	releasenode(int, Node *);
void	tfree(void *);
void	tl_explain(int);
void	tl_UnGetchar(void);
void	tl_parse(void);
void	tl_yyerror(char *);
void	trans(Node *);

void    mk_alternating(Node *);
void    mk_generalized();
void    mk_buchi();

ATrans *dup_trans(ATrans *);
ATrans *merge_trans(ATrans *, ATrans *);
void do_merge_trans(ATrans **, ATrans *, ATrans *);

int  *new_set(int);
int  *clear_set(int *, int);
int  *make_set(int , int);
void copy_set(int *, int *, int);
int  *dup_set(int *, int);
void merge_sets(int *, int *, int);
void do_merge_sets(int *, int *, int *, int);
int  *intersect_sets(int *, int *, int);
void add_set(int *, int);
void rem_set(int *, int);
void spin_print_set(int *, int*);
void print_set(int *, int);
int  empty_set(int *, int);
int  empty_intersect_sets(int *, int *, int);
int  same_sets(int *, int *, int);
int  included_set(int *, int *, int);
int  in_set(int *, int);
int  *list_set(int *, int);

int timeval_subtract (struct timeval *, struct timeval *, struct timeval *);

void put_uform(void);

#define ZN	(Node *)0
#define ZS	(Symbol *)0
#define Nhash	255    	
#define True	tl_nn(TRUE,  ZN, ZN)
#define False	tl_nn(FALSE, ZN, ZN)
#define Not(a)	push_negation(tl_nn(NOT, a, ZN))
#define rewrite(n)	canonical(right_linked(n))

typedef Node	*Nodeptr;
#define YYSTYPE	 Nodeptr

#define Debug(x)	{ if (0) printf(x); }
#define Debug2(x,y)	{ if (tl_verbose) printf(x,y); }
#define Dump(x)		{ if (0) dump(x); }
#define Explain(x)	{ if (tl_verbose) tl_explain(x); }

#define Assert(x, y)	{ if (!(x)) { tl_explain(y); \
			  Fatal(": assertion failed\n"); } }
#define min(x,y)        ((x<y)?x:y)



/***** ltl2ba : alternating.c *****/




/********************************************************************\
|*              Structures and shared variables                     *|
\********************************************************************/
// VARS
extern FILE *tl_out;
extern int tl_verbose, tl_stats, tl_simp_diff;

Node **label;
char **sym_table;
ATrans **transition;
int *final_set, node_id = 1, sym_id = 0, node_size, sym_size;
int astate_count = 0, atrans_count = 0;

ATrans *build_alternating(Node *p);

/********************************************************************\
|*              Generation of the alternating automaton             *|
\********************************************************************/

int calculate_node_size(Node *p) {
  switch(p->ntyp) {
  case AND:
  case OR:
  case U_OPER:
  case V_OPER:
    return(calculate_node_size(p->lft) + calculate_node_size(p->rgt) + 1);
#ifdef NXT
  case NEXT:
    return(calculate_node_size(p->lft) + 1);
#endif
  default:
    return 1;
    break;
  }
}

int calculate_sym_size(Node *p) {
  switch(p->ntyp) {
  case AND:
  case OR:
  case U_OPER:
  case V_OPER:
    return(calculate_sym_size(p->lft) + calculate_sym_size(p->rgt));
#ifdef NXT
  case NEXT:
    return(calculate_sym_size(p->lft));
#endif
  case NOT:
  case PREDICATE:
    return 1;
  default:
    return 0;
  }
}

ATrans *dup_trans(ATrans *trans) {
  ATrans *result;
  if(!trans) return trans;
  result = emalloc_atrans();
  copy_set(trans->to,  result->to,  0);
  copy_set(trans->pos, result->pos, 1);
  copy_set(trans->neg, result->neg, 1);
  return result;
}

void do_merge_trans(ATrans **result, ATrans *trans1, ATrans *trans2) {
  if(!trans1 || !trans2) {
    free_atrans(*result, 0);
    *result = (ATrans *)0;
    return;
  }
  if(!*result)
    *result = emalloc_atrans();
  do_merge_sets((*result)->to, trans1->to,  trans2->to,  0);
  do_merge_sets((*result)->pos, trans1->pos, trans2->pos, 1);
  do_merge_sets((*result)->neg, trans1->neg, trans2->neg, 1);
  if(!empty_intersect_sets((*result)->pos, (*result)->neg, 1)) {
    free_atrans(*result, 0);
    *result = (ATrans *)0;
  }
}

ATrans *merge_trans(ATrans *trans1, ATrans *trans2) {
  ATrans *result = emalloc_atrans();
  do_merge_trans(&result, trans1, trans2);
  return result;
}

int already_done(Node *p) {
  int i;
  for(i = 1; i<node_id; i++) 
    if (isequal(p, label[i])) 
      return i;
  return -1;
}

int get_sym_id(char *s) {
  int i;
  for(i=0; i<sym_id; i++) 
    if (!strcmp(s, sym_table[i])) 
      return i;
  sym_table[sym_id] = s;
  return sym_id++;
}

ATrans *ltl2ba_boolean(Node *p) {
  ATrans *t1, *t2, *lft, *rgt, *result = (ATrans *)0;
  switch(p->ntyp) {
  case TRUE:
    result = emalloc_atrans();
    clear_set(result->to,  0);
    clear_set(result->pos, 1);
    clear_set(result->neg, 1);
  case FALSE:
    break;
  case AND:
    lft = ltl2ba_boolean(p->lft);
    rgt = ltl2ba_boolean(p->rgt);
    for(t1 = lft; t1; t1 = t1->nxt) {
      for(t2 = rgt; t2; t2 = t2->nxt) {
  ATrans *tmp = merge_trans(t1, t2);
  if(tmp) {
    tmp->nxt = result;
    result = tmp;
  }
      }
    }
    free_atrans(lft, 1);
    free_atrans(rgt, 1);
    break;
  case OR:
    lft = ltl2ba_boolean(p->lft);
    for(t1 = lft; t1; t1 = t1->nxt) {
      ATrans *tmp = dup_trans(t1);
      tmp->nxt = result;
      result = tmp;
    }
    free_atrans(lft, 1);
    rgt = ltl2ba_boolean(p->rgt);
    for(t1 = rgt; t1; t1 = t1->nxt) {
      ATrans *tmp = dup_trans(t1);
      tmp->nxt = result;
      result = tmp;
    }
    free_atrans(rgt, 1);
    break;
  default:
    build_alternating(p);
    result = emalloc_atrans();
    clear_set(result->to,  0);
    clear_set(result->pos, 1);
    clear_set(result->neg, 1);
    add_set(result->to, already_done(p));
  }
  return result;
}

ATrans *build_alternating(Node *p) {
  ATrans *t1, *t2, *t = (ATrans *)0;
  int node = already_done(p);
  if(node >= 0) return transition[node];

  switch (p->ntyp) {

  case TRUE:
    t = emalloc_atrans();
    clear_set(t->to,  0);
    clear_set(t->pos, 1);
    clear_set(t->neg, 1);
  case FALSE:
    break;

  case PREDICATE:
    t = emalloc_atrans();
    clear_set(t->to,  0);
    clear_set(t->pos, 1);
    clear_set(t->neg, 1);
    add_set(t->pos, get_sym_id(p->sym->name));
    break;

  case NOT:
    t = emalloc_atrans();
    clear_set(t->to,  0);
    clear_set(t->pos, 1);
    clear_set(t->neg, 1);
    add_set(t->neg, get_sym_id(p->lft->sym->name));
    break;

#ifdef NXT
  case NEXT:                                            
    t = ltl2ba_boolean(p->lft);
    break;
#endif

  case U_OPER:    /* p U q <-> q || (p && X (p U q)) */
    for(t2 = build_alternating(p->rgt); t2; t2 = t2->nxt) {
      ATrans *tmp = dup_trans(t2);  /* q */
      tmp->nxt = t;
      t = tmp;
    }
    for(t1 = build_alternating(p->lft); t1; t1 = t1->nxt) {
      ATrans *tmp = dup_trans(t1);  /* p */
      add_set(tmp->to, node_id);  /* X (p U q) */
      tmp->nxt = t;
      t = tmp;
    }
    add_set(final_set, node_id);
    break;

  case V_OPER:    /* p V q <-> (p && q) || (p && X (p V q)) */
    for(t1 = build_alternating(p->rgt); t1; t1 = t1->nxt) {
      ATrans *tmp;

      for(t2 = build_alternating(p->lft); t2; t2 = t2->nxt) {
  tmp = merge_trans(t1, t2);  /* p && q */
  if(tmp) {
    tmp->nxt = t;
    t = tmp;
  }
      }

      tmp = dup_trans(t1);  /* p */
      add_set(tmp->to, node_id);  /* X (p V q) */
      tmp->nxt = t;
      t = tmp;
    }
    break;

  case AND:
    t = (ATrans *)0;
    for(t1 = build_alternating(p->lft); t1; t1 = t1->nxt) {
      for(t2 = build_alternating(p->rgt); t2; t2 = t2->nxt) {
  ATrans *tmp = merge_trans(t1, t2);
  if(tmp) {
    tmp->nxt = t;
    t = tmp;
  }
      }
    }
    break;

  case OR:
    t = (ATrans *)0;
    for(t1 = build_alternating(p->lft); t1; t1 = t1->nxt) {
      ATrans *tmp = dup_trans(t1);
      tmp->nxt = t;
      t = tmp;
    }
    for(t1 = build_alternating(p->rgt); t1; t1 = t1->nxt) {
      ATrans *tmp = dup_trans(t1);
      tmp->nxt = t;
      t = tmp;
    }
    break;

  default:
    break;
  }

  transition[node_id] = t;
  label[node_id++] = p;
  return(t);
}

/********************************************************************\
|*        Simplification of the alternating automaton               *|
\********************************************************************/

void simplify_atrans(ATrans **trans) {
  ATrans *t, *father = (ATrans *)0;
  for(t = *trans; t;) {
    ATrans *t1;
    for(t1 = *trans; t1; t1 = t1->nxt) {
      if((t1 != t) && 
   included_set(t1->to,  t->to,  0) &&
   included_set(t1->pos, t->pos, 1) &&
   included_set(t1->neg, t->neg, 1))
  break;
    }
    if(t1) {
      if (father)
  father->nxt = t->nxt;
      else
  *trans = t->nxt;
      free_atrans(t, 0);
      if (father)
  t = father->nxt;
      else
  t = *trans;
      continue;
    }
    atrans_count++;
    father = t;
    t = t->nxt;
  }
}

void simplify_astates() {
  ATrans *t;
  int i, *acc = make_set(-1, 0); /* no state is accessible initially */

  for(t = transition[0]; t; t = t->nxt, i = 0)
    merge_sets(acc, t->to, 0); /* all initial states are accessible */

  for(i = node_id - 1; i > 0; i--) {
    if (!in_set(acc, i)) { /* frees unaccessible states */
      label[i] = ZN;
      free_atrans(transition[i], 1);
      transition[i] = (ATrans *)0;
      continue;
    }
    astate_count++;
    simplify_atrans(&transition[i]);
    for(t = transition[i]; t; t = t->nxt)
      merge_sets(acc, t->to, 0);
  }

  tfree(acc);
}

/********************************************************************\
|*            Display of the alternating automaton                  *|
\********************************************************************/

void print_alternating() {
  int i;
  ATrans *t;

  fprintf(tl_out, "init :\n");
  for(t = transition[0]; t; t = t->nxt) {
    print_set(t->to, 0);
    fprintf(tl_out, "\n");
  }
  
  for(i = node_id - 1; i > 0; i--) {
    if(!label[i])
      continue;
    fprintf(tl_out, "state %i : ", i);
    dump(label[i]);
    fprintf(tl_out, "\n");
    for(t = transition[i]; t; t = t->nxt) {
      if (empty_set(t->pos, 1) && empty_set(t->neg, 1))
  fprintf(tl_out, "1");
      print_set(t->pos, 1);
      if (!empty_set(t->pos,1) && !empty_set(t->neg,1)) fprintf(tl_out, " & ");
      print_set(t->neg, 2);
      fprintf(tl_out, " -> ");
      print_set(t->to, 0);
      fprintf(tl_out, "\n");
    }
  }
}

/********************************************************************\
|*                       Main method                                *|
\********************************************************************/

void mk_alternating(Node *p) {
  node_size = calculate_node_size(p) + 1; /* number of states in the automaton */
  label = (Node **) tl_emalloc(node_size * sizeof(Node *));
  transition = (ATrans **) tl_emalloc(node_size * sizeof(ATrans *));
  node_size = node_size / (8 * sizeof(int)) + 1;

  sym_size = calculate_sym_size(p); /* number of predicates */
  if(sym_size) sym_table = (char **) tl_emalloc(sym_size * sizeof(char *));
  sym_size = sym_size / (8 * sizeof(int)) + 1;
  
  final_set = make_set(-1, 0);
  transition[0] = ltl2ba_boolean(p); /* generates the alternating automaton */

  if(tl_simp_diff) {
    simplify_astates(); /* keeps only accessible states */
  }
  
  releasenode(1, p);
  tfree(label);
}


/***** ltl2ba : buchi.c *****/



/********************************************************************\
|*              Structures and shared variables                     *|
\********************************************************************/

// VARS
extern GState **init, *gstates;
extern int tl_verbose, tl_stats, tl_simp_diff, tl_simp_fly, tl_simp_scc,
  init_size, *final;

extern int gstate_id;

BState *bstack, *bstates, *bremoved;
static BScc *_scc_stack;
int ltl2ba_accept, bstate_count = 0, btrans_count = 0;
static int rank;

/********************************************************************\
|*        Simplification of the generalized Buchi automaton         *|
\********************************************************************/

void free_bstate(BState *s) {
  free_btrans(s->trans->nxt, s->trans, 1);
  tfree(s);
}

BState *remove_bstate(BState *s, BState *s1) {
  BState *prv = s->prv;
  s->prv->nxt = s->nxt;
  s->nxt->prv = s->prv;
  free_btrans(s->trans->nxt, s->trans, 0);
  s->trans = (BTrans *)0;
  s->nxt = bremoved->nxt;
  bremoved->nxt = s;
  s->prv = s1;
  for(s1 = bremoved->nxt; s1 != bremoved; s1 = s1->nxt)
    if(s1->prv == s)
      s1->prv = s->prv;
  return prv;
} 

void copy_btrans(BTrans *from, BTrans *to) {
  to->to    = from->to;
  copy_set(from->pos, to->pos, 1);
  copy_set(from->neg, to->neg, 1);
}

int simplify_btrans() {
  BState *s;
  BTrans *t, *t1;
  int changed = 0;

  for (s = bstates->nxt; s != bstates; s = s->nxt)
    for (t = s->trans->nxt; t != s->trans;) {
      t1 = s->trans->nxt;
      copy_btrans(t, s->trans);
      while((t == t1) || (t->to != t1->to) ||
            !included_set(t1->pos, t->pos, 1) ||
            !included_set(t1->neg, t->neg, 1))
        t1 = t1->nxt;
      if(t1 != s->trans) {
        BTrans *free = t->nxt;
        t->to    = free->to;
        copy_set(free->pos, t->pos, 1);
        copy_set(free->neg, t->neg, 1);
        t->nxt   = free->nxt;
        if(free == s->trans) s->trans = t;
        free_btrans(free, 0, 0);
        changed++;
      }
      else
        t = t->nxt;
    }
      
  return changed;
}

int same_btrans(BTrans *s, BTrans *t) {
  return((s->to == t->to) &&
   same_sets(s->pos, t->pos, 1) &&
   same_sets(s->neg, t->neg, 1));
}

void remove_btrans(BState *to) {
  BState *s;
  BTrans *t;
  for (s = bstates->nxt; s != bstates; s = s->nxt)
    for (t = s->trans->nxt; t != s->trans; t = t->nxt)
      if (t->to == to) { /* transition to a state with no transitions */
  BTrans *free = t->nxt;
  t->to = free->to;
  copy_set(free->pos, t->pos, 1);
  copy_set(free->neg, t->neg, 1);
  t->nxt   = free->nxt;
  if(free == s->trans) s->trans = t;
  free_btrans(free, 0, 0);
      }
}

void retarget_all_btrans() {
  BState *s;
  BTrans *t;
  for (s = bstates->nxt; s != bstates; s = s->nxt)
    for (t = s->trans->nxt; t != s->trans; t = t->nxt)
      if (!t->to->trans) { /* t->to has been removed */
  t->to = t->to->prv;
  if(!t->to) { /* t->to has no transitions */
    BTrans *free = t->nxt;
    t->to = free->to;
    copy_set(free->pos, t->pos, 1);
    copy_set(free->neg, t->neg, 1);
    t->nxt   = free->nxt;
    if(free == s->trans) s->trans = t;
    free_btrans(free, 0, 0);
  }
      }
  while(bremoved->nxt != bremoved) { /* clean the 'removed' list */
    s = bremoved->nxt;
    bremoved->nxt = bremoved->nxt->nxt;
    tfree(s);
  }
}

int all_btrans_match(BState *a, BState *b) {
  BTrans *s, *t;

  if (((a->final == ltl2ba_accept) || (b->final == ltl2ba_accept)) &&
      (a->final + b->final != 2 * ltl2ba_accept)  /* final condition of a and b differs */
      && a->incoming >=0   /* a is not in a trivial SCC */
      && b->incoming >=0)  /* b is not in a trivial SCC */
    return 0;  /* states can not be matched */

  for (s = a->trans->nxt; s != a->trans; s = s->nxt) { 
                                /* all transitions from a appear in b */
    copy_btrans(s, b->trans);
    t = b->trans->nxt;
    while(!same_btrans(s, t))
      t = t->nxt;
    if(t == b->trans) return 0;
  }
  for (s = b->trans->nxt; s != b->trans; s = s->nxt) { 
                                /* all transitions from b appear in a */
    copy_btrans(s, a->trans);
    t = a->trans->nxt;
    while(!same_btrans(s, t))
      t = t->nxt;
    if(t == a->trans) return 0;
  }
  return 1;
}

int simplify_bstates() {
  BState *s, *s1, *s2;
  int changed = 0;

  for (s = bstates->nxt; s != bstates; s = s->nxt) {
    if(s->trans == s->trans->nxt) { /* s has no transitions */
      s = remove_bstate(s, (BState *)0);
      changed++;
      continue;
    }
    bstates->trans = s->trans;
    bstates->final = s->final;
    s1 = s->nxt;
    while(!all_btrans_match(s, s1))
      s1 = s1->nxt;
    if(s1 != bstates) { /* s and s1 are equivalent */
      /* we now want to remove s and replace it by s1 */
      if(s1->incoming == -1) {  /* s1 is in a trivial SCC */
        s1->final = s->final; /* change the final condition of s1 to that of s */
        s1->incoming = s->incoming;
      }
      s = remove_bstate(s, s1);
      changed++;
    }
  }
  retarget_all_btrans();

  for (s = bstates->nxt; s != bstates; s = s->nxt) {  /* For all states s*/
    for (s2 = s->nxt; s2 != bstates; s2 = s2->nxt) {  /*  and states s2 to the right of s */
      if(s->final == s2->final && s->id == s2->id) {  /* if final and id match */
        s->id = ++gstate_id;                          /* disambiguate by assigning unused id */
      }
    }
  }

  return changed;
}

int bdfs(BState *s) {
  BTrans *t;
  BScc *c;
  BScc *scc = (BScc *)tl_emalloc(sizeof(BScc));
  scc->bstate = s;
  scc->rank = rank;
  scc->theta = rank++;
  scc->nxt = _scc_stack;
  _scc_stack = scc;

  s->incoming = 1;

  for (t = s->trans->nxt; t != s->trans; t = t->nxt) {
    if (t->to->incoming == 0) {
      int result = bdfs(t->to);
      scc->theta = min(scc->theta, result);
    }
    else {
      for(c = _scc_stack->nxt; c != 0; c = c->nxt)
  if(c->bstate == t->to) {
    scc->theta = min(scc->theta, c->rank);
    break;
  }
    }
  }
  if(scc->rank == scc->theta) {
    if(_scc_stack == scc) { /* s is alone in a scc */
      s->incoming = -1;
      for (t = s->trans->nxt; t != s->trans; t = t->nxt)
  if (t->to == s)
    s->incoming = 1;
    }
    _scc_stack = scc->nxt;
  }
  return scc->theta;
}

void simplify_bscc() {
  BState *s;
  rank = 1;
  _scc_stack = 0;

  if(bstates == bstates->nxt) return;

  for(s = bstates->nxt; s != bstates; s = s->nxt)
    s->incoming = 0; /* state color = white */

  bdfs(bstates->prv);

  for(s = bstates->nxt; s != bstates; s = s->nxt)
    if(s->incoming == 0)
      remove_bstate(s, 0);
}


/********************************************************************\
|*              Generation of the Buchi automaton                   *|
\********************************************************************/

BState *find_bstate(GState **state, int final, BState *s) {
  if((s->gstate == *state) && (s->final == final)) return s; /* same state */

  s = bstack->nxt; /* in the stack */
  bstack->gstate = *state;
  bstack->final = final;
  while(!(s->gstate == *state) || !(s->final == final))
    s = s->nxt;
  if(s != bstack) return s;

  s = bstates->nxt; /* in the solved states */
  bstates->gstate = *state;
  bstates->final = final;
  while(!(s->gstate == *state) || !(s->final == final))
    s = s->nxt;
  if(s != bstates) return s;

  s = bremoved->nxt; /* in the removed states */
  bremoved->gstate = *state;
  bremoved->final = final;
  while(!(s->gstate == *state) || !(s->final == final))
    s = s->nxt;
  if(s != bremoved) return s;

  s = (BState *)tl_emalloc(sizeof(BState)); /* creates a new state */
  s->gstate = *state;
  s->id = (*state)->id;
  s->incoming = 0;
  s->final = final;
  s->trans = emalloc_btrans(); /* sentinel */
  s->trans->nxt = s->trans;
  s->nxt = bstack->nxt;
  bstack->nxt = s;
  return s;
}

int next_final(int *set, int fin) {
  if((fin != ltl2ba_accept) && in_set(set, final[fin + 1]))
    return next_final(set, fin + 1);
  return fin;
}

void make_btrans(BState *s) {
  int state_trans = 0;
  GTrans *t;
  BTrans *t1;
  BState *s1;
  if(s->gstate->trans)
    for(t = s->gstate->trans->nxt; t != s->gstate->trans; t = t->nxt) {
      int fin = next_final(t->final, (s->final == ltl2ba_accept) ? 0 : s->final);
      BState *to = find_bstate(&t->to, fin, s);
      
      for(t1 = s->trans->nxt; t1 != s->trans;) {
  if(tl_simp_fly && 
     (to == t1->to) &&
     included_set(t->pos, t1->pos, 1) &&
     included_set(t->neg, t1->neg, 1)) { /* t1 is redondant */
    BTrans *free = t1->nxt;
    t1->to->incoming--;
    t1->to = free->to;
    copy_set(free->pos, t1->pos, 1);
    copy_set(free->neg, t1->neg, 1);
    t1->nxt   = free->nxt;
    if(free == s->trans) s->trans = t1;
    free_btrans(free, 0, 0);
    state_trans--;
  }
  else if(tl_simp_fly &&
    (t1->to == to ) &&
    included_set(t1->pos, t->pos, 1) &&
    included_set(t1->neg, t->neg, 1)) /* t is redondant */
    break;
  else
    t1 = t1->nxt;
      }
      if(t1 == s->trans) {
  BTrans *trans = emalloc_btrans();
  trans->to = to;
  trans->to->incoming++;
  copy_set(t->pos, trans->pos, 1);
  copy_set(t->neg, trans->neg, 1);
  trans->nxt = s->trans->nxt;
  s->trans->nxt = trans;
  state_trans++;
      }
    }
  
  if(tl_simp_fly) {
    if(s->trans == s->trans->nxt) { /* s has no transitions */
      free_btrans(s->trans->nxt, s->trans, 1);
      s->trans = (BTrans *)0;
      s->prv = (BState *)0;
      s->nxt = bremoved->nxt;
      bremoved->nxt = s;
      for(s1 = bremoved->nxt; s1 != bremoved; s1 = s1->nxt)
  if(s1->prv == s)
    s1->prv = (BState *)0;
      return;
    }
    bstates->trans = s->trans;
    bstates->final = s->final;
    s1 = bstates->nxt;
    while(!all_btrans_match(s, s1))
      s1 = s1->nxt;
    if(s1 != bstates) { /* s and s1 are equivalent */
      free_btrans(s->trans->nxt, s->trans, 1);
      s->trans = (BTrans *)0;
      s->prv = s1;
      s->nxt = bremoved->nxt;
      bremoved->nxt = s;
      for(s1 = bremoved->nxt; s1 != bremoved; s1 = s1->nxt)
  if(s1->prv == s)
    s1->prv = s->prv;
      return;
    }
  }
  s->nxt = bstates->nxt; /* adds the current state to 'bstates' */
  s->prv = bstates;
  s->nxt->prv = s;
  bstates->nxt = s;
  btrans_count += state_trans;
  bstate_count++;
}

/********************************************************************\
|*                  Display of the Buchi automaton                  *|
\********************************************************************/

void print_buchi(BState *s) {
  BTrans *t;
  if(s == bstates) return;

  print_buchi(s->nxt); /* begins with the last state */

  fprintf(tl_out, "state ");
  if(s->id == -1)
    fprintf(tl_out, "init");
  else {
    if(s->final == ltl2ba_accept)
      fprintf(tl_out, "accept");
    else
      fprintf(tl_out, "T%i", s->final);
    fprintf(tl_out, "_%i", s->id);
  }
  fprintf(tl_out, "\n");
  for(t = s->trans->nxt; t != s->trans; t = t->nxt) {
    if (empty_set(t->pos, 1) && empty_set(t->neg, 1))
      fprintf(tl_out, "1");
    print_set(t->pos, 1);
    if (!empty_set(t->pos, 1) && !empty_set(t->neg, 1)) fprintf(tl_out, " & ");
    print_set(t->neg, 2);
    fprintf(tl_out, " -> ");
    if(t->to->id == -1) 
      fprintf(tl_out, "init\n");
    else {
      if(t->to->final == ltl2ba_accept)
  fprintf(tl_out, "accept");
      else
  fprintf(tl_out, "T%i", t->to->final);
      fprintf(tl_out, "_%i\n", t->to->id);
    }
  }
}

void print_spin_buchi() {
    BTrans *t;
    BState *s;
    int accept_all = 0;

    if(bstates->nxt == bstates) { /* empty automaton */
        // fprintf(tl_out, "never {    /* ");
        strcat(final_never_claim, "never {");
        
        // put_uform();
        
        // fprintf(tl_out, " */\n");
        // fprintf(tl_out, "T0_init:\n");
        strcat(final_never_claim, "T0_init:\n");
        // fprintf(tl_out, "\tfalse;\n");
        strcat(final_never_claim, "\tfalse;\n");
        // fprintf(tl_out, "}\n");
        strcat(final_never_claim, "}");
        return;
    }

    if(bstates->nxt->nxt == bstates && bstates->nxt->id == 0) { /* true */
        // fprintf(tl_out, "never {    /* ");
        strcat(final_never_claim, "never {");

        // put_uform();

        // fprintf(tl_out, " */\n");
        // fprintf(tl_out, "accept_init:\n");
        strcat(final_never_claim, "accept_init:\n");
        // fprintf(tl_out, "\tif\n");
        strcat(final_never_claim, "\tif\n");
        // fprintf(tl_out, "\t:: (1) -> goto accept_init\n");
        strcat(final_never_claim, "\t:: (1) -> goto accept_init\n");
        // fprintf(tl_out, "\tfi;\n");
        strcat(final_never_claim, "\tfi;\n");
        // fprintf(tl_out, "}\n");
        strcat(final_never_claim, "}");
        return;
    }

    // fprintf(tl_out, "never { /* ");
    strcat(final_never_claim, "never {\n");
    // put_uform();
    // fprintf(tl_out, " */\n");

    for(s = bstates->prv; s != bstates; s = s->prv) {
        if(s->id == 0) { /* accept_all at the end */
            accept_all = 1;
            continue;
        }

        if(s->final == ltl2ba_accept) {
            // fprintf(tl_out, "accept_");
            strcat(final_never_claim, "accept_");
        } else {
            // fprintf(tl_out, "T%i_", s->final);
            sprintf(final_never_claim, "%sT%i_", final_never_claim, s->final);
        }

        if(s->id == -1){
            // fprintf(tl_out, "init:\n");
            strcat(final_never_claim, "init:\n");
        } else {
            // fprintf(tl_out, "S%i:\n", s->id);
            sprintf(final_never_claim, "%sS%i:\n", final_never_claim, s->id);
        }

        if(s->trans->nxt == s->trans) {
            // fprintf(tl_out, "\tfalse;\n");
            strcat(final_never_claim, "\tfalse;\n");
            continue;
        }

        // fprintf(tl_out, "\tif\n");
        strcat(final_never_claim, "\tif\n");

        for(t = s->trans->nxt; t != s->trans; t = t->nxt) {
            BTrans *t1;
            // fprintf(tl_out, "\t:: (");
            strcat(final_never_claim, "\t:: (");
            spin_print_set(t->pos, t->neg);
            
            for(t1 = t; t1->nxt != s->trans;) {
                if (t1->nxt->to->id == t->to->id && t1->nxt->to->final == t->to->final) {
                    // fprintf(tl_out, ") || (");
                    strcat(final_never_claim, ") || (");
                    spin_print_set(t1->nxt->pos, t1->nxt->neg);
                    t1->nxt = t1->nxt->nxt;
                } else {
                    t1 = t1->nxt;
                }
            }

            // fprintf(tl_out, ") -> goto ");
            strcat(final_never_claim, ") -> goto ");

            if (t->to->final == ltl2ba_accept) {
                // fprintf(tl_out, "accept_");
                strcat(final_never_claim, "accept_");
            } else {
                // fprintf(tl_out, "T%i_", t->to->final);
                sprintf(final_never_claim, "%sT%i_", final_never_claim, t->to->final);
            }

            if(t->to->id == 0) {
                // fprintf(tl_out, "all\n");
                strcat(final_never_claim, "all\n");
            } else if(t->to->id == -1) {
                // fprintf(tl_out, "init\n");
                strcat(final_never_claim, "init\n");
            } else {
                // fprintf(tl_out, "S%i\n", t->to->id);
                sprintf(final_never_claim, "%sS%i\n", final_never_claim, t->to->id);
            }
        }

        // fprintf(tl_out, "\tfi;\n");
        strcat(final_never_claim, "\tfi;\n");
    }

    if (accept_all) {
        // fprintf(tl_out, "accept_all:\n");
        strcat(final_never_claim, "accept_all:\n");
        // fprintf(tl_out, "\tskip\n");
        strcat(final_never_claim, "\tskip\n");
    }

    // fprintf(tl_out, "}\n");
    strcat(final_never_claim, "}\n");
}

/********************************************************************\
|*                       Main method                                *|
\********************************************************************/

void mk_buchi() {
  int i;
  BState *s = (BState *)tl_emalloc(sizeof(BState));
  GTrans *t;
  BTrans *t1;
  ltl2ba_accept = final[0] - 1;
  
  bstack        = (BState *)tl_emalloc(sizeof(BState)); /* sentinel */
  bstack->nxt   = bstack;
  bremoved      = (BState *)tl_emalloc(sizeof(BState)); /* sentinel */
  bremoved->nxt = bremoved;
  bstates       = (BState *)tl_emalloc(sizeof(BState)); /* sentinel */
  bstates->nxt  = s;
  bstates->prv  = s;

  s->nxt        = bstates; /* creates (unique) inital state */
  s->prv        = bstates;
  s->id = -1;
  s->incoming = 1;
  s->final = 0;
  s->gstate = 0;
  s->trans = emalloc_btrans(); /* sentinel */
  s->trans->nxt = s->trans;
  for(i = 0; i < init_size; i++) 
    if(init[i])
      for(t = init[i]->trans->nxt; t != init[i]->trans; t = t->nxt) {
  int fin = next_final(t->final, 0);
  BState *to = find_bstate(&t->to, fin, s);
  for(t1 = s->trans->nxt; t1 != s->trans;) {
    if(tl_simp_fly && 
       (to == t1->to) &&
       included_set(t->pos, t1->pos, 1) &&
       included_set(t->neg, t1->neg, 1)) { /* t1 is redondant */
      BTrans *free = t1->nxt;
      t1->to->incoming--;
      t1->to = free->to;
      copy_set(free->pos, t1->pos, 1);
      copy_set(free->neg, t1->neg, 1);
      t1->nxt   = free->nxt;
      if(free == s->trans) s->trans = t1;
      free_btrans(free, 0, 0);
    }
  else if(tl_simp_fly &&
    (t1->to == to ) &&
    included_set(t1->pos, t->pos, 1) &&
    included_set(t1->neg, t->neg, 1)) /* t is redondant */
    break;
    else
      t1 = t1->nxt;
  }
  if(t1 == s->trans) {
    BTrans *trans = emalloc_btrans();
    trans->to = to;
    trans->to->incoming++;
    copy_set(t->pos, trans->pos, 1);
    copy_set(t->neg, trans->neg, 1);
    trans->nxt = s->trans->nxt;
    s->trans->nxt = trans;
  }
      }
  
  while(bstack->nxt != bstack) { /* solves all states in the stack until it is empty */
    s = bstack->nxt;
    bstack->nxt = bstack->nxt->nxt;
    if(!s->incoming) {
      free_bstate(s);
      continue;
    }
    make_btrans(s);
  }

  retarget_all_btrans();

  if(tl_simp_diff) {
    simplify_btrans();
    if(tl_simp_scc) simplify_bscc();
    while(simplify_bstates()) { /* simplifies as much as possible */
      simplify_btrans();
      if(tl_simp_scc) simplify_bscc();
    }
  }

  print_spin_buchi();
}



/***** ltl2ba : cache.c *****/


typedef struct Cache {
  Node *before;
  Node *after;
  int same;
  struct Cache *nxt;
} Cache;

// VARS
static Cache  *stored = (Cache *) 0;
static unsigned long  Caches, CacheHits;

static int  ismatch(Node *, Node *);
int sameform(Node *, Node *);

void cache_dump(void) {
  Cache *d; int nr=0;

  printf("\nCACHE DUMP:\n");
  for (d = stored; d; d = d->nxt, nr++) {
    if (d->same) continue;
    printf("B%3d: ", nr); dump(d->before); printf("\n");
    printf("A%3d: ", nr); dump(d->after); printf("\n");
  }
  printf("============\n");
}

Node *
in_cache(Node *n)
{ Cache *d; int nr=0;

  for (d = stored; d; d = d->nxt, nr++)
    if (isequal(d->before, n))
    { CacheHits++;
      if (d->same && ismatch(n, d->before)) return n;
      return dupnode(d->after);
    }
  return ZN;
}

Node *
cached(Node *n)
{ Cache *d;
  Node *m;

  if (!n) return n;
  if ((m = in_cache(n)))
    return m;

  Caches++;
  d = (Cache *) tl_emalloc(sizeof(Cache));
  d->before = dupnode(n);
  d->after  = Canonical(n); /* n is released */

  if (ismatch(d->before, d->after))
  { d->same = 1;
    releasenode(1, d->after);
    d->after = d->before;
  }
  d->nxt = stored;
  stored = d;
  return dupnode(d->after);
}

void
cache_stats(void)
{
  printf("cache stores     : %9ld\n", Caches);
  printf("cache hits       : %9ld\n", CacheHits);
}

void
releasenode(int all_levels, Node *n)
{
  if (!n) return;

  if (all_levels)
  { releasenode(1, n->lft);
    n->lft = ZN;
    releasenode(1, n->rgt);
    n->rgt = ZN;
  }
  tfree((void *) n);
}

Node *
tl_nn(int t, Node *ll, Node *rl)
{ Node *n = (Node *) tl_emalloc(sizeof(Node));

  n->ntyp = (short) t;
  n->lft  = ll;
  n->rgt  = rl;

  return n;
}

Node *
getnode(Node *p)
{ Node *n;

  if (!p) return p;

  n =  (Node *) tl_emalloc(sizeof(Node));
  n->ntyp = p->ntyp;
  n->sym  = p->sym; /* same name */
  n->lft  = p->lft;
  n->rgt  = p->rgt;

  return n;
}

Node *
dupnode(Node *n)
{ Node *d;

  if (!n) return n;
  d = getnode(n);
  d->lft = dupnode(n->lft);
  d->rgt = dupnode(n->rgt);
  return d;
}

int
one_lft(int ntyp, Node *x, Node *in)
{
  if (!x)  return 1;
  if (!in) return 0;

  if (sameform(x, in))
    return 1;

  if (in->ntyp != ntyp)
    return 0;

  if (one_lft(ntyp, x, in->lft))
    return 1;

  return one_lft(ntyp, x, in->rgt);
}

int
all_lfts(int ntyp, Node *from, Node *in)
{
  if (!from) return 1;

  if (from->ntyp != ntyp)
    return one_lft(ntyp, from, in);

  if (!one_lft(ntyp, from->lft, in))
    return 0;

  return all_lfts(ntyp, from->rgt, in);
}

int
sametrees(int ntyp, Node *a, Node *b)
{
  if (!all_lfts(ntyp, a, b))
    return 0;

  return all_lfts(ntyp, b, a);
}

int /* a better isequal() */
sameform(Node *a, Node *b)
{
  if (!a && !b) return 1;
  if (!a || !b) return 0;
  if (a->ntyp != b->ntyp) return 0;

  if (a->sym
  &&  b->sym
  &&  strcmp(a->sym->name, b->sym->name) != 0)
    return 0;

  switch (a->ntyp) {
  case TRUE:
  case FALSE:
    return 1;
  case PREDICATE:
    if (!a->sym || !b->sym) fatal("sameform...");
    return !strcmp(a->sym->name, b->sym->name);

  case NOT:
#ifdef NXT
  case NEXT:
#endif
    return sameform(a->lft, b->lft);
  case U_OPER:
  case V_OPER:
    if (!sameform(a->lft, b->lft))
      return 0;
    if (!sameform(a->rgt, b->rgt))
      return 0;
    return 1;

  case AND:
  case OR:  /* the hard case */
    return sametrees(a->ntyp, a, b);

  default:
    printf("type: %d\n", a->ntyp);
    fatal("cannot happen, sameform");
  }

  return 0;
}

int
isequal(Node *a, Node *b)
{
  if (!a && !b)
    return 1;

  if (!a || !b)
  { if (!a)
    { if (b->ntyp == TRUE)
        return 1;
    } else
    { if (a->ntyp == TRUE)
        return 1;
    }
    return 0;
  }
  if (a->ntyp != b->ntyp)
    return 0;

  if (a->sym
  &&  b->sym
  &&  strcmp(a->sym->name, b->sym->name) != 0)
    return 0;

  if (isequal(a->lft, b->lft)
  &&  isequal(a->rgt, b->rgt))
    return 1;

  return sameform(a, b);
}

static int
ismatch(Node *a, Node *b)
{
  if (!a && !b) return 1;
  if (!a || !b) return 0;
  if (a->ntyp != b->ntyp) return 0;

  if (a->sym
  &&  b->sym
  &&  strcmp(a->sym->name, b->sym->name) != 0)
    return 0;

  if (ismatch(a->lft, b->lft)
  &&  ismatch(a->rgt, b->rgt))
    return 1;

  return 0;
}

int
any_term(Node *srch, Node *in)
{
  if (!in) return 0;

  if (in->ntyp == AND)
    return  any_term(srch, in->lft) ||
      any_term(srch, in->rgt);

  return isequal(in, srch);
}

int
any_and(Node *srch, Node *in)
{
  if (!in) return 0;

  if (srch->ntyp == AND)
    return  any_and(srch->lft, in) &&
      any_and(srch->rgt, in);

  return any_term(srch, in);
}

int
any_lor(Node *srch, Node *in)
{
  if (!in) return 0;

  if (in->ntyp == OR)
    return  any_lor(srch, in->lft) ||
      any_lor(srch, in->rgt);

  return isequal(in, srch);
}

int
anywhere(int tok, Node *srch, Node *in)
{
  if (!in) return 0;

  switch (tok) {
  case AND: return any_and(srch, in);
  case  OR: return any_lor(srch, in);
  case   0: return any_term(srch, in);
  }
  fatal("cannot happen, anywhere");
  return 0;
}



/***** ltl2ba : generalized.c *****/


/********************************************************************\
|*              Structures and shared variables                     *|
\********************************************************************/

// VARS
extern ATrans **transition;
extern int tl_verbose, tl_stats, tl_simp_diff, tl_simp_fly, tl_fjtofj,
  tl_simp_scc, *final_set, node_id;
extern char **sym_table;

GState *gstack, *gremoved, *gstates, **init;
static GScc *scc_stack;
int init_size = 0, gstate_id = 1, gstate_count = 0, gtrans_count = 0;
int *fin, *final, scc_id, scc_size, *bad_scc;

void print_generalized();

/********************************************************************\
|*        Simplification of the generalized Buchi automaton         *|
\********************************************************************/

void free_gstate(GState *s) {
  free_gtrans(s->trans->nxt, s->trans, 1);
  tfree(s->nodes_set);
  tfree(s);
}

GState *remove_gstate(GState *s, GState *s1) {
  GState *prv = s->prv;
  s->prv->nxt = s->nxt;
  s->nxt->prv = s->prv;
  free_gtrans(s->trans->nxt, s->trans, 0);
  s->trans = (GTrans *)0;
  tfree(s->nodes_set);
  s->nodes_set = 0;
  s->nxt = gremoved->nxt;
  gremoved->nxt = s;
  s->prv = s1;
  for(s1 = gremoved->nxt; s1 != gremoved; s1 = s1->nxt)
    if(s1->prv == s)
      s1->prv = s->prv;
  return prv;
} 

void copy_gtrans(GTrans *from, GTrans *to) {
  to->to = from->to;
  copy_set(from->pos,   to->pos,   1);
  copy_set(from->neg,   to->neg,   1);
  copy_set(from->final, to->final, 0);
}

int same_gtrans(GState *a, GTrans *s, GState *b, GTrans *t, int use_scc) {
  if((s->to != t->to) ||
     ! same_sets(s->pos, t->pos, 1) ||
     ! same_sets(s->neg, t->neg, 1))
    return 0; /* transitions differ */
  if(same_sets(s->final, t->final, 0))
    return 1; /* same transitions exactly */
  if( use_scc &&
      ( in_set(bad_scc, a->incoming) ||
        in_set(bad_scc, b->incoming) ||
        (a->incoming != s->to->incoming) ||
        (b->incoming != t->to->incoming) ) )
    return 1;
  return 0;
  /* below is the old test to check whether acceptance conditions may be ignored */
  if(!use_scc)
    return 0; /* transitions differ */
  if( (a->incoming == b->incoming) && (a->incoming == s->to->incoming) )
    return 0; /* same scc: acceptance conditions must be taken into account */
  return 1; /* same transitions up to acceptance conditions */
}

int simplify_gtrans() {
  int changed = 0;
  GState *s;
  GTrans *t, *t1;

  for(s = gstates->nxt; s != gstates; s = s->nxt) {
    t = s->trans->nxt;
    while(t != s->trans) { /* tries to remove t */
      copy_gtrans(t, s->trans);
      t1 = s->trans->nxt;
      while ( !((t != t1) 
          && (t1->to == t->to) 
          && included_set(t1->pos, t->pos, 1) 
          && included_set(t1->neg, t->neg, 1) 
          && (included_set(t->final, t1->final, 0)  /* acceptance conditions of t are also in t1 or may be ignored */
              || (tl_simp_scc && ((s->incoming != t->to->incoming) || in_set(bad_scc, s->incoming))))) )
        t1 = t1->nxt;
      if(t1 != s->trans) { /* remove transition t */
        GTrans *free = t->nxt;
        t->to = free->to;
        copy_set(free->pos, t->pos, 1);
        copy_set(free->neg, t->neg, 1);
        copy_set(free->final, t->final, 0);
        t->nxt = free->nxt;
        if(free == s->trans) s->trans = t;
        free_gtrans(free, 0, 0);
        changed++;
      }
      else
        t = t->nxt;
    }
  }
  
  return changed;
}

void retarget_all_gtrans() {
  GState *s;
  GTrans *t;
  int i;
  for (i = 0; i < init_size; i++)
    if (init[i] && !init[i]->trans) /* init[i] has been removed */
      init[i] = init[i]->prv;
  for (s = gstates->nxt; s != gstates; s = s->nxt)
    for (t = s->trans->nxt; t != s->trans; )
      if (!t->to->trans) { /* t->to has been removed */
  t->to = t->to->prv;
  if(!t->to) { /* t->to has no transitions */
    GTrans *free = t->nxt;
    t->to = free->to;
    copy_set(free->pos, t->pos, 1);
    copy_set(free->neg, t->neg, 1);
    copy_set(free->final, t->final, 0);
    t->nxt   = free->nxt;
    if(free == s->trans) s->trans = t;
    free_gtrans(free, 0, 0);
  }
  else
    t = t->nxt;
      }
      else
  t = t->nxt;
  while(gremoved->nxt != gremoved) { /* clean the 'removed' list */
    s = gremoved->nxt;
    gremoved->nxt = gremoved->nxt->nxt;
    if(s->nodes_set) tfree(s->nodes_set);
    tfree(s);
  }
}

int all_gtrans_match(GState *a, GState *b, int use_scc) {
  GTrans *s, *t;
  for (s = a->trans->nxt; s != a->trans; s = s->nxt) { 
    copy_gtrans(s, b->trans);
    t = b->trans->nxt;
    while(!same_gtrans(a, s, b, t, use_scc)) t = t->nxt;
    if(t == b->trans) return 0;
  }
  for (t = b->trans->nxt; t != b->trans; t = t->nxt) { 
    copy_gtrans(t, a->trans);
    s = a->trans->nxt;
    while(!same_gtrans(a, s, b, t, use_scc)) s = s->nxt;
    if(s == a->trans) return 0;
  }
  return 1;
}

int simplify_gstates() {
  int changed = 0;
  GState *a, *b;

  for(a = gstates->nxt; a != gstates; a = a->nxt) {
    if(a->trans == a->trans->nxt) { /* a has no transitions */
      a = remove_gstate(a, (GState *)0);
      changed++;
      continue;
    }
    gstates->trans = a->trans;
    b = a->nxt;
    while(!all_gtrans_match(a, b, tl_simp_scc)) b = b->nxt;
    if(b != gstates) { /* a and b are equivalent */
      if(a->incoming > b->incoming) /* scc(a) is trivial */
        a = remove_gstate(a, b);
      else /* either scc(a)=scc(b) or scc(b) is trivial */ 
        remove_gstate(b, a);
      changed++;
    }
  }
  retarget_all_gtrans();

  return changed;
}

int gdfs(GState *s) {
  GTrans *t;
  GScc *c;
  GScc *scc = (GScc *)tl_emalloc(sizeof(GScc));
  scc->gstate = s;
  scc->rank = rank;
  scc->theta = rank++;
  scc->nxt = scc_stack;
  scc_stack = scc;

  s->incoming = 1;

  for (t = s->trans->nxt; t != s->trans; t = t->nxt) {
    if (t->to->incoming == 0) {
      int result = gdfs(t->to);
      scc->theta = min(scc->theta, result);
    }
    else {
      for(c = scc_stack->nxt; c != 0; c = c->nxt)
  if(c->gstate == t->to) {
    scc->theta = min(scc->theta, c->rank);
    break;
  }
    }
  }
  if(scc->rank == scc->theta) {
    while(scc_stack != scc) {
      scc_stack->gstate->incoming = scc_id;
      scc_stack = scc_stack->nxt;
    }
    scc->gstate->incoming = scc_id++;
    scc_stack = scc->nxt;
  }
  return scc->theta;
}

void simplify_gscc() {
  GState *s;
  GTrans *t;
  int i, **scc_final;
  rank = 1;
  scc_stack = 0;
  scc_id = 1;

  if(gstates == gstates->nxt) return;

  for(s = gstates->nxt; s != gstates; s = s->nxt)
    s->incoming = 0; /* state color = white */

  for(i = 0; i < init_size; i++)
    if(init[i] && init[i]->incoming == 0)
      gdfs(init[i]);

  scc_final = (int **)tl_emalloc(scc_id * sizeof(int *));
  for(i = 0; i < scc_id; i++)
    scc_final[i] = make_set(-1,0);

  for(s = gstates->nxt; s != gstates; s = s->nxt)
    if(s->incoming == 0)
      s = remove_gstate(s, 0);
    else
      for (t = s->trans->nxt; t != s->trans; t = t->nxt)
        if(t->to->incoming == s->incoming)
          merge_sets(scc_final[s->incoming], t->final, 0);

  scc_size = (scc_id + 1) / (8 * sizeof(int)) + 1;
  bad_scc=make_set(-1,2);

  for(i = 0; i < scc_id; i++)
    if(!included_set(final_set, scc_final[i], 0))
       add_set(bad_scc, i);

  for(i = 0; i < scc_id; i++)
    tfree(scc_final[i]);
  tfree(scc_final);
}

/********************************************************************\
|*        Generation of the generalized Buchi automaton             *|
\********************************************************************/

int is_final(int *from, ATrans *at, int i) {
  ATrans *t;
  int in_to;
  if((tl_fjtofj && !in_set(at->to, i)) ||
    (!tl_fjtofj && !in_set(from,  i))) return 1;
  in_to = in_set(at->to, i);
  rem_set(at->to, i);
  for(t = transition[i]; t; t = t->nxt)
    if(included_set(t->to, at->to, 0) &&
       included_set(t->pos, at->pos, 1) &&
       included_set(t->neg, at->neg, 1)) {
      if(in_to) add_set(at->to, i);
      return 1;
    }
  if(in_to) add_set(at->to, i);
  return 0;
}

GState *find_gstate(int *set, GState *s) {
  if(same_sets(set, s->nodes_set, 0)) return s; /* same state */
  s = gstack->nxt; /* in the stack */
  gstack->nodes_set = set;
  while(!same_sets(set, s->nodes_set, 0))
    s = s->nxt;
  if(s != gstack) return s;

  s = gstates->nxt; /* in the solved states */
  gstates->nodes_set = set;
  while(!same_sets(set, s->nodes_set, 0))
    s = s->nxt;
  if(s != gstates) return s;

  s = gremoved->nxt; /* in the removed states */
  gremoved->nodes_set = set;
  while(!same_sets(set, s->nodes_set, 0))
    s = s->nxt;
  if(s != gremoved) return s;

  s = (GState *)tl_emalloc(sizeof(GState)); /* creates a new state */
  s->id = (empty_set(set, 0)) ? 0 : gstate_id++;
  s->incoming = 0;
  s->nodes_set = dup_set(set, 0);
  s->trans = emalloc_gtrans(); /* sentinel */
  s->trans->nxt = s->trans;
  s->nxt = gstack->nxt;
  gstack->nxt = s;
  return s;
}

void make_gtrans(GState *s) {
  int i, *list, state_trans = 0, trans_exist = 1;
  GState *s1;
  ATrans *t1;
  AProd *prod = (AProd *)tl_emalloc(sizeof(AProd)); /* initialization */
  prod->nxt = prod;
  prod->prv = prod;
  prod->prod = emalloc_atrans();
  clear_set(prod->prod->to,  0);
  clear_set(prod->prod->pos, 1);
  clear_set(prod->prod->neg, 1);
  prod->trans = prod->prod;
  prod->trans->nxt = prod->prod;
  list = list_set(s->nodes_set, 0);

  for(i = 1; i < list[0]; i++) {
    AProd *p = (AProd *)tl_emalloc(sizeof(AProd));
    p->astate = list[i];
    p->trans = transition[list[i]];
    if(!p->trans) trans_exist = 0;
    p->prod = merge_trans(prod->nxt->prod, p->trans);
    p->nxt = prod->nxt;
    p->prv = prod;
    p->nxt->prv = p;
    p->prv->nxt = p;
  }

  while(trans_exist) { /* calculates all the transitions */
    AProd *p = prod->nxt;
    t1 = p->prod;
    if(t1) { /* solves the current transition */
      GTrans *trans, *t2;
      clear_set(fin, 0);
      for(i = 1; i < final[0]; i++)
  if(is_final(s->nodes_set, t1, final[i]))
    add_set(fin, final[i]);
      for(t2 = s->trans->nxt; t2 != s->trans;) {
  if(tl_simp_fly &&
     included_set(t1->to, t2->to->nodes_set, 0) &&
     included_set(t1->pos, t2->pos, 1) &&
     included_set(t1->neg, t2->neg, 1) &&
     same_sets(fin, t2->final, 0)) { /* t2 is redondant */
    GTrans *free = t2->nxt;
    t2->to->incoming--;
    t2->to = free->to;
    copy_set(free->pos, t2->pos, 1);
    copy_set(free->neg, t2->neg, 1);
    copy_set(free->final, t2->final, 0);
    t2->nxt   = free->nxt;
    if(free == s->trans) s->trans = t2;
    free_gtrans(free, 0, 0);
    state_trans--;
  }
  else if(tl_simp_fly &&
    included_set(t2->to->nodes_set, t1->to, 0) &&
    included_set(t2->pos, t1->pos, 1) &&
    included_set(t2->neg, t1->neg, 1) &&
    same_sets(t2->final, fin, 0)) {/* t1 is redondant */
    break;
  }
  else {
    t2 = t2->nxt;
  }
      }
      if(t2 == s->trans) { /* adds the transition */
  trans = emalloc_gtrans();
  trans->to = find_gstate(t1->to, s);
  trans->to->incoming++;
  copy_set(t1->pos, trans->pos, 1);
  copy_set(t1->neg, trans->neg, 1);
  copy_set(fin,   trans->final, 0);
  trans->nxt = s->trans->nxt;
  s->trans->nxt = trans;
  state_trans++;
      }
    }
    if(!p->trans)
      break;
    while(!p->trans->nxt) /* calculates the next transition */
      p = p->nxt;
    if(p == prod)
      break;
    p->trans = p->trans->nxt;
    do_merge_trans(&(p->prod), p->nxt->prod, p->trans);
    p = p->prv;
    while(p != prod) {
      p->trans = transition[p->astate];
      do_merge_trans(&(p->prod), p->nxt->prod, p->trans);
      p = p->prv;
    }
  }
  
  tfree(list); /* free memory */
  while(prod->nxt != prod) {
    AProd *p = prod->nxt;
    prod->nxt = p->nxt;
    free_atrans(p->prod, 0);
    tfree(p);
  }
  free_atrans(prod->prod, 0);
  tfree(prod);

  if(tl_simp_fly) {
    if(s->trans == s->trans->nxt) { /* s has no transitions */
      free_gtrans(s->trans->nxt, s->trans, 1);
      s->trans = (GTrans *)0;
      s->prv = (GState *)0;
      s->nxt = gremoved->nxt;
      gremoved->nxt = s;
      for(s1 = gremoved->nxt; s1 != gremoved; s1 = s1->nxt)
  if(s1->prv == s)
  s1->prv = (GState *)0;
      return;
    }
    
    gstates->trans = s->trans;
    s1 = gstates->nxt;
    while(!all_gtrans_match(s, s1, 0))
      s1 = s1->nxt;
    if(s1 != gstates) { /* s and s1 are equivalent */
      free_gtrans(s->trans->nxt, s->trans, 1);
      s->trans = (GTrans *)0;
      s->prv = s1;
      s->nxt = gremoved->nxt;
      gremoved->nxt = s;
      for(s1 = gremoved->nxt; s1 != gremoved; s1 = s1->nxt)
  if(s1->prv == s)
    s1->prv = s->prv;
      return;
    }
  }

  s->nxt = gstates->nxt; /* adds the current state to 'gstates' */
  s->prv = gstates;
  s->nxt->prv = s;
  gstates->nxt = s;
  gtrans_count += state_trans;
  gstate_count++;
}

/********************************************************************\
|*            Display of the generalized Buchi automaton            *|
\********************************************************************/

void reverse_print_generalized(GState *s) {
  GTrans *t;
  if(s == gstates) return;

  reverse_print_generalized(s->nxt); /* begins with the last state */

  fprintf(tl_out, "state %i (", s->id);
  print_set(s->nodes_set, 0);
  fprintf(tl_out, ") : %i\n", s->incoming);
  for(t = s->trans->nxt; t != s->trans; t = t->nxt) {
    if (empty_set(t->pos, 1) && empty_set(t->neg, 1))
      fprintf(tl_out, "1");
    print_set(t->pos, 1);
    if (!empty_set(t->pos, 1) && !empty_set(t->neg, 1)) fprintf(tl_out, " & ");
    print_set(t->neg, 1);
    fprintf(tl_out, " -> %i : ", t->to->id);
    print_set(t->final, 0);
    fprintf(tl_out, "\n");
  }
}

void print_generalized() {
  int i;
  fprintf(tl_out, "init :\n");
  for(i = 0; i < init_size; i++)
    if(init[i])
      fprintf(tl_out, "%i\n", init[i]->id);
  reverse_print_generalized(gstates->nxt);
}

/********************************************************************\
|*                       Main method                                *|
\********************************************************************/

void mk_generalized() {
  ATrans *t;
  GState *s;

  fin = new_set(0);
  bad_scc = 0; /* will be initialized in simplify_gscc */
  final = list_set(final_set, 0);

  gstack        = (GState *)tl_emalloc(sizeof(GState)); /* sentinel */
  gstack->nxt   = gstack;
  gremoved      = (GState *)tl_emalloc(sizeof(GState)); /* sentinel */
  gremoved->nxt = gremoved;
  gstates       = (GState *)tl_emalloc(sizeof(GState)); /* sentinel */
  gstates->nxt  = gstates;
  gstates->prv  = gstates;

  for(t = transition[0]; t; t = t->nxt) { /* puts initial states in the stack */
    s = (GState *)tl_emalloc(sizeof(GState));
    s->id = (empty_set(t->to, 0)) ? 0 : gstate_id++;
    s->incoming = 1;
    s->nodes_set = dup_set(t->to, 0);
    s->trans = emalloc_gtrans(); /* sentinel */
    s->trans->nxt = s->trans;
    s->nxt = gstack->nxt;
    gstack->nxt = s;
    init_size++;
  }

  if(init_size) init = (GState **)tl_emalloc(init_size * sizeof(GState *));
  init_size = 0;
  for(s = gstack->nxt; s != gstack; s = s->nxt)
    init[init_size++] = s;

  while(gstack->nxt != gstack) { /* solves all states in the stack until it is empty */
    s = gstack->nxt;
    gstack->nxt = gstack->nxt->nxt;
    if(!s->incoming) {
      free_gstate(s);
      continue;
    }
    make_gtrans(s);
  }

  retarget_all_gtrans();

  tfree(gstack);
  /*for(i = 0; i < node_id; i++) // frees the data from the alternating automaton */
  /*free_atrans(transition[i], 1);*/
  free_all_atrans();
  tfree(transition);

  if(tl_simp_diff) {
    if (tl_simp_scc) simplify_gscc();
    simplify_gtrans();
    if (tl_simp_scc) simplify_gscc();
    while(simplify_gstates()) { /* simplifies as much as possible */
      if (tl_simp_scc) simplify_gscc();
      simplify_gtrans();
      if (tl_simp_scc) simplify_gscc();
    }
  }
}
  



/***** ltl2ba : lex.c *****/
#include <ctype.h>


// VARS
static Symbol *symtab[Nhash+1];
static int  tl_lex(void);

extern YYSTYPE  tl_yylval;
char  yytext[2048];

#define Token(y)        tl_yylval = tl_nn(y,ZN,ZN); return y

int
isalnum_(int c)
{       return (isalnum(c) || c == '_');
}

int
hash(char *s)
{       int h=0;

        while (*s)
        {       h += *s++;
                h <<= 1;
                if (h&(Nhash+1))
                        h |= 1;
        }
        return h&Nhash;
}

static void
getword(int first, int (*tst)(int))
{ int i=0; char c;

  yytext[i++]= (char ) first;
  while (tst(c = tl_Getchar()))
    yytext[i++] = c;
  yytext[i] = '\0';
  tl_UnGetchar();
}

static int
follow(int tok, int ifyes, int ifno)
{ int c;
  char buf[32];
  extern int tl_yychar;

  if ((c = tl_Getchar()) == tok)
    return ifyes;
  tl_UnGetchar();
  tl_yychar = c;
  sprintf(buf, "expected '%c'", tok);
  tl_yyerror(buf);  /* no return from here */
  return ifno;
}

int
tl_yylex(void)
{ int c = tl_lex();
#if 0
  printf("c = %d\n", c);
#endif
  return c;
}

static int
tl_lex(void)
{ int c;

  do {
    c = tl_Getchar();
    yytext[0] = (char ) c;
    yytext[1] = '\0';

    if (c <= 0)
    { Token(';');
    }

  } while (c == ' '); /* '\t' is removed in tl_main.c */

  if (islower(c))
  { getword(c, isalnum_);
    if (strcmp("true", yytext) == 0)
    { Token(TRUE);
    }
    if (strcmp("false", yytext) == 0)
    { Token(FALSE);
    }
    tl_yylval = tl_nn(PREDICATE,ZN,ZN);
    tl_yylval->sym = tl_lookup(yytext);
    return PREDICATE;
  }
  if (c == '<')
  { c = tl_Getchar();
    if (c == '>')
    { Token(EVENTUALLY);
    }
    if (c != '-')
    { tl_UnGetchar();
      tl_yyerror("expected '<>' or '<->'");
    }
    c = tl_Getchar();
    if (c == '>')
    { Token(EQUIV);
    }
    tl_UnGetchar();
    tl_yyerror("expected '<->'");
  }
  if (c == 'N')
  { c = tl_Getchar();
    if (c != 'O')
    { tl_UnGetchar();
      tl_yyerror("expected 'NOT'");
    }
    c = tl_Getchar();
    if (c == 'T')
    { Token(NOT);
    }
    tl_UnGetchar();
    tl_yyerror("expected 'NOT'");
  }

  switch (c) {
  case '/' : c = follow('\\', AND, '/'); break;
  case '\\': c = follow('/', OR, '\\'); break;
  case '&' : c = follow('&', AND, '&'); break;
  case '|' : c = follow('|', OR, '|'); break;
  case '[' : c = follow(']', ALWAYS, '['); break;
  case '-' : c = follow('>', IMPLIES, '-'); break;
  case '!' : c = NOT; break;
  case 'U' : c = U_OPER; break;
  case 'V' : c = V_OPER; break;
#ifdef NXT
  case 'X' : c = NEXT; break;
#endif
  default  : break;
  }
  Token(c);
}

Symbol *
tl_lookup(char *s)
{ Symbol *sp;
  int h = hash(s);

  for (sp = symtab[h]; sp; sp = sp->next)
    if (strcmp(sp->name, s) == 0)
      return sp;

  sp = (Symbol *) tl_emalloc(sizeof(Symbol));
  sp->name = (char *) tl_emalloc(strlen(s) + 1);
  strcpy(sp->name, s);
  sp->next = symtab[h];
  symtab[h] = sp;

  return sp;
}

Symbol *
getsym(Symbol *s)
{ Symbol *n = (Symbol *) tl_emalloc(sizeof(Symbol));

  n->name = s->name;
  return n;
}



/***** ltl2ba : mem.c *****/


#if 1
#define log(e, u, d)  event[e][(int) u] += (long) d;
#else
#define log(e, u, d)
#endif

#define A_LARGE   80
#define A_USER    0x55000000
#define NOTOOBIG  32768

#define POOL    0
#define ALLOC   1
#define FREE    2
#define NREVENT   3

extern  unsigned long All_Mem;
extern  int tl_verbose;

// VARS
ATrans *atrans_list = (ATrans *)0;
GTrans *gtrans_list = (GTrans *)0;
BTrans *btrans_list = (BTrans *)0;

int aallocs = 0, afrees = 0, apool = 0;
int gallocs = 0, gfrees = 0, gpool = 0;
int ballocs = 0, bfrees = 0, bpool = 0;

union M {
  long size;
  union M *link;
};

static union M *freelist[A_LARGE];
static long req[A_LARGE];
static long event[NREVENT][A_LARGE];

void *
tl_emalloc(int U)
{ union M *m;
    long r, u;
  void *rp;

  u = (long) ((U-1)/sizeof(union M) + 2);

  if (u >= A_LARGE)
  { log(ALLOC, 0, 1);
    if (tl_verbose)
    printf("tl_spin: memalloc %ld bytes\n", u);
    m = (union M *) emalloc((int) u*sizeof(union M));
    All_Mem += (unsigned long) u*sizeof(union M);
  } else
  { if (!freelist[u])
    { r = req[u] += req[u] ? req[u] : 1;
      if (r >= NOTOOBIG)
        r = req[u] = NOTOOBIG;
      log(POOL, u, r);
      freelist[u] = (union M *)
        emalloc((int) r*u*sizeof(union M));
      All_Mem += (unsigned long) r*u*sizeof(union M);
      m = freelist[u] + (r-2)*u;
      for ( ; m >= freelist[u]; m -= u)
        m->link = m+u;
    }
    log(ALLOC, u, 1);
    m = freelist[u];
    freelist[u] = m->link;
  }
  m->size = (u|A_USER);

  for (r = 1; r < u; )
    (&m->size)[r++] = 0;

  rp = (void *) (m+1);
  memset(rp, 0, U);
  return rp;
}

void
tfree(void *v)
{ union M *m = (union M *) v;
  long u;

  --m;
  if ((m->size&0xFF000000) != A_USER)
    Fatal("releasing a free block");

  u = (m->size &= 0xFFFFFF);
  if (u >= A_LARGE)
  { log(FREE, 0, 1);
    /* free(m); */
  } else
  { log(FREE, u, 1);
    m->link = freelist[u];
    freelist[u] = m;
  }
}

ATrans* emalloc_atrans() {
  ATrans *result;
  if(!atrans_list) {
    result = (ATrans *)tl_emalloc(sizeof(GTrans));
    result->pos = new_set(1);
    result->neg = new_set(1);
    result->to  = new_set(0);
    apool++;
  }
  else {
    result = atrans_list;
    atrans_list = atrans_list->nxt;
    result->nxt = (ATrans *)0;
  }
  aallocs++;
  return result;
}

void free_atrans(ATrans *t, int rec) {
  if(!t) return;
  if(rec) free_atrans(t->nxt, rec);
  t->nxt = atrans_list;
  atrans_list = t;
  afrees++;
}

void free_all_atrans() {
  ATrans *t;
  while(atrans_list) {
    t = atrans_list;
    atrans_list = t->nxt;
    tfree(t->to);
    tfree(t->pos);
    tfree(t->neg);
    tfree(t);
  }
}

GTrans* emalloc_gtrans() {
  GTrans *result;
  if(!gtrans_list) {
    result = (GTrans *)tl_emalloc(sizeof(GTrans));
    result->pos   = new_set(1);
    result->neg   = new_set(1);
    result->final = new_set(0);
    gpool++;
  }
  else {
    result = gtrans_list;
    gtrans_list = gtrans_list->nxt;
  }
  gallocs++;
  return result;
}

void free_gtrans(GTrans *t, GTrans *sentinel, int fly) {
  gfrees++;
  if(sentinel && (t != sentinel)) {
    free_gtrans(t->nxt, sentinel, fly);
    if(fly) t->to->incoming--;
  }
  t->nxt = gtrans_list;
  gtrans_list = t;
}

BTrans* emalloc_btrans() {
  BTrans *result;
  if(!btrans_list) {
    result = (BTrans *)tl_emalloc(sizeof(BTrans));
    result->pos = new_set(1);
    result->neg = new_set(1);
    bpool++;
  }
  else {
    result = btrans_list;
    btrans_list = btrans_list->nxt;
  }
  ballocs++;
  return result;
}

void free_btrans(BTrans *t, BTrans *sentinel, int fly) {
  bfrees++;
  if(sentinel && (t != sentinel)) {
    free_btrans(t->nxt, sentinel, fly);
    if(fly) t->to->incoming--;
  }
  t->nxt = btrans_list;
  btrans_list = t;
}

void
a_stats(void)
{ long  p, a, f;
  int i;

  printf(" size\t  pool\tallocs\t frees\n");

  for (i = 0; i < A_LARGE; i++)
  { p = event[POOL][i];
    a = event[ALLOC][i];
    f = event[FREE][i];

    if(p|a|f)
    printf("%5d\t%6ld\t%6ld\t%6ld\n",
      i, p, a, f);
  }

  printf("atrans\t%6d\t%6d\t%6d\n", 
         apool, aallocs, afrees);
  printf("gtrans\t%6d\t%6d\t%6d\n", 
         gpool, gallocs, gfrees);
  printf("btrans\t%6d\t%6d\t%6d\n", 
         bpool, ballocs, bfrees);
}



/***** ltl2ba : parse.c *****/


extern int  tl_yylex(void);
// VARS
extern int  tl_verbose, tl_simp_log;

int tl_yychar = 0;
YYSTYPE tl_yylval;

static Node *tl_formula(void);
static Node *tl_factor(void);
static Node *tl_level(int);

static int  prec[2][4] = {
  { U_OPER,  V_OPER, 0, 0},  /* left associative */
  { OR, AND, IMPLIES, EQUIV, }, /* left associative */
};

static int
implies(Node *a, Node *b)
{
  return
    (isequal(a,b) ||
     b->ntyp == TRUE ||
     a->ntyp == FALSE ||
     (b->ntyp == AND && implies(a, b->lft) && implies(a, b->rgt)) ||
     (a->ntyp == OR && implies(a->lft, b) && implies(a->rgt, b)) ||
     (a->ntyp == AND && (implies(a->lft, b) || implies(a->rgt, b))) ||
     (b->ntyp == OR && (implies(a, b->lft) || implies(a, b->rgt))) ||
     (b->ntyp == U_OPER && implies(a, b->rgt)) ||
     (a->ntyp == V_OPER && implies(a->rgt, b)) ||
     (a->ntyp == U_OPER && implies(a->lft, b) && implies(a->rgt, b)) ||
     (b->ntyp == V_OPER && implies(a, b->lft) && implies(a, b->rgt)) ||
     ((a->ntyp == U_OPER || a->ntyp == V_OPER) && a->ntyp == b->ntyp && 
         implies(a->lft, b->lft) && implies(a->rgt, b->rgt)));
}

static Node *
bin_simpler(Node *ptr)
{ Node *a, *b;

  if (ptr)
  switch (ptr->ntyp) {
  case U_OPER:
    if (ptr->rgt->ntyp == TRUE
    ||  ptr->rgt->ntyp == FALSE
    ||  ptr->lft->ntyp == FALSE)
    { ptr = ptr->rgt;
      break;
    }
    if (implies(ptr->lft, ptr->rgt)) /* NEW */
    { ptr = ptr->rgt;
            break;
    }
    if (ptr->lft->ntyp == U_OPER
    &&  isequal(ptr->lft->lft, ptr->rgt))
    { /* (p U q) U p = (q U p) */
      ptr->lft = ptr->lft->rgt;
      break;
    }
    if (ptr->rgt->ntyp == U_OPER
    &&  implies(ptr->lft, ptr->rgt->lft))
    { /* NEW */
      ptr = ptr->rgt;
      break;
    }
#ifdef NXT
    /* X p U X q == X (p U q) */
    if (ptr->rgt->ntyp == NEXT
    &&  ptr->lft->ntyp == NEXT)
    { ptr = tl_nn(NEXT,
        tl_nn(U_OPER,
          ptr->lft->lft,
          ptr->rgt->lft), ZN);
            break;
    }

    /* NEW : F X p == X F p */
    if (ptr->lft->ntyp == TRUE &&
        ptr->rgt->ntyp == NEXT) {
      ptr = tl_nn(NEXT, tl_nn(U_OPER, True, ptr->rgt->lft), ZN);
      break;
    }

#endif

    /* NEW : F G F p == G F p */
    if (ptr->lft->ntyp == TRUE &&
        ptr->rgt->ntyp == V_OPER &&
        ptr->rgt->lft->ntyp == FALSE &&
        ptr->rgt->rgt->ntyp == U_OPER &&
        ptr->rgt->rgt->lft->ntyp == TRUE) {
      ptr = ptr->rgt;
      break;
    }

    /* NEW */
    if (ptr->lft->ntyp != TRUE && 
        implies(push_negation(tl_nn(NOT, dupnode(ptr->rgt), ZN)), 
          ptr->lft))
    {       ptr->lft = True;
            break;
    }
    break;
  case V_OPER:
    if (ptr->rgt->ntyp == FALSE
    ||  ptr->rgt->ntyp == TRUE
    ||  ptr->lft->ntyp == TRUE)
    { ptr = ptr->rgt;
      break;
    }
    if (implies(ptr->rgt, ptr->lft))
    { /* p V p = p */ 
      ptr = ptr->rgt;
      break;
    }
    /* F V (p V q) == F V q */
    if (ptr->lft->ntyp == FALSE
    &&  ptr->rgt->ntyp == V_OPER)
    { ptr->rgt = ptr->rgt->rgt;
      break;
    }
#ifdef NXT
    /* NEW : G X p == X G p */
    if (ptr->lft->ntyp == FALSE &&
        ptr->rgt->ntyp == NEXT) {
      ptr = tl_nn(NEXT, tl_nn(V_OPER, False, ptr->rgt->lft), ZN);
      break;
    }
#endif
    /* NEW : G F G p == F G p */
    if (ptr->lft->ntyp == FALSE &&
        ptr->rgt->ntyp == U_OPER &&
        ptr->rgt->lft->ntyp == TRUE &&
        ptr->rgt->rgt->ntyp == V_OPER &&
        ptr->rgt->rgt->lft->ntyp == FALSE) {
      ptr = ptr->rgt;
      break;
    }

    /* NEW */
    if (ptr->rgt->ntyp == V_OPER
    &&  implies(ptr->rgt->lft, ptr->lft))
    { ptr = ptr->rgt;
      break;
    }

    /* NEW */
    if (ptr->lft->ntyp != FALSE && 
        implies(ptr->lft, 
          push_negation(tl_nn(NOT, dupnode(ptr->rgt), ZN))))
    {       ptr->lft = False;
            break;
    }
    break;
#ifdef NXT
  case NEXT:
    /* NEW : X G F p == G F p */
    if (ptr->lft->ntyp == V_OPER &&
        ptr->lft->lft->ntyp == FALSE &&
        ptr->lft->rgt->ntyp == U_OPER &&
        ptr->lft->rgt->lft->ntyp == TRUE) {
      ptr = ptr->lft;
      break;
    }
    /* NEW : X F G p == F G p */
    if (ptr->lft->ntyp == U_OPER &&
        ptr->lft->lft->ntyp == TRUE &&
        ptr->lft->rgt->ntyp == V_OPER &&
        ptr->lft->rgt->lft->ntyp == FALSE) {
      ptr = ptr->lft;
      break;
    }
    break;
#endif
  case IMPLIES:
    if (implies(ptr->lft, ptr->rgt))
      { ptr = True;
      break;
    }
    ptr = tl_nn(OR, Not(ptr->lft), ptr->rgt);
    ptr = rewrite(ptr);
    break;
  case EQUIV:
    if (implies(ptr->lft, ptr->rgt) &&
        implies(ptr->rgt, ptr->lft))
      { ptr = True;
      break;
    }
    a = rewrite(tl_nn(AND,
      dupnode(ptr->lft),
      dupnode(ptr->rgt)));
    b = rewrite(tl_nn(AND,
      Not(ptr->lft),
      Not(ptr->rgt)));
    ptr = tl_nn(OR, a, b);
    ptr = rewrite(ptr);
    break;
  case AND:
    /* p && (q U p) = p */
    if (ptr->rgt->ntyp == U_OPER
    &&  isequal(ptr->rgt->rgt, ptr->lft))
    { ptr = ptr->lft;
      break;
    }
    if (ptr->lft->ntyp == U_OPER
    &&  isequal(ptr->lft->rgt, ptr->rgt))
    { ptr = ptr->rgt;
      break;
    }

    /* p && (q V p) == q V p */
    if (ptr->rgt->ntyp == V_OPER
    &&  isequal(ptr->rgt->rgt, ptr->lft))
    { ptr = ptr->rgt;
      break;
    }
    if (ptr->lft->ntyp == V_OPER
    &&  isequal(ptr->lft->rgt, ptr->rgt))
    { ptr = ptr->lft;
      break;
    }

    /* (p U q) && (r U q) = (p && r) U q*/
    if (ptr->rgt->ntyp == U_OPER
    &&  ptr->lft->ntyp == U_OPER
    &&  isequal(ptr->rgt->rgt, ptr->lft->rgt))
    { ptr = tl_nn(U_OPER,
        tl_nn(AND, ptr->lft->lft, ptr->rgt->lft),
        ptr->lft->rgt);
      break;
    }

    /* (p V q) && (p V r) = p V (q && r) */
    if (ptr->rgt->ntyp == V_OPER
    &&  ptr->lft->ntyp == V_OPER
    &&  isequal(ptr->rgt->lft, ptr->lft->lft))
    { ptr = tl_nn(V_OPER,
        ptr->rgt->lft,
        tl_nn(AND, ptr->lft->rgt, ptr->rgt->rgt));
      break;
    }
#ifdef NXT
    /* X p && X q == X (p && q) */
    if (ptr->rgt->ntyp == NEXT
    &&  ptr->lft->ntyp == NEXT)
    { ptr = tl_nn(NEXT,
        tl_nn(AND,
          ptr->rgt->lft,
          ptr->lft->lft), ZN);
      break;
    }
#endif

    /* (p V q) && (r U q) == p V q */
    if (ptr->rgt->ntyp == U_OPER
    &&  ptr->lft->ntyp == V_OPER
    &&  isequal(ptr->lft->rgt, ptr->rgt->rgt))
    { ptr = ptr->lft;
      break;
    }

    if (isequal(ptr->lft, ptr->rgt) /* (p && p) == p */
    ||  ptr->rgt->ntyp == FALSE /* (p && F) == F */
    ||  ptr->lft->ntyp == TRUE  /* (T && p) == p */
    ||  implies(ptr->rgt, ptr->lft))/* NEW */
    { ptr = ptr->rgt;
      break;
    } 
    if (ptr->rgt->ntyp == TRUE  /* (p && T) == p */
    ||  ptr->lft->ntyp == FALSE /* (F && p) == F */
    ||  implies(ptr->lft, ptr->rgt))/* NEW */
    { ptr = ptr->lft;
      break;
    }
    
    /* NEW : F G p && F G q == F G (p && q) */
    if (ptr->lft->ntyp == U_OPER &&
        ptr->lft->lft->ntyp == TRUE &&
        ptr->lft->rgt->ntyp == V_OPER &&
        ptr->lft->rgt->lft->ntyp == FALSE &&
        ptr->rgt->ntyp == U_OPER &&
        ptr->rgt->lft->ntyp == TRUE &&
        ptr->rgt->rgt->ntyp == V_OPER &&
        ptr->rgt->rgt->lft->ntyp == FALSE)
      {
        ptr = tl_nn(U_OPER, True,
        tl_nn(V_OPER, False,
              tl_nn(AND, ptr->lft->rgt->rgt,
              ptr->rgt->rgt->rgt)));
        break;
      }

    /* NEW */
    if (implies(ptr->lft, 
          push_negation(tl_nn(NOT, dupnode(ptr->rgt), ZN)))
     || implies(ptr->rgt, 
          push_negation(tl_nn(NOT, dupnode(ptr->lft), ZN))))
    {       ptr = False;
            break;
    }
    break;

  case OR:
    /* p || (q U p) == q U p */
    if (ptr->rgt->ntyp == U_OPER
    &&  isequal(ptr->rgt->rgt, ptr->lft))
    { ptr = ptr->rgt;
      break;
    }

    /* p || (q V p) == p */
    if (ptr->rgt->ntyp == V_OPER
    &&  isequal(ptr->rgt->rgt, ptr->lft))
    { ptr = ptr->lft;
      break;
    }

    /* (p U q) || (p U r) = p U (q || r) */
    if (ptr->rgt->ntyp == U_OPER
    &&  ptr->lft->ntyp == U_OPER
    &&  isequal(ptr->rgt->lft, ptr->lft->lft))
    { ptr = tl_nn(U_OPER,
        ptr->rgt->lft,
        tl_nn(OR, ptr->lft->rgt, ptr->rgt->rgt));
      break;
    }

    if (isequal(ptr->lft, ptr->rgt) /* (p || p) == p */
    ||  ptr->rgt->ntyp == FALSE /* (p || F) == p */
    ||  ptr->lft->ntyp == TRUE  /* (T || p) == T */
    ||  implies(ptr->rgt, ptr->lft))/* NEW */
    { ptr = ptr->lft;
      break;
    } 
    if (ptr->rgt->ntyp == TRUE  /* (p || T) == T */
    ||  ptr->lft->ntyp == FALSE /* (F || p) == p */
    ||  implies(ptr->lft, ptr->rgt))/* NEW */
    { ptr = ptr->rgt;
      break;
    }

    /* (p V q) || (r V q) = (p || r) V q */
    if (ptr->rgt->ntyp == V_OPER
    &&  ptr->lft->ntyp == V_OPER
    &&  isequal(ptr->lft->rgt, ptr->rgt->rgt))
    { ptr = tl_nn(V_OPER,
        tl_nn(OR, ptr->lft->lft, ptr->rgt->lft),
        ptr->rgt->rgt);
      break;
    }

    /* (p V q) || (r U q) == r U q */
    if (ptr->rgt->ntyp == U_OPER
    &&  ptr->lft->ntyp == V_OPER
    &&  isequal(ptr->lft->rgt, ptr->rgt->rgt))
    { ptr = ptr->rgt;
      break;
    }   
    
    /* NEW : G F p || G F q == G F (p || q) */
    if (ptr->lft->ntyp == V_OPER &&
        ptr->lft->lft->ntyp == FALSE &&
        ptr->lft->rgt->ntyp == U_OPER &&
        ptr->lft->rgt->lft->ntyp == TRUE &&
        ptr->rgt->ntyp == V_OPER &&
        ptr->rgt->lft->ntyp == FALSE &&
        ptr->rgt->rgt->ntyp == U_OPER &&
        ptr->rgt->rgt->lft->ntyp == TRUE)
      {
        ptr = tl_nn(V_OPER, False,
        tl_nn(U_OPER, True,
              tl_nn(OR, ptr->lft->rgt->rgt,
              ptr->rgt->rgt->rgt)));
        break;
      }

    /* NEW */
    if (implies(push_negation(tl_nn(NOT, dupnode(ptr->rgt), ZN)),
          ptr->lft)
     || implies(push_negation(tl_nn(NOT, dupnode(ptr->lft), ZN)),
          ptr->rgt))
    {       ptr = True;
            break;
    }
    break;
  }
  return ptr;
}

static Node *
bin_minimal(Node *ptr)
{       if (ptr)
  switch (ptr->ntyp) {
  case IMPLIES:
    return tl_nn(OR, Not(ptr->lft), ptr->rgt);
  case EQUIV:
    return tl_nn(OR, 
           tl_nn(AND,dupnode(ptr->lft),dupnode(ptr->rgt)),
           tl_nn(AND,Not(ptr->lft),Not(ptr->rgt)));
  }
  return ptr;
}

static Node *
tl_factor(void)
{ Node *ptr = ZN;

  switch (tl_yychar) {
  case '(':
    ptr = tl_formula();
    if (tl_yychar != ')')
      tl_yyerror("expected ')'");
    tl_yychar = tl_yylex();
    goto simpl;
  case NOT:
    ptr = tl_yylval;
    tl_yychar = tl_yylex();
    ptr->lft = tl_factor();
    ptr = push_negation(ptr);
    goto simpl;
  case ALWAYS:
    tl_yychar = tl_yylex();

    ptr = tl_factor();

    if(tl_simp_log) {
      if (ptr->ntyp == FALSE
          ||  ptr->ntyp == TRUE)
        break;  /* [] false == false */
      
      if (ptr->ntyp == V_OPER)
        { if (ptr->lft->ntyp == FALSE)
          break;  /* [][]p = []p */
        
        ptr = ptr->rgt; /* [] (p V q) = [] q */
        }
    }

    ptr = tl_nn(V_OPER, False, ptr);
    goto simpl;
#ifdef NXT
  case NEXT:
    tl_yychar = tl_yylex();

    ptr = tl_factor();

    if ((ptr->ntyp == TRUE || ptr->ntyp == FALSE)&& tl_simp_log)
      break;  /* X true = true , X false = false */

    ptr = tl_nn(NEXT, ptr, ZN);
    goto simpl;
#endif
  case EVENTUALLY:
    tl_yychar = tl_yylex();

    ptr = tl_factor();

    if(tl_simp_log) {
      if (ptr->ntyp == TRUE
          ||  ptr->ntyp == FALSE)
        break;  /* <> true == true */

      if (ptr->ntyp == U_OPER
          &&  ptr->lft->ntyp == TRUE)
        break;  /* <><>p = <>p */

      if (ptr->ntyp == U_OPER)
        { /* <> (p U q) = <> q */
          ptr = ptr->rgt;
          /* fall thru */
        }
    }

    ptr = tl_nn(U_OPER, True, ptr);
  simpl:
    if (tl_simp_log) 
      ptr = bin_simpler(ptr);
    break;
  case PREDICATE:
    ptr = tl_yylval;
    tl_yychar = tl_yylex();
    break;
  case TRUE:
  case FALSE:
    ptr = tl_yylval;
    tl_yychar = tl_yylex();
    break;
  }
  if (!ptr) tl_yyerror("expected predicate");
  return ptr;
}

static Node *
tl_level(int nr)
{ int i; Node *ptr = ZN;

  if (nr < 0)
    return tl_factor();

  ptr = tl_level(nr-1);
again:
  for (i = 0; i < 4; i++)
    if (tl_yychar == prec[nr][i])
    { tl_yychar = tl_yylex();
      ptr = tl_nn(prec[nr][i],
        ptr, tl_level(nr-1));
      if(tl_simp_log) ptr = bin_simpler(ptr);
      else ptr = bin_minimal(ptr);
      goto again;
    }
  if (!ptr) tl_yyerror("syntax error");
  return ptr;
}

static Node * tl_formula(void) { 
    tl_yychar = tl_yylex();
    return tl_level(1); /* 2 precedence levels, 1 and 0 */  
}

void tl_parse(void) {
    Node *n = tl_formula();
 
    if (tl_verbose) { 
        printf("formula: ");
        put_uform();
        printf("\n");
    }
    
    trans(n);
}



/***** ltl2ba : rewrt.c *****/


// VARS
extern int  tl_verbose;

static Node *can = ZN;

Node *
right_linked(Node *n)
{
  if (!n) return n;

  if (n->ntyp == AND || n->ntyp == OR)
    while (n->lft && n->lft->ntyp == n->ntyp)
    { Node *tmp = n->lft;
      n->lft = tmp->rgt;
      tmp->rgt = n;
      n = tmp;
    }

  n->lft = right_linked(n->lft);
  n->rgt = right_linked(n->rgt);

  return n;
}

Node *
canonical(Node *n)
{ Node *m;  /* assumes input is right_linked */

  if (!n) return n;
  if ((m = in_cache(n)))
    return m;

  n->rgt = canonical(n->rgt);
  n->lft = canonical(n->lft);

  return cached(n);
}

Node *
push_negation(Node *n)
{ Node *m;

  Assert(n->ntyp == NOT, n->ntyp);

  switch (n->lft->ntyp) {
  case TRUE:
    releasenode(0, n->lft);
    n->lft = ZN;
    n->ntyp = FALSE;
    break;
  case FALSE:
    releasenode(0, n->lft);
    n->lft = ZN;
    n->ntyp = TRUE;
    break;
  case NOT:
    m = n->lft->lft;
    releasenode(0, n->lft);
    n->lft = ZN;
    releasenode(0, n);
    n = m;
    break;
  case V_OPER:
    n->ntyp = U_OPER;
    goto same;
  case U_OPER:
    n->ntyp = V_OPER;
    goto same;
#ifdef NXT
  case NEXT:
    n->ntyp = NEXT;
    n->lft->ntyp = NOT;
    n->lft = push_negation(n->lft);
    break;
#endif
  case  AND:
    n->ntyp = OR;
    goto same;
  case  OR:
    n->ntyp = AND;

same:   m = n->lft->rgt;
    n->lft->rgt = ZN;

    n->rgt = Not(m);
    n->lft->ntyp = NOT;
    m = n->lft;
    n->lft = push_negation(m);
    break;
  }

  return rewrite(n);
}

static void
addcan(int tok, Node *n)
{ Node  *m, *prev = ZN;
  Node  **ptr;
  Node  *N;
  Symbol  *s, *t; int cmp;

  if (!n) return;

  if (n->ntyp == tok)
  { addcan(tok, n->rgt);
    addcan(tok, n->lft);
    return;
  }
#if 0
  if ((tok == AND && n->ntyp == TRUE)
  ||  (tok == OR  && n->ntyp == FALSE))
    return;
#endif
  N = dupnode(n);
  if (!can) 
  { can = N;
    return;
  }

  s = DoDump(N);
  if (can->ntyp != tok) /* only one element in list so far */
  { ptr = &can;
    goto insert;
  }

  /* there are at least 2 elements in list */
  prev = ZN;
  for (m = can; m->ntyp == tok && m->rgt; prev = m, m = m->rgt)
  { t = DoDump(m->lft);
    cmp = strcmp(s->name, t->name);
    if (cmp == 0) /* duplicate */
      return;
    if (cmp < 0)
    { if (!prev)
      { can = tl_nn(tok, N, can);
        return;
      } else
      { ptr = &(prev->rgt);
        goto insert;
  } } }

  /* new entry goes at the end of the list */
  ptr = &(prev->rgt);
insert:
  t = DoDump(*ptr);
  cmp = strcmp(s->name, t->name);
  if (cmp == 0) /* duplicate */
    return;
  if (cmp < 0)
    *ptr = tl_nn(tok, N, *ptr);
  else
    *ptr = tl_nn(tok, *ptr, N);
}

static void
marknode(int tok, Node *m)
{
  if (m->ntyp != tok)
  { releasenode(0, m->rgt);
    m->rgt = ZN;
  }
  m->ntyp = -1;
}

Node *
Canonical(Node *n)
{ Node *m, *p, *k1, *k2, *prev, *dflt = ZN;
  int tok;

  if (!n) return n;

  tok = n->ntyp;
  if (tok != AND && tok != OR)
    return n;

  can = ZN;
  addcan(tok, n);
#if 1
  Debug("\nA0: "); Dump(can); 
  Debug("\nA1: "); Dump(n); Debug("\n");
#endif
  releasenode(1, n);

  /* mark redundant nodes */
  if (tok == AND)
  { for (m = can; m; m = (m->ntyp == AND) ? m->rgt : ZN)
    { k1 = (m->ntyp == AND) ? m->lft : m;
      if (k1->ntyp == TRUE)
      { marknode(AND, m);
        dflt = True;
        continue;
      }
      if (k1->ntyp == FALSE)
      { releasenode(1, can);
        can = False;
        goto out;
    } }
    for (m = can; m; m = (m->ntyp == AND) ? m->rgt : ZN)
    for (p = can; p; p = (p->ntyp == AND) ? p->rgt : ZN)
    { if (p == m
      ||  p->ntyp == -1
      ||  m->ntyp == -1)
        continue;
      k1 = (m->ntyp == AND) ? m->lft : m;
      k2 = (p->ntyp == AND) ? p->lft : p;

      if (isequal(k1, k2))
      { marknode(AND, p);
        continue;
      }
      if (anywhere(OR, k1, k2))
      { marknode(AND, p);
        continue;
      }
      if (k2->ntyp == U_OPER
      &&  anywhere(AND, k2->rgt, can))
      { marknode(AND, p);
        continue;
      } /* q && (p U q) = q */
  } }
  if (tok == OR)
  { for (m = can; m; m = (m->ntyp == OR) ? m->rgt : ZN)
    { k1 = (m->ntyp == OR) ? m->lft : m;
      if (k1->ntyp == FALSE)
      { marknode(OR, m);
        dflt = False;
        continue;
      }
      if (k1->ntyp == TRUE)
      { releasenode(1, can);
        can = True;
        goto out;
    } }
    for (m = can; m; m = (m->ntyp == OR) ? m->rgt : ZN)
    for (p = can; p; p = (p->ntyp == OR) ? p->rgt : ZN)
    { if (p == m
      ||  p->ntyp == -1
      ||  m->ntyp == -1)
        continue;
      k1 = (m->ntyp == OR) ? m->lft : m;
      k2 = (p->ntyp == OR) ? p->lft : p;

      if (isequal(k1, k2))
      { marknode(OR, p);
        continue;
      }
      if (anywhere(AND, k1, k2))
      { marknode(OR, p);
        continue;
      }
      if (k2->ntyp == V_OPER
      &&  k2->lft->ntyp == FALSE
      &&  anywhere(AND, k2->rgt, can))
      { marknode(OR, p);
        continue;
      } /* p || (F V p) = p */
  } }
  for (m = can, prev = ZN; m; ) /* remove marked nodes */
  { if (m->ntyp == -1)
    { k2 = m->rgt;
      releasenode(0, m);
      if (!prev)
      { m = can = can->rgt;
      } else
      { m = prev->rgt = k2;
        /* if deleted the last node in a chain */
        if (!prev->rgt && prev->lft
        &&  (prev->ntyp == AND || prev->ntyp == OR))
        { k1 = prev->lft;
          prev->ntyp = prev->lft->ntyp;
          prev->sym = prev->lft->sym;
          prev->rgt = prev->lft->rgt;
          prev->lft = prev->lft->lft;
          releasenode(0, k1);
        }
      }
      continue;
    }
    prev = m;
    m = m->rgt;
  }
out:
#if 1
  Debug("A2: "); Dump(can); Debug("\n");
#endif
  if (!can)
  { if (!dflt)
      fatal("cannot happen, Canonical");
    return dflt;
  }

  return can;
}



/***** ltl2ba : set.c *****/


// VARS
extern int node_size, sym_size, scc_size;
extern char **sym_table;

int mod = 8 * sizeof(int);

#define set_size(t) (t==1?sym_size:(t==2?scc_size:node_size))

int *new_set(int type) /* creates a new set */
{
  return (int *)tl_emalloc(set_size(type) * sizeof(int));
}

int *clear_set(int *l, int type) /* clears the set */
{
  int i;
  for(i = 0; i < set_size(type); i++) {
    l[i] = 0;
  }
  return l;
}

int *make_set(int n, int type) /* creates the set {n}, or the empty set if n = -1 */
{
  int *l = clear_set(new_set(type), type);
  if(n == -1) return l;
  l[n/mod] = 1 << (n%mod);
  return l;
}

void copy_set(int *from, int *to, int type) /* copies a set */
{
  int i;
  for(i = 0; i < set_size(type); i++)
    to[i] = from[i];
}

int *dup_set(int *l, int type) /* duplicates a set */
{
  int i, *m = new_set(type);
  for(i = 0; i < set_size(type); i++)
    m[i] = l[i];
  return m;
}
  
void merge_sets(int *l1, int *l2, int type) /* puts the union of the two sets in l1 */
{
  int i;
  for(i = 0; i < set_size(type); i++)
    l1[i] = l1[i] | l2[i];
}

void do_merge_sets(int *l, int *l1, int *l2, int type) /* makes the union of two sets */
{
  int i;
  for(i = 0; i < set_size(type); i++)
    l[i] = l1[i] | l2[i];
}

int *intersect_sets(int *l1, int *l2, int type) /* makes the intersection of two sets */
{
  int i, *l = new_set(type);
  for(i = 0; i < set_size(type); i++)
    l[i] = l1[i] & l2[i];
  return l;
}

int empty_intersect_sets(int *l1, int *l2, int type) /* tests intersection of two sets */
{
  int i, test = 0;
  for(i = 0; i < set_size(type); i++)
    test |= l1[i] & l2[i];
  return !test;
}

void add_set(int *l, int n) /* adds an element to a set */
{
  l[n/mod] |= 1 << (n%mod);
}

void rem_set(int *l, int n) /* removes an element from a set */
{
  l[n/mod] &= (-1 - (1 << (n%mod)));
}

void spin_print_set(int *pos, int *neg) /* prints the content of a set for spin */
{
  int i, j, start = 1;
  for(i = 0; i < sym_size; i++) 
    for(j = 0; j < mod; j++) {
      if(pos[i] & (1 << j)) {
  if(!start) {
    // fprintf(tl_out, " && ");
    strcat(final_never_claim, " && ");
  }
  // fprintf(tl_out, "%s", sym_table[mod * i + j]);
  sprintf(final_never_claim, "%s%s", final_never_claim, sym_table[mod * i + j]);
  start = 0;
      }
      if(neg[i] & (1 << j)) {
  if(!start) {
    // fprintf(tl_out, " && ");
    strcat(final_never_claim, " && ");
  }
  // fprintf(tl_out, "!%s", sym_table[mod * i + j]);
  sprintf(final_never_claim, "%s!%s", final_never_claim, sym_table[mod * i + j]);
  start = 0;
      }
    }
  if(start) {
    // fprintf(tl_out, "1");
    strcat(final_never_claim, "1");
  }
}

void print_set(int *l, int type) /* prints the content of a set */
{
  int i, j, start = 1;;
  if(type != 1) fprintf(tl_out, "{");
  for(i = 0; i < set_size(type); i++) 
    for(j = 0; j < mod; j++)
      if(l[i] & (1 << j)) {
        switch(type) {
          case 0: case 2:
            if(!start) fprintf(tl_out, ",");
            fprintf(tl_out, "%i", mod * i + j);
            break;
          case 1:
            if(!start) fprintf(tl_out, " & ");
            fprintf(tl_out, "%s", sym_table[mod * i + j]);
            break;
        }
        start = 0;
      }
  if(type != 1) fprintf(tl_out, "}");
}

int empty_set(int *l, int type) /* tests if a set is the empty set */
{
  int i, test = 0;
  for(i = 0; i < set_size(type); i++)
    test |= l[i];
  return !test;
}

int same_sets(int *l1, int *l2, int type) /* tests if two sets are identical */
{
  int i, test = 1;
  for(i = 0; i < set_size(type); i++)
    test &= (l1[i] == l2[i]);
  return test;
}

int included_set(int *l1, int *l2, int type) 
{                    /* tests if the first set is included in the second one */
  int i, test = 0;
  for(i = 0; i < set_size(type); i++)
    test |= (l1[i] & ~l2[i]);
  return !test;
}

int in_set(int *l, int n) /* tests if an element is in a set */
{
  return(l[n/mod] & (1 << (n%mod)));
}

int *list_set(int *l, int type) /* transforms a set into a list */
{
  int i, j, size = 1, *list;
  for(i = 0; i < set_size(type); i++)
    for(j = 0; j < mod; j++) 
      if(l[i] & (1 << j))
  size++;
  list = (int *)tl_emalloc(size * sizeof(int));
  list[0] = size;
  size = 1;
  for(i = 0; i < set_size(type); i++)
    for(j = 0; j < mod; j++) 
      if(l[i] & (1 << j))
  list[size++] = mod * i + j;
  return list;
}
  



/***** ltl2ba : trans.c *****/


// VARS
extern int tl_verbose, tl_terse, tl_errs;

int Stack_mx=0, Max_Red=0, Total=0;
static char dumpbuf[2048];

#ifdef NXT
int
only_nxt(Node *n)
{
        switch (n->ntyp) {
        case NEXT:
                return 1;
        case OR:
        case AND:
                return only_nxt(n->rgt) && only_nxt(n->lft);
        default:
                return 0;
        }
}
#endif

int
dump_cond(Node *pp, Node *r, int first)
{       Node *q;
        int frst = first;

        if (!pp) return frst;

        q = dupnode(pp);
        q = rewrite(q);

        if (q->ntyp == PREDICATE
        ||  q->ntyp == NOT
#ifndef NXT
        ||  q->ntyp == OR
#endif
        ||  q->ntyp == FALSE)
        {       if (!frst) fprintf(tl_out, " && ");
                dump(q);
                frst = 0;
#ifdef NXT
        } else if (q->ntyp == OR)
        {       if (!frst) fprintf(tl_out, " && ");
                fprintf(tl_out, "((");
                frst = dump_cond(q->lft, r, 1);

                if (!frst)
                        fprintf(tl_out, ") || (");
                else
                {       if (only_nxt(q->lft))
                        {       fprintf(tl_out, "1))");
                                return 0;
                        }
                }

                frst = dump_cond(q->rgt, r, 1);

                if (frst)
                {       if (only_nxt(q->rgt))
                                fprintf(tl_out, "1");
                        else
                                fprintf(tl_out, "0");
                        frst = 0;
                }

                fprintf(tl_out, "))");
#endif
        } else  if (q->ntyp == V_OPER
                && !anywhere(AND, q->rgt, r))
        {       frst = dump_cond(q->rgt, r, frst);
        } else  if (q->ntyp == AND)
        {
                frst = dump_cond(q->lft, r, frst);
                frst = dump_cond(q->rgt, r, frst);
        }

        return frst;
}

static void
sdump(Node *n)
{
  switch (n->ntyp) {
  case PREDICATE: strcat(dumpbuf, n->sym->name);
      break;
  case U_OPER:  strcat(dumpbuf, "U");
      goto common2;
  case V_OPER:  strcat(dumpbuf, "V");
      goto common2;
  case OR:  strcat(dumpbuf, "|");
      goto common2;
  case AND: strcat(dumpbuf, "&");
common2:    sdump(n->rgt);
common1:    sdump(n->lft);
      break;
#ifdef NXT
  case NEXT:  strcat(dumpbuf, "X");
      goto common1;
#endif
  case NOT: strcat(dumpbuf, "!");
      goto common1;
  case TRUE:  strcat(dumpbuf, "T");
      break;
  case FALSE: strcat(dumpbuf, "F");
      break;
  default:  strcat(dumpbuf, "?");
      break;
  }
}

Symbol *
DoDump(Node *n)
{
  if (!n) return ZS;

  if (n->ntyp == PREDICATE)
    return n->sym;

  dumpbuf[0] = '\0';
  sdump(n);
  return tl_lookup(dumpbuf);
}

void trans(Node *p) 
{ 
  if (!p || tl_errs) return;
  
  if (tl_verbose || tl_terse) { 
    fprintf(tl_out, "\t/* Normlzd: ");
    dump(p);
    fprintf(tl_out, " */\n");
  }
  if (tl_terse)
    return;

  mk_alternating(p);
  mk_generalized();
  mk_buchi();
}





/***** ltl2ba : main.c *****/

// VARS
FILE    *tl_out;

int tl_stats     = 0; /* time and size stats */ 
int tl_simp_log  = 1; /* logical simplification */
int tl_simp_diff = 1; /* automata simplification */
int tl_simp_fly  = 1; /* on the fly simplification */
int tl_simp_scc  = 1; /* use scc simplification */
int tl_fjtofj    = 1; /* 2eme fj */
int tl_errs      = 0;
int tl_verbose   = 0;
int tl_terse     = 0;
unsigned long   All_Mem = 0;

// static char uform[4096];
static char *uform = (char*)malloc(4096 * sizeof(char));
static int  hasuform=0, cnt=0;
static char     **ltl_file = (char **)0;
static char     **add_ltl  = (char **)0;
static char     out1[64];

static void tl_endstats(void);
static void non_fatal(const char *);

static void alldone(int estatus) {
    if (strlen(out1) > 0) {
        printf("%s", (char *)out1);
        exit(-1);
    }
    
    exit(estatus);
}

FILE *cpyfile(char *src, char *tgt) {
    return NULL;
}

char * emalloc(int n) {
    char *tmp;
    
    if (!(tmp = (char *) malloc(n))) {
        fatal("not enough memory");
    }
    
    memset(tmp, 0, n);
    return tmp;
}

int tl_Getchar(void) {
    if (cnt < hasuform){
        return uform[cnt++];
    }
    
    cnt++;
    return -1;
}

void put_uform(void) {
    fprintf(tl_out, "%s", uform);
}

void tl_UnGetchar(void) {
    if (cnt > 0) cnt--;
}

static void usage(void) {
    printf("usage: ltl2ba [-flag] -f 'formula'\n");
    printf("                   or -F file\n");
    printf(" -f 'formula'\ttranslate LTL ");
    printf("into never claim\n");
    printf(" -d\t\tdisplay automata (D)escription at each step\n");
    printf(" -s\t\tcomputing time and automata sizes (S)tatistics\n");
    printf(" -l\t\tdisable (L)ogic formula simplification\n");
    printf(" -p\t\tdisable a-(P)osteriori simplification\n");
    printf(" -o\t\tdisable (O)n-the-fly simplification\n");
    printf(" -c\t\tdisable strongly (C)onnected components simplification\n");
    printf(" -a\t\tdisable trick in (A)ccepting conditions\n");

    alldone(1);
}

int tl_main(int argc, char *argv[]) {
    int i;

    while (argc > 1 && argv[1][0] == '-') {
        switch (argv[1][1]) {
            case 'f':   argc--; argv++;
                for (i = 0; i < argv[1][i]; i++) {
                    if (argv[1][i] == '\t' ||  argv[1][i] == '\"' ||  argv[1][i] == '\n') argv[1][i] = ' ';
                }

                strcpy(uform, argv[1]);
                printf("%s", uform);
                hasuform = strlen(uform);
                break;
            default :   usage();
        }

        argc--; argv++;
    }

    if (hasuform == 0) usage();

    tl_parse();

    if (tl_stats) tl_endstats();

    return tl_errs;
}


// main was here


static void tl_endstats(void) {
    printf("\ntotal memory used: %9ld\n", All_Mem);
    a_stats();
}

#define Binop(a)        \
        fprintf(tl_out, "(");   \
        dump(n->lft);       \
        fprintf(tl_out, a); \
        dump(n->rgt);       \
        fprintf(tl_out, ")")

void dump(Node *n) {
    if (!n) return;

    switch(n->ntyp) {
    case OR:    Binop(" || "); break;
    case AND:   Binop(" && "); break;
    case U_OPER:    Binop(" U ");  break;
    case V_OPER:    Binop(" V ");  break;
#ifdef NXT
    case NEXT:
        fprintf(tl_out, "X");
        fprintf(tl_out, " (");
        dump(n->lft);
        fprintf(tl_out, ")");
        break;
#endif
    case NOT:
        fprintf(tl_out, "!");
        fprintf(tl_out, " (");
        dump(n->lft);
        fprintf(tl_out, ")");
        break;
    case FALSE:
        fprintf(tl_out, "false");
        break;
    case TRUE:
        fprintf(tl_out, "true");
        break;
    case PREDICATE:
        fprintf(tl_out, "(%s)", n->sym->name);
        break;
    case -1:
        fprintf(tl_out, " D ");
        break;
    default:
        printf("Unknown token: ");
        tl_explain(n->ntyp);
        break;
    }
}

void tl_explain(int n) {
    switch (n) {
    case ALWAYS:    printf("[]"); break;
    case EVENTUALLY: printf("<>"); break;
    case IMPLIES:   printf("->"); break;
    case EQUIV: printf("<->"); break;
    case PREDICATE: printf("predicate"); break;
    case OR:    printf("||"); break;
    case AND:   printf("&&"); break;
    case NOT:   printf("!"); break;
    case U_OPER:    printf("U"); break;
    case V_OPER:    printf("V"); break;
#ifdef NXT
    case NEXT:  printf("X"); break;
#endif
    case TRUE:  printf("true"); break;
    case FALSE: printf("false"); break;
    case ';':   printf("end of formula"); break;
    default:    printf("%c", n); break;
    }
}

static void non_fatal(const char *s1) {
    extern int tl_yychar;
    int i;

    printf("ltl2ba: ");
    fputs(s1, stdout);
    
    if (tl_yychar != -1 && tl_yychar != 0) {   printf(", saw '");
        tl_explain(tl_yychar);
        printf("'");
    }
    
    printf("\nltl2ba: %s\n---------", uform);
    
    for (i = 0; i < cnt; i++) {
        printf("-");
    }
    
    printf("^\n");
    fflush(stdout);
    tl_errs++;
}

void tl_yyerror(char *s1) {
    Fatal(s1);
}

void Fatal(const char *s1) {
  non_fatal(s1);
  alldone(1);
}

void fatal(const char *s1) {
    non_fatal(s1);
    alldone(1);
}
